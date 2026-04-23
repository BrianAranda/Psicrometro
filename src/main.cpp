#include <Arduino.h>
#include "sistema_psicrometrico.h"

unsigned long tAnterior = 0;
const long intervalo = 5000;    // Como esta en milisegundos esto es 5 segundos

void setup() {
  Serial.begin(115200);
  SistemaPsicrometrico::inicializar();
}

void loop() {
  unsigned long tActual = millis();

  if (tActual - tAnterior >= intervalo) {
    tAnterior = tActual;
    SistemaPsicrometrico::actualizar();
  }

  // Cuando se implemente el sistema web se llamaría a SistemaPsicrometrico::getUltimosDatos()
}