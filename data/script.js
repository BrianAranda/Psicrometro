var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var enteredData = "";
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    //document.getElementById('humedad').innerHTML = event.data;
    var obj = JSON.parse(event.data);
    document.getElementById("humedad").innerHTML = obj.humedad;
    document.getElementById("temperatura").innerHTML = obj.temperatura;
    document.getElementById("fechayhora").innerHTML = obj.fechayhora;

    if (obj.progreso !== undefined) {
        var p = obj.progreso;
        var percentage = (p / 720) * 100;
        document.getElementById("progresoTexto").innerHTML = p + "/720 (" + percentage.toFixed(1) + "%)";
        document.getElementById("progresoBarra").style.width = percentage + "%";
    }
}

document.getElementById("guardar_fecha").addEventListener("click", guardar_fecha);
function guardar_fecha() {
    const fecha = document.getElementById("fecha").value; // yyyy-mm-dd
    const hora = document.getElementById("hora").value;   // hh:mm

    const msg = {
        fecha: `${fecha}`,
        hora: `${hora}`
    };
    const message = JSON.stringify(msg)

    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(message);
        console.log("Sent to ESP32:", message);
    } else {
        console.warn("WebSocket not connected. Message not sent:", message);
    }
}