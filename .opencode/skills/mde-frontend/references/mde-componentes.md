# MDE-Frontend: Patrones de Componentes — Syllogismus Compositorius

> Tabla de Contenidos:
> 1. Atomic Design Ontológico
> 2. Feature-Sliced Design: Dirección de Dependencias
> 3. React Patterns en Silogismos
> 4. Tailwind + MDE: Sistema de Clases
> 5. Storybook como Tribunal de Verificación

---

## 1. Atomic Design Ontológico

### Jerarquía de Seres Visuales

```
Átomo ──► Molécula ──► Organismo ──► Template ──► Página
 simple    compuesto     complejo     abstracto     concreto
```

### Reglas de Categorización Formal

#### Átomo (Sustancia Simple)

```tsx
// Silogismo de categorización:
// Mayor: Todo ente visual indivisible que no posee estado propio es Átomo.
// Menor: <Button> recibe props y renderiza; no decide ni recuerda.
// Conclusio: <Button> es Átomo.

interface ButtonProps {
  variant: 'primary' | 'secondary' | 'ghost';
  size: 'sm' | 'md' | 'lg';
  children: React.ReactNode;
  onClick?: () => void;
}

// Axioma Cromático aplicado:
// Mayor: Todo botón primary debe poseer el máximo contraste con el fondo.
// Menor: El fondo es white (L=100).
// Conclusio: bg-[hsl(0,0%,0%)] text-white (ratio ∞:1)
export const Button: React.FC<ButtonProps> = ({ variant, size, children, onClick }) => {
  const base = "inline-flex items-center justify-center font-medium transition-all";
  
  const variants = {
    primary:   "bg-[hsl(0,0%,0%)] text-white hover:bg-[hsl(0,0%,15%)]",
    secondary: "bg-[hsl(0,0%,96%)] text-[hsl(0,0%,15%)] hover:bg-[hsl(0,0%,90%)]",
    ghost:     "bg-transparent text-[hsl(0,0%,15%)] hover:bg-[hsl(0,0%,96%)]"
  };
  
  const sizes = {
    sm: "px-3 py-1.5 text-sm rounded-lg",   // 12px / 6px ≈ 2.0  ✓ E(n)/E(n-1) ∈ [1.5,2]
    md: "px-4 py-2 text-base rounded-xl",      // 16px / 8px = 2.0   ✓ 
    lg: "px-6 py-3 text-lg rounded-2xl"        // 24px / 12px = 2.0  ✓
  };
  
  return (
    <button 
      className={`${base} ${variants[variant]} ${sizes[size]}`}
      onClick={onClick}
    >
      {children}
    </button>
  );
};
```

#### Molécula (Compositio Prima)

```tsx
// Mayor: Todo ente compuesto de átomos con función unitaria emergente es Molécula.
// Menor: <SearchField> combina Input + Button para buscar.
// Conclusio: <SearchField> es Molécula.

interface SearchFieldProps {
  onSearch: (query: string) => void;
  placeholder?: string;
}

export const SearchField: React.FC<SearchFieldProps> = ({ onSearch, placeholder }) => {
  const [query, setQuery] = useState("");
  
  // Causa Eficiente: el usuario presiona Enter o clickea buscar
  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (query.trim()) onSearch(query.trim());
  };
  
  return (
    <form onSubmit={handleSubmit} className="flex gap-2">
      <input
        type="search"
        value={query}
        onChange={(e) => setQuery(e.target.value)}
        placeholder={placeholder}
        className="flex-1 px-4 py-2 rounded-xl border border-gray-200 
                   focus:outline-none focus:ring-2 focus:ring-black/5"
        aria-label="Búsqueda"
      />
      <Button variant="primary" size="md" onClick={() => onSearch(query)}>
        <SearchIcon className="w-4 h-4" />
      </Button>
    </form>
  );
};
```

#### Organismo (Sustancia Compleja)

```tsx
// Mayor: Todo ente visual con estado propio, Causa Final completa 
//        e independencia operativa es Organismo.
// Menor: <ProductCard> maneja wishlist, tiene imágenes, precio, rating.
// Conclusio: <ProductCard> es Organismo, no Molécula.

interface ProductCardProps {
  product: {
    id: string;
    name: string;
    price: number;
    image: string;
    rating: number;
    inStock: boolean;
  };
}

export const ProductCard: React.FC<ProductCardProps> = ({ product }) => {
  const [isWishlisted, setIsWishlisted] = useState(false);
  
  // KPI Verum: Todo producto en stock debe indicarse semánticamente
  const stockLabel = product.inStock ? "En stock" : "Agotado";
  
  return (
    <article className="group w-[320px] rounded-2xl overflow-hidden 
                        border border-gray-100 hover:shadow-lg transition-shadow">
      {/* Imagen: Causa Material (el producto físico) */}
      <div className="relative aspect-square bg-gray-50">
        <img 
          src={product.image} 
          alt={product.name}
          loading="lazy"
          className="w-full h-full object-cover group-hover:scale-105 transition-transform"
        />
        {/* Wishlist: Causa Eficiente */}
        <button
          onClick={() => setIsWishlisted(!isWishlisted)}
          aria-label={isWishlisted ? "Remover de favoritos" : "Añadir a favoritos"}
          className="absolute top-3 right-3 p-2 rounded-full bg-white/80 
                     hover:bg-white transition-colors"
        >
          <HeartIcon className={`w-5 h-5 ${isWishlisted ? "fill-red-500 text-red-500" : ""}`} />
        </button>
      </div>
      
      {/* Info: Causa Formal */}
      <div className="p-6 space-y-2">
        <h3 className="font-medium text-lg leading-tight">{product.name}</h3>
        <div className="flex items-center gap-2">
          <StarRating rating={product.rating} />
          <span className="text-sm text-gray-500">({product.rating})</span>
        </div>
        <div className="flex items-center justify-between pt-2">
          <span className="text-xl font-semibold">${product.price}</span>
          <span className={`text-sm ${product.inStock ? "text-green-600" : "text-red-500"}`}>
            {stockLabel}
          </span>
        </div>
      </div>
    </article>
  );
};
```

#### Template (Figura sine Materia)

```tsx
// Mayor: Todo ente que define posición sin contenido real es Template.
// Menor: <ProductGrid> distribuye slots sin datos concretos.
// Conclusio: <ProductGrid> es Template.

interface ProductGridProps {
  header: React.ReactNode;   // Slot: Organismo Header
  filters: React.ReactNode;  // Slot: Organismo SidebarFilters  
  products: React.ReactNode; // Slot: array de <ProductCard />
  footer: React.ReactNode;   // Slot: Organismo Footer
}

export const ProductGrid: React.FC<ProductGridProps> = ({ header, filters, products, footer }) => {
  // Proporción Áurea: sidebar = contenido / Φ
  const sidebarWidth = "calc(100% / 1.618 / 2)";
  
  return (
    <div className="min-h-screen flex flex-col">
      <header>{header}</header>
      
      <main className="flex-1 flex max-w-[1440px] mx-auto w-full">
        <aside style={{ width: sidebarWidth }} className="border-r border-gray-100">
          {filters}
        </aside>
        
        <section className="flex-1 p-8">
          <div className="grid grid-cols-3 gap-6">
            {products}
          </div>
        </section>
      </main>
      
      <footer>{footer}</footer>
    </div>
  );
};
```

#### Page (Actus Completus)

```tsx
// Mayor: Todo ente que instancia Templates con datos reales y conecta 
//        routing es Page.
// Menor: /products muestra ProductGrid poblada con datos de API.
// Conclusio: ProductsPage es Page.

export const ProductsPage = () => {
  const { data: products, isLoading } = useProductsQuery();
  
  if (isLoading) return <ProductGridSkeleton />; // Estado potencia → actus pendiente
  
  return (
    <ProductGrid
      header={<Header />}
      filters={<ProductFilters />}
      products={products?.map(p => <ProductCard key={p.id} product={p} />)}
      footer={<Footer />}
    />
  );
};
```

---

## 2. Feature-Sliced Design: Dirección de Dependencias

### Jerarquía de Capas (Anillos Concéntricos)

```
        ┌─────────────────┐
        │      app/       │  ← Configuración global, providers
        ├─────────────────┤
        │     pages/      │  ← Routing, composición de páginas
        ├─────────────────┤
        │    widgets/     │  ← Bloques reutilizables de página
        ├─────────────────┤
        │    features/    │  ← Acciones de usuario (login, checkout)
        ├─────────────────┤
        │    entities/    │  ← Modelos de negocio (User, Cart, Product)
        ├─────────────────┤
        │     shared/     │  ← UI-kit, lib, config, api (estable)
        └─────────────────┘
        
        Flujo de imports: ↑ SOLO HACIA ARRIBA ↑
        Una capa inferior NUNCA importa de una superior.
```

### Ejemplo de Estructura

```
src/
├── app/
│   ├── providers/
│   │   └── AppProviders.tsx      // QueryClient, Router, Theme
│   ├── routing/
│   │   └── routes.tsx            // Definición de rutas
│   └── store/                    // Estado global (Zustand/Redux)
│
├── pages/
│   ├── HomePage/
│   ├── ProductsPage/
│   └── CheckoutPage/
│
├── widgets/
│   ├── Header/
│   ├── Footer/
│   ├── ProductCarousel/
│   └── CartSummary/
│
├── features/
│   ├── addToCart/
│   │   ├── ui/
│   │   │   └── AddToCartButton.tsx
│   │   ├── model/
│   │   │   └── cartStore.ts
│   │   └── api/
│   │       └── cartApi.ts
│   ├── applyPromoCode/
│   └── userAuth/
│
├── entities/
│   ├── product/
│   │   ├── model/
│   │   │   ├── types.ts          // Product, ProductDTO
│   │   │   └── productModel.ts   // Mappers DTO → Domain
│   │   ├── ui/
│   │   │   └── ProductName.tsx   // Componente atómico de producto
│   │   └── api/
│   │       └── productApi.ts
│   └── user/
│
└── shared/
    ├── ui/                         // Design System: Button, Input, Card
    ├── lib/                        // Utilidades puras
    ├── api/                        // Cliente HTTP configurado
    └── config/                     // Constantes, breakpoints
```

### Regla de Importación

```tsx
// ✅ CORRECTO: shared/ es referenciado por todos
import { Button } from "@/shared/ui";

// ✅ CORRECTO: entities/ referencia shared/
import { httpClient } from "@/shared/api";

// ✅ CORRECTO: features/ referencia entities/ y shared/
import { Product } from "@/entities/product";
import { Button } from "@/shared/ui";

// ❌ PROHIBIDO: shared/ NO puede importar de features/
import { useCart } from "@/features/addToCart";  // VIOLACIÓN ONTOLOGICA

// ❌ PROHIBIDO: entities/ NO puede importar de widgets/
import { Header } from "@/widgets/Header";       // VIOLACIÓN ONTOLOGICA
```

---

## 3. React Patterns en Silogismos

### Compound Components (Compositio Implicita)

```tsx
// Mayor: Las partes de un ente que comparten estado implícito 
//        forman un todo teleológico superior.
// Menor: Tabs.List, Tabs.Trigger y Tabs.Content deben sincronizarse 
//        en el valor activo.
// Conclusio: Se implementan via Context API.

// Usage:
<Tabs defaultValue="overview">
  <Tabs.List>
    <Tabs.Trigger value="overview">Resumen</Tabs.Trigger>
    <Tabs.Trigger value="analytics">Analytics</Tabs.Trigger>
    <Tabs.Trigger value="settings">Configuración</Tabs.Trigger>
  </Tabs.List>
  <Tabs.Content value="overview"><OverviewPanel /></Tabs.Content>
  <Tabs.Content value="analytics"><AnalyticsPanel /></Tabs.Content>
  <Tabs.Content value="settings"><SettingsPanel /></Tabs.Content>
</Tabs>
```

### Render Props (Inversio Dependentiae)

```tsx
// Mayor: Las abstracciones no deben depender de detalles empíricos.
// Menor: DataFetcher no debe conocer cómo se renderizan los datos.
// Conclusio: Recibe función render como prop.

<DataFetcher url="/api/users">
  {(data, isLoading) => (
    isLoading ? <Skeleton /> : <UserList users={data} />
  )}
</DataFetcher>
```

### Custom Hooks (Extractio Formae)

```tsx
// Mayor: Toda lógica que se repite en múltiples componentes 
//        debe extraerse como forma pura (hook).
// Menor: useWindowSize se usa en Header, ProductGrid y Footer.
// Conclusio: Extraer a hook compartido.

export const useWindowSize = () => {
  const [size, setSize] = useState({ width: 0, height: 0 });
  
  useEffect(() => {
    const handleResize = () => {
      setSize({ width: window.innerWidth, height: window.innerHeight });
    };
    
    handleResize(); // Actus inicial
    window.addEventListener("resize", handleResize);
    
    return () => window.removeEventListener("resize", handleResize); // Cleanup potencia
  }, []);
  
  return size;
};
```

### Higher-Order Component (Elevatio Ontologica)

```tsx
// Mayor: Todo componente que requiere autenticación debe ser elevado 
//        a un plano ontológico superior donde el acceso está garantizado.
// Menor: <Dashboard /> requiere que el usuario esté autenticado.
// Conclusio: Envolver con withAuth.

function withAuth<P extends object>(
  WrappedComponent: React.ComponentType<P>
): React.FC<P> {
  return (props) => {
    const { user, isLoading } = useAuth();
    
    if (isLoading) return <LoadingSpinner />;
    if (!user) return <Navigate to="/login" />;
    
    return <WrappedComponent {...props} user={user} />;
  };
}

// Uso:
export const Dashboard = withAuth(DashboardPage);
```

---

## 4. Tailwind + MDE: Sistema de Clases

### Tokens MDE Mapeados a Tailwind

| Token MDE | Valor | Tailwind | Uso |
|-----------|-------|----------|-----|
| `color-veritas` | `#000000` | `text-black` | Texto primario (máximo contraste) |
| `color-gravitas` | `hsl(0,0%,15%)` | `text-[hsl(0,0%,15%)]` | Headings |
| `color-subtilis` | `hsl(0,0%,45%)` | `text-[hsl(0,0%,45%)]` | Texto secundario |
| `color-fatum` | `hsl(0,0%,90%)` | `border-[hsl(0,0%,90%)]` | Bordes sutiles |
| `color-pura` | `#FFFFFF` | `bg-white` | Fondos |
| `color-silva` | `hsl(210,45%,55%)` | `bg-[hsl(210,45%,55%)]` | Acción/info |
| `color-aurea` | `hsl(30,50%,55%)` | `bg-[hsl(30,50%,55%)]` | Advertencia/CTA secundario |
| `color-rubra` | `hsl(350,60%,50%)` | `bg-[hsl(350,60%,50%)]` | Error/destructivo |

### Sistema de Espaciado

```css
/* El espaciado sigue la sucesión de Fibonacci aproximada */
.space-atomus     {  4px }  /* p-1  */
.space-molecula   {  8px }  /* p-2  */
.space-organismus { 16px }  /* p-4  */
.space-templum    { 24px }  /* p-6  */
.space-aureus-1   { 32px }  /* p-8  */
.space-aureus-2   { 48px }  /* p-12 */
.space-aureus-3   { 64px }  /* p-16 */
.space-aureus-4   { 96px }  /* p-24 */
.space-aureus-5   { 128px } /* p-32 */
```

### Proporción Áurea en Grid

```tsx
// Mayor: Las columnas principales deben guardar proporción Φ.
// Menor: El layout tiene sidebar + contenido.
// Conclusio: grid-cols-[1fr_1.618fr] para sidebar|content 
//            o grid-cols-[1.618fr_1fr] para content|sidebar.

<div className="grid grid-cols-[1fr_1.618fr] gap-8">
  <aside>Sidebar</aside>
  <main>Contenido principal</main>
</div>
```

---

## 5. Storybook como Tribunal de Verificación

### Cada componente MDE requiere:

1. **Silogismo de existencia** (por qué existe el componente)
2. **Variantes visuales** (todos los estados posibles)
3. **KPI auditables** (contraste, nodos, accesibilidad)

### Ejemplo de Story MDE

```tsx
// Button.stories.tsx
import type { Meta, StoryObj } from "@storybook/react";
import { Button } from "./Button";

/**
 * SYLLOGISMUS ESSENTIAE:
 * Mayor: Toda acción que el usuario puede ejecutar debe ser convocada 
 *        por un ente visualmente destacado.
 * Menor: Las acciones primarias requieren máxima prominencia.
 * Conclusio: Button es el átomo convocador de acciones.
 */
const meta: Meta<typeof Button> = {
  title: "Atoms/Button",
  component: Button,
  parameters: {
    // KPI Verum: Debe pasar a11y audit automático
    a11y: {
      config: {
        rules: [{ id: "color-contrast", enabled: true }],
      },
    },
  },
};

export default meta;
type Story = StoryObj<typeof Button>;

export const Primary: Story = {
  args: {
    variant: "primary",
    size: "md",
    children: "Continuar",
  },
  // KPI Pulchrum: ratio del padding debe aproximar Φ/4
  parameters: {
    chromatic: { diffThreshold: 0.03 },
  },
};

export const Secondary: Story = {
  args: {
    variant: "secondary", 
    size: "md",
    children: "Cancelar",
  },
};

export const Ghost: Story = {
  args: {
    variant: "ghost",
    size: "sm",
    children: "Más info",
  },
};

// Estado de carga: Causa Eficiente suspendida
export const Loading: Story = {
  args: {
    variant: "primary",
    size: "md",
    children: "Cargando...",
    disabled: true,
  },
};
```
