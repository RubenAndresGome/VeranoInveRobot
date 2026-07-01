# PDCA 016: Instalacion Completa de Infraestructura MDE

**Fecha:** 2026-06-28 | **Estado:** COMPLETADO | **Build:** SUCCESS

---

## Objetivo

Completar la infraestructura MDE: instalar skills externas (concilio-salamanca, mde-frontend), crear opencode.json con MCP wiring, ampliar Ockham Razor con metafisica tomista y polimorfismo ontologico.

---

## PLAN (Planificar)

### Acciones
1. Extraer `.skill` ZIPs del repositorio externo MDE a `.opencode/skills/`
2. Crear `opencode.json` con MCP server codebase-memory-mcp + registro de 5 skills
3. Ampliar Ockham Razor con Secciones 7-10 (metafisica tomista + grafo de analogias)
4. Documentar en PDCA_016
5. Actualizar .gitignore y README

---

## DO (Hacer)

### T1: opencode.json
- Creado en raiz del proyecto
- MCP server: codebase-memory-mcp v0.8.1 (binary path local)
- Skills registradas: supervisor-metadialectico, ockham-razor, grafo-dependencias, concilio-salamanca, mde-frontend
- Rules: always-apply-5s, pre-commit-audit, require-syllogism, require-hilemorfismo-check, ockham-before-new-entity
- Permissions: codebase-memory-mcp tools, bash pio/codebase-memory-mcp

### T2: Skills externas instaladas
- `concilio_salamanca.skill` → `.opencode/skills/concilio-salamanca/` (SKILL.md + reference/ + config.yaml)
- `mde-frontend.skill` → `.opencode/skills/mde-frontend/` (SKILL.md + references/ + scripts/)
- Estructura aplanada (ZIP traia subcarpeta duplicada)

### T3: Ockham Razor ampliado

**Secciones nuevas agregadas:**

| Seccion | Contenido | Lineas |
|---------|-----------|--------|
| **7. Metafisica Tomista del Codigo** | 9 categorias: Esse, Essentia, Substantia, Accidentia, Potentia, Actus, Unum, Bonum, Verum. Cada una con equivalente en codigo y verificacion via CBMM. Silogismo del Ser. | ~40 |
| **8. Grafo de Analogias — Polimorfismo Ontologico** | Analogia de Atribucion (herencia), Proporcionalidad Propia (interfaces), Proporcionalidad Impropia (metafora). Procedimiento de verificacion en 3 fases (A-B-C). Indice de analogia con umbrales 80%/50%. Ejemplo completo con FastMotor vs MotorStandard. | ~80 |
| **9. Dialogo Ockham-Aquinas** | Coloquio permanente entre el nominalista (Ockham: no multiplicar) y el realista moderado (Aquinas: el bien se difunde, analogia entis). Sintesis por el Magister Determinans. | ~30 |
| **10. Principios No-Negociables (Ampliado)** | De 5 a 8 principios: se agregan Analogia Entis, Actus et Potentia, Unum et Bonum | ~10 |

---

## CHECK (Verificar)

### Archivos creados/modificados

| Archivo | Accion | Lineas |
|---------|--------|--------|
| `opencode.json` | CREADO | 40 |
| `.opencode/skills/concilio-salamanca/` | CREADO (4 archivos) | ~25KB |
| `.opencode/skills/mde-frontend/` | CREADO (6 archivos) | ~68KB |
| `.opencode/skills/ockham-razor.md` | AMPLIADO (+4 secciones, de 165 a ~330 lineas) | +165 |
| `.mde_history/2_seiton_order/PDCA_016_MDE_Setup.md` | CREADO | ~80 |

### Build
```
pio run → SUCCESS
RAM 14.9% | Flash 28.1%
```

### Skills registradas en opencode.json

| Skill | Tipo | Proposito |
|-------|------|-----------|
| supervisor-metadialectico | Local (propio) | Auditoria por no-contradiccion, 5S, mayeutica |
| ockham-razor | Local (propio) | Hilemorfismo + polimorfismo tomista con grafo |
| grafo-dependencias | Local (propio) | Congruencia codigo vs grafo CBMM |
| concilio-salamanca | Externo (instalado) | Tribunal de 39 agentes, auditoria escolastica |
| mde-frontend | Externo (instalado) | Validacion KPI, paleta de colores, componentes |

---

## ACT (Estandarizar)

### Reglas establecidas
1. Antes de crear cualquier entidad nueva → Ockham Razor verifica hilemorfismo + analogia
2. Toda decision de diseno → dialogo Ockham-Aquinas documentado
3. Todo metodo nuevo → distinguir potentia (declarado) de actus (llamado via trace_path)
4. Toda clase nueva → verificar si es accidente (extender) o nueva essentia (crear)
5. Pipeline MDE: Supervisor → Ockham → Concilio → Grafo-Dependencias → Build

### Para colaboradores nuevos
```bash
# Instalar codebase-memory-mcp (una vez)
curl -fsSL https://raw.githubusercontent.com/DeusData/codebase-memory-mcp/main/install.sh | bash

# Indexar el proyecto
codebase-memory-mcp cli index_repository '{"repo_path": "."}'

# Editar opencode.json con el path correcto del binario

# Verificar herramientas
codebase-memory-mcp --version
```
