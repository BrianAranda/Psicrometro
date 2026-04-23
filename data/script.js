var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Iniciando conexión WebSocket...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Conexión establecida');
}

function onClose(event) {
    console.log('Conexión cerrada. Reintentando en 2s...');
    setTimeout(initWebSocket, 2000);
}

// Procesamiento del JSON enviado por el ESP32
function onMessage(event) {
    // console.log("Datos recibidos: ", event.data);
    try {
        var obj = JSON.parse(event.data);

        // 1. Datos Externos (Psicrómetro)
        if (obj.tbs !== undefined) document.getElementById("val-tbs").innerHTML = obj.tbs.toFixed(1);
        if (obj.tbh !== undefined) document.getElementById("val-tbh").innerHTML = obj.tbh.toFixed(1);
        if (obj.humedad !== undefined) document.getElementById("val-hr").innerHTML = obj.humedad;

        // 2. Datos Internos (Gabinete/DHT)
        if (obj.tempDHT !== undefined) document.getElementById("val-temp-int").innerHTML = obj.tempDHT.toFixed(1);
        if (obj.humDHT !== undefined) document.getElementById("val-hum-int").innerHTML = obj.humDHT.toFixed(0);

        // 3. Sistema (Fecha y Datalogger)
        if (obj.fechayhora !== undefined) document.getElementById("fechayhora").innerHTML = obj.fechayhora;

        if (obj.progreso !== undefined) {
            var p = obj.progreso;
            var max_samples = 720;
            var percentage = (p / max_samples) * 100;
            document.getElementById("progresoTexto").innerHTML = p + "/" + max_samples + " (" + percentage.toFixed(1) + "%)";
            document.getElementById("progresoBarra").style.width = percentage + "%";
        }
    } catch (e) {
        console.error("Error procesando el JSON: ", e);
    }
}

// Configuración de Fecha y Hora
document.getElementById("guardar_fecha").addEventListener("click", guardar_fecha);

function guardar_fecha() {
    const fecha = document.getElementById("fecha").value; 
    const hora = document.getElementById("hora").value;   

    if(!fecha || !hora) {
        alert("Por favor, selecciona una fecha y hora válidas.");
        return;
    }

    const msg = {
        fecha: fecha,
        hora: hora
    };
    
    const message = JSON.stringify(msg);

    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(message);
        console.log("Enviado al ESP32:", message);
        // Opcional: Feedback visual de que se envió
        let btn = document.getElementById("guardar_fecha");
        btn.innerHTML = "¡Enviado!";
        btn.style.backgroundColor = "var(--accent-green)";
        setTimeout(() => { 
            btn.innerHTML = "Guardar"; 
            btn.style.backgroundColor = "var(--accent-blue)";
        }, 2000);
    } else {
        console.warn("WebSocket no conectado. Mensaje no enviado.");
        alert("No hay conexión con el equipo.");
    }
}