# Psicrometro

# Instrucciones de Carga de Archivos (LittleFS)

Este directorio contiene los archivos de la interfaz web que serán servidos por el ESP32. Para que el sistema funcione, estos archivos deben grabarse en la memoria Flash del microcontrolador.

## Pasos para la carga en PlatformIO:

1. **Preparar la carpeta:** Asegúrate de que todos los archivos necesarios (`index.html`, `style.css`, `script.js`, etc.) estén dentro de esta carpeta `data/`.
2. **Cerrar el Monitor Serial:** Asegúrate de que la terminal del Monitor Serial no esté ocupando el puerto.
3. **Abrir el Menú de PlatformIO:** Haz clic en el ícono de la la extensión en la barra lateral izquierda de VS Code.
4. **Localizar las Tareas del Proyecto:**
   - Busca la carpeta de tu entorno (ej. `Default` o `env:esp32dev`).
   - Despliega la sección **Platform**.
5. **Generar la Imagen:** Haz clic en **"Build Filesystem Image"**. Esto creará el archivo binario con el contenido de esta carpeta.
6. **Subir al ESP32:** Una vez que termine el paso anterior, haz clic en **"Upload Filesystem Image"**.

**Nota importante:** Este proceso es independiente de la subida del código principal (`Upload`). Si realizas cambios en el HTML, debes repetir estos pasos para que se reflejen en el dispositivo.