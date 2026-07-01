# MDE-Frontend: Validación de KPIs Trascendentales

> Tabla de Contenidos:
> 1. KPI Verum (Accesibilidad/Lógica)
> 2. KPI Bonum (Rendimiento/Usabilidad)
> 3. KPI Pulchrum (Proporción/Armonía)
> 4. Reporte de Auditio Trascendentalis
> 5. Uso del Script validador_kpi.py

---

## 1. KPI Verum (Verum — Accesibilidad/Lógica)

> *"Si falla, la UI 'miente' al usuario y al lector de pantalla."*

### Métricas Componentes

| Sub-KPI | Umbral | Herramienta | Descripción |
|---------|--------|-------------|-------------|
| `contrast_ratio` | ≥ 4.5:1 (AA), ≥ 7:1 (AAA) | axe, Lighthouse | Contraste texto/fondo |
| `html_semantics` | 100% | HTML Validator | Uso correcto de etiquetas semánticas |
| `aria_correctness` | 0 errores | axe | Roles, labels, landmarks correctos |
| `keyboard_navigable` | 100% | Manual + tabIndex audit | Todos los elementos interactivos alcanzables |
| `focus_visible` | 100% | Manual + CSS audit | Indicador de foco visible en todos los estados |

### Silogismos de Verdad

```
Mayor: Todo texto que pretende ser leído por humanos debe tener contraste 
       suficiente con su fondo (WCAG 2.2, criterio 1.4.3).
Menor: El texto del botón secundario es gray-400 (#9CA3AF) sobre white (#FFFFFF).
Conclusio: Ratio = 2.6:1 < 4.5:1 → VERUM VIOLADO. 
          Debe cambiarse a gray-600 (#4B5563) que da ratio 5.7:1.
```

```
Mayor: Todo elemento interactivo debe tener un nombre accesible 
       (WCAG 2.2, criterio 4.1.2).
Menor: El icono de favorito es un <button> con un <svg> sin aria-label.
Conclusio: VERUM VIOLADO. Añadir aria-label="Añadir a favoritos" 
          o aria-labelledby apuntando al tooltip.
```

---

## 2. KPI Bonum (Bonum — Rendimiento/Usabilidad)

> *"Si es pesada o inestable, es 'mala' para el sistema y el usuario."*

### Métricas Componentes

| Sub-KPI | Umbral | Herramienta | Descripción |
|---------|--------|-------------|-------------|
| `dom_nodes` | < 1500 (total), < 150 (componente) | Lighthouse | Nodos DOM |
| `cls` | 0 | Lighthouse | Cumulative Layout Shift |
| `lcp` | < 2.5s | Lighthouse | Largest Contentful Paint |
| `inp` | < 200ms | Lighthouse | Interaction to Next Paint |
| `ttfb` | < 600ms | Lighthouse | Time to First Byte |
| `js_bundle` | < 200KB (gzipped) | webpack-bundle-analyzer | Tamaño del bundle JS |
| `image_weight` | < 500KB (hero), < 100KB (thumbs) | Lighthouse | Peso de imágenes |

### Silogismos de Bondad

```
Mayor: Todo layout que se mueve después de la carga inicial viola 
       la estabilidad visual y la confianza del usuario.
Menor: La imagen del hero no tiene width/height explícitos.
Conclusio: CLS > 0 → BONUM VIOLADO. Añadir aspect-ratio o dimensiones 
          exactas al <img> para reservar espacio.
```

```
Mayor: Un componente que excede 150 nodos DOM internos sufre de obesidad 
       ontológica y debe descomponerse.
Menor: <ProductCard> tiene 180 nodos incluyendo el tooltip y el carousel miniatura.
Conclusio: BONUM VIOLADO. Extraer <ProductTooltip> y <ProductThumbs> 
          como componentes independientes (lazy loaded).
```

---

## 3. KPI Pulchrum (Pulchrum — Proporción/Armonía)

> *"Si es caótico, es 'feo' ontológicamente."*

### Métricas Componentes

| Sub-KPI | Umbral | Herramienta | Descripción |
|---------|--------|-------------|-------------|
| `palette_std` | < 15% | generador_paleta.py | Desviación estándar de HSL |
| `golden_angle_dist` | ≈ 137.5° ± 15° | generador_paleta.py | Distancia de hue entre colores |
| `phi_layout` | TRUE | validador_kpi.py | Layout conforma proporción áurea |
| `spacing_continuity` | TRUE | validador_kpi.py | Espaciado sigue sucesión Fibonacci |
| `gradient_stops` | ≤ 3 | Manual | Número de paradas en gradientes |
| `type_scale` | Ratio 1.25–1.5 | Manual | Escala tipográfica modular |

### Silogismos de Belleza

```
Mayor: Una paleta cromática armónica tiene desviación estándar < 15% 
       en el espacio HSL (armonía análoga) O distancia de hue ≈ 137.5° 
       (complementario áureo).
Menor: La paleta actual tiene σ = 22% y la distancia entre hues es 45°.
Conclusio: PULCHRUM VIOLADO. Reducir a paleta análoga con σ < 15% 
          O reconfigurar con separación de 137.5°.
```

```
Mayor: Toda composición visual que respeta la proporción áurea 
       (Φ = 1.618...) exhibe armonía matemática objetiva.
Menor: El sidebar tiene width=300px y el content tiene width=700px 
       en un viewport de 1440px.
Conclusio: 300/700 = 0.428 ≠ 1/Φ (0.618). PULCHRUM VIOLADO. 
          Ajustar a sidebar=336px y content=544px (544/336 = 1.619 ≈ Φ).
```

---

## 4. Reporte de Auditio Trascendentalis

### Formato Estándar

```
╔════════════════════════════════════════════════════════════════╗
║            A U D I T I O   T R A N S C E N D E N T A L I S     ║
╠════════════════════════════════════════════════════════════════╣
║ Fecha: 2025-06-25T14:32:00Z                                    ║
║ Componente: <ProductCard variant="hero" />                       ║
║ Revisor: Magister Determinans (Automated)                        ║
╠════════════════════════════════════════════════════════════════╣
║                                                                 ║
║  KPI VERUM (Accesibilidad/Lógica):                              ║
║  ├── Contraste texto: 17:1 ≥ 4.5:1 ................... ✓      ║
║  ├── Contraste UI: 3.2:1 ≥ 3.0:1 ..................... ✓      ║
║  ├── Semántica HTML5: 8/8 tags correctos ............. ✓      ║
║  ├── ARIA: 0 violaciones, 3 landmarks ................ ✓      ║
║  ├── Navegación teclado: 5/5 elementos ............... ✓      ║
║  └── Foco visible: outline-2 outline-offset-2 ........ ✓      ║
║  RESULTADO: VERUM VALIDADO ............................ ✓✓✓    ║
║                                                                 ║
║  KPI BONUM (Rendimiento/Usabilidad):                            ║
║  ├── Nodos DOM: 23 < 150 ............................. ✓      ║
║  ├── CLS: 0 .......................................... ✓      ║
║  ├── LCP: 1.4s < 2.5s ................................ ✓      ║
║  ├── INP: 56ms < 200ms ............................... ✓      ║
║  ├── Bundle: 45KB < 200KB ............................ ✓      ║
║  └── Imágenes: 320KB total (hero 420KB → optimizada) .. ✓      ║
║  RESULTADO: BONUM VALIDADO ............................ ✓✓✓    ║
║                                                                 ║
║  KPI PULCHRUM (Proporción/Armonía):                             ║
║  ├── σ paleta HSL: 7.2% < 15% ........................ ✓      ║
║  ├── ΔHue promedio: 138.1° ≈ 137.5° .................. ✓      ║
║  ├── Layout Φ-conforme: 889/550 = 1.617 .............. ✓      ║
║  ├── Espaciado Fibonacci: [8,16,24,48] ............... ✓      ║
║  ├── Gradient stops: 3 ≤ 3 ........................... ✓      ║
║  └── Type scale ratio: 1.25 .......................... ✓      ║
║  RESULTADO: PULCHRUM VALIDADO ......................... ✓✓✓    ║
║                                                                 ║
╠════════════════════════════════════════════════════════════════╣
║  DETERMINATIO: ESSENTIA PURA — APROBADO SIN OBJECIONES        ║
╠════════════════════════════════════════════════════════════════╣
║  Silogismo de validación:                                       ║
║  Mayor: Todo componente que cumple Verum, Bonum y Pulchrum      ║
║         ha alcanzado la perfección ontológica.                  ║
║  Menor: <ProductCard> cumple los 18 sub-KPIs auditados.         ║
║  Conclusio: <ProductCard> es esencialmente puro y apto para     ║
║             producción.                                         ║
╚════════════════════════════════════════════════════════════════╝
```

---

## 5. Uso del Script validador_kpi.py

### Instalación

```bash
pip install beautifulsoup4 cssutils coloraide
```

### Ejecución

```bash
# Validar un archivo HTML/TSX
python scripts/validador_kpi.py --file src/components/ProductCard.tsx --type component

# Validar una paleta de colores
python scripts/generador_paleta.py --base-hue 210 --count 4 --output json

# Generar reporte completo
python scripts/validador_kpi.py --file src/App.tsx --type page --report full
```

### Integración CI/CD

```yaml
# .github/workflows/mde-frontend.yml
name: MDE Frontend Validation
on: [push, pull_request]
jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Run MDE KPI Validator
        run: |
          pip install beautifulsoup4 cssutils coloraide
          python scripts/validador_kpi.py --dir src/ --report summary
```
