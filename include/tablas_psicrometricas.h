#pragma once // Para compilarlo una sola vez, recomendación de ia
#include <Arduino.h> // Para uint8_t y PROGMEM

// Definición de las tablas como externas para solo llamarlas una unica vez
extern const uint8_t tablaA[26][21];
extern const uint8_t tablaB[17][21];