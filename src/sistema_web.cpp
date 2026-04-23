#include "sistema_web.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

namespace SistemaWeb {

    // Creamos el servidor en el puerto 80 (el puerto estándar para páginas web)
    AsyncWebServer server(80);

    void inicializar() {
        Serial.println("\nConfigurando Sistema Web ...");
        
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

        // Encendemos el servidor
        server.begin();
        Serial.println("Servidor Web Asíncrono iniciado. Escuchando peticiones...");
        Serial.println("");
    }

    void actualizar() {
        // El ESPAsyncWebServer trabaja en segundo plano (asíncrono).
        // Este espacio es para los WebSockets
    }
}