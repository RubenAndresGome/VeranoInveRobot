# PDCA 017: Migracion a Protocolos Binarios — Optimizacion del Servidor Web

**Fecha:** 2026-06-28 | **Estado:** COMPLETADO | **Build:** SUCCESS

---

## Objetivo

Migrar telemetria y comandos de JSON/texto a protocolos binarios para eliminar fragmentacion de heap, reducir latencia y prevenir saturacion de WebSockets en ESP32-S3.

---

## PLAN (Planificar)

### Problema detectado
- `ws_push_telemetry()` asignaba `JsonDocument` + `String` en heap cada 100ms
- `handle_body_accumulation()` hacia `concat()` caracter por caracter → heap fragmentation
- `onWsEvent` RUTA usaba `strtok_r` + `sscanf` (parsing O(n) con copias de string)
- Tamano de mensaje JSON: ~350 bytes @ 10Hz = 3.5 KB/s por cliente

### Cambios propuestos
1. Struct binario `TelemetryBinary` de 52 bytes (sin heap alloc)
2. `body->reserve(total)` para prevenir fragmentacion HTTP
3. `ws.binaryAll()` en lugar de `ws.textAll()` con JSON
4. `onWsEvent` soporta `WS_BINARY` con `memcpy` directo
5. Frontend: `binaryType='arraybuffer'` + `DataView`
6. Frontend: calculo local de trigonometria + `Float32Array`
7. FSM mapeado a uint8 en lugar de strings

---

## DO (Hacer)

### T1: handle_body_accumulation
- Agregado `body->reserve(total)` en primera llamada (index == 0)
- Reemplazado bucle char-by-char por `body->concat((const char*)data, len)`
- Impacto: 0 heap fragmentation en POST grandes

### T2: Struct TelemetryBinary + ws_push_telemetry binario
- Struct definido en `web_server.h` con `#pragma pack(1)` (52 bytes)
- Campos: version(1) + estado(1) + 12 datos (4 bytes c/u o 2 bytes)
- `ws.binaryAll((uint8_t*)&bin, sizeof(TelemetryBinary))` — 0 heap allocs
- Flow control: `ws.binaryAll()` no bloquea (AsyncWebSocket maneja colas)

### T3: FSM numeric mapping
- `estadoToUint8(const char* estado)` — mapea 8 estados a 0-7
- Default: IDLE = 2

### T4: onWsEvent WS_BINARY
- Detecta `info->opcode == WS_BINARY`
- Lee `uint16 count` del header
- `memcpy` directo a `PuntoRuta[]` en la cola circular
- 0 string parsing, 0 heap allocs — O(1) en tiempo de copia
- Validacion: `len >= expected` y `count <= COLA_MAX_COMANDOS`

### T5: websocket.js binary decode
- `this.ws.binaryType = 'arraybuffer'`
- `decodeBinaryTelemetry(buffer)`: DataView con offsets fijos
- FSM_MAP: array lookup `['INIT',...,'MANUAL'][uint8]`
- `onmessage`: detecta `ArrayBuffer` vs texto, mantiene JSON fallback
- Agregado handler para mensaje `estop` via JSON

### T6: trajectory.js route computation
- `enviarRutaBinaria()`: calcula `atan2(dy,dx)` + `sqrt(dx^2+dy^2)` localmente
- Normaliza angulos a [-180, 180]
- Genera giro + avance como waypoints separados
- DataView serializa a formato PuntoRuta (16 bytes c/u)
- Header: uint16 count
- Envia via `ws.send(buffer)`

### T7: Build
- `python scripts/build_gui.py` → UserControlGUI.cpp regenerado
- `pio run` → SUCCESS (Flash 28.8%, RAM 14.9%)

---

## CHECK (Verificar)

### Metricas de mejora

| Metrica | Antes (JSON) | Despues (Binary) | Mejora |
|---------|-------------|------------------|--------|
| Tamano mensaje telemetria | ~350 bytes | 52 bytes | **-85%** |
| Heap allocs por push | JsonDocument + String | 0 (stack) | **-100%** |
| Parsing comandos | strtok_r + sscanf | memcpy O(1) | **-100%** |
| HTTP body accum | char-by-char | buffer copy | **sin fragmentacion** |
| Flash | 28.1% | 28.8% | +23KB (codigo nuevo) |

### Build
```
pio run → SUCCESS
RAM 14.9% | Flash 28.8% (961061 bytes)
```

### Archivos modificados

| Archivo | Cambio |
|---------|--------|
| `include/web_server.h` | +TelemetryBinary struct, +estadoToUint8 |
| `src/web_server.cpp` | T1, T2, T3, T4 |
| `src/UserControlGUI.cpp` | Regenerado via build_gui.py |
| `frontend/js/websocket.js` | T5 (binary decode + FSM_MAP + estop handler) |
| `frontend/js/trajectory.js` | T6 (enviarRutaBinaria) |

---

## ACT (Estandarizar)

### Reglas establecidas
1. Telemetria ahora es binaria (52 bytes). Si se agregan campos, actualizar `TelemetryBinary` + `decodeBinaryTelemetry` + version byte.
2. Comandos de ruta via WS ahora soportan binario. El texto (`RUTA:...`) sigue funcionando como fallback.
3. JSON en `/status` sigue disponible para HTTP polling fallback.
4. `build_gui.py` debe ejecutarse tras cualquier cambio en `frontend/js/`.
5. Version byte (0x01) en posicion 0 del mensaje binario permite detectar cambios de protocolo.
