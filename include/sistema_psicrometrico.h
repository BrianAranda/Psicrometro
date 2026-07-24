#pragma once
#include <Arduino.h>

// Tipo de dato para modularizar y hacer mas sencilla la lógica de main
// El SistemaPsicrometrico va a manejar el display y sensores
namespace SistemaPsicrometrico {

    // Datos de los sensores utiles a exportar
    struct DatosSensores {
        float tbs;          // Bulbo Seco (Exterior)
        float tbh;          // Bulbo Húmedo (Exterior)
        uint8_t humedad;    // Humedad Calculada (Exterior)
        float tempDHT;      // Temperatura Interna (DHT)
        float humDHT;       // Humedad Interna (DHT)
        float bateriaV;     // Tensión del banco de baterías (V)
        uint8_t bateriaPct; // Estado de carga estimado (0-100 %)
    };

    // Función que inicializa el hardware del display y sensores
    void inicializar();             

    // Lee sensores, calcula humedad y actualiza el display
    void actualizar();              
    
    // Función Getter para obtener los datos a mostrar y guardar 
    DatosSensores getUltimosDatos();     
}