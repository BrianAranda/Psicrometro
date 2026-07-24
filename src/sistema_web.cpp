#include "sistema_web.h"
#include "sistema_psicrometrico.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <time.h>
#include <sys/time.h>

namespace SistemaWeb {

    // Creamos el servidor en el puerto 80 (el puerto estándar para páginas web)
    AsyncWebServer server(80);
    // Creamos el objeto WebSocket
    AsyncWebSocket ws("/ws"); 

    // Mensaje que se muestra (y con el que se compara el candado del datalogger)
    // mientras el usuario todavía no sincronizó la fecha/hora desde la web.
    const char* MSG_ESPERANDO = "Esperando ingreso de sincronización";

    // Variables del Datalogger
    const int muestras_max = 720;
    
    struct RegistroMet {
        char timestamp[20];
        float tbs;
        float tbh;
        uint8_t humedad;
        float tempDHT;
        float humDHT;
    };

    RegistroMet historial[muestras_max];
    int indiceHistorial = 0;
    int cantidadMuestras = 0;

    // Funciones para el RTC
    void initTimeZone(){
        setenv("TZ", "ART3", 1);  // Configuración para Argentina UTC-3
        tzset();
    }

    void setRTC(int day, int month, int year, int hour, int minute){
        struct tm t = {0};
        t.tm_year = year - 1900; 
        t.tm_mon  = month - 1;   
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min  = minute;
        t.tm_sec  = 0;
        t.tm_isdst = -1;

        time_t now = mktime(&t);
        struct timeval tv;
        tv.tv_sec = now;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        
        Serial.printf("Reloj interno actualizado a: %02d/%02d/%04d %02d:%02d\n", day, month, year, hour, minute);
    }

    // Manejo de mensajes WebSocket desde la web
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        
        // Si el mensaje es de texto y está completo
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            
            // Convertimos los datos crudos a un String
            String msg = "";
            for (size_t i = 0; i < len; i++) {
                msg += (char)data[i];
            }
            Serial.print("Mensaje recibido por WS: ");
            Serial.println(msg);

            // Deserializamos el JSON (Ej: {"fecha":"2026-04-20","hora":"14:30"})
            JsonDocument doc; 
            DeserializationError error = deserializeJson(doc, msg);
            if (error) {
                Serial.print("Error leyendo JSON: ");
                Serial.println(error.f_str());
                return;
            }

            // Extraemos y configuramos la hora si existen esas variables
            if (doc.containsKey("fecha") && doc.containsKey("hora")) {
                const char *fecha = doc["fecha"];
                const char *hora = doc["hora"];

                // El HTML envía yyyy-mm-dd
                int year = strtol(fecha, NULL, 10);
                int month = strtol(fecha + 5, NULL, 10);
                int day = strtol(fecha + 8, NULL, 10);

                // El HTML envía hh:mm
                int hour = strtol(hora, NULL, 10);
                int minute = strtol(hora + 3, NULL, 10);

                setRTC(day, month, year, hour, minute);
            }
        }
    }

    // Eventos del WebSocket
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        switch (type) {
            case WS_EVT_CONNECT:
                Serial.printf("Cliente WebSocket #%u conectado desde %s\n", client->id(), client->remoteIP().toString().c_str());
                break;
            case WS_EVT_DISCONNECT:
                Serial.printf("Cliente WebSocket #%u desconectado\n", client->id());
                break;
            case WS_EVT_DATA:
                handleWebSocketMessage(arg, data, len);
                break;
            case WS_EVT_PONG:
            case WS_EVT_ERROR:
                break;
        }
    }

    void inicializar() {
        Serial.println("Configurando Sistema Web ...");
        
        initTimeZone(); // Iniciamos el reloj

        // Montamos el sistema de archivos
        if (!LittleFS.begin()) {
            Serial.println("Error: No se pudo montar LittleFS.");
            Serial.println("¿Se cargaron los archivos web en PlatformIO?");
            return;
        }
        Serial.println("Disco LittleFS montado correctamente.");

        // Le decimos al servidor qué hacer cuando alguien entra a la IP principal
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            // Enviamos el archivo index.html
            request->send(LittleFS, "/index.html", "text/html");
        });

        // Si el HTML pide un CSS o un JS, el servidor lo busca y lo envía solo
        server.serveStatic("/", LittleFS, "/");

        // Configuramos los WebSockets
        ws.onEvent(onEvent);
        server.addHandler(&ws);

        // Ruta para descargar el csv
        server.on("/download.csv", HTTP_GET, [](AsyncWebServerRequest *request) {
            // Usamos un "Stream" para enviar el archivo línea por línea y no explotar la RAM
            AsyncResponseStream *response = request->beginResponseStream("text/csv");
            response->addHeader("Content-Disposition", "attachment; filename=\"psicrometro_datalogger.csv\"");
            
            // Imprimimos la cabecera del CSV
            response->print("Fecha_Hora,TBS(C),TBH(C),HR(%),Temp_Int(C),Hum_Int(%)\n");
            
            // Calculamos desde dónde empezar a leer (por si el buffer dio la vuelta completa)
            int inicio = (cantidadMuestras < muestras_max) ? 0 : indiceHistorial;
            
            // Imprimimos las filas
            for (int i = 0; i < cantidadMuestras; i++) {
                int pos = (inicio + i) % muestras_max;
                response->printf("%s,%.1f,%.1f,%d,%.1f,%.0f\n", 
                    historial[pos].timestamp,
                    historial[pos].tbs,
                    historial[pos].tbh,
                    historial[pos].humedad,
                    historial[pos].tempDHT,
                    historial[pos].humDHT
                );
            }
            request->send(response);
        });

        // Encendemos el servidor
        server.begin();
        Serial.println("Servidor Web Asíncrono iniciado. Escuchando peticiones...");
        Serial.println("");

    }

    void actualizar() {
        // Limpia a los clientes que se hayan desconectado abruptamente (ej: cerraron el navegador)
        // Esto evita que la memoria RAM se llene de conexiones fantasma
        ws.cleanupClients();
    }

    // Función para obtener la hora formateada
    void getTimestamp(char *buffer, size_t size) {
        time_t now;
        time(&now);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);

        int year = timeinfo.tm_year + 1900;
        
        // Si el año es menor a 2020, significa que arrancamos en 1969 
        // y el usuario aún no presionó el botón "Guardar" en la web.
        if (year < 2020) {
            snprintf(buffer, size, "%s", MSG_ESPERANDO);
        } else {
            snprintf(buffer, size, "%02d/%02d/%04d %02d:%02d",
                     timeinfo.tm_mday, timeinfo.tm_mon + 1, year,
                     timeinfo.tm_hour, timeinfo.tm_min);
        }
    }

    // Envio de los datos a la web
    void emitirTelemetria() {
        // Si nadie está mirando la página web:
        // cortamos la función para no gastar RAM armando un JSON inútil.
        if (ws.count() == 0) return; 

        // Buscamos los datos del módulo de psicrometro
        SistemaPsicrometrico::DatosSensores datos = SistemaPsicrometrico::getUltimosDatos();
        
        // Armamos el JSON
        JsonDocument doc;

        // Empaquetamos Datos Externos (Solo si el sensor no está desconectado)
        // Usamos > -100 porque si falla, la librería devuelve -127.0
        if (datos.tbs > -100.0) doc["tbs"] = datos.tbs;
        if (datos.tbh > -100.0) doc["tbh"] = datos.tbh;
        if (datos.humedad != 255) doc["humedad"] = datos.humedad;

        // Empaquetamos Datos Internos (Solo si no son NaN)
        if (!isnan(datos.tempDHT)) doc["tempDHT"] = datos.tempDHT;
        if (!isnan(datos.humDHT)) doc["humDHT"] = datos.humDHT;

        // Empaquetamos el estado del banco de baterías
        doc["bateriaPct"] = datos.bateriaPct;
        doc["bateriaV"] = datos.bateriaV;

        // Empaquetamos la Hora Actual
        char timestamp[40];
        getTimestamp(timestamp, sizeof(timestamp));
        doc["fechayhora"] = timestamp;

        doc["progreso"] = cantidadMuestras; // Le avisa a la barra verde cuánto se ha llenado

        // Convertimos a texto y disparamos por el túnel a todos los conectados
        String jsonString;
        serializeJson(doc, jsonString);
        ws.textAll(jsonString);
    }

    // Función de registro del Datalogger
    void registrarMuestra() {
        char timestampActual[40];
        getTimestamp(timestampActual, sizeof(timestampActual));

        // No guardamos en el historial hasta haber seteado una hora exacta manualmente
        if (String(timestampActual) == MSG_ESPERANDO) {
            return;
        }

        SistemaPsicrometrico::DatosSensores datos = SistemaPsicrometrico::getUltimosDatos();

        // Llenamos el bloque de memoria actual
        strlcpy(historial[indiceHistorial].timestamp, timestampActual, sizeof(historial[indiceHistorial].timestamp));
        historial[indiceHistorial].tbs = datos.tbs;
        historial[indiceHistorial].tbh = datos.tbh;
        historial[indiceHistorial].humedad = datos.humedad;
        historial[indiceHistorial].tempDHT = datos.tempDHT;
        historial[indiceHistorial].humDHT = datos.humDHT;

        // Avanzamos el índice (Buffer circular: vuelve a cero al llegar a 720)
        indiceHistorial = (indiceHistorial + 1) % muestras_max;
        if (cantidadMuestras < muestras_max) {
            cantidadMuestras++;
        }
    }
}