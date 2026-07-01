---
name: mde-frontend
description: >
  Frontend development guided by the Metadialectic Scholastic Method (MDE) — a 
  neurosymbolic framework that subjects every line of code to deductive syllogism, 
  Aristotelian causal analysis, and transcendental KPI validation (Verum/Bonum/Pulchrum). 
  Use when building React + TypeScript + Tailwind CSS + Three.js/WebGL interfaces 
  that demand ontological purity: component architecture, UI composition, color systems, 
  shader design, animation choreography, and layout proportionality. Triggers for any 
  frontend task where design decisions must be justified by formal logic rather than taste.
  Covers Atomic Design hierarchy, Feature-Sliced Design boundaries, React composition 
  patterns, R3F scene construction, and aesthetic determinatio via syllogistic reasoning.
---

# MDE-Frontend: Motor Silogístico del Diseño Web

> *"Cogito Ergo Code — Ningún elemento DOM será renderizado sin demostrar su necesidad 
> a través del silogismo deductivo."*

## Stack Ontológico

| Capa | Tecnología | Causa Formal |
|------|-----------|--------------|
| Materia | HTML5 Semántico | Estructura ontológica del DOM |
| Forma | React 19 + TypeScript strict | Composición funcional tipada |
| Apariencia | Tailwind CSS 3.4 | Utilidades atómicas sin cascada caótica |
| Espacio | CSS Grid + Flexbox | Geometría de proporción áurea |
| Movimiento | Framer Motion / GSAP | Cinemática de transiciones |
| Profundidad | Three.js + React Three Fiber + TSL | Realidad sintética WebGPU/WebGL |

## Workflow Silogístico (Disputatio Frontend)

Toda tarea frontend debe seguir el ciclo de la *quaestio disputata*:

```
1. Análisis Ontológico del Requerimiento
   └─ Sujeto, Causa Final, Estilo Axiomático
2. Syllogismus Compositorius (Composición)
   └─ Premisa Mayor → Premisa Menor → Determinatio
3. Calculus Cromatibus (Validación cromática)
   └─ Matriz Mc, KPI Verum (contraste), KPI Pulchrum (armonía)
4. Determinatio Code (Output técnico)
   └─ Código con justificación silogística como comentario
5. Auditio Trascendentalis (Validación final)
   └─ KPI Verum ≥ 4.5:1, KPI Bonum < 150 nodos, KPI Pulchrum < 15% σ
```

## 10 Axiomas Trascendentales del Frontend MDE

### I. Principium Contradictionis DOM
Ningún nodo puede poseer simultáneamente el predicado de "visible" y "no visible" 
en el mismo estado y contexto de renderizado. Un componente que escondido por CSS 
pero presente en el árbol contradice el PNC.

### II. Principium Tertii Exclusi Rendering
Un componente o bien debe renderizarse, o bien no debe renderizarse. 
No existe término medio ontológico: `display: none` con estado activo es una 
violación del *tertium non datur*.

### III. Actus et Potentia Componentis
El código fuente existe en *potentia* hasta que React lo monta en el DOM. 
El análisis estático (linters) examina la potencia; el análisis dinámico (runtime) 
confirma el acto. Ningún componente debe actualizarse a un acto que contravenga 
sus precondiciones lógicas.

### IV. Quattuor Causae UI
Toda interfaz debe justificarse bajo las cuatro causas aristotélicas:
- **Causa Material**: ¿Qué datos/estructuras la componen?
- **Causa Formal**: ¿Qué patrón de diseño (Atom, Molecule, Organism) la configura?
- **Causa Eficiente**: ¿Qué evento/estado la actualiza?
- **Causa Final**: ¿Qué acción del usuario culmina su existencia?

### V. Hylemorphismum SOLID
La sustancia de un componente es compuesto de **materia** (props, estado) 
y **forma** (JSX, estructura render). Un componente que maneja fetching 
(materia empírica) Y controla routing (forma de navegación) carece de esencia unitaria: 
viola SRP y debe dividirse en *Container* (materia) y *Presentational* (forma).

### VI. Syllogismus Aestheticus
Todo output de UI debe ser precedido por su silogismo justificativo:
```
Premisa Mayor (Axioma):    Ley matemática, percepción o lógica de negocio
Premisa Menor (Caso):      Estado actual de datos o restricción espacial
Conclusio (Determinatio):  Propiedad CSS o componente React exacto a inyectar
```

### VII. Metrum Aureum Layout
El espaciado y layout se miden en módulos áureos (Φ = 1.618...), no en píxeles 
arbitrarios. El padding interno de un contenedor de ancho W debe ser W/(Φ×4). 
El ritmo vertical sigue la sucesión E = [4, 8, 16, 24, 32, 48, 64, 96] px.

### VIII. Matrix Chromatica Covariantis
La paleta es una matriz de covarianza cromática Mc, no una lista de hexadecimales. 
Toda paleta optimiza el KPI de Diferenciación Relacional (DR): la distancia euclidiana 
entre Hues debe aproximar el ángulo dorado (137.5°). Si S_index > 0.7, emitir 
alerta de "Fatiga Ocular Estocástica".

### IX. Gradientus Continuitas
Todo gradiente es una función continua f(x) o f(r,θ). Gradientes lineales 
minimalistas son Escalonadas de Heaviside; gradientes orgánicos usan interpolación 
de Bézier cúbica. Prohibición ontológica: máximo 3 paradas de color (Navaja de Ockham).

### X. Trinitas Transcendentalis
Toda interfaz se evalúa bajo tres KPIs:
- **VERUM** (Accesibilidad/Lógica): Contraste ≥ 4.5:1 AND semántica HTML5 = 100%
- **BONUM** (Rendimiento/Usabilidad): DOM Nodes < 150 AND CLS = 0
- **PULCHRUM** (Proporción/Armonía): σ(HSL palette) < 15% OR Hue distance ≈ 137.5°

## Patrones de Componentes (Syllogismus Compositorius)

### Atomic Design como Jerarquía Ontológica

Los cinco niveles de Atomic Design son grados de complejidad ontológica creciente:

| Nivel | Esencia | Responsabilidad | Ejemplo |
|-------|---------|----------------|---------|
| **Átomo** | *Sustancia simple* | Ente indivisible; estado puro de presentación | `<Button>`, `<Input>`, color `#000` |
| **Molécula** | *Compositio prima* | Función unitaria emergente de átomos | `<SearchBox>` = Input + Button |
| **Organismo** | *Sustancia compleja* | Independencia operativa; propio estado | `<Header>`, `<ProductCard>` |
| **Template** | *Figura sine materia* | Wireframe de layout; posición sin contenido | Página con slots vacíos |
| **Page** | *Actus completus* | Instancia concreta con datos reales | `/dashboard` poblada |

**Silogismo de categorización:**
```
Mayor: Todo ente visual con estado propio y Causa Final completa es Organismo.
Menor: <CartSummary> maneja useState(cartItems) y culmina en checkout.
Conclusio: <CartSummary> es Organismo, no Molécula.
```

### Feature-Sliced Design como Dirección de Dependencias

Las capas FSD son anillos ontológicos concéntricos; el flujo de dependencias 
solo puede ir de capas externas hacia internas:

```
app/         → Configuración, providers, routing (Causa Eficiente)
pages/       → Composición de widgets por ruta
widgets/     → Bloques reutilizables de página
features/    → Acciones de usuario (Causa Final) → login, checkout
entities/    → Modelos de negocio → User, Cart, Product
shared/      → UI-kit, lib, config, api (Causa Material + Formal)
```

**Prohibición**: Una capa "inferior" (shared) NUNCA importa de una "superior" (features). 
Esto violaría el Principio de Dirección de Dependencia Escolástica.

### React Component Patterns

**Compound Components** (Compositio Implicita):
```tsx
// Mayor: Las partes de un ente que comparten estado implícito forman un todo teleológico.
// Menor: Tabs.List, Tabs.Trigger y Tabs.Content comparten el estado activeTab.
// Conclusio: Se implementan via React Context como componentes compuestos.
<Tabs>
  <Tabs.List>
    <Tabs.Trigger value="1">Panel 1</Tabs.Trigger>
  </Tabs.List>
  <Tabs.Content value="1">Contenido</Tabs.Content>
</Tabs>
```

**Container/Presentational** (Hylemorphismum):
```tsx
// Container = Causa Material + Eficiente (fetching, state)
// Presentational = Causa Formal (JSX puro, props)
// Mayor: La forma sustancial confiere esencia unitaria.
// Menor: UserContainer maneja fetching Y renderizado.
// Conclusio: Dividir en UserContainer (materia) + UserProfile (forma).
```

**Render Props** (Inversio Dependentiae):
```tsx
// Mayor: Las abstracciones no deben depender de detalles empíricos.
// Menor: Un <DataFetcher> genérico recibe función render como prop.
// Conclusio: La lógica de fetching es independiente de la presentación.
```

## Sistema Cromático Trascendental

### Matriz de Covarianza Cromática (Mc)

```
Mc = [H1, H2, H3, S_mean, L_mean, DR]

donde:
  H1, H2, H3 = Hues primarios en espacio cilíndrico HSL [0-360]
  DR = Diferenciación Relacional = min(|Hi - Hj|) aprox 137.5°
  S_index = Σ(Si²)/N  →  alerta si > 0.7
```

### Reglas de Determinatio Cromática

| Escenario | Silogismo | Output |
|-----------|-----------|--------|
| Fondo L=0.9 | Mayor: El CTA requiere máximo contraste. Menor: Fondo L=0.9. | `bg-[#000000]` (ratio 17:1) |
| Paleta caótica | Mayor: σ(HSL) > 15% implica desorden. Menor: σ actual = 22%. | Reducir a paleta análoga o complementaria áurea |
| Saturación excesiva | Mayor: S_index > 0.7 produce fatiga. Menor: S_index = 0.85. | Reducir saturación un 30%, migrar a tonos pastel |

### Gradientes como Funciones Continuas

| Tipo | Función matemática | Caso de uso |
|------|-------------------|-------------|
| Minimalista | Escalón de Heaviside H(x) | Corte puro, sin difuminado |
| Orgánico | Bézier cúbica C(t), t∈[0,1] | Transiciones suaves, continuidad f'(x)=0 |
| Radial | f(r,θ) en coordenadas polares | Focos de atención, halos lumínicos |

**Prohibición ontológica**: Ningún gradiente tendrá más de 3 paradas de color. 
Cada parola adicional viola el Principio de Economía de Entes (Navaja de Ockham).

## Geometría de Fibonacci y Layout

### Ritmo Vertical

El espaciado no es arbitrario. Se extrae de la sucesión:
```
E = [4, 8, 16, 24, 32, 48, 64, 96, 128]

Condición: E(n)/E(n-1) ∈ [1.5, 2.0]
```

### Proporción Áurea en Layouts

```
Mayor: Toda jerarquía visual requiere que el contenedor y el margen 
       guarden una proporción que refleje orden matemático cósmico.
Menor: El ancho del viewport es W.
Conclusio: Ancho contenido = W/Φ. Margen lateral = (W - W/Φ)/2.
          Si W = 1440px → contenido = 889px, margen = 275.5px c/u.
```

### Grid System

```css
/* Grid basado en Φ */
.grid-mde {
  display: grid;
  gap: calc(1rem * 1.618);      /* 25.888px */
  grid-template-columns: repeat(12, 1fr);
  padding: calc(100vw / (1.618 * 4));  /* Vacío Estructurado Áureo */
}
```

## Three.js / React Three Fiber — Ontología de la Escena 3D

### Las Cuatro Causas de una Escena R3F

| Causa | Analogía Escolástica | Implementación Técnica |
|-------|---------------------|----------------------|
| **Material** | La geometría, texturas, buffers GPU | `<boxGeometry>`, `<meshStandardMaterial>` |
| **Formal** | El grafo de escena, jerarquía de nodos | `<Canvas>`, `<group>`, `<mesh>` |
| **Eficiente** | El render loop, useFrame, eventos | `useFrame((state) => ...)`, raycaster |
| **Final** | La experiencia del usuario, storytelling | Interactividad, cámara guiada, narrative |

### Syllogismus 3D

```
Mayor: Todo objeto 3D que busca ser percibido debe poseer luz y sombra 
       que revele su volumen (Causa Formal).
Menor: La escena actual carece de fuentes lumínicas direccionales.
Conclusio: Se añade <directionalLight intensity={1.5} position={[5,5,5]} />.
          Sin esta luz, el mesh es un ente incompleto (forma sin actualización).
```

### Reglas de Performance (KPI Bonum extendido)

- **Draw calls < 100**: Usar `<InstancedMesh>` para objetos repetidos
- **VRAM**: Texturas KTX2/Basis Universal, nunca PNG crudo en producción
- **Geometría**: Draco compression, LOD con `<Detailed distances={[0,20,50]}>`
- **Memoria**: Disposición explícita en cleanup `useEffect` 
  (`geometry.dispose()`, `material.dispose()`, `texture.dispose()`)

### Shaders como Cálculo Ratiocinator

Los shaders son el *calculus ratiocinator* visual: deducciones matemáticas 
ejecutadas por el GPU para cada píxel.

```glsl
// Premisa Mayor: La luz que incide sobre una superficie sigue la ley de Lambert.
// Premisa Menor: normalWorld y viewDirection son vectores unitarios.
// Conclusio: float intensity = max(dot(normal, lightDir), 0.0);

// Versión TSL (Three Shader Language) — preferida para WebGPU:
import { Fn, float, vec3, normalWorld, viewDirection } from 'three/tsl';

const fresnel = Fn(([power]) => {
  const dotNV = normalWorld.dot(viewDirection).saturate();
  return float(1).sub(dotNV).pow(power);
});
```

**Regla**: Preferir TSL sobre GLSL/WGSL crudo. TSL maneja cross-compilación 
y uniforms automáticamente. Raw shaders requieren mantener dos codebases.

## Referencias (cargar según contexto)

- **Principios filosóficos detallados**: `references/mde-principios.md`
- **Patrones de componentes + ejemplos**: `references/mde-componentes.md`
- **WebGL/Three.js + shaders**: `references/mde-shader-gl.md`
- **Validación de KPIs trascendentales**: `references/mde-kpi-validator.md`

## Scripts de Validación

- **Validador KPI**: `scripts/validador_kpi.py` — Verifica Verum/Bonum/Pulchrum 
  en archivos HTML/CSS/TSX dados
- **Generador de Paleta**: `scripts/generador_paleta.py` — Genera paleta MDE 
  con ángulo dorado y calcula DR
