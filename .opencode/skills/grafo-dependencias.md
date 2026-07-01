# 🕸️ Skill: Análisis de Dependencias sobre el Grafo de Conocimiento (Indexator Graphicus)

## Propósito y Contexto
Esta skill garantiza la congruencia absoluta entre el ecosistema de código fuente real y su representación abstracta en el Grafo de Conocimiento (`codebase-memory-mcp`).
- **Objetivo**: Detectar y mitigar el acoplamiento excesivo, prevenir dependencias circulares, rastrear librerías fantasma, y mantener la topología del software fiel a su implementación física.
- **Ámbito**: Archivos de configuración de dependencias (ej. `platformio.ini`), declaraciones de inclusión (`#include`, `import`), y relaciones estructurales entre módulos en el *Concilio de Salamanca*.

---

## 📋 Procedimiento de Ejecución (Paso a Paso)

### Fase 1: Extracción Física de Dependencias (Materia)
Extraer las dependencias formales y locales directamente del código.
```powershell
# Extraer includes de componentes locales y dependencias estándar
rg "#include\s+[\"<](?!Arduino|WiFi)[a-zA-Z0-9_/\.]+[\">]" src/ include/

# Verificar las librerías formalmente declaradas en PlatformIO
cat platformio.ini | grep -i "lib_deps" -A 5
```

### Fase 2: Sincronización con el Grafo de Conocimiento (Forma)
Contraste de la realidad física del código con el modelo mental (Knowledge Graph).
1. Consultar el grafo (ej. vía herramienta `search_graph` o inspección de `.mde_history`) por las relaciones estructurales reportadas para los módulos involucrados.
2. Identificar Contradicciones Topológicas:
   - **Dependencia Fantasma (Falso Positivo)**: El grafo indica que el módulo A depende del módulo B, pero no hay `#include B.h` ni acoplamiento físico en A.
   - **Dependencia Oculta (Falso Negativo)**: El módulo A invoca al módulo C físicamente, pero el grafo no documenta esta interacción.
   - **Ciclo de Dependencia**: A -> B y B -> A (indicador crítico de deuda arquitectural).

### Fase 3: Resolución Metadialéctica (Sustancia)
- **Purgar dependencias no utilizadas**: Si `platformio.ini` lista una librería que ningún archivo incluye, eliminarla para reducir superficie de ataque y tiempos de compilación.
- **Refactorización de Ciclos**: Si se detecta un ciclo, romperlo utilizando Inversión de Control (callbacks, punteros a funciones, o interfaces puras) apoyándose en el principio de separación del MDE.
- **Actualizar Grafo**: Ordenar al orquestador MDE actualizar el Knowledge Graph para reflejar la realidad del acoplamiento.

---

## ⚖️ Principios y Reglas de Decisión
1. **Principio de Aislamiento (Ponytail Rung 7)**: Los módulos deben interactuar mediante la mínima superficie (interfaz) necesaria que funcione. El acoplamiento es un pasivo, no un activo.
2. **Hilemorfismo Dependiente**: Un módulo (materia) solo se integra funcionalmente al sistema (forma) mediante sus interfaces de dependencia explícitas. Toda dependencia implícita viola la "Razón Suficiente".
3. **No-Contradicción Topológica**: Si el diseño dicta un flujo direccional (ej. `Web -> FSM -> Hardware`), una inclusión en sentido opuesto (`Hardware.h` incluyendo a `Web.h`) es una contradicción material que debe resolverse inmediatamente.

---

## ⬜ Checklist de Verificación (Pre-Commit)
- [ ] Las librerías de terceros en `platformio.ini` (`lib_deps`) coinciden exactamente con los `#include` usados en el código.
- [ ] Ningún archivo de la capa de bajo nivel (ej. `hal_pins.cpp`) incluye directrices de capas de alto nivel (ej. web server o GUI).
- [ ] No existen referencias circulares entre encabezados locales comprobables lógicamente.
- [ ] El grafo de conocimiento ha sido notificado/actualizado para reflejar con exactitud cualquier modificación topológica en las dependencias.
