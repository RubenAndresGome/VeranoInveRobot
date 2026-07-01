# Auditoría de Fallos del Servidor Web (ESP32-S3)

Se ha realizado una auditoría del código fuente del proyecto (`src/web_server.cpp`, `src/main.cpp`, `src/UserControlGUI.cpp`) para identificar las causas subyacentes de los problemas reportados. A continuación se detallan los hallazgos:

## 1. Congelamiento del Microcontrolador (Crash / OOM)

> [!CAUTION]
> Fuga de Memoria y Fragmentación Severa del Heap en Peticiones POST.

El problema principal radica en cómo se manejan los cuerpos de las peticiones POST (rutas, rutinas JSON) en el servidor asíncrono. 

**Causa en el código (`src/web_server.cpp:199`):**
```cpp
static void handle_body_accumulation(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!request->_tempObject) {
        request->_tempObject = new String();
    }
    String* body = (String*)request->_tempObject;
    for (size_t i = 0; i < len; i++) {
        body->concat((char)data[i]);
    }
}
```
* **Explicación:** La función concatena la información entrante carácter por carácter (`body->concat`). Al recibir una ruta de laberinto (que puede ser muy larga o en formato JSON), esta concatenación fuerza a la clase `String` a reubicar y expandir dinámicamente la memoria decenas o cientos de veces. Esto produce una fragmentación crítica en el Heap (Memoria RAM) y agota la memoria rápidamente, provocando un *Guru Meditation Error* o un reinicio súbito. Además, al ejecutarse dentro del contexto de interrupción/tarea de `AsyncWebServer`, bloquea la ejecución, lo que dispara el *Watchdog Timer (WDT)*.

## 2. Caída del WebSocket y Peticiones Asíncronas

> [!WARNING]
> Saturación de la Cola de Mensajes de WebSocket (Buffer Overflow).

El panel frontal depende de una conexión de WebSocket (o Polling) fluida para el sistema de telemetría y el lienzo (canvas).

**Causa en el código (`src/web_server.cpp:591` y `src/main.cpp:837`):**
```cpp
// En web_server_loop()
if (ahora - tUltimoWSPush > 100) {
    tUltimoWSPush = ahora;
    ws_push_telemetry();
}
```
* **Explicación:** La telemetría se envía por WebSockets a 10 Hz (cada 100ms) a todos los clientes. La librería `ESPAsyncWebServer` usa una cola de mensajes en memoria para cada cliente conectado. Si el cliente (el navegador Android) sufre latencia, procesa la interfaz lentamente, o suspende el hilo, la cola del ESP32 se llena. La función `ws.textAll()` intenta encolar forzosamente más mensajes sin verificar si hay espacio (`client->queueIsFull()`). Una vez que la cola se llena, la conexión TCP se ahoga, el socket se rompe y el navegador se queda con el *spinner* de carga infinito sin recibir respuestas.

## 3. Inestabilidad de mDNS (`robot.local`) en Android

> [!NOTE]
> Limitaciones del Protocolo mDNS en Android

Hacia el final del escenario, se intentó recargar la interfaz con la dirección mDNS: `robot.local`.

**Causa Externa:**
* El ESP32 inicializa correctamente `MDNS.begin("robot")`. Sin embargo, el sistema operativo **Android NO soporta la resolución de nombres `.local` (mDNS) de forma nativa** a nivel de sistema ni en el navegador Chrome estándar. A diferencia de iOS, macOS, o Windows que integran "Bonjour/Avahi", un dispositivo Android enviará la petición `robot.local` a los servidores DNS normales de internet o de datos móviles (la cual lógicamente falla) en lugar de buscar en la red local.

* **Solución recomendada para el usuario:** Depender de la dirección IP fija que asigna el Access Point (ej. `http://192.168.4.1`) o forzar a los usuarios a acceder mediante el *Captive Portal* de la red WiFi del robot, el cual ya está configurado en el código, pero que en las nuevas pestañas no funcionará vía `robot.local`.

---

## Plan de Acción Recomendado

1. **Reparar la asignación dinámica de Strings:** Modificar `handle_body_accumulation` para pre-reservar la memoria total (`body->reserve(total)`) y usar copias de bloque completo en lugar de concatenaciones por carácter.
2. **Control de Flujo en WebSockets:** Implementar verificación de `queueIsFull()` antes de empujar nueva telemetría. Reducir la frecuencia de telemetría o habilitarla sólo cuando haya movimiento/cambio de estados significativos.
3. **Manejo de Tiempos en Tareas:** Evitar el uso intensivo de `String` y conversiones JSON `serializeJson()` / `deserializeJson()` dentro de interrupciones asíncronas para que no colapse el Watchdog.
