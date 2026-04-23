#include "sistema_wifi.h"
#include <WiFi.h>

namespace SistemaWifi {

    // Configuraciones de red como Access Point (es una red aislada sin conexión a internet)
    const char* AP_SSID    = "Proyecto Embebidos";    // Nombre de la red
    const char* AP_PASS    = "humedades";             // Contraseña de la red 
    const int   AP_CHANNEL = 1;             // Canal por el que se conecta (entre 1 y 13) 
    const bool  AP_HIDDEN  = false;         // Para mostrar a todos la red
    const int   AP_MAX_CON = 4;             // Número máximo de dispositivos conectados a la vez

    // IPs fijas para montar el Access Point
    IPAddress local_IP(10, 10, 10, 10);     // Dirección IPv4 a conectarse luego en la web
    IPAddress gateway(10, 10, 10, 10);      // Salida a internet por router, formalismo nomas 
    IPAddress subnet(255, 255, 255, 0);     // Mascara de Red, deja espacio para 254 conexiones

    void inicializar() {
        Serial.println("Configurando Access Point ...");

        // Aplicar la configuración IP y levantar el Access Point
        WiFi.softAPConfig(local_IP, gateway, subnet); 
        WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, AP_HIDDEN, AP_MAX_CON); 
        
        // Muestra las cosas por serial (todavia no por display)
        Serial.println("Red creada correctamente:");
        Serial.print("Nombre (SSID): ");    Serial.println(AP_SSID);
        Serial.print("Contraseña: ");       Serial.println(AP_PASS);
        Serial.print("IP del sistema: ");   Serial.println(WiFi.softAPIP());
    }
}