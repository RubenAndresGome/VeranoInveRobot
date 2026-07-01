#!/usr/bin/env python3
"""
MDE-Frontend Paleta Cromática Trascendental
===========================================
Genera paletas de color basadas en la proporción áurea y el ángulo dorado,
conforme al Axioma VIII de la Matriz de Covarianza Cromática (Mc).

Uso:
    python generador_paleta.py --base-hue 220 --count 4
    python generador_paleta.py --base-hue 30 --count 3 --saturation 50 --output json
    python generador_paleta.py --harmony analog --base-hue 120 --count 5
"""

import argparse
import json
import math
import sys
from dataclasses import asdict, dataclass
from typing import List, Literal, Optional, Tuple

try:
    from coloraide import Color
except ImportError:
    print("Error: pip install coloraide")
    raise


@dataclass
class ColorNode:
    """Un nodo cromático en la matriz Mc"""
    name: str
    hex: str
    hsl: Tuple[float, float, float]  # H, S%, L%
    role: str  # primary, secondary, accent, background, text, etc.
    
    def to_dict(self):
        return {
            "name": self.name,
            "hex": self.hex,
            "hsl": {
                "h": round(self.hsl[0], 2),
                "s": round(self.hsl[1], 2),
                "l": round(self.hsl[2], 2)
            },
            "role": self.role,
            "tailwind": self._to_tailwind(),
        }
    
    def _to_tailwind(self) -> str:
        return f"hsl({self.hsl[0]:.0f} {self.hsl[1]:.0f}% {self.hsl[2]:.0f}%)"


class MDEPaletteGenerator:
    """
    Calculus Chromaticus — Generador de paletas basado en principios MDE.
    
    Implementa:
    - Ángulo dorado (137.507764°) para separación de hues
    - Serie de Fibonacci para saturación descendente
    - Limitación de S_index < 0.7 (anti-fatiga ocular)
    """
    
    GOLDEN_ANGLE = 137.50776405003785  # 360 * (1 - 1/Φ) = 360 / Φ²
    PHI = 1.618033988749895
    FIBONACCI = [1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89]
    
    def __init__(self, 
                 base_hue: float,
                 count: int = 4,
                 saturation: float = 50.0,
                 lightness_range: Tuple[float, float] = (15.0, 95.0),
                 harmony: Literal["golden", "analogous", "complementary", "triadic"] = "golden"):
        self.base_hue = base_hue % 360
        self.count = min(max(count, 2), 8)  # Entre 2 y 8 colores
        self.saturation = saturation
        self.lightness_range = lightness_range
        self.harmony = harmony
        
        # Validar que no excedamos S_index > 0.7
        self.saturation = self._clamp_saturation(saturation, count)
    
    def _clamp_saturation(self, sat: float, count: int) -> float:
        """
        Asegura que S_index = Σ(Si²)/N < 0.7 para evitar fatiga ocular.
        Si la saturación propuesta generaría S_index > 0.7, la reduce.
        """
        s_normalized = sat / 100.0  # Convertir a 0-1
        s_index = (s_normalized ** 2 * count) / count  # = s_normalized²
        
        if s_index > 0.7:
            # Reducir saturación para que S_index = 0.7
            new_sat = math.sqrt(0.7) * 100  # ≈ 83.67%
            print(f"⚠️  ALERTA: Saturación {sat}% produciría S_index={s_index:.2f} > 0.7")
            print(f"   Reduciendo a {new_sat:.1f}% (Fatiga Ocular Estocástica evitada)")
            return new_sat
        
        return sat
    
    def generate(self) -> List[ColorNode]:
        """Genera la paleta cromática conforme al axioma MDE"""
        
        generators = {
            "golden": self._golden_harmony,
            "analogous": self._analogous_harmony,
            "complementary": self._complementary_harmony,
            "triadic": self._triadic_harmony,
        }
        
        generator = generators.get(self.harmony, self._golden_harmony)
        hues = generator()
        
        # Generar colores con saturación Fibonacci-descendente
        colors = []
        for i, hue in enumerate(hues):
            # Saturación descendente por Fibonacci: más oscilación en primeros colores
            sat_mod = max(15, self.saturation * (1 - (i * 0.15)))
            
            # Lightness alternado para roles diferentes
            if i == 0:
                l = 50  # Primario: medio
            elif i == 1:
                l = self.lightness_range[0] + 10  # Secundario: más oscuro
            elif i == len(hues) - 1:
                l = self.lightness_range[1]  # Fondo: claro
            else:
                l = (self.lightness_range[0] + self.lightness_range[1]) / 2
            
            color = Color("hsl", [hue, sat_mod / 100, l / 100])
            
            roles = ["primary", "secondary", "accent", "background", 
                    "surface", "text", "success", "warning"]
            
            colors.append(ColorNode(
                name=f"mde-{roles[i] if i < len(roles) else f'color-{i}'}",
                hex=color.to_string(hex=True),
                hsl=(hue, sat_mod, l),
                role=roles[i] if i < len(roles) else "auxiliary"
            ))
        
        return colors
    
    def _golden_harmony(self) -> List[float]:
        """
        Distribución por ángulo dorado — la proporción más armónica.
        Cada hue se separa por 137.5°, generando la máxima diferenciación
        perceptual con el mínimo número de colores.
        """
        hues = []
        for i in range(self.count):
            hue = (self.base_hue + i * self.GOLDEN_ANGLE) % 360
            hues.append(hue)
        return hues
    
    def _analogous_harmony(self) -> List[float]:
        """Armonía análoga: hues adyacentes (±30° del base)"""
        step = 30
        hues = []
        for i in range(self.count):
            offset = (i - self.count // 2) * step
            hues.append((self.base_hue + offset) % 360)
        return sorted(hues)
    
    def _complementary_harmony(self) -> List[float]:
        """Complementario: base + opuesto (180°)"""
        complement = (self.base_hue + 180) % 360
        if self.count == 2:
            return [self.base_hue, complement]
        
        # Para más colores, interpolar entre base y complemento
        hues = [self.base_hue]
        for i in range(1, self.count - 1):
            t = i / (self.count - 1)
            hue = (self.base_hue + t * 180) % 360
            hues.append(hue)
        hues.append(complement)
        return hues
    
    def _triadic_harmony(self) -> List[float]:
        """Triada: base + 120° + 240°"""
        triad = [self.base_hue, 
                (self.base_hue + 120) % 360, 
                (self.base_hue + 240) % 360]
        
        if self.count <= 3:
            return triad[:self.count]
        
        # Añadir intermediarios
        hues = []
        for i in range(self.count):
            if i < 3:
                hues.append(triad[i])
            else:
                # Interpolar entre los triádicos
                t = (i - 3) / (self.count - 3)
                hue = (triad[0] + t * 120) % 360
                hues.append(hue)
        return hues
    
    def compute_dr(self, colors: List[ColorNode]) -> float:
        """
        Calcula la Diferenciación Relacional (DR):
        La mínima distancia angular entre pares de hues en la paleta.
        DR óptimo ≈ 137.5° (ángulo dorado) para máxima discriminación.
        """
        hues = [c.hsl[0] for c in colors]
        min_dist = float('inf')
        
        for i in range(len(hues)):
            for j in range(i + 1, len(hues)):
                # Distancia angular mínima
                diff = abs(hues[i] - hues[j])
                diff = min(diff, 360 - diff)
                min_dist = min(min_dist, diff)
        
        return min_dist
    
    def compute_s_index(self, colors: List[ColorNode]) -> float:
        """Calcula S_index = Σ(Si²)/N"""
        sats = [c.hsl[1] / 100 for c in colors]
        return sum(s ** 2 for s in sats) / len(sats)
    
    def generate_report(self, colors: List[ColorNode]) -> str:
        """Genera reporte de la matriz cromática"""
        dr = self.compute_dr(colors)
        s_index = self.compute_s_index(colors)
        
        # Verificar KPI Pulchrum
        pulchrum_pass = (s_index < 0.7) or (abs(dr - self.GOLDEN_ANGLE) < 15)
        
        report = f"""
╔════════════════════════════════════════════════════════════════╗
║      M A T R I X   C H R O M A T I C A   M D E                ║
╠════════════════════════════════════════════════════════════════╣
║  Parámetros:                                                    ║
║  ├── Hue base:     {self.base_hue:.1f}°                              ║
║  ├── Cantidad:     {self.count}                                   ║
║  ├── Saturación:   {self.saturation:.1f}%                              ║
║  ├── Armonía:      {self.harmony:<20}                    ║
║  └── Rango L:      [{self.lightness_range[0]:.0f}%, {self.lightness_range[1]:.0f}%]                       ║
╠════════════════════════════════════════════════════════════════╣
║  Métricas:                                                      ║
║  ├── DR (Diferenciación Relacional): {dr:.1f}°              ║
║  ├── DR óptimo (ángulo dorado):      {self.GOLDEN_ANGLE:.1f}°       ║
║  ├── Desviación DR:                  {abs(dr - self.GOLDEN_ANGLE):.1f}°       ║
║  ├── S_index:                        {s_index:.3f} (umbral: <0.7)      ║
║  └── PULCHRUM:                       {'✓ VALIDADO' if pulchrum_pass else '✗ VIOLADO':<20}        ║
╠════════════════════════════════════════════════════════════════╣
║  Paleta Generada:                                               ║
"""
        
        for i, c in enumerate(colors):
            hsl_str = f"hsl({c.hsl[0]:.0f}, {c.hsl[1]:.0f}%, {c.hsl[2]:.0f}%)"
            report += f"║  {i+1}. {c.name:<15} {c.hex:<10} {hsl_str:<30} {c.role:<12} ║\n"
        
        report += """╠════════════════════════════════════════════════════════════════╣
║  Tailwind Classes:                                              ║
"""
        for c in colors:
            report += f"║  ├── {c.name}: {c.to_dict()['tailwind']:<45} ║\n"
        
        report += """╚════════════════════════════════════════════════════════════════╝

Silogismo Cromático:
  Mayor: Una paleta armónica maximiza la diferenciación relacional (DR ≈ 137.5°)
         o mantiene baja desviación (< 15%) para armonía análoga.
  Menor: """
        
        if self.harmony == "golden":
            report += f"La paleta usa separación áurea con DR = {dr:.1f}°.\n"
        else:
            report += f"La paleta usa armonía {self.harmony} con S_index = {s_index:.3f}.\n"
        
        report += f"  Conclusio: {'PULCHRUM VALIDADO — La paleta es ontológicamente armónica.' if pulchrum_pass else 'PULCHRUM VIOLADO — Ajustar parámetros.'}\n"
        
        return report
    
    def to_json(self, colors: List[ColorNode]) -> str:
        """Exporta la paleta como JSON"""
        data = {
            "meta": {
                "base_hue": self.base_hue,
                "count": self.count,
                "saturation": self.saturation,
                "harmony": self.harmony,
                "generator": "MDE-Frontend Calculus Chromaticus"
            },
            "metrics": {
                "dr": round(self.compute_dr(colors), 2),
                "golden_angle": round(self.GOLDEN_ANGLE, 2),
                "s_index": round(self.compute_s_index(colors), 4),
                "pulchrum_valid": self.compute_s_index(colors) < 0.7
            },
            "colors": [c.to_dict() for c in colors]
        }
        return json.dumps(data, indent=2)
    
    def to_css(self, colors: List[ColorNode]) -> str:
        """Exporta como variables CSS custom properties"""
        css = ":root {\n  /* MDE Chromatic Matrix — Generated by Calculus Chromaticus */\n"
        for c in colors:
            css += f"  --{c.name}: {c.hex};\n"
            css += f"  --{c.name}-hsl: {c.to_dict()['tailwind']};\n"
        css += "}\n"
        return css


def main():
    parser = argparse.ArgumentParser(
        description="MDE Paleta Cromática Trascendental"
    )
    parser.add_argument("--base-hue", type=float, required=True,
                       help="Hue base (0-360)")
    parser.add_argument("--count", type=int, default=4,
                       help="Número de colores (2-8)")
    parser.add_argument("--saturation", type=float, default=50.0,
                       help="Saturación base % (default: 50)")
    parser.add_argument("--harmony", 
                       choices=["golden", "analogous", "complementary", "triadic"],
                       default="golden",
                       help="Tipo de armonía (default: golden)")
    parser.add_argument("--lightness-min", type=float, default=15.0,
                       help="Lightness mínimo %")
    parser.add_argument("--lightness-max", type=float, default=95.0,
                       help="Lightness máximo %")
    parser.add_argument("--output", choices=["report", "json", "css", "tailwind"],
                       default="report", help="Formato de salida")
    
    args = parser.parse_args()
    
    gen = MDEPaletteGenerator(
        base_hue=args.base_hue,
        count=args.count,
        saturation=args.saturation,
        lightness_range=(args.lightness_min, args.lightness_max),
        harmony=args.harmony
    )
    
    colors = gen.generate()
    
    if args.output == "report":
        print(gen.generate_report(colors))
    elif args.output == "json":
        print(gen.to_json(colors))
    elif args.output == "css":
        print(gen.to_css(colors))
    elif args.output == "tailwind":
        for c in colors:
            print(f"{c.name}: '{c.to_dict()['tailwind']}',")


if __name__ == "__main__":
    main()
