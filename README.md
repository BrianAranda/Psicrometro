# Psicrómetro digital

Sistema embebido sobre **ESP32** que mide la humedad relativa ambiente por el método
psicrométrico (bulbo seco / bulbo húmedo), la registra localmente y la publica en una
interfaz web servida por el propio equipo, para su uso en la cámara de marchitado de una
planta experimental de té (ambiente cercano al 100 % de humedad, con cortes frecuentes de
energía e internet).

Proyecto para promoción de la asignatura **Técnicas Digitales 2 y Sistemas Embebidos**
Facultad de Ingeniería, U.Na.M. Autores: **Aranda, Brian Ezequiel** y **Klopsztein, Leonel Agustín**.

---

## Características

- **Psicrómetro digital**: dos sensores de temperatura **DS18B20** (bulbo seco y bulbo
  húmedo) y cálculo de humedad relativa por **tablas psicrométricas** cargadas en memoria.
- **Monitoreo interno de la caja**: sensor **DHT** para temperatura y humedad dentro del
  gabinete (detección de fallo de sellos).
- **Medición de batería**: lectura de la tensión del banco por ADC (GPIO33) y estimación
  del estado de carga en %.
- **Pantalla local**: LCD I2C 20x4 con la telemetría en vivo.
- **Servidor web propio (Access Point)**: dashboard HTML/CSS/JS servido desde LittleFS,
  sin depender de internet.
- **Telemetría en tiempo real** por WebSocket, sin recargar la página.
- **Datalogger** en buffer circular en RAM (720 muestras) y **exportación a CSV** "al vuelo".
- **Reloj (RTC) por sincronización manual** desde el navegador al inicio de la jornada.

---

## Hardware

| Componente            | Detalle                                  | Pin ESP32              |
|-----------------------|------------------------------------------|------------------------|
| DS18B20 (TBS y TBH)   | Bus OneWire compartido (2 sensores)      | GPIO4                  |
| DHT (interno)         | Temp./humedad del gabinete               | GPIO16                 |
| LCD I2C 20x4          | Dirección I2C `0x27`                     | SDA GPIO21 / SCL GPIO22|
| Batería (divisor)     | ADC1_CH5, divisor resistivo              | GPIO33                 |

- **Placa**: NodeMCU-32S (`board = nodemcu-32s`).
- Los dos DS18B20 comparten el bus OneWire y se distinguen por su **dirección ROM** de 64
  bits, definidas en `src/sistema_psicrometrico.cpp` (`direccionTBS` / `direccionTBH`). Si
  se reemplaza un sensor, hay que actualizar esa dirección (se obtiene con un scanner OneWire).
- **Batería**: se lee a través de un divisor resistivo que baja la tensión del banco al
  rango seguro del ADC (0–3,3 V). El factor `RATIO_DIVISOR` y los límites `BATERIA_LLENA` /
  `BATERIA_VACIA` se ajustan en `src/sistema_psicrometrico.cpp` (ver *Calibración*).

---

## Estructura del proyecto

```
Codigo Proyecto/
├── platformio.ini            # Configuración de PlatformIO y dependencias
├── include/                  # Cabeceras (.h) de cada módulo
│   ├── sistema_psicrometrico.h
│   ├── sistema_wifi.h
│   ├── sistema_web.h
│   └── tablas_psicrometricas.h
├── src/                      # Implementación del firmware (.cpp)
│   ├── main.cpp              # Orquestador no bloqueante (millis)
│   ├── sistema_psicrometrico.cpp   # Sensores, LCD, cálculo de HR y batería
│   ├── sistema_wifi.cpp            # Access Point
│   ├── sistema_web.cpp             # Servidor web, WebSocket, RTC y datalogger
│   └── tablas_psicrometricas.cpp   # Tablas de HR en PROGMEM
└── data/                     # Frontend servido por LittleFS
    ├── index.html
    ├── style.css
    └── script.js
```

Arquitectura del firmware: `main.cpp` orquesta tres tareas por tiempo con `millis()`
(lectura de sensores, registro del datalogger y mantenimiento del servidor), delegando la
lógica en los módulos `SistemaPsicrometrico`, `SistemaWifi` y `SistemaWeb`.

---

## Entorno de desarrollo

- **[PlatformIO](https://platformio.org/)** (extensión de VS Code) o `pio` por línea de comandos.
- Framework Arduino para ESP32.
- Dependencias (se instalan solas al compilar, están declaradas en `platformio.ini`):
  `LiquidCrystal_I2C`, `OneWire`, `DallasTemperature`, `DHT` (markruys),
  `ESPAsyncWebServer`, `AsyncTCP`, `ArduinoJson`.

> El proyecto tiene **dos binarios independientes**: el **firmware** (código de `src/`) y la
> **imagen del sistema de archivos** (contenido de `data/`). Se suben por separado.

---

## 1) Cargar el firmware en el ESP32

Sube el programa (todo lo de `src/` e `include/`).

### Desde VS Code (PlatformIO)
1. Conectá el ESP32 por USB (con el driver CP210x instalado; está en
   `Código ESP32/Driver detector ESP32/`).
2. En la barra inferior de PlatformIO:
   - **✓ (Build)** para compilar.
   - **→ (Upload)** para compilar y subir el firmware.
3. Alternativa: panel de PlatformIO → *Project Tasks* → `env:nodemcu-32s` → *General* →
   **Upload**.

### Desde la terminal
```bash
pio run                 # Compila el firmware
pio run -t upload       # Compila y sube el firmware al ESP32
pio device monitor      # Abre el monitor serial (115200 baudios)
```

---

## 2) Cargar los archivos web (LittleFS)

Sube el contenido de `data/` (la interfaz). **Es un paso separado del anterior**: cada vez
que cambies `index.html`, `style.css` o `script.js` tenés que repetir esto para que el
cambio llegue al equipo.

> **Cerrá el Monitor Serial antes de subir** (ocupa el puerto y hace fallar la carga).

### Desde VS Code (PlatformIO)
1. Panel de PlatformIO → *Project Tasks* → `env:nodemcu-32s` → **Platform**.
2. **Build Filesystem Image**: genera la imagen con el contenido de `data/`.
3. **Upload Filesystem Image**: sube esa imagen al ESP32.

### Desde la terminal
```bash
pio run -t buildfs      # Genera la imagen LittleFS a partir de data/
pio run -t uploadfs     # Sube la imagen del sistema de archivos al ESP32
```

---

## Uso

1. Alimentá el equipo. En el monitor serial deberías ver la creación del Access Point.
2. Desde un celular o PC, conectate a la red WiFi:
   - **SSID**: `Proyecto Embebidos`
   - **Contraseña**: `humedades`
3. Abrí el navegador en **`http://10.10.10.10`**.
4. En **Sincronización RTC**, cargá la fecha y hora e presioná **Guardar**. Recién a partir
   de ese momento el datalogger empieza a registrar (antes muestra "Esperando ingreso de
   sincronización"). Hay que hacerlo al inicio de cada jornada / tras cada reinicio.
5. La telemetría se actualiza sola. Para descargar el histórico, usá **Descargar CSV**.

---

## Calibración de la batería

En `src/sistema_psicrometrico.cpp`:

```cpp
const float RATIO_DIVISOR = 4.06;   // (R1 + R2) / R2  -> V_bateria = V_pin * RATIO_DIVISOR
const float BATERIA_LLENA = 13;     // V con la batería cargada    -> 100 %
const float BATERIA_VACIA = 8;      // V con la batería descargada -> 0 %
```

Para ajustar `RATIO_DIVISOR` con el equipo real: medí con un tester la tensión de la
batería y la tensión en el pin 33, y calculá `RATIO_DIVISOR = V_bateria / V_pin`. Ajustá
`BATERIA_LLENA` y `BATERIA_VACIA` a los límites de tu batería.

---

## Ramas del repositorio

El desarrollo se hizo por incrementos, cada uno en su rama:

- `sensores`: DS18B20, DHT, LCD y cálculo de HR.
- `wifi`: Access Point.
- `web`: servidor web y dashboard.
- `sockets`: WebSocket, RTC, datalogger, exportación CSV y batería (rama más completa).
- `main`: integración final.
- `demo`: copia del frontend para publicar el panel de control en **GitHub Pages** con datos simulados

> La rama `demo` funciona sin necesidad del equipo físico. Agrega `data/simulador.js`.
> Queda accesible en `https://brianaranda.github.io/Psicrometro/data/`.
> **No cargar la rama `demo` al ESP32.** porque `simulador.js` desactiva la conexión real.