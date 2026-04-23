#include "sistema_wifi.h"
#include <WiFi.h>

namespace SistemaWifi {

    // --- Configuraciones de red aislada (SoftAP) ---
    const char* AP_SSID    = "ESP32-AP";
    const char* AP_PASS    = "humedades";
    const int   AP_CHANNEL = 1;
    const bool  AP_HIDDEN  = false;
    const int   AP_MAX_CON = 4; // Máximo 4 dispositivos conectados a la vez

    // IPs fijas definidas por tu compañero
    IPAddress local_IP(192, 168, 4, 10);
    IPAddress gateway(192, 168, 4, 9);
    IPAddress subnet(255, 255, 255, 0);

    void inicializar() {
        Serial.println("\n--- Iniciando Sistema de Red ---");
        Serial.println("Configurando ESP32 como Access Point (Modo Aislado)...");

        // Aplicar la configuración de IP estática
        WiFi.softAPConfig(local_IP, gateway, subnet); 
        
        // Levantar la red WiFi
        WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, AP_HIDDEN, AP_MAX_CON); 

        Serial.print("Red creada exitosamente. Nombre (SSID): ");
        Serial.println(AP_SSID);
        Serial.print("Conectate con el celular e ingresa a la IP: ");
        Serial.println(WiFi.softAPIP());
        Serial.println("--------------------------------\n");
    }
}