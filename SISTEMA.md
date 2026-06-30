# Sistema de Control — Carro Robot Autonomo ESP32-S3

> Documentacion tecnica del firmware de control para robot movil con ESP32-S3,
> driver L298N, encoder LM393, IMU MPU6050 y control via web embebida.

---

## 1. Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                        BATERIA LiPo 12V                         │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                    ┌───────┴───────┐
                    │   L298N       │
                    │ Driver Motor  │── 5V regulado ──> ESP32-S3 VIN
                    │               │── OUT1/OUT2 ──> Motor Izquierdo
                    │               │── OUT3/OUT4 ──> Motor Derecho
                    └───────┬───────┘
                            │
            ┌───────────────┼───────────────┐
            │ENA/IN1-IN4    │               │
            │(GPIO 15-18,   │               │
            │ 8, 9)         │               │
            ▼               ▼               ▼
    ┌───────────────┐ ┌───────────┐ ┌───────────────┐
    │   ESP32-S3    │ │ MPU6050   │ │ Encoders      │
    │               │ │ IMU I2C   │ │ LM393         │
    │ WebServer:80  │ │ GPIO6/7   │ │ GPIO4/5       │
    │ FSM principal │ │           │ │               │
    │ PID control   │ │           │ │               │
    └───────┬───────┘ └───────────┘ └───────────────┘
            │
            ├── GPIO38 ── LED IDLE (Verde)
            ├── GPIO39 ── LED MOVING (Azul)
            ├── GPIO40 ── LED ERROR (Rojo)
            └── GPIO41 ── Boton E-STOP (NA -> 3.3V)
```

### Componentes de software

| Modulo | Archivo | Funcion |
|--------|---------|---------|
| **FSM** | `main.cpp` | Maquina de estados de 7 estados, non-blocking |
| **HAL** | `hal_pins.cpp/h` | Abstraccion de pines GPIO (migrable a otros MCU) |
| **Web** | `web_server.cpp/h` | Servidor HTTP + captive portal + DNS + mDNS |
| **UI** | `UserControlGUI.cpp` | Panel de control web (HTML+CSS+JS embebido) |
| **Cmd** | `comandos.cpp/h` | Segmentacion automatica de comandos de movimiento |
| **Config** | `config_manager.cpp/h` | Configuracion persistente via NVS (Preferences) |
| **Debug** | `debug_logger.h` | Log condicional (activable en build flags) |
| **WS** | `web_server.cpp` | WebSocket bidireccional (push 10Hz + comandos en tiempo real) |

---

## 2. Red WiFi y Acceso

### Modo Access Point Puro

El ESP32 **siempre genera su propia red WiFi**. No depende de routers externos.

| Parametro | Valor |
|-----------|-------|
| SSID | `ROBOT-AUTONOMO-S3` |
| Password | `(definida en credentials.h)` |
| IP fija | `192.168.4.1` |
| mDNS | `http://robot.local/` |
| Puerto | `80` (HTTP) |

### Como conectar

```
1. Encender el robot (esperar ~10s a que arranque)
2. Telefono/PC: WiFi → buscar red "ROBOT-AUTONOMO-S3"
3. Conectar con la password definida en `credentials.h`
4. El telefono ABRE AUTOMATICAMENTE el navegador con el panel de control
   (Esto funciona por el captive portal integrado)
5. Si no abre automaticamente:
   - Abrir Chrome/Safari y escribir: http://192.168.4.1
   - O: http://robot.local
```

### Captive Portal (compatibilidad iOS/Android/Windows)

El ESP32 intercepta las verificaciones de conectividad de los sistemas operativos:

| Sistema | Ruta que consulta | Respuesta |
|---------|-------------------|-----------|
| iOS | `captive.apple.com/hotspot-detect.html` | Pagina con `<meta refresh>` a `/` |
| Android | `connectivitycheck.gstatic.com/generate_204` | `204 No Content` |
| Windows | `ncsi.txt` | `"Microsoft NCSI"` texto plano |
| Cualquier dominio | DNS wildcard resuelve a `192.168.4.1` | Pagina de control |

### Endpoint de red

```
GET /network
→ Pagina HTML con: SSID, IP, mDNS, MAC
→ Util para debug sin monitor serial
```

---

## 3. Protocolo de Comandos

### Via Web UI (recomendado)

El panel web (`http://192.168.4.1`) tiene:

| Control | Descripcion |
|---------|-------------|
| **Avanzar / Retroceder** | Avance/retroceso de 50cm a velocidad configurable |
| **Girar izq/der** | Giro de 90° |
| **Stop** | Parada inmediata |
| **E-STOP** | Emergencia: detiene todo, requiere reset manual |
| **Slider velocidad** | PWM 50-255 |
| **Laberinto X/Y** | Navegacion por coordenadas con waypoints |
| **Rutinas JSON** | Secuencias programables (cuadrado, zigzag, etc.) |
| **Consola raw** | Comandos serial directos estilo G-code |

### Via HTTP API

#### Endpoints REST

| Metodo | Ruta | Body | Accion |
|--------|------|------|--------|
| `POST` | `/cmd` | `tipo=avance&valor1=100&valor2=150` | Avanzar 100cm a PWM 150 |
| `POST` | `/cmd` | `tipo=giro&valor1=90&valor2=120` | Girar 90° a PWM 120 |
| `POST` | `/cmd` | `tipo=estop` | Parada de emergencia |
| `POST` | `/cmd` | `tipo=parada` | Parada suave |
| `POST` | `/cmd` | `tipo=config&valor1=50&valor2=80&extra1=400&extra2=2` | Configurar segmentacion |
| `POST` | `/raw` | `cmd=A100,200` | Comando raw estilo G-code |
| `GET` | `/status` | - | JSON con telemetria completa |
| `POST` | `/api` | `{"tipo":"rutina","pasos":[...]}` | Rutina JSON |
| `GET` | `/network` | - | Info de red en HTML |

#### Comandos Raw (protocolo G-code)

```
A<dist>,<vel>  → Avanzar       ej: A100,200  (100cm a PWM 200)
G<ang>,<vel>   → Girar         ej: G90,150   (90° a PWM 150)
P              → Parar
E              → E-STOP (emergencia)
R              → Reset desde E-Stop
?              → Estado actual
```

### Via WebSocket (recomendado para rendimiento)

| Protocolo | Endpoint | Direccion |
|-----------|----------|-----------|
| WebSocket | `ws://192.168.4.1/ws` | Bidireccional |

**Telemetria push (servidor -> cliente):** JSON a 10Hz con formato:
```json
{"tipo":"telem","distancia":45.3,"angulo":0.5,"pulsosIzq":142,...}
```

**Comandos (cliente -> servidor):** Strings planos sin JSON:
```
MV:F,150         Movimiento manual: Forward @ PWM 150
MV:B,200         Retroceso @ PWM 200
MV:L,130         Giro izquierda @ PWM 130
MV:R,130         Giro derecha @ PWM 130
MV:S,0           Parada
ESTOP             Parada de emergencia
PING              Keep-alive (responde {"tipo":"pong"})
RUTA:X10.0Y20.0;X30.0Y40.0   Ruta por waypoints absolutos
```

**Fallback HTTP:** Si el WebSocket se cae, el frontend hace polling a `/status` cada 500ms y envia comandos via `POST /cmd`.

### Via Serial (115200 baud)

Mismos comandos que raw, enviados por puerto serie:

```
A100,200  → Avanzar 100cm
G90,150   → Girar 90°
P         → Parar
E         → E-Stop
R         → Reset
?         → Estado
```

---

## 4. Maquina de Estados (FSM)

```
         ┌─────────┐
         │  INIT   │
         └────┬────┘
              │
         ┌────▼────────┐
         │ CALIBRATING │  ← Calibracion IMU (500 muestras, ~2s)
         └────┬────────┘
              │
         ┌────▼───┐
    ┌───│  IDLE   │◄──────────┐
    │   └────┬────┘           │
    │        │                 │
    │   ┌────▼──────┐    ┌────▼──────┐
    │   │ ADVANCING │    │  TURNING  │
    │   └────┬──────┘    └────┬──────┘
    │        │                 │
    │   ┌────▼──────┐    ┌────▼──────┐
    │   │  BRAKING  │◄───┘           │
    │   └────┬──────┘                 │
    │        │                        │
    └────────┘                        │
                                      │
    ┌──────────┐                      │
    │  E-STOP  │◄─────────────────────┘
    └────┬─────┘        (cualquier estado + boton E-Stop)
         │
         │ Solo con comando 'R'
         └──> IDLE
```

### Estados

| Estado | Descripcion | Duracion tipica |
|--------|-------------|-----------------|
| `INIT` | Arranque inicial, configuracion de pines | ~500ms |
| `CALIBRATING` | Calibracion del giroscopio MPU6050 | ~2s (500 muestras) |
| `IDLE` | Esperando comandos. Procesa cola web y serial | Indefinido |
| `ADVANCING` | Avance recto con PID diferencial + desaceleracion | Segun distancia |
| `TURNING` | Giro sobre el eje con reduccion de velocidad al final | Segun angulo |
| `BRAKING` | Pausa entre movimientos (absorbe inercia) | 200ms (400ms si segmentado) |
| `ESTOP` | Emergencia: motores detenidos, solo reset con 'R' | Indefinido |

---

## 5. Telemetria

### Canal principal: WebSocket (push 10Hz)

El servidor envia telemetria JSON a todos los clientes WebSocket cada 100ms.
El frontend recibe los datos via `ws.onmessage` y actualiza la UI en tiempo real.

### Canal fallback: HTTP `/status` (polling 500ms)

```json
{
  "clientes": 1,
  "connected": true,
  "telemetria": {
    "distancia": 45.3,
    "angulo": 0.5,
    "pulsosIzq": 142,
    "pulsosDer": 138,
    "pwmIzq": 150,
    "pwmDer": 150,
    "estado": "ADVANCING",
    "posX": 23.4,
    "posY": -1.2,
    "orientacion": 0.02
  }
}
```

| Campo | Unidad | Descripcion |
|-------|--------|-------------|
| `distancia` | cm | Distancia recorrida en el movimiento actual |
| `angulo` | grados | Angulo acumulado desde inicio del giro |
| `pulsosIzq/Der` | pulsos | Conteo de encoder (disco 20 ranuras) |
| `pwmIzq/Der` | 0-255 | PWM actual aplicado a cada motor |
| `estado` | - | Estado actual de la FSM |
| `posX/Y` | cm | Odometria global (dead reckoning) |
| `orientacion` | radianes | Orientacion global acumulada |

---

## 6. Parametros de Control

### PID Diferencial

| Parametro | Valor | Efecto |
|-----------|-------|--------|
| `Kp` | 1.0 | Correccion proporcional por diferencia de encoder |
| `Ki` | 0.1 | Correccion integral (elimina error acumulado) |
| `Kd` | 0.0 | Derivativa desactivada |

### Segmentacion de movimiento

| Parametro | Default | Descripcion |
|-----------|---------|-------------|
| `SEGMENTO_MAX_CM` | 50cm | Distancia maxima por segmento |
| `FACTOR_VELOCIDAD_SEGMENTO` | 80% | Reduccion de velocidad en segmentos |
| `TIEMPO_PAUSA_SEGMENTO_MS` | 400ms | Pausa entre segmentos |
| `MARGEN_ERROR_CM` | 2cm | Tolerancia de llegada |

### Seguridad

| Mecanismo | Descripcion |
|-----------|-------------|
| **E-Stop fisico** | GPIO41, ISR en flanco RISING + polling redundante. Latching hasta reset manual |
| **E-Stop por software** | Comando `E` o boton E-STOP en web |
| **Timeouts** | Avance: estimado+5s max. Giro: 10s max. Frenado: 2s max |
| **Desaceleracion** | Rampa PWM desde 15cm antes del objetivo, minimo 25% |

---

## 7. Pin Mapping (HAL)

| Funcion | GPIO | Notas |
|---------|------|-------|
| **ENA** (PWM Motor Izq) | 15 | LEDC canal 0, 40kHz, 8-bit |
| **IN1** (Dir Motor Izq bit 0) | 16 | HIGH+LOW = avance |
| **IN2** (Dir Motor Izq bit 1) | 17 | |
| **ENB** (PWM Motor Der) | 18 | LEDC canal 1 |
| **IN3** (Dir Motor Der bit 0) | 8 | |
| **IN4** (Dir Motor Der bit 1) | 9 | Antes GPIO3 (strapping fix) |
| **Encoder Izq** (D0) | 4 | INPUT_PULLUP, ISR RISING |
| **Encoder Der** (D0) | 5 | INPUT_PULLUP, ISR RISING |
| **SDA** (MPU6050 I2C) | 6 | I2C a 400kHz |
| **SCL** (MPU6050 I2C) | 7 | |
| **LED IDLE** (Verde) | 38 | OUTPUT, 220Ω |
| **LED MOVING** (Azul) | 39 | OUTPUT, 220Ω |
| **LED ERROR** (Rojo) | 40 | OUTPUT, 220Ω |
| **E-STOP** (Boton NA) | 41 | INPUT_PULLDOWN, +10kΩ externo |

---

## 8. Estructura del Proyecto

```
VeranoInve/
├── src/
│   ├── main.cpp           ← FSM principal, setup/loop
│   ├── web_server.cpp     ← Servidor HTTP, captive portal, DNS
│   ├── UserControlGUI.cpp ← Panel web (HTML+CSS+JS embebido)
│   ├── comandos.cpp       ← Segmentacion de movimientos
│   ├── config_manager.cpp ← Configuracion NVS persistente
│   └── hal_pins.cpp       ← Inicializacion de pines GPIO
├── include/
│   ├── config.h           ← Parametros fisicos y de control
│   ├── hal_pins.h         ← Definicion de pines
│   ├── web_server.h       ← API del servidor web
│   ├── comandos.h         ← API de comandos
│   ├── config_manager.h   ← API de configuracion NVS
│   ├── UserControlGUI.h   ← API de interfaz web
│   └── debug_logger.h     ← Macros de log condicional
├── test/
│   └── test_segmentacion.cpp ← Tests unitarios de segmentacion
├── platformio.ini         ← Configuracion de compilacion
└── verano ensamble carro/
    ├── diagrama_ensamble.html  ← Diagrama de conexiones interactivo
    ├── diagrama_ensamble.js    ← Logica del diagrama
    └── diagrama_ensamble.css   ← Estilos del diagrama
```

---

## 9. Compilacion y Carga

```bash
# Compilar
pio run

# Compilar + subir + monitor serial
pio run --target upload --target monitor

# Tests unitarios (nativo, no requiere ESP32)
pio test -e native

# Debug verbose (activar prints de telemetria)
# Descomentar en platformio.ini: -DDEBUG_ENABLED=1
```

### Recursos

| Recurso | Uso | Total |
|---------|-----|-------|
| RAM | ~48KB (15%) | 320KB |
| Flash | ~895KB (27%) | 3.3MB |

---

## 10. Diagrama de Ensamble

El archivo `verano ensamble carro/diagrama_ensamble.html` es un visor EDA interactivo
que muestra el esquema de conexiones del robot:

- **3 modos de vista:** Cables fisicos, Buses+Labels, Esquema puro
- **Filtros:** PWR, GND, Senal
- **Resaltado interactivo:** Click en cable/bus/componente
- **Zoom + Pan**
- **0 dependencias externas** (CSS inline, fuentes con fallback local)

---

## 11. Flujo de arranque

```
1. setup()
   ├── Serial.begin(115200)
   ├── hal_pins_init()          ← GPIO, PWM, I2C
   ├── config_manager_init()    ← NVS (Preferences)
   ├── motorDetener()           ← Motores en seguro
   ├── attachInterrupt() x3     ← Encoders izq/der + E-Stop
   ├── inicializarIMU()         ← MPU6050 I2C
   ├── web_server_init()        ← AP + mDNS + DNS + HTTP
   └── tAnterior = micros()

2. loop() [cada ~1-5ms]
   1. Calcular dt
   2. leerIMU(dt)               ← Integracion + filtro complementario
   3. Verificar E-Stop (ISR + polling)
   4. Ejecutar tick del estado actual
   5. procesarComandoSerial()
   6. enviarTelemetriaSerial()
   7. web_server_loop()         ← DNS + HTTP + WebSocket push (10Hz)
   8. Medir duracion del loop
```

---

## 12. Resolucion de problemas

| Problema | Causa probable | Solucion |
|----------|---------------|----------|
| No aparece la red WiFi | ESP32 aun arrancando | Esperar 10-15s tras encender |
| Conectado pero no carga la pagina | El telefono uso datos moviles en vez del WiFi | Desactivar datos moviles, reconectar al WiFi del robot |
| Pagina en blanco | Cache del navegador | Forzar recarga: Ctrl+F5 o borrar cache |
| El robot no avanza recto | PID sin calibrar | Ajustar Kp/Ki en config.h segun superficie |
| Los giros no son precisos | Offset IMU mal calibrado | Verificar calibracion en monitor serial |
| E-Stop no responde | Boton mal conectado | Verificar GPIO41 con 10kΩ pull-down a GND |
| Error `NO_AP_FOUND` en serial | Normal en modo AP puro | Es esperado, el AP se crea igualmente |
