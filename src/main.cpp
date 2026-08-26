#include <Arduino.h>
#include "sistema_psicrometrico.h"
#include "sistema_wifi.h"
#include "sistema_web.h"

// Temporizadores
unsigned long tAnterior = 0;
const long intervalo = 2000;      // Como esta en milisegundos esto es 5 segundos
unsigned long tAnteriorLog = 0;
const long intervaloLog = 2000;   // Se deja rápido para ver el funcionamiento

void setup() {
  Serial.begin(115200);
  SistemaWifi::inicializar();
  SistemaWeb::inicializar();
  SistemaPsicrometrico::inicializar();
}

void loop() {
  unsigned long tActual = millis();

  // Tarea de Pantalla, o sea actualizar los datos sensados y la web
  if (tActual - tAnterior >= intervalo) {
    tAnterior = tActual;
    SistemaPsicrometrico::actualizar();
    SistemaWeb::emitirTelemetria();
  }

  // Tarea de Registro Histórico, o sea los datos que se guardan en el csv
  if (tActual - tAnteriorLog >= intervaloLog) {
    tAnteriorLog = tActual;
    SistemaWeb::registrarMuestra();
  }

  // Mantenimiento del Servidor, solo limpia sesiones cerradas
  SistemaWeb::actualizar();   
}