# Plan de Implementación: Modo Esquema Puro y Conexiones por Etiquetas (Net Labels)

Este plan detalla la reestructuración del visor de ensamble para incorporar un modo de visualización basado en etiquetas de red (net labels/ports) similar a las herramientas de diseño EDA profesionales (como Proteus o Altium). Esto reducirá el desorden visual del cableado cruzado y proporcionará una vista pura del esquema.

---

## 🏛️ Aplicación de la Skill: Supervisor Metadialéctico

### 📑 Fase I: Detección y Análisis de Contradicciones
* **Contradicción C-15 (Saturación vs Legibilidad)**:
  * **Tesis**: Dibujar cables físicos de extremo a extremo es útil para el ensamble físico paso a paso.
  * **Antítesis**: La abundancia de cables largos cruza todo el lienzo y satura la vista, dificultando el análisis de la arquitectura del microcontrolador de forma rápida.
  * **Resolución**: Implementar una arquitectura híbrida conmutadora en la interfaz gráfica que permita alternar entre el **Modo Físico (Cableado)** y el **Modo Lógico (Etiquetas de Puerto / Net Labels)**, ocultando opcionalmente paneles complementarios.

---

## 📋 Cambios Propuestos

### 1. Interfaz de Usuario (UI) y Toolbar
* **Control de Modos**: Agregar botones en la barra de herramientas superior para conmutar entre:
  * **Vista con Cables**: Muestra las líneas de conexión SVG.
  * **Esquema Puro**: Oculta las líneas de conexión y activa las etiquetas de red.
* **Control de Panel**: Asegurar que la tabla lateral y la leyenda se puedan ocultar o colapsar individualmente para liberar espacio en el lienzo.

### 2. Estilos CSS (`diagrama_ensamble.css`)
* Creación de clases de diseño para las **etiquetas de puerto (Net Flags)**:
  * Flechas o banderas de dirección de señal (`.net-tag-input`, `.net-tag-output`, `.net-tag-power`, `.net-tag-gnd`).
  * Colores HSL curados para diferenciar alimentación (+5V, +12V, 3.3V), tierra (GND), y buses de control.
  * Estilos de ocultación/transición fluida entre modos.

### 3. Lógica JavaScript (`diagrama_ensamble.js`)
* Agregar un estado global `viewMode = 'wires' | 'labels'`.
* Mapear cada pin de los componentes a una etiqueta de red única (`netLabel`).
* En **Modo de Etiquetas**:
  * Ocultar los `<path>` del SVG.
  * Generar y posicionar dinámicamente las etiquetas de puerto (banderas con el nombre de la red) junto a cada pin.
  * Mantener la interactividad: al hacer clic en una etiqueta de pin, resaltar todos los pines del diagrama que compartan la misma red (conectividad lógica).

---

## 🛠️ Modificaciones Específicas por Archivo

### [MODIFY] [diagrama_ensamble.html](file:///d:/WindowsProyects/Antigravity/VeranoInve/verano%20ensamble%20carro/diagrama_ensamble.html)
* Añadir controles de selección de visualización en la barra de herramientas.
* Configurar atributos `data-net` en los divs de pines (`.comp-pin`) para definir las redes lógicas directamente en la estructura HTML.

### [MODIFY] [diagrama_ensamble.css](file:///d:/WindowsProyects/Antigravity/VeranoInve/verano%20ensamble%20carro/diagrama_ensamble.css)
* Añadir el diseño responsivo de las etiquetas de puerto y botones de conmutación.
* Añadir animaciones de fade-in/fade-out para el cambio de modos.

### [MODIFY] [diagrama_ensamble.js](file:///d:/WindowsProyects/Antigravity/VeranoInve/verano%20ensamble%20carro/diagrama_ensamble.js)
* Refactorizar el dibujo para contemplar ambos modos de renderizado.
* Implementar lógica de búsqueda de red común para el resaltado por etiquetas.

---

## 🧪 Plan de Verificación

### Pruebas Manuales
1. Conmutar al **Modo Esquema Puro** y verificar que desaparezcan las líneas de conexión SVG.
2. Comprobar que aparezcan banderas/etiquetas legibles al lado de cada pin con el nombre del puerto/función de red (ej: `GP15`, `5V_VIN`, `GND`).
3. Hacer clic en el pin `5V_VIN` del ESP32 y verificar que resalte la etiqueta de alimentación `5V OUT` del L298N.
4. Ocultar la tabla de conexiones y verificar que el lienzo se redimensione limpiamente ocupando toda la pantalla.


<!-- Plan (Objetivo) Do (Implementacion) -->
