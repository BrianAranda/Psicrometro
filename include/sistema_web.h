#pragma once
#include <Arduino.h>

namespace SistemaWeb {
    // Monta el disco duro y enciende el servidor web
    void inicializar();
    
    // Para los WebSockets
    void actualizar(); 

    // Para enviar los datos a la página
    void emitirTelemetria();

    // Para el Datalogger (guardar los datos)
    void registrarMuestra();
}