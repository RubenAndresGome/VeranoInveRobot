# PDCA 010: Auditoria MDE Fase 1 — Defectos Estructurales

**Fecha:** 2026-06-27 | **Estado:** COMPLETADO | **Build:** SUCCESS

---

## Objetivo

Auditar el proyecto con metodologia Model-Driven Engineering buscando bugs, documentacion desactualizada y desviaciones de la arquitectura declarada.

---

## Plan (Objetivo) (Objetivo)

### Alcance
- `src/UserControlGUI.cpp` — UI embebida (HTML/CSS/JS)
- `src/web_server.cpp` — Servidor HTTP/WS
- `include/web_server.h` — Declaraciones y documentacion
- `SISTEMA.md` — Documentacion de sistema
- `IMPLEMENTACION.md` — Tracking de cambios

### Criterios de inspeccion
1. Correccion funcional (bugs)
2. Consistencia entre documentacion e implementacion
3. Codigo muerto o no utilizado
4. Convenciones de estilo

---

## Do (Implementacion) (Implementacion)

### A1: cardLog() no acumulaba entradas
- **Archivo:** `src/UserControlGUI.cpp:324`
- **Raiz:** `el.innerHTML = '...'` (asignacion) en vez de `+=` (concatenacion)
- **Impacto:** Logs locales mostraban solo 1 linea (la ultima)
- **Fix:** `createElement('div')` + `appendChild()` + limpieza circular (max 20)

### A2: S.ws nunca asignada
- **Archivo:** `src/UserControlGUI.cpp:310`
- **Raiz:** `S.ws=null` en objeto de estado vs `ws = new WebSocket(...)` global implicito
- **Impacto:** `S.ws` siempre null; codigo futuro podria fallar al consultarlo
- **Fix:** Eliminado `ws:null` de `S`, declarado `let ws=null` explicito

### A3: Emojis en strings UI
- **Archivo:** `src/UserControlGUI.cpp:98,128,134`
- **Raiz:** `🎮 JOYSTICK...`, `⚠ E-STOP...`, `🗺️ PLANIFICADOR...`
- **Impacto:** Renderizado inconsistente en fuentes monospace
- **Fix:** Reemplazados por `[JOYSTICK]`, `E-STOP INMEDIATO`, `[PLANIFICADOR]`

### A4: web_server.h documentacion obsoleta
- **Archivo:** `include/web_server.h:5-18`
- **Raiz:** Decia "Tailwind CSS CDN", "servidor sincrono", "SSE"
- **Realidad:** CSS inline zero-deps, AsyncWebServer, WebSocket primario
- **Fix:** Bloque de comentarios actualizado

### A5: SISTEMA.md sin WebSocket
- **Archivo:** `SISTEMA.md`
- **Raiz:** Documentacion no mencionaba el canal WebSocket
- **Fix:** Agregada seccion "Via WebSocket" con protocolo, tabla de componentes actualizada

### A6-A8: Documentacion de resultados
- Creado `AUDITORIA.md` con informe completo
- Actualizado `IMPLEMENTACION.md` con referencia a auditoria

---

## CHECK (Verificar)

```
$ pio run
RAM:   14.9% (48804 / 327680 bytes)
Flash: 28.3% (945741 / 3342336 bytes)
Estado: SUCCESS
```

### Validacion funcional
- [x] cardLog muestra hasta 20 lineas historicas por card
- [x] Variable `ws` declarada explicitamente, sin shadowing
- [x] UI sin emojis Unicode no funcionales
- [x] Header docs reflejan AsyncWebServer + WebSocket + CSS propio
- [x] SISTEMA.md documenta WebSocket como canal primario

### Defectos encontrados vs corregidos
| Categoria | Encontrados | Corregidos |
|-----------|-------------|------------|
| Criticos | 1 | 1 |
| Medios | 2 | 2 |
| Bajos | 5 | 3 |

---

## ACT (Estandarizar)

### Lecciones aprendidas
1. **innerHTML += es fragil** en logs — usar `createElement` + `appendChild` para append seguro
2. **Variables globales implicitias** (sin `let`/`const`) son fuente de bugs silenciosos
3. **Documentacion de headers** se desincroniza facilmente — auditar tras cada refactor mayor
4. **Emojis en UI embebida** — prohibir; usar ASCII art o texto estructurado

### Reglas para futuro
- Todo log acumulativo debe usar DOM API (createElement), no innerHTML
- Toda variable compartida entre modulos JS debe declararse con `let`/`const` en scope global
- Tras cambios de arquitectura, auditar documentacion de headers y SISTEMA.md
- Prohibido el uso de emojis en strings de UI; usar `[ETIQUETA]` como convencion

### Archivos modificados
| Archivo | Cambios |
|---------|---------|
| `src/UserControlGUI.cpp` | cardLog fix, S.ws cleanup, emoji removal |
| `include/web_server.h` | Documentacion actualizada |
| `SISTEMA.md` | Seccion WebSocket agregada |
| `IMPLEMENTACION.md` | Referencia a auditoria |
| `AUDITORIA.md` | Nuevo: informe completo de auditoria |
