# 🏛️ Arquitectura del Supervisor Metadialéctico Escolástico

## Verificación Formal de Código mediante Principio de No-Contradicción

### 1. Fundamento Ontológico

```
AXIOMA SUPREMO: Una proposición y su negación no pueden ser ambas verdaderas
al mismo tiempo y en el mismo sentido — Aristóteles, Metafísica IV, 1005b
```

Aplicado al código: **Dos fuentes no pueden definir valores diferentes para la misma constante, pin, o estado y ambas ser correctas.** Si el código fuente dice `PIN_IN4 = 9` y el diagrama dice `GPIO 3`, hay una contradicción que debe resolverse.

---

### 2. Concilio de Salamanca (Arquitectura Multi-Agente)

El sistema opera bajo un modelo jerárquico Director/Ejecutor (Cognitive Mixture of Agents) para optimizar eficiencia y capacidad de razonamiento:

- **Magister Determinans (Director / MDE)**: Orquestador cognitivo principal. Encargado de la síntesis judicial final. Integrado nativamente con `open-design` y `spec-kit` para orquestar la coherencia de diseño de UI escalable y proveer la interfaz hacia agentes de diseño especializados.
- **Magister Processus Integri (Inquisidor)**: Supervisor de la metodología PDCA/Scrum. Garantiza que toda tarea se registre y justifique mediante el formato de rastreo JSON/Markdown de bajo consumo de tokens (ej. `_numero_plan_PDCA_`).
- **Arquímedes (Inquisidor)**: Guardián de Clean Code y principios SOLID.
- **Custos Impacti (Inquisidor)**: Analista del impacto de refactorización y deuda arquitectural.

*Traducción Trans-Skill*: La comunicación entre agentes distribuidos se apoya en silogismos lógicos, compresión de predicados y álgebra booleana para preservar la integridad arquitectural en todo momento.

---

### 2. Procedimiento de Verificación (Aplicable a cualquier proyecto)

#### Fase I — Detección de Contradicciones (Opponens)

Recorrer **todas las fuentes de verdad** del proyecto y comparar:

| # | Verificación | Cómo ejecutar |
|---|-------------|---------------|
| 1 | **Pin mapping**: `hal_pins.cpp` vs `diagram.json` vs documentación | grep `GPIO\|PIN_` en todos los archivos |
| 2 | **Constantes físicas**: `config.h` vs código legacy vs datos empíricos | Comparar `#define` vs valores hardcodeados |
| 3 | **API calls**: versión de Core (v2.x vs v3.x) vs API usada | Verificar `platformio.ini` platform version |
| 4 | **Wire.begin duplicado**: HAL vs módulos independientes | grep `Wire.begin` |
| 5 | **Estructura prometida vs real**: plan de arquitectura vs archivos existentes | `glob **/*.h` vs lo prometido en docs |
| 6 | **Dependencias declaradas vs usadas**: `platformio.ini` vs `#include` | Comparar `lib_deps` vs includes reales |
| 7 | **Protocolos**: WiFi vs Bluetooth, async vs sync | Verificar capacidad del MCU |
| 8 | **Colas/estructuras**: declaradas vs consumidas | Verificar `std::queue` es leída en algún `loop()` |

**Ejecución práctica:**
```powershell
# Detectar Wire.begin duplicado
rg "Wire\.begin" src/ include/

# Detectar pines conflictivos
rg "GPIO|PIN_" src/ include/ wokwi/ --no-filename | sort | uniq

# Detectar constantes con múltiples definiciones
rg "#define\s+\w+" include/ src/ --no-filename | sort
```

#### Fase II — Análisis Causal Aristotélico (Proponens)

| Causa | Pregunta | Verificación |
|-------|----------|-------------|
| **Material** | ¿La memoria asignada es suficiente? | Stack size, heap fragmentation, string lengths |
| **Formal** | ¿La arquitectura FSM es completa? | ¿Todas las transiciones tienen timeout? ¿Hay estados inalcanzables? |
| **Eficiente** | ¿El toolchain produce binario correcto? | `pio run` exitoso, sin warnings críticos |
| **Final** | ¿El código cumple su propósito? | ¿Los comandos documentados coinciden con los implementados? |

#### Fase III — Cálculo Leibniziano (Verificación Formal con Herramientas)

```
Characteristica Universalis (IR/AST) + Calculus Ratiocinator (Checks)
```

| Herramienta | Aplicación en VeranoInve |
|-------------|------------------------|
| **Compilador** (`pio run`) | Calculus básico: ¿el código es sintácticamente válido? |
| **cppcheck** | Análisis estático: memory leaks, null pointers, buffer overflow |
| **Grep estructural** | Verificar todas las transiciones FSM tienen `cambiarEstado()` |
| **Wokwi simulation** | Characteristica visual: ¿el diagrama coincide con hal_pins.cpp? |

**Comando de verificación estática:**
```powershell
pio run 2>&1 | Select-String "error|warning"
```

#### Fase IV — Determinatio (Resolución)

Para cada contradicción encontrada:

1. **Identificar** la fuente canónica (ej: `hal_pins.cpp` es autoridad para pines)
2. **Resolver** la contradicción eliminando la fuente incorrecta
3. **Documentar** en `analisis_completo_veranoinve.md`
4. **Verificar** que tras el fix, `grep` no encuentra la versión descartada

---

### 3. Aplicación al Proyecto VeranoInve

#### Contradicciones Detectadas y Resueltas

| ID | Contradicción | Estado | Resolución |
|----|--------------|--------|------------|
| C-01 | Pin mapping L298N legacy vs actual | ✅ Resuelto | `hal_pins.cpp` es fuente de verdad |
| C-02 | GPIO3 strapping como IN4 | ✅ Resuelto | `PIN_IN4 = 9` |
| C-03 | API PWM v2.x vs v3.x | ✅ Resuelto | Usar `ledcSetup` + `ledcAttachPin` (v2.x) |
| C-04 | ADC2 vs digital en GPIO4-7 | ✅ Falso positivo | Digital no requiere ADC |
| C-05 | AsyncWebServer vs WebServer síncrono | ✅ Resuelto | Polling con WebServer síncrono |
| C-06 | lib_deps AsyncTCP faltantes | ✅ Resuelto | No necesarios (polling) |
| C-07 | Cm=1.25 vs CM_POR_PULSO teórico | ⏳ Pendiente | Requiere calibración empírica |
| C-08 | Arquitectura modular vs monolítica | ✅ Decidido | Ponytail: monolítico hasta que duela |
| C-09 | Bluetooth Classic vs WiFi | ✅ Resuelto | WiFi (S3 no soporta Classic BT) |
| C-10 | Odometría X/Y faltante | ✅ Resuelto | `posX`, `posY`, `orientacionGlobal` |
| C-11 | Cola comandos desconectada | ✅ Resuelto | `web_server_has_command()` en IDLE |
| C-12 | Wire.begin duplicado | ✅ Resuelto | Eliminado de `inicializarIMU()` |
| C-13 | Múltiples entornos IDE/Agent | ✅ Resuelto | Rastreo centralizado vía PDCA `_numero_plan_PDCA_` |
| C-14 | Riesgo hardware: 12V a VIN | ✅ Resuelto | LDO soporta hasta 5.5V máximo (Datasheet ESP32) |
| C-15 | Saturación vs Legibilidad visual | ✅ Resuelto | Arquitectura híbrida (Modo Físico vs Etiquetas de Puerto) |

#### Verificación Rápida Post-Cambio

```powershell
# 1. Verificar no hay pines GPIO3 en código fuente (debía ser GPIO9)
rg "GPIO.*\b3\b|PIN.*=.*\b3\b" src/ include/ wokwi/

# 2. Verificar un solo Wire.begin
rg "Wire\.begin" src/

# 3. Verificar todas las funciones declaradas tienen implementación
rg "^void \w+\(" include/ | % { $_.Line -replace '.*void (\w+).*', '$1' }

# 4. Verificar consistencia include ↔ lib_deps
rg "#include <(Adafruit|WiFi|WebServer|queue)>" src/
```

---

### 4. Principios Ponytail (Complemento al Supervisor)

```
Rung 1: Does the standard library do it?      → Usar Arduino built-in
Rung 2: Does the codebase already do it?       → Reusar patrón existente
Rung 3: Is there an installed dependency?      → Usar lib ya agregada
Rung 4: Native platform feature?               → ESP32 LEDC, portMUX
Rung 5: Already-installed dependency?          → Adafruit MPU6050
Rung 6: One line deletion?                     → Wire.begin duplicado
Rung 7: Minimum code that works?               → FSM 7 estados
```

### 5. Checklist de Auditoría Pre-Commit

```
□ pio run compila sin errores
□ No hay Wire.begin duplicados
□ Pin mapping coincide en hal_pins.cpp y wokwi/diagram.json
□ Todas las transiciones FSM tienen timeout
□ La cola de comandos es consumida en algún loop()
□ Las constantes físicas tienen comentario con fórmula
□ No hay #include de librerías no declaradas en lib_deps
□ El HTML embebido no excede ~15KB (límite práctico ESP32)
□ Los nombres de funciones siguen camelCase consistente
□ Cada estado FSM tiene LED indicador asociado
```

---

### 6. Metodologia 5S (Disciplina Operativa)

Aplicar al inicio de cada sesion PDCA y despues de cada commit:

| Fase | Japones | Accion Concreta |
|------|---------|-----------------|
| **SEIRI** (Clasificar) | 整理 | Separar archivos modificados de los no tocados. Mover backups a `1_seiri_sort/`. Identificar dead code. |
| **SEITON** (Ordenar) | 整頓 | Agrupar cambios por capa: HAL → Config → FSM → Web → UI. Cada commit refleja UNA capa. |
| **SEISO** (Limpiar) | 清掃 | Eliminar comentarios FIX obsoletos, variables no usadas, includes redundantes. |
| **SEIKETSU** (Estandarizar) | 清潔 | Verificar contra `_schema.json`. Todo PDCA debe tener: plan, do, check, act. Silogismo documentado. |
| **SHITSUKE** (Sostener) | 躾 | Actualizar `_index.json` con metricas. Ejecutar build. Verificar git status limpio. |

---

### 7. Metodo Mayeutico — Abogado del Diablo (Elenchus)

Para cada decision de diseno o fix propuesto, ejecutar el dialogo socratico:

```
Paso 1 — TESIS (Proponens):
  "Se debe crear/modificar X porque permite Y"

Paso 2 — ANTITESIS (Opponens / Diabolus):
  "Asumamos que NO se debe crear/modificar X"
  - ¿Que se rompe si no se hace?
  - ¿Existe entidad Z que ya cumple Y? (convocar Navaja de Ockham)
  - ¿El costo de NO hacerlo es mayor que el costo de hacerlo?
  - ¿Hay razon suficiente? (principio de razon suficiente)

Paso 3 — SINTESIS (Determinans):
  - Si la negacion genera contradiccion insostenible → tesis VALIDA
  - Si la negacion es sostenible sin contradiccion → tesis RECHAZADA
  - Si hay duda → aplazar (Ponytail: minima abstraccion)

Paso 4 — SILOGISMO RESULTANTE:
  - Documentar en PDCA como:
    PREMISA MAYOR: [principio general]
    PREMISA MENOR: [hecho especifico]
    CONCLUSION: [decision]
```

### 8. Formato Silogistico Escolastico

Todo agente filosofico debe estructurar sus conclusiones en formato escolastico:

```
Quaestio: [la pregunta a resolver]

Videtur quod sic...
  [argumentos a favor de la tesis]
  [evidencia concreta del codigo/documentacion]

Sed contra...
  [argumentos en contra / objeciones]
  [principio violado si se acepta la tesis]

Respondeo dicendum quod...
  [veredicto razonado]
  [condiciones bajo las cuales aplica]

Ad primum... Ad secundum...
  [respuesta a cada objecion]
```

### 9. Compatibilidad Neo-Tomista

Los agentes filosoficos operan bajo estos principios no-negociables:

| Principio | Definicion | Aplicacion |
|-----------|-----------|------------|
| **No-Contradiccion** | A y no-A no pueden ser ambos verdaderos | Dos fuentes no definen diferente el mismo valor |
| **Razon Suficiente** | Nada existe sin razon suficiente | Toda entidad nueva requiere justificacion |
| **Hilemorfismo** | Materia + Forma = Sustancia | Verificar si archivo (materia) + firma (forma) ya existen |
| **Navaja de Ockham** | No multiplicar entes sin necesidad | Preferir reuso sobre creacion |
| **Predicados No-Contradictorios** | Sujeto y predicado deben ser logicamente compatibles | `motorAvanzar(0, 0)` es predicado contradictorio si no mueve |
| **Silogismo Valido** | Premisa mayor + premisa menor → conclusion necesaria | Toda decision se reduce a silogismo documentable |

El nucleo puede migrarse a Neo-Tomismo simplemente reemplazando la metafisica subyacente: el principio de no-contradiccion, razon suficiente, e hilemorfismo son directamente compatibles con la tradicion tomista. Los agentes NO son relativistas — operan sobre verdad objetiva verificable en el codigo.
