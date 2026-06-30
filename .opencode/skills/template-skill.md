# 🛠️ Nombre de la Skill: [Ej. Optimización de Memoria en ESP32]

## Propósito y Contexto
[Describe brevemente para qué sirve esta skill y qué problemas previene o resuelve.]
- **Objetivo**: [Ej. Prevenir fugas de memoria y fragmentación de heap]
- **Ámbito**: [Ej. Archivos C++ dentro de src/ e include/]

---

## 📋 Procedimiento de Ejecución (Paso a Paso)
[Instrucciones secuenciales y comandos exactos que el agente o tú deben ejecutar para aplicar la skill.]

### Fase 1: Detección y Análisis
```powershell
# Ejemplo de comando para buscar patrones problemáticos (ej. uso de String en lugar de char arrays)
rg "String\s+\w+" src/ include/
```

### Fase 2: Corrección / Optimización
- [ ] Regla de diseño 1: [Ej. Reemplazar String dinámicos por `snprintf` o `std::string` si el scope es local]
- [ ] Regla de diseño 2: [Ej. Pasar strings grandes por referencia constante `const String&`]

---

## ⚖️ Principios y Reglas de Decisión
[Heurísticas que guían la toma de decisiones al aplicar la skill, inspiradas por ejemplo en principios Ponytail.]
1. **Principio 1**: [Ej. Priorizar memoria estática sobre dinámica (stack vs heap)]
2. **Principio 2**: [Ej. Mantener el tamaño del búfer al mínimo requerido + 1 para el carácter nulo]

---

## ⬜ Checklist de Verificación (Pre-Commit)
[Lista de chequeo final que garantiza que la skill se ejecutó correctamente y no rompió nada.]
- [ ] El código compila sin advertencias de uso de memoria (`pio run`)
- [ ] Se verificaron los límites de tamaño en buffers de caracteres (`sizeof`)
- [ ] No existen objetos de tipo `String` declarados de forma global en los headers
