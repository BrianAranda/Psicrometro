#include "sistema_psicrometrico.h"
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "tablas_psicrometricas.h"

namespace SistemaPsicrometrico {

    // GPIO (ver pinout) donde el DS18B20 esta conectado
    const int oneWireBus = 4;
    OneWire oneWire(oneWireBus);
    DallasTemperature sensors(&oneWire);
    // Configuracion para el display (el 0x27 se tomo con el scanner)
    LiquidCrystal_I2C lcd(0x27, 20, 4);

    // Direcciones MAC de cada sensor (fueron tomadas con otro script)
    DeviceAddress direccionTBS = { 0x28, 0x3E, 0xF3, 0x7C, 0x00, 0x00, 0x00, 0xB2 }; 
    DeviceAddress direccionTBH = { 0x28, 0xAC, 0x00, 0x7A, 0x00, 0x00, 0x00, 0x8C }; 

    // Valores iniciales por seguridad
    DatosSensores ultimosDatos = {0.0, 0.0, 255};

    // Función que devuelve la humedad o 255 si está fuera de rango
    uint8_t calcularHumedad(float tbs, float tbh) {
        float deltaT = tbs - tbh;

        // Error físico: El bulbo húmedo nunca debería ser más caliente que el seco
        if (deltaT < 0) return 255; 

        int tbh_round = round(tbh);

        // Menor a 20°C solo Tabla A
        if (tbh_round < 20) {
            if (deltaT > 10) return 255; // Fuera del rango de columnas
            int col = round(deltaT * 2.0); // Pasos de 0.5
            return pgm_read_byte(&tablaA[tbh_round][col]);
        }
        
        // Mayor a 25°C solo Tabla B
        else if (tbh_round > 25) {
            if (tbh_round > 99 || deltaT > 20.0) return 255;
            int fila = round((tbh_round - 20.0) / 5.0); // Saltos de 5 en 5
            int col = round(deltaT);                    // Pasos de 1.0
            return pgm_read_byte(&tablaB[fila][col]);
        }
        
        // Entre 20°C y 25°C por solapamiento
        else {
            // Si el Delta T entra en la Tabla A tiene mayor precisión
            if (deltaT <= 10.0) {
            int col = round(deltaT * 2.0);
            return pgm_read_byte(&tablaA[tbh_round][col]);
            } 
            // Sino la Tabla A no alcanza entonces usamos la B
            else if (deltaT <= 20.0) {
            int fila = round((tbh_round - 20.0) / 5.0); 
            int col = round(deltaT);
            return pgm_read_byte(&tablaB[fila][col]);
            } 
            // Si el Delta T es mayor a 20, ninguna tabla lo cubre
            else {
            return 255; 
            }
        }
    }

    // Inicialización, o sea lo que pasaría en el setup ahre
    void inicializar() {
        sensors.begin();
        lcd.init();
        lcd.backlight();

        // Configurar la resolución a 12 bits (máxima precisión: 0.0625°C)
        sensors.setResolution(direccionTBS, 12);
        sensors.setResolution(direccionTBH, 12);
        
        lcd.setCursor(0, 0);
        lcd.print("Proyecto Embebidos");
    }

    // Actualiza la lectura de los datos, o sea lo que pasaría en el loop xd
    void actualizar() {
        
        sensors.requestTemperatures();
        ultimosDatos.tbs = sensors.getTempC(direccionTBS);
        ultimosDatos.tbh = sensors.getTempC(direccionTBH);

        // Esto porque se me desconecto sin querer y salto error 
        if (ultimosDatos.tbs == DEVICE_DISCONNECTED_C || ultimosDatos.tbh == DEVICE_DISCONNECTED_C) {
            lcd.setCursor(0, 1);
            lcd.print("Sensor desconectado");
            return;
        }

        ultimosDatos.humedad = calcularHumedad(ultimosDatos.tbs, ultimosDatos.tbh);

        // Actualización del display
        lcd.setCursor(0, 1);
        lcd.print("TBS: "); lcd.print(ultimosDatos.tbs, 2); lcd.print((char)223); lcd.print("C  ");
        lcd.setCursor(0, 2);
        lcd.print("TBH: "); lcd.print(ultimosDatos.tbh, 2); lcd.print((char)223); lcd.print("C  ");
        lcd.setCursor(0, 3);
        lcd.print("HR: ");
        if (ultimosDatos.humedad == 255) lcd.print("Fuera de rango");
        else { lcd.print(ultimosDatos.humedad); lcd.print("%    "); }
    }

    DatosSensores getUltimosDatos() {
        return ultimosDatos;
    }
}