// Desactiva la conexión WebSocket real
initWebSocket = function () {
    console.log("MODO DEMO: datos simulados (sin conexión al ESP32).");
};

// Conexión "abierta" ficticia
websocket = { readyState: 1, send: function () {} }; // 1 === WebSocket.OPEN

// Estado simulado
var demo = {
    tbs: 24.0,       
    tbh: 20.5,       
    tempInt: 27.0,   
    humInt: 55.0,    
    bateriaV: 12.6,  
    progreso: 0      
};

// Aproximación simple de la humedad relativa
function humedadDemo(tbs, tbh) {
    var delta = tbs - tbh;
    if (delta < 0) return 255;
    return Math.max(0, Math.min(100, Math.round(100 - delta * 6.2)));
}

// Estado de carga a partir de la tensión
function bateriaPctDemo(v) {
    return Math.max(0, Math.min(100, Math.round((v - 8) / (13 - 8) * 100)));
}

// Fecha y hora en el formato que emite el ESP32 
function formatearFecha(d) {
    var p = function (n) { return (n < 10 ? "0" : "") + n; };
    return p(d.getDate()) + "/" + p(d.getMonth() + 1) + "/" + d.getFullYear() +
           " " + p(d.getHours()) + ":" + p(d.getMinutes());
}
function fechaHoraDemo() {
    return formatearFecha(new Date()); // reloj actual, para el encabezado del panel
}

// Registro histórico para la descarga del CSV
var sincronizado = false; // el datalogger no arranca hasta sincronizar la hora (botón Guardar)
var MAX_MUESTRAS = 720;
var historial = [];
var horaLog = new Date(); // marca de tiempo simulada del datalogger (avanza 2 min por muestra)

function registrarMuestra(obj) {
    historial.push({
        fechayhora: formatearFecha(horaLog),
        tbs: obj.tbs,
        tbh: obj.tbh,
        humedad: obj.humedad,
        tempDHT: obj.tempDHT,
        humDHT: obj.humDHT
    });
    horaLog = new Date(horaLog.getTime() + 2 * 60 * 1000); // +2 min, como el intervalo real
    if (historial.length > MAX_MUESTRAS) historial.shift(); // buffer circular
}

// Arma el CSV y dispara la descarga como archivo local.
function descargarCSV() {
    var lineas = ["Fecha_Hora,TBS(C),TBH(C),HR(%),Temp_Int(C),Hum_Int(%)"];
    for (var i = 0; i < historial.length; i++) {
        var m = historial[i];
        lineas.push(
            m.fechayhora + "," +
            m.tbs.toFixed(1) + "," +
            m.tbh.toFixed(1) + "," +
            m.humedad + "," +
            m.tempDHT.toFixed(1) + "," +
            m.humDHT.toFixed(0)
        );
    }
    var blob = new Blob([lineas.join("\n") + "\n"], { type: "text/csv;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.href = url;
    a.download = "psicrometro_datalogger.csv";
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

// Cambia los valores para que varien
function pasoDemo() {
    demo.tbs += (Math.random() - 0.5) * 0.3;
    demo.tbh += (Math.random() - 0.5) * 0.2;
    if (demo.tbh > demo.tbs - 0.2) demo.tbh = demo.tbs - 0.2; 

    demo.tempInt += (Math.random() - 0.5) * 0.2;
    demo.humInt = Math.max(0, Math.min(100, demo.humInt + (Math.random() - 0.5)));

    demo.bateriaV -= 0.02;                          
    if (demo.bateriaV < 11.8) demo.bateriaV = 12.7; 

    if (sincronizado && demo.progreso < 720) demo.progreso++;
}

// Arma el mismo objeto JSON que enviaría el ESP32 y lo entrega a onMessage().
function emitirDemo() {
    pasoDemo();
    var obj = {
        tbs: demo.tbs,
        tbh: demo.tbh,
        humedad: humedadDemo(demo.tbs, demo.tbh),
        tempDHT: demo.tempInt,
        humDHT: demo.humInt,
        bateriaPct: bateriaPctDemo(demo.bateriaV),
        bateriaV: demo.bateriaV,
        fechayhora: sincronizado ? fechaHoraDemo() : "Esperando ingreso de sincronización",
        progreso: demo.progreso
    };
    if (sincronizado) registrarMuestra(obj);
    onMessage({ data: JSON.stringify(obj) });
}

window.addEventListener('load', function () {
    // En la demo, el botón "Descargar CSV" genera el archivo en el navegador
    var btnCsv = document.querySelector('a.secondary-btn[href="/download.csv"]');
    if (btnCsv) {
        btnCsv.addEventListener('click', function (e) {
            e.preventDefault();
            descargarCSV();
        });
    }

    // En la demo, el botón "Guardar" (Sincronización RTC) arranca el datalogger
    var btnGuardar = document.getElementById('guardar_fecha');
    if (btnGuardar) {
        btnGuardar.addEventListener('click', function () {
            var fecha = document.getElementById('fecha').value;
            var hora = document.getElementById('hora').value;
            if (fecha && hora && !sincronizado) {
                var f = fecha.split('-'), h = hora.split(':');
                horaLog = new Date(f[0], f[1] - 1, f[2], h[0], h[1], 0); // el registro arranca en la hora ingresada
                sincronizado = true;
            }
        });
    }

    emitirDemo();                  
    setInterval(emitirDemo, 2000); 
});
