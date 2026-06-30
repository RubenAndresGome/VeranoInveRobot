# PDCA 011: Auditoria MDE Fase 2 — Analisis Profundo (codebase-memory)

**Fecha:** 2026-06-27 | **Estado:** COMPLETADO | **Build:** SUCCESS

---

## Objetivo

Segunda pasada de auditoria usando mental model de knowledge graph (call chains, state transitions, data flow, race conditions) inspirado en la metodologia de codebase-memory-mcp.

---

## Plan (Objetivo) (Objetivo)

### Alcance extendido
- `src/main.cpp` — FSM, PID, odometria, loop principal
- `src/comandos.cpp` — Segmentacion de movimientos
- `src/web_server.cpp` — WebSocket handler, RUTA parser
- `src/UserControlGUI.cpp` — Canvas trayectoria, WebSocket frontend
- `include/config.h` — Constantes de control

### Vectores de ataque
1. Race conditions en variables compartidas
2. Estados no alcanzables o transiciones invalidas en FSM
3. Data flow: telemetria inconsistente entre modulos
4. Call chains: strtok no reentrante, const-correctness
5. Cross-browser: CSS sin vendor prefixes
6. Resource lifecycle: WebSocket reconnect stacking

---

## Do (Implementacion) (Implementacion)

### C1: Doble waypoint en touch devices
- **Archivo:** `src/UserControlGUI.cpp:508-509`
- **Raiz:** `mousedown` + `touchstart` ambos disparan en dispositivos tactiles
- **Impacto:** 2 waypoints por cada tap en canvas de trayectoria
- **Fix:** Timestamp `_lastTouch` + guard en mousedown (<300ms skip)

### C2: WebSocket reconnect stacking
- **Archivo:** `src/UserControlGUI.cpp:707`
- **Raiz:** `setTimeout(initWS, 3000)` sin guardia de reentrada
- **Impacto:** Multiples conexiones WS simultaneas en WiFi intermitente
- **Fix:** Flag `wsReconnecting` previene stacking de timers

### M1: PID integral windup
- **Archivo:** `src/main.cpp:446` + `include/config.h`
- **Raiz:** `pidIntegral += error * dt` sin clamping
- **Impacto:** Lurch del robot al liberarse de un obstaculo
- **Fix:** `constrain(pidIntegral, -MAX_PID_INTEGRAL, MAX_PID_INTEGRAL)` con `MAX_PID_INTEGRAL = 500.0f`

### M2: Safari fullscreen CSS
- **Archivo:** `src/UserControlGUI.cpp:49-51`
- **Raiz:** `:fullscreen` sin prefijo `:-webkit-full-screen`
- **Impacto:** Cards en fullscreen sin layout en Safari/iOS
- **Fix:** Duplicadas reglas con vendor prefix

### M3: E-STOP sin reset via WebSocket
- **Archivo:** `src/web_server.cpp:458`
- **Raiz:** WS handler solo procesaba PING, MV:, RUTA:, ESTOP
- **Impacto:** Usuario web bloqueado en E-STOP (solo serial podia resetear)
- **Fix:** Agregado handler `RESET` que limpia cola circular

### M4: Telemetria sobrescribe estado MANUAL
- **Archivo:** `src/UserControlGUI.cpp:676`
- **Raiz:** `applyTelemetry()` actualizaba FSM incluso durante control manual
- **Impacto:** UI flickering entre MANUAL y ADVANCING con joystick
- **Fix:** Early return si `S.fsm === 'MANUAL'`

### L1: strtok no reentrante
- **Archivo:** `src/web_server.cpp:484,525`
- **Fix:** `strtok()` -> `strtok_r()` con `saveptr` local

### L5: Security headers ausentes
- **Archivo:** `src/web_server.cpp:119`
- **Fix:** `X-Content-Type-Options: nosniff` + `X-Frame-Options: SAMEORIGIN` en `/status`

### L7: const cast en buffer WS
- **Archivo:** `src/web_server.cpp:456`
- **Fix:** `char* msg = (char*)data` -> `const char* msg = (const char*)data`

---

## CHECK (Verificar)

```
$ pio run
RAM:   14.9% (48804 / 327680 bytes)
Flash: 28.3% (946449 / 3342336 bytes)
Estado: SUCCESS
```

### Validacion funcional
- [x] Canvas trayectoria: 1 tap = 1 waypoint en moviles
- [x] WebSocket: solo 1 reconexion pendiente a la vez
- [x] PID: integral clamp entre [-500, 500]
- [x] Fullscreen: funciona en Chrome + Safari
- [x] E-STOP: reseteable desde Web UI (comando RESET)
- [x] Joystick: sin flickering de estado FSM
- [x] RUTA parsing: usa strtok_r (thread-safe)
- [x] /status: incluye security headers

### Defectos encontrados vs corregidos
| Categoria | Encontrados | Corregidos |
|-----------|-------------|------------|
| Criticos | 2 | 2 |
| Medios | 4 | 4 |
| Bajos | 3 | 3 |

---

## ACT (Estandarizar)

### Lecciones aprendidas
1. **Eventos touch+mouse** en canvas — siempre agregar guardia de 300ms para evitar doble disparo
2. **Reconexion WebSocket** — usar flag de guardia; `setTimeout` sin proteccion = stacking
3. **PID en embedded** — siempre clamp el termino integral; el windup es silencioso hasta que no lo es
4. **CSS sin prefijos** — probar en Safari; `:fullscreen` != `:-webkit-full-screen`
5. **E-STOP latching** — si la FSM tiene estado de emergencia, asegurar ruta de salida en TODOS los canales de comando
6. **Telemetria vs control manual** — la telemetria NUNCA debe sobrescribir comandos locales del usuario

### Reglas para futuro
- Todo event listener en canvas debe incluir guardia anti-doble-disparo touch/mouse
- Toda reconexion de red debe usar flag de guardia; patron: `if(!reconnecting){flag=true; setTimeout(()=>{flag=false; reconnect()}, delay)}`
- `pidIntegral` debe tener clamping en todo PID de sistema fisico
- Verificar vendor prefixes en Safari antes de declarar CSS completo
- Nuevos comandos WS: verificar que todos los estados FSM tengan ruta de entrada Y salida

### Archivos modificados
| Archivo | Cambios |
|---------|---------|
| `src/UserControlGUI.cpp` | C1, C2, M2, M4 |
| `src/main.cpp` | M1 |
| `src/web_server.cpp` | M3, L1, L5, L7 |
| `include/config.h` | M1 (MAX_PID_INTEGRAL) |
| `AUDITORIA.md` | Fase 2 documentada |
| `IMPLEMENTACION.md` | Fase 9 agregada |
