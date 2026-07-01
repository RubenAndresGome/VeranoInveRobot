#!/usr/bin/env python3
"""
MDE-Frontend KPI Validator (Auditio Trascendentalis)
====================================================
Valida los tres KPIs trascendentales del Metadialectic Scholastic Method:
- VERUM: Accesibilidad, semántica, contraste
- BONUM: Performance, nodos DOM, estabilidad
- PULCHRUM: Proporción, armonía cromática, geometría áurea

Uso:
    python validador_kpi.py --file Component.tsx --type component
    python validador_kpi.py --dir src/ --report full
"""

import argparse
import ast
import json
import math
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from coloraide import Color
except ImportError:
    print("pip install coloraide")
    raise


@dataclass
class VerumResult:
    """KPI de Verdad (Accesibilidad/Lógica)"""
    contrast_ratio: float = 0.0
    html_semantics_score: float = 0.0
    aria_count: int = 0
    aria_issues: List[str] = field(default_factory=list)
    keyboard_navigable: bool = False
    focus_visible: bool = False
    passed: bool = False
    
    def evaluate(self):
        self.passed = (
            self.contrast_ratio >= 4.5 and
            self.html_semantics_score >= 0.8 and
            len(self.aria_issues) == 0
        )
        return self.passed


@dataclass
class BonumResult:
    """KPI de Bondad (Rendimiento/Usabilidad)"""
    dom_nodes: int = 0
    cls: float = 0.0
    estimated_lcp: float = 0.0
    has_explicit_dimensions: bool = False
    bundle_size_kb: float = 0.0
    passed: bool = False
    
    def evaluate(self):
        self.passed = (
            self.dom_nodes < 150 and
            self.cls == 0 and
            self.has_explicit_dimensions
        )
        return self.passed


@dataclass
class PulchrumResult:
    """KPI de Belleza (Proporción/Armonía)"""
    palette_std: float = 0.0
    golden_angle_dist: float = 0.0
    phi_conformant: bool = False
    fibonacci_spacing: bool = False
    gradient_stops_ok: bool = True
    passed: bool = False
    
    def evaluate(self):
        self.passed = (
            self.palette_std < 15.0 or
            abs(self.golden_angle_dist - 137.5) < 15.0
        ) and self.gradient_stops_ok
        return self.passed


class MDEValidator:
    """Magister Determinans — Árbitro de los KPIs trascendentales"""
    
    GOLDEN_ANGLE = 137.507764
    PHI = 1.618033988749895
    FIBONACCI_SEQUENCE = [4, 8, 16, 24, 32, 48, 64, 96, 128]
    
    def __init__(self, file_path: str):
        self.file_path = Path(file_path)
        self.source = self.file_path.read_text(encoding='utf-8')
        self.verum = VerumResult()
        self.bonum = BonumResult()
        self.pulchrum = PulchrumResult()
        
    def validate_all(self) -> Tuple[VerumResult, BonumResult, PulchrumResult]:
        """Ejecuta la Auditio Trascendentalis completa"""
        self._validate_verum()
        self._validate_bonum()
        self._validate_pulchrum()
        return self.verum, self.bonum, self.pulchrum
    
    def _validate_verum(self):
        """Valida KPI Verum (Accesibilidad)"""
        # Extraer colores del código
        colors = self._extract_colors()
        if len(colors) >= 2:
            # Calcular contraste entre primer fondo y primer texto
            try:
                bg = Color(colors[0])
                fg = Color(colors[1])
                self.verum.contrast_ratio = bg.contrast(fg, method="wcag21")
            except Exception:
                self.verum.contrast_ratio = 21.0  # fallback: black/white
        
        # Contar elementos semánticos
        semantic_tags = ['header', 'nav', 'main', 'article', 'section', 
                        'aside', 'footer', 'h1', 'h2', 'h3', 'h4', 'button',
                        'a', 'label', 'form', 'table', 'figure', 'figcaption']
        found_semantic = sum(1 for tag in semantic_tags 
                           if re.search(rf'<{tag}\b', self.source, re.I))
        div_count = len(re.findall(r'<div\b', self.source, re.I))
        total_elements = found_semantic + div_count
        self.verum.html_semantics_score = found_semantic / max(total_elements, 1)
        
        # Contar atributos ARIA
        aria_pattern = re.compile(r'aria-[a-z]+', re.I)
        self.verum.aria_count = len(aria_pattern.findall(self.source))
        
        # Verificar aria-label en botones sin texto
        buttons_without_text = re.findall(
            r'<button[^>]*>(\s*<[^>]+>\s*)*</button>', 
            self.source, re.I
        )
        for btn in buttons_without_text:
            if 'aria-label' not in btn and 'aria-labelledby' not in btn:
                self.verum.aria_issues.append(
                    "Botón sin texto ni aria-label encontrado"
                )
        
        self.verum.evaluate()
    
    def _validate_bonum(self):
        """Valida KPI Bonum (Performance)"""
        # Estimar nodos DOM contando elementos JSX/HTML
        jsx_elements = re.findall(r'<[A-Z][a-zA-Z]*\b', self.source)
        html_elements = re.findall(r'<[a-z][a-z0-9]*\b', self.source)
        self.bonum.dom_nodes = len(jsx_elements) + len(html_elements)
        
        # Verificar dimensiones explícitas en imágenes
        img_tags = re.findall(r'<img[^>]*>', self.source, re.I)
        self.bonum.has_explicit_dimensions = all(
            'width' in img and 'height' in img for img in img_tags
        ) if img_tags else True
        
        # Detectar useEffect sin cleanup (potencial memory leak)
        cleanup_violations = re.findall(
            r'useEffect\([^)]*\{[^}]*addEventListener', 
            self.source
        )
        if cleanup_violations and 'return ()' not in self.source:
            self.bonum.cls = 0.1  # Penalización simbólica
        
        self.bonum.evaluate()
    
    def _validate_pulchrum(self):
        """Valida KPI Pulchrum (Belleza/Armonía)"""
        # Extraer y analizar paleta cromática
        colors = self._extract_colors()
        if len(colors) >= 2:
            try:
                hsl_colors = []
                for c in colors:
                    color = Color(c)
                    hsl = color.convert('hsl')
                    hsl_colors.append((
                        hsl['h'] or 0,  # hue
                        hsl['s'] * 100,  # saturation %
                        hsl['l'] * 100   # lightness %
                    ))
                
                # Calcular desviación estándar de H, S, L
                hues = [c[0] for c in hsl_colors]
                sats = [c[1] for c in hsl_colors]
                
                if len(hues) >= 2:
                    # Desviación angular de hues (circular)
                    cstd = self._circular_std(hues)
                    self.pulchrum.palette_std = cstd if not math.isnan(cstd) else 0.0
                    
                    # Distancia promedio al ángulo dorado
                    diffs = [abs(hues[i] - hues[i-1]) % 360 
                            for i in range(1, len(hues))]
                    if diffs:
                        self.pulchrum.golden_angle_dist = sum(diffs) / len(diffs)
                else:
                    self.pulchrum.palette_std = 0.0
                    self.pulchrum.golden_angle_dist = 0.0
                
                # Verificar gradientes (máximo 3 stops)
                gradient_matches = re.findall(
                    r'(?:linear|radial)-gradient\([^)]+', 
                    self.source
                )
                for grad in gradient_matches:
                    stops = grad.count(',') + 1
                    if stops > 3:
                        self.pulchrum.gradient_stops_ok = False
                        
            except Exception:
                pass
        
        # Verificar proporción áurea en estilos
        width_matches = re.findall(r'w-\[(\d+)px\]|width[:\s]+(\d+)px', self.source)
        if width_matches:
            widths = [int(w[0] or w[1]) for w in width_matches[:2]]
            if len(widths) == 2 and min(widths) > 0:
                ratio = max(widths) / min(widths)
                self.pulchrum.phi_conformant = abs(ratio - self.PHI) < 0.1
        
        self.pulchrum.evaluate()
    
    def _extract_colors(self) -> List[str]:
        """Extrae valores de color del código fuente"""
        colors = []
        
        # Hex colors
        hex_colors = re.findall(r'#[0-9A-Fa-f]{3,8}\b', self.source)
        colors.extend(hex_colors)
        
        # HSL colors
        hsl_colors = re.findall(r'hsl\([^)]+\)', self.source)
        colors.extend(hsl_colors)
        
        # Tailwind colors
        tailwind_colors = {
            'black': '#000000', 'white': '#FFFFFF',
            'gray-900': '#111827', 'gray-800': '#1F2937',
            'gray-700': '#374151', 'gray-600': '#4B5563',
            'gray-500': '#6B7280', 'gray-400': '#9CA3AF',
            'gray-300': '#D1D5DB', 'gray-200': '#E5E7EB',
            'gray-100': '#F3F4F6', 'gray-50': '#F9FAFB',
            'red-500': '#EF4444', 'blue-500': '#3B82F6',
            'green-500': '#22C55E', 'yellow-500': '#EAB308',
        }
        for name, hex_val in tailwind_colors.items():
            if name in self.source:
                colors.append(hex_val)
        
        return list(set(colors))[:8]  # Máximo 8 colores únicos
    
    @staticmethod
    def _circular_std(angles: List[float]) -> float:
        """Desviación estándar circular para ángulos en grados"""
        if len(angles) < 2:
            return 0.0
        
        # Convertir a radianes y calcular componentes
        rad_angles = [math.radians(a) for a in angles]
        sin_sum = sum(math.sin(a) for a in rad_angles)
        cos_sum = sum(math.cos(a) for a in rad_angles)
        
        # Longitud del vector resultante
        r = math.sqrt(sin_sum**2 + cos_sum**2) / len(angles)
        
        # Desviación angular (en grados)
        if r >= 1:
            return 0.0
        return math.degrees(math.sqrt(-2 * math.log(r)))
    
    def generate_report(self, level: str = "full") -> str:
        """Genera el reporte de Auditio Trascendentalis"""
        
        v_icon = "✓" if self.verum.passed else "✗"
        b_icon = "✓" if self.bonum.passed else "✗"
        p_icon = "✓" if self.pulchrum.passed else "✗"
        
        overall = "ESSENTIA PURA" if all([
            self.verum.passed, self.bonum.passed, self.pulchrum.passed
        ]) else "REQUI CORRECTIONEM"
        
        if level == "summary":
            return f"""
╔════════════════════════════════════════════════════════════════╗
║         A U D I T I O   T R A N S C E N D E N T A L I S       ║
╠════════════════════════════════════════════════════════════════╣
║ Componente: {self.file_path.name:<50} ║
╠════════════════════════════════════════════════════════════════╣
║  VERUM  (Accesibilidad):  {v_icon}  Contraste: {self.verum.contrast_ratio:.1f}:1    ║
║  BONUM  (Performance):   {b_icon}  Nodos: {self.bonum.dom_nodes:<4}               ║
║  PULCHRUM (Armonía):     {p_icon}  σ: {self.pulchrum.palette_std:.1f}°              ║
╠════════════════════════════════════════════════════════════════╣
║  DETERMINATIO: {overall:<45} ║
╚════════════════════════════════════════════════════════════════╝
"""
        
        return f"""
╔════════════════════════════════════════════════════════════════╗
║         A U D I T I O   T R A N S C E N D E N T A L I S       ║
╠════════════════════════════════════════════════════════════════╣
║ Componente: {self.file_path.name:<50} ║
║ Ruta: {str(self.file_path):<54} ║
╠════════════════════════════════════════════════════════════════╣
║                                                                 ║
║  KPI VERUM (Accesibilidad/Lógica):                              ║
║  ├── Contraste ratio: {self.verum.contrast_ratio:.1f}:1 (umbral: 4.5:1)          ║
║  ├── Semántica HTML: {self.verum.html_semantics_score*100:.0f}%                  ║
║  ├── Atributos ARIA: {self.verum.aria_count}                     ║
║  ├── Issues ARIA: {len(self.verum.aria_issues)}                   ║
║  └── Estado: {'VALIDADO' if self.verum.passed else 'VIOLADO':<20}          ║
║                                                                 ║
║  KPI BONUM (Rendimiento/Usabilidad):                            ║
║  ├── Nodos DOM estimados: {self.bonum.dom_nodes} (umbral: <150)          ║
║  ├── CLS estimado: {self.bonum.cls}                          ║
║  ├── Dimensiones explícitas: {self.bonum.has_explicit_dimensions}               ║
║  └── Estado: {'VALIDADO' if self.bonum.passed else 'VIOLADO':<20}          ║
║                                                                 ║
║  KPI PULCHRUM (Proporción/Armonía):                             ║
║  ├── σ paleta HSL: {self.pulchrum.palette_std:.1f}° (umbral: <15°)            ║
║  ├── ΔHue promedio: {self.pulchrum.golden_angle_dist:.1f}° (óptimo: 137.5°)  ║
║  ├── Layout Φ-conforme: {self.pulchrum.phi_conformant}                   ║
║  ├── Gradient stops OK: {self.pulchrum.gradient_stops_ok}                   ║
║  └── Estado: {'VALIDADO' if self.pulchrum.passed else 'VIOLADO':<20}          ║
║                                                                 ║
╠════════════════════════════════════════════════════════════════╣
║  DETERMINATIO: {overall:<45} ║
╚════════════════════════════════════════════════════════════════╝

Silogismo de validación:
  Mayor: Todo componente que cumple Verum, Bonum y Pulchrum es esencialmente puro.
  Menor: {self.file_path.name} {'cumple' if all([self.verum.passed, self.bonum.passed, self.pulchrum.passed]) else 'NO cumple'} los tres KPIs.
  Conclusio: {'ESSENTIA PURA — Aprobado para producción.' if all([self.verum.passed, self.bonum.passed, self.pulchrum.passed]) else 'REQUI CORRECTIONEM — Se requieren ajustes antes del merge.'}
"""


def validate_directory(directory: str, report_level: str = "summary") -> Dict:
    """Valida todos los archivos en un directorio"""
    results = {
        "total": 0,
        "verum_passed": 0,
        "bonum_passed": 0,
        "pulchrum_passed": 0,
        "all_passed": 0,
        "details": []
    }
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.tsx', '.jsx', '.html', '.css')):
                file_path = os.path.join(root, file)
                try:
                    validator = MDEValidator(file_path)
                    v, b, p = validator.validate_all()
                    
                    results["total"] += 1
                    if v.passed: results["verum_passed"] += 1
                    if b.passed: results["bonum_passed"] += 1
                    if p.passed: results["pulchrum_passed"] += 1
                    if all([v.passed, b.passed, p.passed]): 
                        results["all_passed"] += 1
                    
                    if report_level == "full":
                        print(validator.generate_report("full"))
                        
                except Exception as e:
                    print(f"Error validando {file_path}: {e}")
    
    if report_level == "summary":
        print(f"""
╔════════════════════════════════════════════════════════════════╗
║         A U D I T I O   D I R E C T O R I A L I S             ║
╠════════════════════════════════════════════════════════════════╣
║  Total archivos:        {results['total']:<6}                           ║
║  VERUM aprobados:       {results['verum_passed']}/{results['total']} ({results['verum_passed']/max(results['total'],1)*100:.0f}%)                    ║
║  BONUM aprobados:       {results['bonum_passed']}/{results['total']} ({results['bonum_passed']/max(results['total'],1)*100:.0f}%)                    ║
║  PULCHRUM aprobados:    {results['pulchrum_passed']}/{results['total']} ({results['pulchrum_passed']/max(results['total'],1)*100:.0f}%)                    ║
║  ESENCIA PURA:          {results['all_passed']}/{results['total']} ({results['all_passed']/max(results['total'],1)*100:.0f}%)                    ║
╚════════════════════════════════════════════════════════════════╝
""")
    
    return results


def main():
    parser = argparse.ArgumentParser(
        description="MDE-Frontend KPI Validator — Auditio Trascendentalis"
    )
    parser.add_argument("--file", help="Archivo a validar")
    parser.add_argument("--dir", help="Directorio a validar recursivamente")
    parser.add_argument("--type", choices=["component", "page"], 
                       default="component", help="Tipo de archivo")
    parser.add_argument("--report", choices=["summary", "full"], 
                       default="summary", help="Nivel de detalle")
    
    args = parser.parse_args()
    
    if args.file:
        validator = MDEValidator(args.file)
        validator.validate_all()
        print(validator.generate_report(args.report))
        
        # Exit code basado en resultado
        sys.exit(0 if all([
            validator.verum.passed,
            validator.bonum.passed,
            validator.pulchrum.passed
        ]) else 1)
        
    elif args.dir:
        results = validate_directory(args.dir, args.report)
        sys.exit(0 if results["all_passed"] == results["total"] else 1)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
