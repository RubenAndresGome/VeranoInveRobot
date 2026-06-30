# 📋 TASK 013: Implementar Correcciones de Auditoría

**Contexto:**
Basado en el `PDCA_013_Fix_Auditoria_Bugs.md` y la reciente auditoría, esta tarea define los pasos concretos para aplicar las correcciones en el código fuente.

---

## 🛠️ Acciones de Implementación

### Tarea 1: Resolución de vulnerabilidades y Memory Leaks (web_server.cpp)
1. **Buffer Overflow (BUG-01):** En `onWsEvent`, reemplazar `data[len] = 0; const char* msg = (const char*)data;` con una copia segura hacia un buffer temporal (e.g. `char msgBuf[256]`).
2. **Memory Leak HTTP (BUG-05):** Añadir código en `server.onNotFound()` y al final de endpoints que consumen `_tempObject` para eliminar el `delete` inseguro y prevenir filtraciones si las peticiones son inválidas.
3. **Optimización WebSocket (BUG-03):** Eliminar la llamada redundante a serialización JSON y WebSocket broadcast (`ws.textAll()`) en `web_server_send_telemetry()`. Mantener el bucle temporalizado de `ws_push_telemetry()` llamado desde `web_server_loop()`.

### Tarea 2: Concurrencia y Thread Safety (web_server.cpp & main.cpp)
1. **Race Conditions de Telemetría (BUG-02):**
   - Declarar `portMUX_TYPE muxTelWeb = portMUX_INITIALIZER_UNLOCKED;`.
   - Proteger el bloque de actualización de variables `tel_*` en `web_server_send_telemetry`.
   - Proteger las lecturas de `tel_*` en la ruta `/status` y `ws_push_telemetry`.
   - Modificar `tel_estado` de `String` a un array `char[16]` estático o protegerlo rigurosamente para evitar el *String thread-safety issue* (BUG-15).
2. **Contexto ISR Inválido (BUG-04):** En `main.cpp`, buscar las instancias de `portENTER_CRITICAL_ISR(&muxSegmentacion)` en rutinas del `loop()` (como `tickFrenado` y el handler de `STATE_IDLE`) y cambiarlas a `portENTER_CRITICAL` sin la terminación `_ISR`.

### Tarea 3: Modificaciones Lógicas (Altos y Medios)
1. **Odometría Giro (BUG-06):** En `tickGiro()` dentro de `main.cpp`, modificar la suma de orientación: `orientacionGlobal += angZ * (PI / 180.0f);` en vez de usar `objetivo.angulo`.
2. **Estado AP Mode (BUG-07, BUG-08, BUG-19):**
   - En `web_server_is_connected()`, cambiar `WiFi.status() == WL_CONNECTED` por `WiFi.softAPgetStationNum() > 0`.
   - En `web_server_get_ip()`, retornar `WiFi.softAPIP().toString()`.
   - En `/status`, cambiar `doc["connected"]` para reflejar la presencia de clientes locales.
3. **Credenciales WiFi (BUG-12):** En `web_server_init()`, usar `config_manager_get_wifi_ssid()` y `config_manager_get_wifi_password()` en lugar de los `#define` hardcodeados.
4. **Flujo E-Stop (BUG-10, BUG-11):** Mejorar los comandos de `RESET` ('R') para que envíen explícitamente a `web_server_enqueue_command` un comando especial de reseteo, o directamente manipulen un callback si es necesario limpiar la bandera general del estado de E-Stop con seguridad.

---

## 🚦 Finalización
Una vez implementados estos cambios, se debe ejecutar una prueba de compilación:
`pio run --target upload`

En caso de que sea exitoso, marcar esta tarea como completada, actualizar el historial de GIT y archivar estos documentos en `.mde_history/Realizado`.
