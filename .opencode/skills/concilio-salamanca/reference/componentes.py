"""
Ejemplos concretos de componentes frontend para referencia de los agentes del Concilio.
Cada componente incluye: especificacion, implementacion correcta, y checklist de auditoria.
"""

from dataclasses import dataclass, field
from typing import List, Optional


@dataclass
class ComponentSpec:
    nombre: str
    categoria: str
    descripcion: str
    props: List[str]
    estados: List[str]
    checklist: List[str]
    ejemplo: str
    anti_patrones_relacionados: List[str] = field(default_factory=list)


COMPONENTES: List[ComponentSpec] = [
    ComponentSpec(
        nombre="Button (Primario)",
        categoria="atomos",
        descripcion="Boton de accion principal. Dispara una operacion asincrona y muestra feedback visual.",
        props=["label", "onClick", "variant ('primary'|'secondary'|'danger')", "disabled", "loading", "size ('sm'|'md'|'lg')"],
        estados=["idle", "hover", "active", "focus-visible", "loading", "disabled", "success", "error"],
        checklist=[
            "Muestra spinner y deshabilita el boton durante loading",
            "El texto es accesible (contraste >= 4.5:1)",
            "Area de click >= 44x44px (WCAG)",
            "Tiene focus ring visible con :focus-visible",
            "Previene doble submit via disabled + debounce",
            "Emite evento una sola vez por click",
            "Soporta keyboard (Enter/Space)",
            "Role='button' si no es <button> nativo",
        ],
        ejemplo='''// Button.tsx
interface ButtonProps {
  label: string;
  onClick: () => Promise<void> | void;
  variant?: "primary" | "secondary" | "danger";
  disabled?: boolean;
  size?: "sm" | "md" | "lg";
}

export function Button({ label, onClick, variant = "primary", disabled, size = "md" }: ButtonProps) {
  const [loading, setLoading] = useState(false);

  const handleClick = async () => {
    if (loading || disabled) return;
    setLoading(true);
    try {
      await onClick();
    } finally {
      setLoading(false);
    }
  };

  return (
    <button
      className={`btn btn-${variant} btn-${size}`}
      onClick={handleClick}
      disabled={disabled || loading}
      aria-busy={loading}
    >
      {loading ? <Spinner size={size} /> : label}
    </button>
  );
}''',
        anti_patrones_relacionados=["AP-014"],
    ),
    ComponentSpec(
        nombre="Modal (Dialogo)",
        categoria="moleculas",
        descripcion="Ventana de dialogo superpuesta que interrumpe el flujo para una accion confirmatoria.",
        props=["open", "onClose", "title", "children", "footer", "size ('sm'|'md'|'lg')", "closeOnEscape", "closeOnOverlay"],
        estados=["closed", "opening", "open", "closing"],
        checklist=[
            "Renderiza en un portal (createPortal) para evitar stacking context",
            "Trap de foco dentro del modal (solo elementos del modal reciben foco)",
            "Restaura foco al elemento que lo abrio al cerrar",
            "Escape cierra el modal",
            "Click fuera del modal cierra (overlay click)",
            "Overlay con backdrop blur o semitransparente",
            "Previene scroll del body (overflow: hidden en body)",
            "Transiciones de entrada/salida (opacity + transform)",
            "ARIA: role='dialog', aria-modal='true', aria-labelledby",
            "z-index via sistema de capas, no hardcodeado",
        ],
        ejemplo='''// Modal.tsx
import { createPortal } from "react-dom";
import { useEffect, useRef } from "react";

interface ModalProps {
  open: boolean;
  onClose: () => void;
  title: string;
  children: React.ReactNode;
  footer?: React.ReactNode;
}

export function Modal({ open, onClose, title, children, footer }: ModalProps) {
  const overlayRef = useRef<HTMLDivElement>(null);
  const previousFocus = useRef<HTMLElement | null>(null);

  useEffect(() => {
    if (open) {
      previousFocus.current = document.activeElement as HTMLElement;
      document.body.style.overflow = "hidden";
    } else {
      document.body.style.overflow = "";
      previousFocus.current?.focus();
    }
    return () => { document.body.style.overflow = ""; };
  }, [open]);

  useEffect(() => {
    const handler = (e: KeyboardEvent) => {
      if (e.key === "Escape") onClose();
    };
    if (open) document.addEventListener("keydown", handler);
    return () => document.removeEventListener("keydown", handler);
  }, [open, onClose]);

  if (!open) return null;

  return createPortal(
    <div
      ref={overlayRef}
      className="modal-overlay"
      onClick={(e) => { if (e.target === overlayRef.current) onClose(); }}
      role="presentation"
    >
      <div className="modal-content" role="dialog" aria-modal="true" aria-labelledby="modal-title">
        <header>
          <h2 id="modal-title">{title}</h2>
          <button onClick={onClose} aria-label="Cerrar">&times;</button>
        </header>
        <main>{children}</main>
        {footer && <footer>{footer}</footer>}
      </div>
    </div>,
    document.body
  );
}''',
        anti_patrones_relacionados=["AP-008"],
    ),
    ComponentSpec(
        nombre="Tabla de Datos (DataTable)",
        categoria="organismos",
        descripcion="Tabla paginada, ordenable y filtrable con columnas configurables para visualizar datos tabulares.",
        props=["columns", "data", "pagination", "sorting", "rowSelection", "loading", "emptyMessage"],
        estados=["loading", "empty", "error", "loaded", "sorting", "filtering"],
        checklist=[
            "Paginacion del lado del servidor para datasets > 100 registros",
            "Ordenamiento por columna con indicador visual (asc/desc)",
            "Filtros por columna con debounce de 300ms",
            "Estado de carga con skeleton o spinner",
            "Estado vacio con mensaje descriptivo e icono",
            "Estado de error con boton de reintento",
            "Filas alternadas (zebra striping) para legibilidad",
            "Sticky header al hacer scroll vertical",
            "Seleccion multiple con checkbox en cada fila",
            "Accesibilidad: th scope='col', caption, aria-sort",
            "Responsive: scroll horizontal en mobile, columnas colapsables",
            "Virtualizacion para > 1000 filas (react-window)",
        ],
        ejemplo='''// DataTable.tsx
import { useQuery } from "@tanstack/react-query";

interface Column<T> {
  key: keyof T;
  header: string;
  sortable?: boolean;
  render?: (value: T[keyof T], row: T) => React.ReactNode;
}

interface DataTableProps<T> {
  columns: Column<T>[];
  fetchData: (params: { page: number; sort?: string; order?: "asc" | "desc" }) => Promise<{ data: T[]; total: number }>;
}

export function DataTable<T extends Record<string, unknown>>({ columns, fetchData }: DataTableProps<T>) {
  const [page, setPage] = useState(0);
  const [sort, setSort] = useState<{ key: string; order: "asc" | "desc" } | null>(null);

  const { data, isLoading, isError, refetch } = useQuery({
    queryKey: ["table", page, sort],
    queryFn: () => fetchData({ page, sort: sort?.key, order: sort?.order }),
  });

  const handleSort = (key: string) => {
    setSort(prev => prev?.key === key
      ? { key, order: prev.order === "asc" ? "desc" : "asc" }
      : { key, order: "asc" }
    );
  };

  if (isLoading) return <TableSkeleton columns={columns.length} rows={10} />;
  if (isError) return <ErrorState message="Error al cargar datos" onRetry={refetch} />;
  if (!data?.data.length) return <EmptyState message="No se encontraron resultados" />;

  return (
    <div className="table-container">
      <table>
        <thead>
          <tr>
            {columns.map(col => (
              <th key={String(col.key)} onClick={() => col.sortable && handleSort(String(col.key))}>
                {col.header}
                {sort?.key === col.key && (sort.order === "asc" ? " \u2191" : " \u2193")}
              </th>
            ))}
          </tr>
        </thead>
        <tbody>
          {data.data.map((row, i) => (
            <tr key={i}>
              {columns.map(col => (
                <td key={String(col.key)}>
                  {col.render ? col.render(row[col.key], row) : String(row[col.key])}
                </td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
      <Pagination page={page} total={data.total} onChange={setPage} />
    </div>
  );
}''',
        anti_patrones_relacionados=["AP-011"],
    ),
    ComponentSpec(
        nombre="Dashboard de Metricas",
        categoria="plantillas",
        descripcion="Panel de control con KPIs, graficos y widgets interactivos que resume el estado del sistema.",
        props=["widgets", "refreshInterval", "timeRange", "layout", "theme"],
        estados=["loading", "partial_error", "stale", "live"],
        checklist=[
            "Cada widget es independiente: el fallo de uno no rompe el dashboard completo",
            "Error boundaries por widget con fallback individual",
            "Refresco via React Query/SWR, no setInterval manual",
            "Skeleton loading especifico por tipo de widget (grafico, numero, tabla)",
            "Stale-while-revalidate: mostrar datos viejos mientras se refresca",
            "Layout responsive: grid adaptable con CSS Grid + media queries",
            "Los widgets de tipo numero muestran delta (% cambio) y sparkline",
            "Exportacion de datos (CSV, PDF) por widget",
            "Filtro global de rango de tiempo que afecta a todos los widgets",
            "Cada widget tiene su propio AbortController para cancelar requests obsoletas",
        ],
        ejemplo='''// Dashboard.tsx
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { ErrorBoundary } from "react-error-boundary";

const queryClient = new QueryClient({
  defaultOptions: {
    queries: {
      staleTime: 30_000,
      refetchInterval: 60_000,
      retry: 2,
    },
  },
});

function WidgetFallback({ error, resetErrorBoundary }: { error: Error; resetErrorBoundary: () => void }) {
  return (
    <div className="widget widget-error">
      <p>Error al cargar widget</p>
      <button onClick={resetErrorBoundary}>Reintentar</button>
    </div>
  );
}

const WIDGETS = {
  kpi: KPIMetricWidget,
  chart: ChartWidget,
  table: TableWidget,
  status: StatusWidget,
} as const;

export function Dashboard({ config }: { config: DashboardConfig }) {
  const [timeRange, setTimeRange] = useState<TimeRange>("24h");

  return (
    <QueryClientProvider client={queryClient}>
      <div className="dashboard">
        <header className="dashboard-header">
          <h1>{config.title}</h1>
          <TimeRangeSelector value={timeRange} onChange={setTimeRange} />
        </header>
        <main className="dashboard-grid" style={{ gridTemplateColumns: `repeat(${config.columns}, 1fr)` }}>
          {config.widgets.map(w => (
            <ErrorBoundary key={w.id} FallbackComponent={WidgetFallback}>
              {React.createElement(WIDGETS[w.type], { ...w.props, timeRange })}
            </ErrorBoundary>
          ))}
        </main>
      </div>
    </QueryClientProvider>
  );
}''',
        anti_patrones_relacionados=["AP-005", "AP-012", "AP-015"],
    ),
]


def buscar_componente(termino: str) -> List[ComponentSpec]:
    termino_lower = termino.lower()
    return [c for c in COMPONENTES if termino_lower in c.nombre.lower()]


def checklist_to_markdown(comp: ComponentSpec) -> str:
    lines = [f"## {comp.nombre} ({comp.categoria})", "", comp.descripcion, ""]
    lines.append("### Checklist de Auditoria")
    for i, item in enumerate(comp.checklist, 1):
        marker = "x" if "no " not in item.lower() else " "
        lines.append(f"- [{marker}] {item}")
    lines.append("")
    lines.append("### Implementacion de Referencia")
    lines.append("```tsx")
    lines.append(comp.ejemplo.strip())
    lines.append("```")
    return "\n".join(lines)


def resumen_componentes() -> str:
    lineas = ["=== CATALOGO DE COMPONENTES DE REFERENCIA ===", ""]
    for cat in ["atomos", "moleculas", "organismos", "plantillas"]:
        comps = [c for c in COMPONENTES if c.categoria == cat]
        if comps:
            lineas.append(f"## {cat.upper()}")
            for c in comps:
                lineas.append(f"  {c.nombre}: {c.descripcion[:80]}...")
            lineas.append("")
    return "\n".join(lineas)
