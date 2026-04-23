#pragma once
#include <Arduino.h>

namespace SistemaWeb {
    // Monta el disco duro y enciende el servidor web
    void inicializar();
    
    // Para los WebSockets
    void actualizar(); 
}