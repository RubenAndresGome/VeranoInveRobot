# PDCA 016: Modo Depuración FSM, Notificaciones CRM e I/O JSON

## 1. Plan
**Problema:** La interfaz carecía de herramientas de introspección avanzada en tiempo real para el Hardware (override de FSM), un historial persistente de alertas, y un mecanismo para persistir los laberintos programados en el cliente.
**Objetivo:** Agregar un menú FSM en el header, un menú desplegable estilo CRM para leer y exportar logs a `.txt`, y capacidad de importar/exportar `waypoints.json`.

## 2. Do
- Modificación en `src/web_server.cpp` (añadido `wsForcedFsmState`) y `main.cpp` (Polling `wsFsmOverrideRequest`) para cambiar el estado de la FSM global según demanda del WebSocket (`FSM:ESTADO`).
- Inyección HTML/CSS (`frontend/index.html` y `styles.css`) de un header con `<select>` de estados y un botón flotante con badge numérico (`btn-glass-crm`).
- Creación de funciones de `window.app`: `forceFSM()`, `toggleNotifications()`, `downloadLogsTXT()`, `exportJSON()` y `importJSON()`.

## 3. Check
- La compilación es exitosa. 
- Los botones descargan dinámicamente un archivo usando el API `Blob`. 
- El MCU actualiza su máquina de estados instantáneamente al recibir el string `FSM:` por el canal bidireccional de WebSocket.
- No hay desbordamiento en memoria porque las notificaciones se muestran en un offcanvas en capas que no interfiere con el render 3D (canvas).

## 4. Act
- Estandarizar este mecanismo I/O JSON para futuras expansiones (como perfiles de PID).
- Documentación de Indexator Graphicus no requiere cambios mayores ya que la topología (`include/` vs `src/`) se mantiene intacta.
- El proyecto incrementa su versión interna de sesiones MDE a 16.
