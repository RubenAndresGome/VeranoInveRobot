# Sistema Actual — Robot Autonomo ESP32-S3

> Documentacion generada automaticamente desde el estado actual del codigo.
> Fecha: 2026-06-27 | Build: SUCCESS | RAM 14.9% | Flash 28.1%

---

## 1. Descripcion del Funcionamiento

El robot opera como **Access Point WiFi autonomo** (no depende de routers externos). Al encenderse:

1. **setup()** — Inicializa HAL, encoders, E-STOP ISR, IMU, config_manager NVS, y servidor web
2. **loop()** — Cada ~1-5ms ejecuta: leer IMU → verificar E-STOP → tick del estado FSM → procesar comandos serial → enviar telemetria → web_server_loop (WS push 10Hz + DNS)
3. **FSM** — Maquina de 8 estados no-bloqueante: INIT → CALIBRATING → IDLE → ADVANCING/TURNING/MANUAL → BRAKING → ESTOP
4. **Web** — Servidor async en puerto 80 con WebSocket en `/ws`, captive portal, DNS wildcard, mDNS
5. **UI** — Panel de control HTML+CSS+JS embebido sin dependencias externas

---

## 2. Arquitectura

```
┌──────────┐   ┌──────────┐   ┌──────────────┐
│  MPU6050 │   │ Encoders │   │   E-STOP      │
│   I2C    │   │  GPIO    │   │   GPIO41 ISR  │
└────┬─────┘   └────┬─────┘   └──────┬────────┘
     │              │                │
     └──────────────┼────────────────┘
                    │
             ┌──────▼──────┐     WiFi AP
             │   ESP32-S3  │◄─────────────── Telefono/PC
             │  FSM (8 est)│     http://192.168.4.1
             │  PID + Odom │
             │  HTTP/WS :80│──── WebSocket ────► Clientes WS
             └──────┬──────┘     push 10Hz JSON
                    │
              ┌─────▼─────┐
              │   L298N   │
              │ OUT1-OUT4 │
              └─────┬─────┘
                    │
            ┌───────┴───────┐
       ┌────▼────┐    ┌────▼────┐
       │ Motor I │    │ Motor D │
       └─────────┘    └─────────┘
```

---

## 3. Maquina de Estados (FSM)

```
         ┌─────────┐
         │  INIT   │
         └────┬────┘
              │
         ┌────▼────────┐
         │ CALIBRATING │  ← 500 muestras IMU (~2s)
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
    │   ┌────▼──────┐         │
    │   │  BRAKING  │◄────────┘
    │   └────┬──────┘
    │        │
    └────────┘
    
    ┌──────────┐                      
    │  E-STOP  │◄── ISR + polling + WS + serial
    └────┬─────┘                      
         │ RESET (serial 'R' o WS 'RESET')
         └──► IDLE

    ┌──────────┐
    │  MANUAL  │◄── Joystick / Teclado / WS MV:
    └────┬─────┘
         │ timeout 500ms sin comando
         └──► IDLE
```

---

## 4. API

### REST Endpoints

| Metodo | Ruta | Descripcion |
|--------|------|-------------|
| `GET` | `/` | Panel de control HTML |
| `GET` | `/status` | Telemetria JSON (mutex protegido) |
| `POST` | `/cmd` | Comandos URL-encoded |
| `POST` | `/api` | Comandos JSON (rutinas, config) |
| `POST` | `/ruta` | Ruta programada (waypoints) |
| `GET`/`POST` | `/raw` | Comandos estilo G-code |
| `GET` | `/network` | Info de red (SSID, IP, MAC) |
| `GET` | `/health` | Health check |
| `WS` | `/ws` | WebSocket bidireccional |

### WebSocket (canal principal)

```
Cliente → Servidor:
  MV:F,150        Movimiento Forward @ PWM 150
  MV:B,200        Retroceso @ PWM 200
  MV:L,130        Giro izquierda @ PWM 130
  MV:R,130        Giro derecha @ PWM 130
  MV:S,0          Parada
  ESTOP           Parada de emergencia
  RESET           Salir de E-STOP (limpia cola + flag)
  RUTA:X1Y2;X3Y4  Ruta por waypoints absolutos
  PING            Keep-alive (responde {"tipo":"pong"})

Servidor → Cliente:
  {"tipo":"telem","distancia":45.3,...}    Push 10Hz
  {"tipo":"estop"}                         Broadcast en emergencia
  {"tipo":"pong"}                          Respuesta PING

*Nota FSM Debug (PDCA 016)*: `FSM:<estado>` fuerza a la FSM principal a saltar al estado enviado (ej. `FSM:CALIBRATING`) bypassando transiciones seguras. Útil para depuración o desatorar el hardware.
```

### Comandos Serial (115200 baud)

```
A<dist>,<vel>   Avanzar       ej: A100,200
G<ang>,<vel>    Girar         ej: G90,150
P               Parar
E               E-STOP
R               Reset
?               Estado actual
```

---

## 5. Pin Mapping

| Funcion | GPIO | Notas |
|---------|------|-------|
| ENA (PWM Motor Izq) | 15 | LEDC canal 0, 40kHz |
| IN1 / IN2 | 16 / 17 | Direccion Motor Izq |
| ENB (PWM Motor Der) | 18 | LEDC canal 1 |
| IN3 / IN4 | 8 / 9 | Direccion Motor Der |
| Encoder Izq / Der | 4 / 5 | INPUT_PULLUP, ISR RISING |
| SDA / SCL (MPU6050) | 6 / 7 | I2C 400kHz |
| LED IDLE / MOVING / ERROR | 38 / 39 / 40 | OUTPUT |
| E-STOP | 41 | INPUT_PULLDOWN |

---

## 6. Estructura del Proyecto

```
VeranoInve/
├── src/
│   ├── main.cpp              # FSM, motor driver, IMU, odometria, PID
│   ├── web_server.cpp         # AsyncHTTP, WebSocket, captive portal, DNS
│   ├── UserControlGUI.cpp     # Panel web embebido (HTML+CSS+JS inline)
│   ├── comandos.cpp           # Segmentacion automatica de movimientos
│   ├── config_manager.cpp     # Configuracion persistente NVS
│   └── hal_pins.cpp           # Inicializacion GPIO
├── include/
│   ├── config.h               # Constantes fisicas y de control
│   ├── credentials_template.h # Template publico de credenciales WiFi
│   ├── credentials.h          # Credenciales reales (gitignored)
│   └── ... (headers)
├── frontend/                   # UI modular (HTML+CSS+JS separados)
├── .opencode/skills/
│   ├── supervisor-metadialectico.md  # Agente filosofico principal
│   └── ockham-razor.md              # Agente hilemorfico + graph queries
├── .mde_history/               # Historial PDCA (16 sesiones)
├── SISTEMA_ACTUAL.md           # Este documento
├── README.md                   # Documentacion principal
└── LICENSE.md                  # Rerum Novarum v1.0
```

---

## 7. Agentes del Sistema

### Supervisor Metadialectico
- **Archivo:** `.opencode/skills/supervisor-metadialectico.md`
- **Metodo:** No-Contradiccion + 4 Causas + Leibnizian Calculus + PDCA
- **Reglas:** 5S operativo, Mayeutica (dialogo socratico), Formato Silogistico Escolastico
- **Principios:** No-Contradiccion, Razon Suficiente, Hilemorfismo, Navaja de Ockham

### Ockham Razor
- **Archivo:** `.opencode/skills/ockham-razor.md`
- **Metodo:** Verificacion hilemorfica (Materia+Forma=Sustancia) via graph queries
- **Herramienta:** codebase-memory-mcp (search_graph, trace_path, query_graph, get_code_snippet)
- **Formato:** Veredicto escolastico (Quaestio → Videtur → Sed contra → Respondeo)

### Indexator Graphicus (Skill de Dependencias)
- **Archivo:** `.opencode/skills/grafo-dependencias.md`
- **Metodo:** Verificacion de acoplamiento fisico vs topologia teorica
- **Resultado Auditoria MDE (Fase 0):** `platformio.ini` coincide exactamente con los `includes` de la capa `src/`. No hay dependencias circulares detectadas entre el WebServer, FSM y Controladores Hardware. El Grafo de Conocimiento local ha sido trazado (mapeo del 100% de `#include`).

---

## 8. Frontend HTML

El panel de control esta embebido en `src/UserControlGUI.cpp` como string C++ raw (`R"EOF(...)EOF"`). A continuacion se presenta el HTML completo extraido:

### Estructura del HTML (813 lineas)

```html
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Autonomo S3</title>
    <style>
        /* ── CSS Design System (zero dependencias) ── */
        /* Variables: slate, cyan, blue, purple, red, amber, green */
        :root{--sl-900:#0f172a;--sl-800:#1e293b;...}
        body{margin:0;font-family:'Courier New',Consolas,monospace;...}
        .panel-tech{clip-path:polygon(...);border:1px solid var(--cy-700)}
        .panel-header{background:linear-gradient(90deg,var(--cy-700),transparent)}
        .btn-cmd{clip-path:polygon(6px 0,100% 0,calc(100%-6px) 100%,0 100%)}
        .btn-estop{animation:pulse-red 2s infinite}
        .motor-spin-cw{animation:spin-cw var(--motor-dur,2s) linear infinite}
        .card-content{flex:1;overflow-y:auto}
        .card-local-log{height:40px;overflow-y:auto}
        .fs-btn{font-size:10px;border:1px solid var(--sl-600)}
        :fullscreen .panel-tech{height:100vh;width:100vw}
    </style>
</head>
<body>

<!-- HEADER: Logo + IP + FSM badge -->
<header>
    <span>ROBOT-AUTONOMO-S3</span>
    <span id="hdr-ip">192.168.4.1</span>
    <span id="fsm-badge" class="fsm-badge">IDLE</span>
</header>

<main>
    <!-- FILA: TELEMETRIA (8 cards de datos) -->
    <section class="grid-8col">
        <div class="panel-tech">Estado: <span id="tel-estado">INIT</span></div>
        <div class="panel-tech">Distancia: <span id="tel-dist">0.0</span> cm</div>
        <div class="panel-tech">Angulo: <span id="tel-ang">0.0</span>°</div>
        <div class="panel-tech">Pos X: <span id="tel-posx">0.0</span></div>
        <div class="panel-tech">Pos Y: <span id="tel-posy">0.0</span></div>
        <div class="panel-tech">PID: <span id="tel-pid">0.0</span></div>
        <div class="panel-tech">Restante: <span id="tel-rest">0.0</span> cm</div>
        <div class="panel-tech">Target: <span id="tel-target">0.0</span> cm</div>
    </section>

    <!-- FILA: CONTROL + PWM -->
    <section class="grid-2col">
        <!-- CARD: Control Vectorial y Trayectoria -->
        <div class="panel-tech" id="card-control">
            <div class="panel-header">
                <span>// CONTROL VECTORIAL Y TRAYECTORIA</span>
                <button class="fs-btn" onclick="toggleFS('card-control')">FS</button>
            </div>
            <div class="card-content">
                <!-- Joystick vectorial (180x180 SVG + knob CSS) -->
                <div id="joystick-container">
                    <div id="joystick-knob"></div>
                </div>
                <!-- Canvas trayectoria dual-layer + zoom + fullscreen glass -->
                <canvas id="canvas-trayectoria"></canvas>
                <svg id="gyro-needle">...</svg>
            </div>
            <div class="card-local-log" id="log-control">></div>
        </div>

        <!-- CARD: PWM Control -->
        <div class="panel-tech" id="card-pwm">
            <div class="panel-header">
                <span>// PWM CONTROL [0-255]</span>
                <button class="fs-btn" onclick="toggleFS('card-pwm')">FS</button>
            </div>
            <div class="card-content">
                <input type="range" id="sl-pwm-i" class="slider-tech" oninput="setPWM('i',this.value)">
                <input type="range" id="sl-pwm-d" class="slider-tech" oninput="setPWM('d',this.value)">
            </div>
            <div class="card-local-log" id="log-pwm">></div>
        </div>
    </section>

    <!-- FILA: MOTORES + CHASIS -->
    <section class="grid-3col">
        <!-- CARD: Motor Izquierdo -->
        <div class="panel-tech" id="card-motor-i">
            <div class="panel-header">
                <span>// MOTOR IZQ — OUT1/OUT2</span>
                <span id="status-mi">STOP</span>
                <button class="fs-btn" onclick="toggleFS('card-motor-i')">FS</button>
            </div>
            <div class="card-content">
                <div id="rotor-i" class="motor-stop">
                    <div class="motor-ring"></div>
                    <div id="marker-i"></div>
                </div>
                <span>Encoder: <span id="enc-i">0</span></span>
            </div>
            <div class="card-local-log" id="log-motor-i">></div>
        </div>

        <!-- CARD: Chasis SVG -->
        <div class="panel-tech" id="card-chasis">
            <div class="panel-header">
                <span>// CHASIS — TOP VIEW</span>
                <button class="fs-btn" onclick="toggleFS('card-chasis')">FS</button>
            </div>
            <div class="card-content">
                <svg viewBox="0 0 160 200">
                    <rect x="35" y="30" width="90" height="140" rx="6"/>
                    <polygon points="80,10 72,28 88,28"/>
                    <rect id="ch-m1" x="18" y="50" width="20" height="40"/>
                    <rect id="ch-m2" x="122" y="50" width="20" height="40"/>
                    <rect x="62" y="90" width="36" height="18"/>
                    <rect x="55" y="120" width="50" height="30"/>
                </svg>
            </div>
            <div class="card-local-log" id="log-chasis">></div>
        </div>

        <!-- CARD: Motor Derecho -->
        <div class="panel-tech" id="card-motor-d">
            <div class="panel-header">
                <span>// MOTOR DER — OUT3/OUT4</span>
                <span id="status-md">STOP</span>
                <button class="fs-btn" onclick="toggleFS('card-motor-d')">FS</button>
            </div>
            <div class="card-content">
                <div id="rotor-d" class="motor-stop">
                    <div class="motor-ring"></div>
                    <div id="marker-d"></div>
                </div>
                <span>Encoder: <span id="enc-d">0</span></span>
            </div>
            <div class="card-local-log" id="log-motor-d">></div>
        </div>
    </section>

    <!-- FILA: LOGS + PIN MAP -->
    <section class="grid-2col">
        <div class="panel-tech" id="card-logs">
            <div class="panel-header">
                <span>// LOGS — REGISTRO DE EVENTOS</span>
                <button onclick="limpiarLog()">LIMPIAR</button>
                <button onclick="exportLog()">EXPORTAR</button>
            </div>
            <div id="log-container" class="log-container"></div>
            <div class="card-local-log" id="log-logs">></div>
        </div>

        <div class="panel-tech">
            <div class="panel-header"><span>// HAL — PIN MAP</span></div>
            <div>ENA: GPIO15 | IN1/IN2: GPIO16/17 | ENB: GPIO18 | IN3/IN4: GPIO8/9</div>
            <div>ENC I/D: GPIO4/5 | I2C: GPIO6/7 | LEDs: GPIO38-40 | E-STOP: GPIO41</div>
        </div>
    </section>
</main>

<script>
// ── ESTADO GLOBAL ──
const S={fsm:'IDLE',pwmI:150,pwmD:150,estI:'stop',estD:'stop',estop:false,logs:[]}

// ── FULLSCREEN ──
function toggleFS(id){
    const el=document.getElementById(id)
    if(!document.fullscreenElement){el.requestFullscreen().catch(()=>{})}
    else{document.exitFullscreen()}
}

// ── CARD LOGS (max 20 lineas historicas) ──
function cardLog(id,msg){
    const el=document.getElementById('log-'+id)
    if(!el)return
    const d=document.createElement('div')
    d.textContent='['+new Date().toTimeString().substring(0,8)+'] '+msg
    el.appendChild(d);el.scrollTop=el.scrollHeight
    while(el.children.length>20)el.removeChild(el.firstChild)
}

// ── LOGS CATEGORIZADOS ──
function log(tipo,msg){...}
function limpiarLog(){...}
function exportLog(){...}

// ── FSM (8 estados) ──
function updFSM(est){...}
function updMotor(lado,est,pwm){
    // Controla animacion CSS (spin-cw/ccw) + colores ring + marker
    // Velocidad de animacion ligada al PWM: --motor-dur
}
function setPWM(lado,val){...}

// ── VIRTUAL JOYSTICK (clase) ──
class VirtualJoystick {
    // Touch + mouse events en window
    // Calcula thrust (0-255) y angulo (-180 a 180)
    // Mapea a direcciones discretas: F/B/L/R
    // Envia comandos via WebSocket MV:dir,speed
}

// ── CANVAS TRAYECTORIA (clase) ──
class TrayectoriaCanvas {
    // Canvas 2D con rejilla, ejes, waypoints, robot SVG
    // Doble buffer en resize
    // Click/touch agrega waypoints (con guardia anti-doble-disparo)
}

// ── WEBSOCKET + HTTP FALLBACK ──
let ws=null, wsFail=0, wsReconnecting=false
function initWS(){
    // Conexion a ws://IP/ws
    // Reconexion automatica cada 3s (con guardia anti-stacking)
    // onmessage: parsea JSON telem y actualiza UI
}
function startPollFallback(){
    // HTTP /status cada 500ms como respaldo
}

// ── KEYBOARD SHORTCUTS ──
// WASD + Space (E-STOP)
// Keydown → WS MV:dir,pwm
// Keyup → WS MV:S,0

// ── INICIO ──
window.onload=()=>{
    initWS()
    new VirtualJoystick('joystick-container','joystick-knob', callback)
    new TrayectoriaCanvas('canvas-trayectoria')
    updFSM('INIT')
}
</script>
</body>
</html>
```

> **Nota:** El codigo HTML completo (813 lineas) se encuentra en `src/UserControlGUI.cpp`. La version modularizada (HTML+CSS+JS separados) esta en `frontend/`.

---

## 9. Sistema de Credenciales

Las credenciales WiFi se gestionan mediante el patron template + gitignore:

```
include/
├── credentials_template.h   ← PUBLICO (placeholders, se sube a git)
└── credentials.h            ← PRIVADO (valores reales, gitignored)
```

**Para un nuevo colaborador:**
```bash
cp include/credentials_template.h include/credentials.h
# Editar credentials.h con SSID y password reales
```

El `config_manager` (NVS) tambien soporta runtime override via API.

---

## 10. Licencia

**Rerum Novarum v1.0** — Basada en GPL 3.0 con clausulas comerciales:
- Diezmo 1%-10% escalado por ingresos comerciales
- Ciclo Septenal: versiones mayores/medias publicas cada 7 anios
- Compliance progresivo (garantia y soporte al superar 4 salarios minimos)

Ver `LICENSE.md` para el texto completo.
