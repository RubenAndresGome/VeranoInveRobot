# PDCA 014: Preparacion para GitHub Publico

**Fecha:** 2026-06-27 | **Estado:** COMPLETADO | **Build:** SUCCESS

---

## Objetivo

Preparar el proyecto para publicacion en GitHub: seguridad de credenciales, .gitignore completo, README, licencia, sanitizacion de datos sensibles, y backup de UI actual.

---

## PLAN (Planificar)

### Acciones
1. Sistema de credenciales — mover WIFI_SSID/PASSWORD fuera de config.h
2. .gitignore ampliado — excluir build artifacts, credenciales, archivos personales
3. Sanitizar documentacion — eliminar passwords reales de docs
4. README.md — documentacion completa para colaboradores
5. LICENSE — GPL 2.0
6. PDCA_014 — este documento (en .mde_history)
7. UI snapshot — backup de UserControlGUI.cpp actual
8. Git init + verificacion

---

## DO (Hacer)

### F1: Sistema de credenciales
- Creado `include/credentials_template.h` — template publico con placeholders
- Creado `include/credentials.h` — valores reales (gitignored)
- `config.h` ahora incluye `credentials.h` en vez de definir WIFI_SSID/PASSWORD
- `config_manager.cpp` sigue usando WIFI_SSID/PASSWORD como fallback (los obtiene via cadena de includes)

### F2: .gitignore
- `include/credentials.h` — credenciales reales
- `compile_commands.json` — auto-generado con paths de usuario
- `*.bak`, `*~`, `__history/` — backups
- `scratch/` — contiene credenciales de otras redes
- `ProyectoDeAutomatizacion/` — otro proyecto con nombres de maquina
- `.agents/`, `.cache/`, `.opencode/`, `.codebase-memory/` — tools locales
- `*.rar`, `*.zip`, `workspace.code-workspace`

### F3: Sanitizacion
- `SISTEMA.md`: passwords reemplazadas por referencia a `credentials.h`
- `BUG_REPORT.md`: password ejemplo reemplazada por `<PASSWORD_ANTERIOR>`

### F4: README.md
- Descripcion del proyecto
- Tabla de hardware
- Pin mapping completo
- Estructura del proyecto
- Instrucciones de compilacion (PlatformIO)
- Configuracion de credenciales (credentials_template.h)
- Conexion al robot (WiFi + captive portal)
- Panel de control (features)
- API (REST + WebSocket + Serial)
- Arquitectura (diagrama ASCII)
- Documentacion relacionada
- Licencia

### F5: LICENSE
- GNU General Public License v2.0

### F6: PDCA_014
- Este documento

### F7: UI Snapshot
- `UserControlGUI.cpp` actual (813 lineas) documentado en este PDCA
- Incluye: WebSocket + fallback HTTP, fullscreen por card, logs locales, joystick vectorial, canvas trayectoria, motor animado, chasis SVG, keyboard shortcuts, PWM dual

---

## CHECK (Verificar)

```
$ pio run
RAM:   14.9% (48820 / 327680 bytes)
Flash: 28.1% (938429 / 3342336 bytes)
Estado: SUCCESS
```

### Verificaciones
- [x] `credentials.h` existe con valores reales
- [x] `credentials_template.h` tiene placeholders (seguro para git)
- [x] `config.h` ya no contiene WIFI_PASSWORD hardcodeado
- [x] `.gitignore` cubre credenciales, build, backups, scratch
- [x] `SISTEMA.md` y `BUG_REPORT.md` sanitizados
- [x] `README.md` completo con instrucciones para colaboradores
- [x] `LICENSE` (GPL 2.0) presente
- [x] Build exitoso con el nuevo sistema de includes

### Archivos nuevos
| Archivo | Proposito |
|---------|-----------|
| `include/credentials_template.h` | Template publico |
| `include/credentials.h` | Credenciales reales (gitignored) |
| `README.md` | Documentacion principal |
| `LICENSE` | GPL 2.0 |
| `resources_data/README.md` | Explicacion del directorio |
| `.mde_history/2_seiton_order/PDCA_014_GitHub_Prep.md` | Este PDCA |

### Archivos modificados
| Archivo | Cambio |
|---------|--------|
| `include/config.h` | `#include "credentials.h"` en vez de defines |
| `.gitignore` | 20+ patrones agregados |
| `SISTEMA.md` | Passwords sanitizadas |
| `BUG_REPORT.md` | Password ejemplo sanitizada |

### UI Snapshot
`UserControlGUI.cpp` — Version actual (813 lineas):
- WebSocket con reconexion automatica + fallback HTTP
- 6 cards modulares con fullscreen + logs locales
- Joystick vectorial (VirtualJoystick class)
- Canvas trayectoria (TrayectoriaCanvas class)
- Motores animados con CSS (spin-cw/ccw)
- Chasis SVG vista superior
- PWM dual sliders
- Keyboard shortcuts WASD
- E-STOP con broadcast WS
- Logs categorizados con export TXT

---

## ACT (Estandarizar)

### Reglas para GitHub
1. Credenciales SIEMPRE en archivo separado gitignored (patron `credentials.h`)
2. Template publico (`credentials_template.h`) documenta el formato esperado
3. `.gitignore` debe cubrir: build/, credenciales, backups, tooling local
4. README debe incluir: hardware, pinout, compilacion, credenciales, API, arquitectura
5. Documentacion sensible revisada antes de cada commit
6. Licencia explicita en raiz del repositorio

### Para el colaborador nuevo
```bash
git clone <url>
cp include/credentials_template.h include/credentials.h
# editar credentials.h
pio run --target upload
```
