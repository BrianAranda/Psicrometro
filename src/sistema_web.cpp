#include "sistema_web.h"
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
        Serial.println("\nConfigurando Sistema Web ...");
        
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
}