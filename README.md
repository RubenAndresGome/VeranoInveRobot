# Robot Autonomo ESP32-S3

Carro robot autonomo con ESP32-S3, driver L298N, encoders LM393, IMU MPU6050 y panel de control web embebido.

## Hardware

| Componente | Modelo | Funcion |
|-----------|--------|---------|
| Microcontrolador | ESP32-S3-DevKitC-1 | FSM principal, WiFi AP, servidor web |
| Driver motor | L298N | Puente H para 2 motores DC |
| Encoders | LM393 (infrarrojo) | Odometria (disco 20 ranuras) |
| IMU | MPU6050 (I2C) | Giroscopio + acelerometro 6DoF |
| Bateria | LiPo 12V | Alimentacion via L298N (5V regulado a ESP32) |
| LEDs estado | 3x 5mm | IDLE (verde), MOVING (azul), ERROR (rojo) |
| E-STOP | Boton NA + 10k pulldown | Parada de emergencia (GPIO41) |

## Pin Mapping

| Funcion | GPIO | Notas |
|---------|------|-------|
| ENA (PWM Motor Izq) | 15 | LEDC canal 0, 40kHz, 8-bit |
| IN1 / IN2 (Dir Motor Izq) | 16 / 17 | HIGH+LOW = avance |
| ENB (PWM Motor Der) | 18 | LEDC canal 1 |
| IN3 / IN4 (Dir Motor Der) | 8 / 9 | |
| Encoder Izq / Der | 4 / 5 | INPUT_PULLUP, ISR RISING |
| SDA / SCL (MPU6050) | 6 / 7 | I2C 400kHz |
| LED IDLE / MOVING / ERROR | 38 / 39 / 40 | OUTPUT, 220ohm |
| E-STOP | 41 | INPUT_PULLDOWN |

## Estructura del Proyecto

```
├── src/               # Codigo fuente principal
│   ├── main.cpp       # FSM, motor driver, IMU, odometria
│   ├── web_server.cpp # HTTP/WS async, captive portal, DNS
│   ├── UserControlGUI.cpp # Panel web embebido (HTML+CSS+JS)
│   ├── comandos.cpp   # Segmentacion de movimientos
│   ├── config_manager.cpp # Configuracion persistente NVS
│   └── hal_pins.cpp   # Inicializacion GPIO
├── include/           # Headers
│   ├── config.h       # Constantes fisicas y de control
│   ├── credentials_template.h # Template de credenciales WiFi
│   └── ...
├── frontend/          # UI modular (HTML+CSS+JS)
├── test/              # Tests unitarios (Unity)
├── resources_data/    # Documentacion de referencia
├── docs/              # Documentacion del sistema
└── .mde_history/      # Historial de auditorias PDCA
```

## Compilacion

### Requisitos

- [PlatformIO](https://platformio.org/) (VS Code extension o CLI)
- ESP32-S3-DevKitC-1 conectado via USB

### Setup inicial

```bash
# 1. Clonar el repositorio
git clone <repo-url>
cd VeranoInve

# 2. Configurar credenciales WiFi
cp include/credentials_template.h include/credentials.h
# Editar include/credentials.h con tu SSID y password

# 3. Compilar
pio run

# 4. Subir al ESP32
pio run --target upload

# 5. Monitor serial
pio device monitor
```

### Credenciales WiFi

El archivo `include/credentials.h` contiene SSID y password del Access Point. **NUNCA se sube a git** (esta en `.gitignore`).

Para crear el tuyo:
```bash
cp include/credentials_template.h include/credentials.h
nano include/credentials.h  # edita WIFI_SSID y WIFI_PASSWORD
```

## Conexion al Robot

1. Encender el robot (esperar ~10s)
2. Conectarse a la red WiFi `ROBOT-AUTONOMO-S3`
3. El navegador se abre automaticamente (captive portal)
4. Si no: abrir `http://192.168.4.1` o `http://robot.local`

## Panel de Control

Accesible en `http://192.168.4.1`:

- **Joystick vectorial**: Control manual con feedback visual instantaneo
- **Canvas de trayectoria**: Dibuja waypoints y envia rutas al robot
- **PWM sliders**: Ajuste independiente de velocidad por motor
- **Keyboard shortcuts**: WASD + Space (E-STOP)
- **Motor visualization**: Animacion de rotores sincronizada con PWM real
- **Chasis SVG**: Vista superior con componentes
- **Logs**: Sistema de logs categorizados con export TXT

## API

| Metodo | Ruta | Descripcion |
|--------|------|-------------|
| `GET` | `/` | Panel de control |
| `GET` | `/status` | Telemetria JSON |
| `POST` | `/cmd` | Comandos (avance, giro, parada) |
| `POST` | `/api` | Comandos JSON (rutinas, config) |
| `POST` | `/ruta` | Ruta programada (waypoints) |
| `POST` | `/raw` | Comandos estilo G-code |
| `GET` | `/network` | Info de red |
| `WS` | `/ws` | WebSocket (telemetria push 10Hz + comandos) |

### Comandos WebSocket

```
MV:F,150   Movimiento: Forward @ PWM 150
MV:B,200   Retroceso @ PWM 200
MV:L,130   Giro izquierda @ PWM 130
MV:R,130   Giro derecha @ PWM 130
MV:S,0     Parada
ESTOP      Parada de emergencia
RESET      Salir de E-STOP
RUTA:X1.0Y2.0;X3.0Y4.0  Ruta por waypoints absolutos
PING       Keep-alive
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

## Arquitectura

```
┌──────────┐   ┌──────────┐   ┌──────────────┐
│  MPU6050 │   │ Encoders │   │   E-STOP      │
│   I2C    │   │  GPIO    │   │   GPIO41 ISR  │
└────┬─────┘   └────┬─────┘   └──────┬────────┘
     │              │                │
     └──────────────┼────────────────┘
                    │
             ┌──────▼──────┐
             │   ESP32-S3  │
             │  FSM (7 est)│
             │  PID + Odometria
             │  WiFi AP    │
             │  HTTP/WS :80│
             └──────┬──────┘
                    │
              ┌─────▼─────┐
              │   L298N   │
              │ OUT1-OUT4 │
              └─────┬─────┘
                    │
            ┌───────┴───────┐
            │               │
       ┌────▼────┐    ┌────▼────┐
       │ Motor I │    │ Motor D │
       └─────────┘    └─────────┘
```

La FSM tiene 7 estados: INIT → CALIBRATING → IDLE → ADVANCING/TURNING/MANUAL → BRAKING → ESTOP. Cada tick de loop es un fragmento de trabajo no-bloqueante (< 5ms).

## Documentacion

- [SISTEMA.md](SISTEMA.md) — Documentacion tecnica completa
- [AUDITORIA.md](AUDITORIA.md) — Historial de auditorias MDE (66 hallazgos, 39 corregidos)
- [IMPLEMENTACION.md](IMPLEMENTACION.md) — Registro de cambios por fase PDCA
- `.mde_history/` — Archivos PDCA con planes, tareas y verificaciones

## Licencia

Este proyecto esta bajo **Rerum Novarum v1.0** — licencia personalizada basada en GPL 3.0 con:
- **Diezmo comercial** (1%-10% escalado por ingresos)
- **Ciclo Septenal** (modificaciones mayores/medias publicas cada 7 anios)
- **Compliance progresivo** (garantia y soporte al superar 4 salarios minimos)

Ver [LICENSE.md](LICENSE.md) para el texto completo.

---

## Metodologia MDE (Model-Driven Engineering)

El proyecto usa un sistema de agentes filosoficos para auditoria y control de calidad:

| Agente | Skill | Funcion |
|--------|-------|---------|
| **Supervisor Metadialectico** | `.opencode/skills/supervisor-metadialectico.md` | No-Contradiccion, 5S, Mayeutica, Silogismos |
| **Ockham Razor** | `.opencode/skills/ockham-razor.md` | Hilemorfismo + Polimorfismo tomista con grafo de conocimiento |
| **Grafo Dependencias** | `.opencode/skills/grafo-dependencias.md` | Congruencia codigo vs knowledge graph |
| **Concilio de Salamanca** | `.opencode/skills/concilio-salamanca/` | Tribunal de 39 agentes IA con debate escolastico |
| **MDE Frontend** | `.opencode/skills/mde-frontend/` | Validacion KPI, componentes UI, paletas de color |

### Herramienta: codebase-memory-mcp

```bash
# Instalar (una vez)
curl -fsSL https://raw.githubusercontent.com/DeusData/codebase-memory-mcp/main/install.sh | bash

# Indexar el proyecto
codebase-memory-mcp cli index_repository '{"repo_path": "."}'
```

### Pipeline MDE

```
Supervisor (no-contradiccion, 5S)
  → Ockham Razor (hilemorfismo + analogia entis con CBMM)
    → Concilio de Salamanca (debate multi-agente)
      → Grafo Dependencias (congruencia)
        → Build (pio run)
```

Historial completo de auditorias en `.mde_history/` (16 sesiones PDCA, 66 hallazgos, 39 corregidos).
