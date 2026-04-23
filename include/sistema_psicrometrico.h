#pragma once
#include <Arduino.h>

// Tipo de dato para modularizar y hacer mas sencilla la lógica de main
// El SistemaPsicrometrico va a manejar el display y sensores
namespace SistemaPsicrometrico {

    // Datos de los sensores utiles a exportar
    struct DatosSensores {
        float tbs;
        float tbh;
        uint8_t humedad;
    };

    // Función que inicializa el hardware del display y sensores
    void inicializar();             

    // Lee sensores, calcula humedad y actualiza el display
    void actualizar();              
    
    // Función Getter para obtener los datos a mostrar y guardar 
    DatosSensores getUltimosDatos();     
}