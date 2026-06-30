# PDCA 015: Agentes Filosoficos — Supervisor Ampliado + Ockham Razor

**Fecha:** 2026-06-27 | **Estado:** COMPLETADO | **Build:** N/A (solo docs/skills)

---

## Objetivo

Formalizar los agentes filosoficos que gobiernan la metodologia del proyecto: ampliar el Supervisor Metadialectico con reglas 5S, Mayeutica y formato Silogistico, y crear el agente Ockham Razor con integracion al grafo de conocimiento.

---

## PLAN (Planificar)

### Acciones
1. Ampliar `supervisor-metadialectico.md` con 4 nuevas secciones
2. Crear `ockham-razor.md` — agente hilemorfico con graph queries
3. Documentar en PDCA_015
4. Crear `SISTEMA_ACTUAL.md` con descripcion completa del sistema + HTML frontend

---

## DO (Hacer)

### Agente 1: Supervisor Metadialectico (AMPLIADO)

**Secciones nuevas agregadas:**

| Seccion | Contenido |
|---------|-----------|
| **6. Metodologia 5S** | SEIRI (clasificar), SEITON (ordenar por capa), SEISO (limpiar FIX obsoletos), SEIKETSU (validar contra schema), SHITSUKE (actualizar metricas) |
| **7. Metodo Mayeutico** | Dialogo socratico: TESIS → ANTITESIS (Diabolus convoca a Ockham) → SINTESIS (Determinans) → SILOGISMO |
| **8. Formato Silogistico Escolastico** | Quaestio → Videtur quod sic → Sed contra → Respondeo dicendum → Ad primum/secundum |
| **9. Compatibilidad Neo-Tomista** | Principios no-negociables: No-Contradiccion, Razon Suficiente, Hilemorfismo, Navaja de Ockham, Predicados No-Contradictorios, Silogismo Valido |

### Agente 2: Ockham Razor (CREADO)

**Archivo:** `.opencode/skills/ockham-razor.md`

**Arquitectura del agente:**

```
Fase I — Consulta al Grafo (codebase-memory-mcp)
  ├── index_repository
  ├── search_graph (name pattern)
  ├── trace_path (call chains)
  ├── query_graph (Cypher)
  └── get_code_snippet (firma)

Fase II — Verificacion Hilemorfica
  ├── MATERIA: ¿archivo existe? (≥80% → RECHAZAR nueva)
  ├── FORMA: ¿firma compatible?
  └── SUSTANCIA: materia+forma vs requerimiento

Fase III — Silogismo de Necesidad
  ├── PREMISA MAYOR: Toda entidad requiere razon suficiente
  ├── PREMISA MENOR: {X} [tiene/no tiene] razon suficiente
  └── CONCLUSION: [crear/no crear]

Fase IV — Veredicto Escolastico
  └── Quaestio → Videtur → Sed contra → Respondeo
```

### Documentacion del Sistema

**Archivo:** `SISTEMA_ACTUAL.md` — Documento con:
1. Descripcion del funcionamiento actual del sistema
2. Arquitectura completa (FSM, Web, API)
3. Codigo HTML del frontend (extraido de UserControlGUI.cpp)
4. Pin mapping actualizado
5. Agentes del sistema documentados

---

## CHECK (Verificar)

### Archivos creados/modificados

| Archivo | Accion | Lineas |
|---------|--------|--------|
| `.opencode/skills/supervisor-metadialectico.md` | AMPLIADO (+4 secciones) | +100 |
| `.opencode/skills/ockham-razor.md` | CREADO | ~160 |
| `.mde_history/2_seiton_order/PDCA_015_Agentes_Filosoficos.md` | CREADO | ~80 |
| `SISTEMA_ACTUAL.md` | CREADO | ~300 |
| `.mde_history/_index.json` | ACTUALIZADO | +1 sesion |

### Verificaciones
- [x] supervisor-metadialectico.md ahora tiene 9 secciones (1-5 originales + 6-9 nuevas)
- [x] ockham-razor.md tiene 4 fases + formato escolastico + ejemplo de veredicto
- [x] Ambos agentes usan formato silogistico compatible con Neo-Tomismo
- [x] Principios no-negociables documentados en ambos agentes
- [x] codebase-memory-mcp integrado como herramienta del agente Ockham
- [x] SISTEMA_ACTUAL.md documenta funcionamiento completo + HTML frontend
- [x] build existente verificado (pio run SUCCESS)

### Concilio de Salamanca actualizado

```
MAGISTER DETERMINANS (Supervisor Metadialectico)
  ├── Reglas originales: No-Contradiccion, Ponytail, Pre-Commit Checklist
  ├── NUEVO: 5S operativo
  ├── NUEVO: Mayeutica (dialogo socratico)
  ├── NUEVO: Formato Silogistico Escolastico
  └── NUEVO: Compatibilidad Neo-Tomista
       │
       │ convoca cuando necesita verificar hilemorfismo
       ▼
OCKHAM RAZOR (Agente Hilemorfico)
  ├── codebase-memory-mcp (graph queries)
  ├── Verificacion Materia+Forma=Sustancia
  ├── Silogismo de Necesidad
  └── Veredicto Escolastico
```

---

## ACT (Estandarizar)

### Reglas establecidas
1. **Antes de cada sesion**: Ejecutar checklist 5S
2. **Ante cada nueva entidad**: Convocar a Ockham Razor (verificar hilemorfismo)
3. **Ante cada decision de diseno**: Ejecutar dialogo Mayeutico (Tesis → Antitesis → Sintesis)
4. **Toda conclusion**: Debe documentarse en formato silogistico escolastico
5. **No relativismo**: Las decisiones se basan en verdad objetiva verificable (grafo, build, tests)
6. **Migrable a Neo-Tomismo**: El nucleo filosofico es compatible por diseno
