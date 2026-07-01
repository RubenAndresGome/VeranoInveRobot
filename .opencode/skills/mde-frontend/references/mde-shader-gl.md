# MDE-Frontend: WebGL/Three.js — Ontología de la Escena 3D

> Tabla de Contenidos:
> 1. Las Cuatro Causas de una Escena R3F
> 2. Ciclo de Vida: Potentia → Actus (Mount/Unmount)
> 3. Shaders como Calculus Ratiocinator
> 4. TSL (Three Shader Language) vs GLSL/WGSL
> 5. Performance: KPI Bonum extendido
> 6. R3F Patterns MDE

---

## 1. Las Cuatro Causas de una Escena R3F

Todo elemento de una escena 3D debe justificarse bajo las cuatro causas:

| Causa | Pregunta Ontológica | Ejemplo en R3F |
|-------|---------------------|----------------|
| **Material** | ¿De qué está hecho? | `<boxGeometry>`, textura KTX2, material PBR |
| **Formal** | ¿Qué estructura tiene? | Grafo de escena: Canvas → Scene → Group → Mesh |
| **Eficiente** | ¿Qué lo actualiza? | `useFrame`, eventos, animaciones, física |
| **Final** | ¿Para qué existe? | Experiencia narrativa, storytelling visual, interacción |

### Silogismo de Escena Completa

```tsx
// SUJETO: Escena de producto (configurator 3D)
//
// CAUSA MATERIAL:
//   - Geometría: GLB comprimido con Draco
//   - Texturas: KTX2/Basis Universal
//   - Iluminación: HDRI environment map
//
// CAUSA FORMAL:
//   - Jerarquía: Canvas > PerspectiveCamera > Scene > Group[meshes]
//   - Orquestación: OrbitControls para navegación
//
// CAUSA EFICIENTE:
//   - useFrame: rotación automática cuando inactivo
//   - onClick: selección de variante de color
//   - onPointerMove: cursor dinámico (grab/grabbing)
//
// CAUSA FINAL:
//   - Permitir al usuario visualizar el producto desde 360°
//   - Métrica: tiempo de interacción > 30s por sesión
//
// Mayor: Toda escena 3D que busca ser percibida debe poseer luz direccional
//        que revele volumen (Causa Formal completa).
// Menor: La escena actual solo tiene luz ambiental (sin sombras).
// Conclusio: Añadir <directionalLight intensity={1.5} position={[5,5,5]} castShadow />
//            + <ambientLight intensity={0.4} /> para relleno.
```

---

## 2. Ciclo de Vida: Potentia → Actus

### Mount (Actualización Ontológica)

```tsx
import { useEffect } from "react";
import { useThree } from "@react-three/fiber";

const ProductMesh = ({ url }: { url: string }) => {
  const { scene } = useGLTF(url);
  const { gl } = useThree();
  
  // En el montaje, la geometría pasa de potentia (GLB en disco) 
  // a actus (buffers GPU activos)
  useEffect(() => {
    console.log("Actus: Geometría actualizada en GPU");
    console.log("Draw calls:", gl.info.render.calls);
    
    // Cleanup: retorno a potentia (liberación de memoria GPU)
    return () => {
      scene.traverse((obj) => {
        if (obj.isMesh) {
          obj.geometry.dispose();
          if (Array.isArray(obj.material)) {
            obj.material.forEach((m) => {
              Object.values(m).forEach((p) => p?.isTexture && p.dispose());
              m.dispose();
            });
          } else {
            Object.values(obj.material).forEach((p) => p?.isTexture && p.dispose());
            obj.material.dispose();
          }
        }
      });
      console.log("Potentia: Recursos GPU liberados");
    };
  }, [scene, gl]);
  
  return <primitive object={scene} />;
};
```

### Instancing (Multiplicatio sine Replicatio)

```tsx
// Mayor: Todo objeto que se repite >10x debe compartir geometría 
//        y material (InstancedMesh) para no multiplicar draw calls.
// Menor: Hay 500 árboles en la escena.
// Conclusio: Usar <InstancedMesh> con matriz de posiciones.

import { InstancedMesh, useGLTF } from "@react-three/drei";
import { useMemo, useRef } from "react";
import * as THREE from "three";

const Forest = ({ count = 500 }: { count: number }) => {
  const meshRef = useRef<THREE.InstancedMesh>(null);
  const { nodes } = useGLTF("/tree.glb");
  
  // 500 posiciones generadas proceduralmente (no repetición ontológica)
  const positions = useMemo(() => {
    const pos = [];
    for (let i = 0; i < count; i++) {
      pos.push([
        (Math.random() - 0.5) * 200,  // x
        0,                               // y
        (Math.random() - 0.5) * 200,  // z
      ]);
    }
    return pos;
  }, [count]);
  
  useEffect(() => {
    if (!meshRef.current) return;
    
    const matrix = new THREE.Matrix4();
    positions.forEach(([x, y, z], i) => {
      matrix.setPosition(x, y, z);
      meshRef.current!.setMatrixAt(i, matrix);
    });
    meshRef.current.instanceMatrix.needsUpdate = true;
  }, [positions]);
  
  // 1 draw call para 500 árboles (vs 500 draw calls individuales)
  return (
    <instancedMesh ref={meshRef} args={[nodes.Tree.geometry, nodes.Tree.material, count]}>
      {/* La geometría y material se comparten (causa formal única) */}
    </instancedMesh>
  );
};
```

---

## 3. Shaders como Calculus Ratiocinator

Los shaders son deducciones matemáticas ejecutadas por el GPU para cada píxel o vértice.

### Shader Minimalista (Determinatio Negra)

```glsl
// Premisa Mayor: Todo vértice en el espacio de clip se proyecta mediante 
//                la matriz MVP (Model-View-Projection).
// Premisa Menor: position es el atributo del vértice en espacio local.
// Conclusio: gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);

// Vertex Shader
void main() {
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}

// Fragment Shader
// Premisa Mayor: El color del píxel debe ser negro puro cuando no hay iluminación.
// Premisa Menor: No se pasan uniforms de iluminación.
// Conclusio: gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
void main() {
  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
```

### Shader con Iluminación Lambertiana

```glsl
// Vertex Shader
varying vec3 vNormal;
varying vec3 vPosition;

void main() {
  vNormal = normalize(normalMatrix * normal);
  vPosition = (modelViewMatrix * vec4(position, 1.0)).xyz;
  gl_Position = projectionMatrix * vec4(vPosition, 1.0);
}

// Fragment Shader
// Premisa Mayor: La reflectancia difusa sigue la ley de Lambert: 
//                I = I0 * max(dot(N, L), 0).
// Premisa Menor: vNormal es la normal interpolada; lightDir es el vector 
//                desde el fragmento hacia la fuente de luz.
// Conclusio: float intensity = max(dot(vNormal, normalize(lightDir)), 0.0);

uniform vec3 lightDir;
uniform vec3 color;
uniform float ambientIntensity;

varying vec3 vNormal;

void main() {
  float lambert = max(dot(vNormal, normalize(lightDir)), 0.0);
  vec3 ambient = color * ambientIntensity;
  vec3 diffuse = color * lambert * (1.0 - ambientIntensity);
  gl_FragColor = vec4(ambient + diffuse, 1.0);
}
```

### Shader Procedural: Onda Sinusoidal

```glsl
// Premisa Mayor: Todo vértice que representa agua debe oscilar 
//                periódicamente según el tiempo.
// Premisa Menor: position.y = 0 (plano base); uTime avanza cada frame.
// Conclusio: float wave = sin(position.x * 2.0 + uTime) * 0.1 
//                       + cos(position.z * 1.5 + uTime) * 0.05;

uniform float uTime;
varying vec2 vUv;

void main() {
  vUv = uv;
  vec3 pos = position;
  
  float wave = sin(pos.x * 2.0 + uTime) * 0.1 
             + cos(pos.z * 1.5 + uTime * 0.5) * 0.05;
  pos.y += wave;
  
  gl_Position = projectionMatrix * modelViewMatrix * vec4(pos, 1.0);
}
```

---

## 4. TSL (Three Shader Language)

TSL es el *characteristica universalis* de Leibniz aplicada a shaders: un lenguaje que se compila a GLSL (WebGL) o WGSL (WebGPU) automáticamente.

### Ejemplo TSL: Material Fresnel

```tsx
import { Fn, float, vec3, normalWorld, viewDirection } from "three/tsl";

// Premisa Mayor: El efecto Fresnel describe que la reflectancia es mayor 
//                en ángulos de visión oblicuos.
// Premisa Menor: normalWorld y viewDirection son vectores unitarios en TSL.
// Conclusio: intensity = pow(1.0 - dot(N, V), power)

const fresnel = Fn(([power]) => {
  const dotNV = normalWorld.dot(viewDirection).saturate();
  return float(1).sub(dotNV).pow(power);
});

// Uso en material
const material = new THREE.MeshStandardNodeMaterial();
material.emissiveNode = fresnel(3.0).mul(new THREE.Color(0x4488ff));
```

### Ejemplo TSL: Ruido Procedural

```tsx
import { mx_noise_float, mx_fractal_noise_float, positionLocal } from "three/tsl";

// Premisa Mayor: La naturaleza exhibe patrones fractales que pueden 
//                aproximarse con ruido Perlin multi-octava.
// Premisa Menor: positionLocal varía por vértice; scale controla frecuencia.
// Conclusio: displacement = fractal_noise(positionLocal * scale, octaves, lacunarity, gain)

const noiseMaterial = Fn(({ scale, octaves, lacunarity, gain, displacement }) => {
  const fbm = mx_fractal_noise_float(positionLocal.mul(scale), octaves, lacunarity, gain);
  return fbm.mul(displacement);
});

material.positionNode = positionLocal.add(
  normalLocal.mul(noiseMaterial({ scale: 5, octaves: 4, lacunarity: 2, gain: 0.5, displacement: 0.2 }))
);
```

### TSL vs GLSL crudo: Decisión Formal

| Criterio | TSL | GLSL/WGSL crudo |
|----------|-----|-----------------|
| Cross-plataforma | WebGL + WebGPU | Solo uno |
| Uniforms | Automáticos | Manuales |
| Type safety | Tipado por nodo | String crudo |
| Composición | Funciones Fn reutilizables | Copy-paste |
| Curva de aprendizaje | Media | Alta |
| Control fino | Bueno | Máximo |

**Silogismo de decisión:**
```
Mayor: Si el proyecto debe soportar WebGL y WebGPU, se requiere abstracción 
       que compile a ambos targets.
Menor: TSL compila automáticamente a GLSL o WGSL según el renderer.
Conclusio: Usar TSL salvo que se requiera optimización de shader específica 
           para una plataforma.
```

---

## 5. Performance: KPI Bonum Extendido

### Métricas 3D

| KPI | Umbral | Herramienta | Acción si falla |
|-----|--------|-------------|-----------------|
| Draw calls | < 100/frame | `renderer.info.render.calls` | Instancing, BatchedMesh |
| VRAM texturas | < 512MB | `renderer.info.memory.textures` | KTX2, resize, atlas |
| VRAM geometría | < 256MB | `renderer.info.memory.geometries` | Draco, LOD, simplificación |
| FPS | ≥ 60 | `r3f-perf`, `stats-gl` | Optimizar shaders, reducir luces |
| LCP (3D como hero) | < 2.5s | Lighthouse | Preload, placeholder 2D |

### Reglas de Performance

```tsx
// 1. Preload antes del render (potentia preparada)
useGLTF.preload("/model.glb");
useTexture.preload("/texture.ktx2");

// 2. Memoizar componentes costosos (evitar re-render innecesario)
const ExpensiveModel = React.memo(({ url }: { url: string }) => {
  const { scene } = useGLTF(url);
  return <primitive object={scene} />;
});

// 3. Toggle visibility en vez de unmount (conservar buffers GPU)
// ❌ Mal: recrea buffers cada vez
{showModel && <Model />}

// ✅ Bien: conserva buffers, solo cambia visibilidad
<Model visible={showModel} />

// 4. LOD (Level of Detail) para modelos complejos
import { Detailed } from "@react-three/drei";

<Detailed distances={[0, 20, 50]}>
  <HighPolyModel />    // 50k triángulos, distancia 0-20
  <MediumPolyModel />  // 10k triángulos, distancia 20-50  
  <LowPolyModel />     // 1k triángulos, distancia 50+
</Detailed>

// 5. Memoria: disposición explícita en cleanup
useEffect(() => {
  const geometry = new THREE.BoxGeometry();
  const material = new THREE.MeshStandardMaterial();
  const mesh = new THREE.Mesh(geometry, material);
  scene.add(mesh);
  
  return () => {
    // Retorno a potentia: liberar memoria GPU
    geometry.dispose();
    material.dispose();
    // Texturas también
    if (material.map) material.map.dispose();
    scene.remove(mesh);
  };
}, []);
```

---

## 6. R3F Patterns MDE

### Canvas Configuration (Causa Eficiente Primera)

```tsx
// Mayor: El Canvas es el ente primordial que actualiza todos los demás 
//        a través del render loop.
// Menor: La escena requiere sombras, post-processing y tonemapping.
// Conclusio: Configurar Canvas con todas las causas eficientes.

import { Canvas } from "@react-three/fiber";
import { EffectComposer, Bloom, ToneMapping } from "@react-three/postprocessing";

<Canvas
  shadows                    // Causa Formal: sistema de sombras
  camera={{ position: [0, 2, 5], fov: 50 }}  // Causa Material: cámara
  gl={{ 
    antialias: true,         // KPI Pulchrum: bordes suaves
    toneMapping: THREE.ACESFilmicToneMapping,  // KPI Pulchrum: color fiel
    toneMappingExposure: 1.2 
  }}
  dpr={[1, 2]}               // KPI Bonum: device pixel ratio adaptativo
>
  <Scene />                  {/* Causa Formal: contenido */}
  
  <EffectComposer>           {/* Causa Eficiente: post-processing */}
    <Bloom intensity={0.5} luminanceThreshold={0.9} />
    <ToneMapping />
  </EffectComposer>
</Canvas>
```

### Eventos como Causa Eficiente

```tsx
// Mayor: Toda interacción del usuario es una Causa Eficiente que debe 
//        actualizar el estado de la escena de manera determinista.
// Menor: El usuario hace hover sobre un producto 3D.
// Conclusio: Escalar el objeto y cambiar el cursor.

const InteractiveProduct = () => {
  const [hovered, setHovered] = useState(false);
  const meshRef = useRef<THREE.Mesh>(null);
  
  useFrame((state, delta) => {
    if (!meshRef.current) return;
    
    // Interpolación suave (lerp) para actus gradual
    const targetScale = hovered ? 1.1 : 1.0;
    meshRef.current.scale.lerp(
      new THREE.Vector3(targetScale, targetScale, targetScale),
      delta * 10  // velocidad de transición
    );
    
    // Rotación sutil continua cuando no hay hover (Causa Final: exploración)
    if (!hovered) {
      meshRef.current.rotation.y += delta * 0.2;
    }
  });
  
  return (
    <mesh
      ref={meshRef}
      onPointerOver={(e) => {
        e.stopPropagation();
        setHovered(true);
        document.body.style.cursor = "pointer";
      }}
      onPointerOut={(e) => {
        e.stopPropagation();
        setHovered(false);
        document.body.style.cursor = "auto";
      }}
      castShadow
      receiveShadow
    >
      <boxGeometry />
      <meshStandardMaterial color={hovered ? "#4488ff" : "#222222"} />
    </mesh>
  );
};
```

### Animaciones Declarativas con Framer Motion 3D

```tsx
// Mayor: Todo movimiento en la escena 3D debe ser suave y continuo 
//        (C1 continuidad del cálculo).
// Menor: El componente debe entrar con escala desde 0 y rotación.
// Conclusio: Usar motion.mesh de Framer Motion 3D.

import { motion } from "framer-motion-3d";

<motion.mesh
  initial={{ scale: 0, rotateY: 0 }}
  animate={{ scale: 1, rotateY: Math.PI * 2 }}
  transition={{ 
    duration: 1.2, 
    ease: [0.4, 0, 0.2, 1],  // cubic-bezier: suavidad garantizada
    delay: 0.3 
  }}
  castShadow
>
  <sphereGeometry args={[1, 64, 64]} />
  <meshStandardMaterial color="#hsl(210,45%,55%)" metalness={0.3} roughness={0.4} />
</motion.mesh>
```

### Suspense como Potentia (Estado de Carga)

```tsx
// Mayor: Mientras un recurso 3D pasa de potentia a actus, 
//        debe mostrarse un sustituto ontológico (skeleton/placeholder).
// Menor: useGLTF es asíncrono.
// Conclusio: Envolver en Suspense con fallback.

import { Suspense } from "react";
import { Html, useProgress } from "@react-three/drei";

const Loader = () => {
  const { progress } = useProgress();
  return (
    <Html center>
      <div className="text-white text-lg font-medium">
        {Math.round(progress)}% — Actualizando ente...
      </div>
    </Html>
  );
};

// Uso:
<Suspense fallback={<Loader />}>
  <ProductMesh url="/product.glb" />
  <Environment preset="city" />
</Suspense>
```
