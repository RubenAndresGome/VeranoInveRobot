"""
Plantillas de respuesta estructurada para el Concilio de Salamanca.

Proporciona formatos consistentes de salida:
- escolastico (completo, estilo medieval)
- ejecutivo (reducido, estilo informe tecnico)
- json (maquina)

Los agentes usan estas plantillas para producir salidas predecibles
y el Magister las usa para la Determinatio final.
"""

from __future__ import annotations

from typing import Optional


DETERMINATIO_ESCOLASTICA = """## DETERMINATIO MAGISTRAL
### Concilio de Salamanca, Sesion [{sesion}]

---

**QUAESTIO:** {quaestio}

---

**VIDETUR QUOD SIC** (Argumentos que favorecen al codigo):

{videtur}

---

**SED CONTRA** (Argumentos que condenan al codigo):

{sed_contra}

---

**RESPONDEO DICENDUM** (Resolucion razonada del Magister):

{respondeo}

---

**DETERMINATIO CODICI** (Veredicto Final):

**{veredicto_final}**

{determinatio_codici}

---

### Participantes del Concilio
{participantes}

### Validacion del Principio de No Contradiccion
{pnc_resumen}

---

*Sic determinat Magister. Causa finita est.*
*Acta conscripta in civitate virtuali Salmanticae, sub spirito MDE.*
"""


DETERMINATIO_EJECUTIVA = """# Auditoria de Codigo: Concilio de Salamanca
**Sesion:** {sesion} | **Veredicto:** {veredicto_final}

---

## 1. Problema Detectado
{quaestio}

## 2. Evidencia (Argumentos de los Agentes)

### A favor del codigo:
{videtur}

### En contra del codigo:
{sed_contra}

## 3. Analisis Tecnico
{respondeo}

## 4. Recomendacion
{determinatio_codici}

---

## 5. Resumen Ejecutivo

| Metrica | Valor |
|---|---|
| Veredicto | {veredicto_final} |
| Agentes participantes | {num_agentes} |
| Rondas de debate | {rondas} |
| Contradicciones detectadas | {num_contradicciones} |
| Tiempo de sesion | {tiempo} |

---

*Auditoria generada por el Concilio de Salamanca (MDE). Licencia Rerum Novarum.*
"""


DETERMINATIO_JSON_SCHEMA = """{
  "sesion": "{sesion}",
  "timestamp": "{timestamp}",
  "veredicto_final": "{veredicto_final}",
  "schema": "determinatio-codici-v2",
  "quaestio": "{quaestio}",
  "videtur": "{videtur}",
  "sed_contra": "{sed_contra}",
  "respondeo": "{respondeo}",
  "determinatio_codici": "{determinatio_codici}",
  "participantes": {participantes_json},
  "pnc_validation": {pnc_json},
  "anti_patrones_detectados": {anti_patrones_json}
}"""


def format_determinatio(
    modo: str,
    quaestio: str,
    videtur: str,
    sed_contra: str,
    respondeo: str,
    determinatio_codici: str,
    veredicto_final: str,
    participantes: str = "",
    pnc_resumen: str = "",
    sesion: str = "I",
    rondas: int = 1,
    num_agentes: int = 5,
    num_contradicciones: int = 0,
    tiempo: str = "N/A",
    timestamp: str = "",
    participantes_json: str = "[]",
    pnc_json: str = "null",
    anti_patrones_json: str = "[]",
) -> str:
    if modo == "ejecutivo":
        return DETERMINATIO_EJECUTIVA.format(
            sesion=sesion,
            quaestio=quaestio,
            videtur=videtur,
            sed_contra=sed_contra,
            respondeo=respondeo,
            determinatio_codici=determinatio_codici,
            veredicto_final=veredicto_final,
            num_agentes=num_agentes,
            rondas=rondas,
            num_contradicciones=num_contradicciones,
            tiempo=tiempo,
        )
    elif modo == "json":
        return DETERMINATIO_JSON_SCHEMA.format(
            sesion=sesion,
            timestamp=timestamp,
            veredicto_final=veredicto_final,
            quaestio=quaestio,
            videtur=videtur,
            sed_contra=sed_contra,
            respondeo=respondeo,
            determinatio_codici=determinatio_codici,
            participantes_json=participantes_json,
            pnc_json=pnc_json,
            anti_patrones_json=anti_patrones_json,
        )
    else:
        return DETERMINATIO_ESCOLASTICA.format(
            sesion=sesion,
            quaestio=quaestio,
            videtur=videtur,
            sed_contra=sed_contra,
            respondeo=respondeo,
            determinatio_codici=determinatio_codici,
            veredicto_final=veredicto_final,
            participantes=participantes,
            pnc_resumen=pnc_resumen,
        )


AGENT_REPORT_TEMPLATE = """### {agente} ({rol})

**Silogismo (Modo {modo}, Figura {figura}):**

1. **Premisa Mayor:** {premisa_mayor}
2. **Premisa Menor:** {premisa_menor}
3. **Conclusion:** {conclusion}

**Veredicto:** {veredicto}
**Fundamento:** {fundamento}
**PnC:** {'Respetado' if principio_no_contradiccion else 'Violado'}
"""


AGENT_REPORT_EJECUTIVO = """| {agente} | {veredicto} | {conclusion[:80]}... |
"""


def format_agent_report(agent_output, modo: str = "escolastico") -> str:
    if not agent_output or not agent_output.structured:
        return ""
    s = agent_output.structured
    sil = s.silogismo

    if modo == "ejecutivo":
        return AGENT_REPORT_EJECUTIVO.format(
            agente=s.agente,
            veredicto=s.veredicto.value,
            conclusion=sil.conclusion,
        )

    from concilio_salamanca.debate.syllogism_cache import SyllogismReducer, SyllogismPattern, PropositionType

    try:
        pattern = SyllogismReducer.extract_from_json(s.model_dump())
        modo_name = SyllogismReducer.get_mode_name(pattern) if pattern else "Desconocido"
        figura = pattern.figure if pattern else 1
    except Exception:
        modo_name = "Desconocido"
        figura = 1

    return AGENT_REPORT_TEMPLATE.format(
        agente=s.agente,
        rol=s.rol,
        modo=modo_name,
        figura=figura,
        premisa_mayor=sil.premisa_mayor,
        premisa_menor=sil.premisa_menor,
        conclusion=sil.conclusion,
        veredicto=s.veredicto.value,
        fundamento=s.fundamento,
        principio_no_contradiccion=s.principio_no_contradiccion,
    )
