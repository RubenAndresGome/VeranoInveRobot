# PDCA 018: Auditoría de Bugs (Memory Leak, Race Conditions) y Áreas de Mejora

## 1. Plan (Auditoría e Identificación)
Tras la ejecución del PDCA 017, la instrucción dictó una **auditoría profunda sobre la nomenclatura, búsqueda de bugs estáticos y proposición de áreas de mejora**. 

**Hallazgos y Bugs Críticos Encontrados:**
1. **[BUG] Memory Leak en Frontend (`app.js`)**: Las notificaciones se acumulan indefinidamente en el array `this.notifLogs` y en el DOM bajo `#notif-list`. Si el robot está en constante operación de telemetría y produce alertas, el navegador consumirá memoria RAM infinitamente, provocando el colapso del canvas de trayectoria.
2. **[BUG] Race Condition en C++ (`web_server.cpp` vs `main.cpp`)**: El valor de la cadena `wsForcedFsmState` es escrito en el hilo asíncrono del WebServer (`onWsEvent`) al tiempo que el hilo principal (Core 1) de `loop()` lo intenta leer. Al no haber un Mutex (Spinlock) que proteja la memoria del buffer de carácteres, se podrían producir lecturas corruptas de memoria si la cadena se sobrescribe exactamente durante una evaluación `if(reqState == "INIT")` en `main.cpp`.
3. **[Nomenclatura]**: Existía un choque de nomenclatura; dos archivos PDCA querían ser el 016. Se corrigió renombrando a 017 y 018 respectivamente, asegurando un rastreo hilemórfico lineal.

**Áreas de Mejora a Futuro:**
- **Refactorización de la FSM (Firmware)**: Requerimos transiciones seguras. Actualmentee si obligamos a la FSM a saltar a `STATE_INIT` desde `STATE_TURNING`, los motores no se detendrán, ya que el estado INIT no tiene ninguna instrucción de frenado en su `switch (nuevo)`. Debería existir un `cleanup()` antes de cambiar de estado.
- **Validación del JSON Laberinto**: Cuando se carga un `waypoints.json`, sólo validamos si es un array, no si los campos `x` e `y` existen, pudiendo romper `canvasTrayectoria`.

## 2. Do (Resolución)
Se arreglarán ambos bugs detectados en este mismo ciclo de acción:

- **En C++**: Añadiré un `portMUX_TYPE muxFsmOverride` a `web_server.h`.
- Modificaré `web_server.cpp` para que adquiera el lock antes de modificar `wsForcedFsmState`.
- Modificaré `main.cpp` para adquirir el lock antes de leer la solicitud.
- **En JS**: Añadiré una condición que limite `notifLogs.length` a un histórico máximo de 100 y retire los elementos obsoletos del DOM para evitar Memory Leaks.

## 3. Check & 4. Act
Ambas soluciones estabilizarán el entorno evitando caídas abruptas y bloqueos atómicos en el microcontrolador ESP32-S3. Se registrará la versión formal en `_index.json` y se ejecutará compilación.
