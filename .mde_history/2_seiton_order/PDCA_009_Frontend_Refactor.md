# PDCA - Refactorización de Frontend MDE

**Fecha:** 2026-06-27
**Objetivo:** Refactorizar la HMI (Human-Machine Interface) aplicando los axiomas de la skill `mde-frontend`, mitigar vulnerabilidades de inyección y establecer un flujo de trabajo modular.

## 1. PLAN (Planificar)
- **Problema:** El archivo `UserControlGUI.cpp` era un monolito de casi 900 líneas que contenía HTML, CSS y JS crudo en una sola variable, lo cual viola el principio de separación de responsabilidades (Hylemorphismum) y dificultaba el mantenimiento. Además, el script de auditoría `concilio-salamanca` reportó el anti-patrón **AP-001** (XSS a través del uso inseguro de `.innerHTML`).
- **Solución Propuesta:** 
  1. Aplicar la metodología **5S** (Seiton) para organizar y respaldar el monolito en `.mde_history/`.
  2. Separar el código en componentes lógicos dentro de un nuevo directorio `/frontend`.
  3. Sustituir la asignación al DOM `.innerHTML = ''` por una alternativa segura (`.replaceChildren()`).
  4. Crear un pipeline de compilación (script Node.js) que automatice la inyección del código frontend en el archivo `UserControlGUI.cpp`.

## 2. DO (Hacer)
- **Respaldo:** Se copió `UserControlGUI.cpp` a `.mde_history/UserControlGUI_monolito.cpp.bak`.
- **Estructura Creada (`/frontend`):**
  - `index.html`: Causa material (estructura DOM).
  - `css/styles.css`: Causa formal (estilos y variables atómicas).
  - `js/app.js`: Orquestador principal (Causa eficiente).
  - `js/websocket.js`: Aislamiento de las comunicaciones de red y telemetría.
  - `js/joystick.js`: Clase aislada para el input analógico.
  - `js/trajectory.js`: Clase aislada para el planificador Canvas.
  - `js/ui.js`: Controladores DOM y lógica FSM.
- **Sanitización:** En `js/ui.js` (función `limpiarLog`), se purgó el uso de `innerHTML` previniendo ataques de inyección XSS.
- **Automatización:** Se creó `scripts/build_gui.js` para leer, minificar/concatenar lógicamente e inyectar el código dentro del literal C++ `R"EOF(...)EOF"`.

## 3. CHECK (Verificar)
- Se ejecutó `node scripts/build_gui.js` verificando que el string de C++ se ensamblara sin romper la sintaxis esperada.
- **Compilación de Firmware:** Se ejecutó `platformio run`. El proceso finalizó en **[SUCCESS] (8.94 segundos)** confirmando que:
  - El código C++ generado es 100% válido.
  - El ESP32 es capaz de almacenar el nuevo frontend sin desbordar el consumo de RAM/Flash asignado (Flash consumida: 28.4%, RAM: 14.9%).

## 4. ACT (Actuar / Estandarizar)
- **Nuevo Flujo de Trabajo:** De ahora en adelante, las modificaciones a la interfaz no deben realizarse sobre el código en C++ (`src/UserControlGUI.cpp`). Cualquier alteración de diseño o comportamiento debe hacerse en `/frontend` y compilarse con el script de build.
- **Mejora Futura Identificada:** Se sugiere integrar el script `build_gui.js` como un **Pre-Action script (Advanced Scripting)** en el archivo `platformio.ini` para que PlatformIO ensamble el frontend automáticamente justo antes de compilar `UserControlGUI.cpp`, eliminando así el paso manual del pipeline.


<!-- Plan (Objetivo) Do (Implementacion) -->
