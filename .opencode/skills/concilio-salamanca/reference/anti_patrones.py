"""
Catalogo de anti-patrones comunes en frontend y backend.
Cada entrada incluye: nombre, severidad, sintomas, silogismo MDE, y correccion.

Formato: los agentes del Concilio pueden referenciar estos anti-patrones
en sus silogismos para acelerar el razonamiento y reducir alucinaciones.
"""

from dataclasses import dataclass, field
from enum import Enum
from typing import List


class Severidad(str, Enum):
    CRITICA = "critica"
    ALTA = "alta"
    MEDIA = "media"
    BAJA = "baja"


class Dominio(str, Enum):
    FRONTEND = "frontend"
    BACKEND = "backend"
    FULLSTACK = "fullstack"
    SEGURIDAD = "seguridad"
    RENDIMIENTO = "rendimiento"
    DATOS = "datos"


@dataclass
class AntiPatron:
    id: str
    nombre: str
    dominio: Dominio
    severidad: Severidad
    sintomas: List[str]
    premisa_mayor: str
    premisa_menor: str
    conclusion: str
    correccion: str
    ejemplo_malo: str = ""
    ejemplo_bueno: str = ""
    referencias: List[str] = field(default_factory=list)


ANTI_PATRONES: List[AntiPatron] = [
    AntiPatron(
        id="AP-001",
        nombre="XSS por innerHTML sin sanitizar",
        dominio=Dominio.SEGURIDAD,
        severidad=Severidad.CRITICA,
        sintomas=[
            "element.innerHTML = datoUsuario",
            "No se usa textContent ni DOMPurify",
            "Variables de usuario interpoladas directamente en HTML",
        ],
        premisa_mayor="Todo codigo que inserta input de usuario en el DOM sin sanitizar es una puerta abierta a XSS.",
        premisa_menor="Esta funcion concatena datos de usuario en innerHTML sin escape.",
        conclusion="Esta funcion es ontologicamente insegura y debe ser condenada.",
        correccion="Usar textContent para texto plano o DOMPurify para HTML seguro.",
        ejemplo_malo="div.innerHTML = '<p>' + userName + '</p>';",
        ejemplo_bueno="div.textContent = userName;",
        referencias=["OWASP XSS Prevention Cheat Sheet"],
    ),
    AntiPatron(
        id="AP-002",
        nombre="SQL Injection por concatenacion de strings",
        dominio=Dominio.SEGURIDAD,
        severidad=Severidad.CRITICA,
        sintomas=[
            "Template literals con variables en queries SQL",
            "db.query(`SELECT * FROM users WHERE id = ${id}`)",
            "Falta de parametrizacion o prepared statements",
        ],
        premisa_mayor="Toda query SQL que recibe parametros del usuario sin prepared statements es una inyeccion SQL.",
        premisa_menor="Esta query concatena directamente la variable del usuario en el string SQL.",
        conclusion="Esta query es ontologicamente defectuosa y expone la base de datos a inyeccion.",
        correccion="Usar prepared statements: db.query('SELECT * FROM users WHERE id = ?', [id])",
        ejemplo_malo='db.query(`SELECT * FROM users WHERE name = "${req.body.name}"`)',
        ejemplo_bueno='db.query("SELECT * FROM users WHERE name = ?", [req.body.name])',
        referencias=["OWASP SQL Injection"],
    ),
    AntiPatron(
        id="AP-003",
        nombre="useEffect sin dependencias vs dependencias infinitas",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.ALTA,
        sintomas=[
            "useEffect sin array de dependencias (se ejecuta en cada render)",
            "useEffect con dependencia que se actualiza dentro del mismo efecto",
            "Ciclos infinitos de re-renderizado",
        ],
        premisa_mayor="Todo efecto colateral que no declara correctamente sus dependencias genera renders innecesarios o bucles infinitos.",
        premisa_menor="Este useEffect carece del array de dependencias adecuado o contiene dependencias autorreferenciales.",
        conclusion="Este componente es ontologicamente inestable y colapsara en ciclos de renderizado.",
        correccion="Declarar exactamente las dependencias que disparan el efecto. Extraer logica a custom hooks si es complejo.",
        ejemplo_malo="useEffect(() => { setCount(count + 1); });",
        ejemplo_bueno="useEffect(() => { setCount(prev => prev + 1); }, []);",
        referencias=["React docs: useEffect"],
    ),
    AntiPatron(
        id="AP-004",
        nombre="Prop drilling excesivo",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.MEDIA,
        sintomas=[
            "Props pasadas a traves de 4+ niveles de componentes",
            "Componentes intermedios que solo pasan props sin usarlas",
            "Dificultad para rastrear el origen de los datos",
        ],
        premisa_mayor="Todo patron de paso de datos que atraviesa mas de 3 capas sin transformacion es una violacion del principio de responsabilidad unica.",
        premisa_menor="Este componente recibe props que no usa, solo para pasarlas a sus hijos en el nivel 4 de anidacion.",
        conclusion="Este arbol de componentes padece prop drilling ontologico y debe ser refactorizado.",
        correccion="Usar Context API, state management (Zustand, Redux), o composicion de componentes.",
        ejemplo_malo="<Abuelo><Padre><Hijo><Nieto prop={data}/></Hijo></Padre></Abuelo>",
        ejemplo_bueno="<Provider value={data}><Consumidor /></Provider>",
        referencias=["React docs: Context"],
    ),
    AntiPatron(
        id="AP-005",
        nombre="Estado derivado sin memoizacion",
        dominio=Dominio.RENDIMIENTO,
        severidad=Severidad.ALTA,
        sintomas=[
            "Calculos costosos en cada render del componente",
            "useMemo/useCallback ausentes en operaciones O(n) o mayores",
            "Filtrados, ordenamientos, mapeos en el cuerpo del componente sin cache",
        ],
        premisa_mayor="Todo calculo derivado del estado que se ejecuta en cada render sin cache es un desperdicio termodinamico de ciclos de CPU.",
        premisa_menor="Esta funcion de filtrado O(n log n) se ejecuta en cada render, incluso cuando sus dependencias no cambiaron.",
        conclusion="Este componente desperdicia energia computacional y debe ser optimizado con memoizacion.",
        correccion="Envolver calculos costosos en useMemo o extraerlos a selectors con Reselect.",
        ejemplo_malo="const filtered = data.filter(x => x.active).sort((a,b) => a.name - b.name);",
        ejemplo_bueno="const filtered = useMemo(() => data.filter(x => x.active).sort((a,b) => a.name - b.name), [data]);",
        referencias=["React docs: useMemo"],
    ),
    AntiPatron(
        id="AP-006",
        nombre="Callback hell / Pyramid of Doom",
        dominio=Dominio.BACKEND,
        severidad=Severidad.MEDIA,
        sintomas=[
            "Callbacks anidados 3+ niveles",
            "Then/catch encadenados sin async/await",
            "Dificultad para leer el flujo de control",
        ],
        premisa_mayor="Todo flujo asincrono anidado mas de 2 niveles sin abstraccion es ilegible e inmantenible.",
        premisa_menor="Esta funcion tiene 5 niveles de callbacks anidados procesando datos en secuencia.",
        conclusion="Este codigo es ontologicamente opaco y colapsara ante cualquier modificacion futura.",
        correccion="Refactorizar con async/await o promesas encadenadas planas.",
        ejemplo_malo="fs.readFile('a.txt', (err, data) => { parse(data, (err, obj) => { save(obj, (err) => { ... }) }) });",
        ejemplo_bueno="const data = await fs.promises.readFile('a.txt'); const obj = parse(data); await save(obj);",
        referencias=["Node.js docs: async/await"],
    ),
    AntiPatron(
        id="AP-007",
        nombre="API sin rate limiting",
        dominio=Dominio.SEGURIDAD,
        severidad=Severidad.CRITICA,
        sintomas=[
            "Endpoints publicos sin limitacion de requests",
            "Falta de CAPTCHA o verificacion en rutas sensibles",
            "Posibilidad de fuerza bruta o DDoS",
        ],
        premisa_mayor="Toda API publica sin rate limiting es un vector de ataque de denegacion de servicio.",
        premisa_menor="Este endpoint de login no tiene rate limiting ni proteccion contra fuerza bruta.",
        conclusion="Este endpoint es ontologicamente vulnerable a ataques de fuerza bruta y DDoS.",
        correccion="Implementar rate limiting (express-rate-limit) y bloquear tras N intentos fallidos.",
        ejemplo_malo="app.post('/login', (req, res) => { /* sin rate limit */ });",
        ejemplo_bueno="app.post('/login', rateLimit({ windowMs: 15*60*1000, max: 5 }), handler);",
        referencias=["OWASP API Security"],
    ),
    AntiPatron(
        id="AP-008",
        nombre="Modal con z-index hardcodeado",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.BAJA,
        sintomas=[
            "z-index: 9999 en modales",
            "Conflictos de stacking context con otros overlays",
            "Tooltips que aparecen detras del modal",
        ],
        premisa_mayor="Toda solucion de posicionamiento que usa valores arbitrarios de z-index sin gestion de capas es fragil ante stacking contexts.",
        premisa_menor="Este modal usa z-index: 99999 sin un sistema de capas definido.",
        conclusion="Este modal colisionara con otros overlays en el futuro, generando bugs de stacking context.",
        correccion="Usar un sistema de capas (ej. @layer o variables CSS) con rangos definidos: base=0, dropdown=100, modal=200, toast=300.",
        ejemplo_malo=".modal { z-index: 99999; }",
        ejemplo_bueno=":root { --z-modal: 200; } .modal { z-index: var(--z-modal); }",
        referencias=["MDN: Stacking context"],
    ),
    AntiPatron(
        id="AP-009",
        nombre="N+1 queries en ORMs",
        dominio=Dominio.RENDIMIENTO,
        severidad=Severidad.ALTA,
        sintomas=[
            "SELECT dentro de un bucle for",
            "Lazy loading no configurado correctamente en ORM",
            "Cientos de queries individuales donde un JOIN bastaria",
        ],
        premisa_mayor="Toda consulta a base de datos que itera N veces ejecutando una subquery por iteracion es N+1 y tiene complejidad O(N) innecesaria.",
        premisa_menor="Este codigo carga usuarios en un bucle y ejecuta una query de posts por cada usuario.",
        conclusion="Este codigo es termodinamicamente ineficiente y debe usar eager loading.",
        correccion="Usar JOIN o eager loading del ORM: User.findAll({ include: [Posts] })",
        ejemplo_malo="for (const user of users) { const posts = await db.query('SELECT * FROM posts WHERE user_id = ?', [user.id]); }",
        ejemplo_bueno="const users = await db.query('SELECT u.*, p.* FROM users u LEFT JOIN posts p ON u.id = p.user_id');",
        referencias=["Sequelize docs: eager loading", "Prisma docs: include"],
    ),
    AntiPatron(
        id="AP-010",
        nombre="Manejo de errores con catch vacio",
        dominio=Dominio.BACKEND,
        severidad=Severidad.ALTA,
        sintomas=[
            "try { ... } catch(e) { }",
            "try { ... } catch(e) { console.log(e) } sin accion correctiva",
            "Errores silenciados que corrompen el estado",
        ],
        premisa_mayor="Todo bloque catch que no registra, notifica o corrige el error es una supresion del no-ser que escondera la corrupcion del estado.",
        premisa_menor="Este bloque catch esta vacio, silenciando cualquier excepcion que ocurra.",
        conclusion="Esta funcion es ontologicamente peligrosa porque oculta fallos que corromperan el estado silenciosamente.",
        correccion="Registrar el error con contexto, notificar al sistema de monitoreo, y devolver un estado de error controlado.",
        ejemplo_malo="try { await processPayment(); } catch(e) { }",
        ejemplo_bueno="try { await processPayment(); } catch(e) { logger.error('Payment failed', { error: e.message, userId }); return { error: 'PAYMENT_FAILED' }; }",
        referencias=["Clean Code: Error Handling"],
    ),
    AntiPatron(
        id="AP-011",
        nombre="Tabla sin paginacion ni virtualizacion",
        dominio=Dominio.RENDIMIENTO,
        severidad=Severidad.ALTA,
        sintomas=[
            "Renderizado de 1000+ filas en una tabla",
            "DOM con miles de nodos simultaneos",
            "Scroll lag y consumo excesivo de memoria",
        ],
        premisa_mayor="Toda tabla que renderiza mas de 100 filas simultaneas sin paginacion o virtualizacion es un desperdicio termodinamico de DOM.",
        premisa_menor="Esta tabla renderiza 5000 filas simultaneamente en el DOM, cada una con 10 elementos hijos.",
        conclusion="Esta tabla es ontologicamente ineficiente y debe implementar paginacion o virtualizacion.",
        correccion="Usar paginacion del lado del servidor o virtualizacion con react-window/react-virtualized.",
        ejemplo_malo="<table>{data.map(row => <tr>{row.cells.map(c => <td>{c}</td>)}</tr>)}</table>",
        ejemplo_bueno="<FixedSizeList height={600} itemCount={data.length} itemSize={40}>{({index}) => <Row data={data[index]} />}</FixedSizeList>",
        referencias=["react-window", "TanStack Table"],
    ),
    AntiPatron(
        id="AP-012",
        nombre="Dashboard con polling sin debounce",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.MEDIA,
        sintomas=[
            "setInterval llamando API cada N segundos sin control",
            "Multiples requests simultaneas al mismo endpoint",
            "Datos desactualizados en pantalla durante refresco",
        ],
        premisa_mayor="Toda actualizacion periodica de datos que no usa debounce ni cancelacion de requests anteriores genera carreras de datos y desperdicio de red.",
        premisa_menor="Este dashboard usa setInterval de 3 segundos sin cancelar la request anterior ni implementar debounce.",
        conclusion="Este dashboard es ontologicamente inestable y sobrecargara la red y el servidor.",
        correccion="Usar React Query/SWR con refetchInterval, abortar requests anteriores con AbortController.",
        ejemplo_malo="setInterval(() => fetch('/api/metrics').then(r => r.json()).then(setData), 3000);",
        ejemplo_bueno="const { data } = useQuery({ queryKey: ['metrics'], queryFn: fetchMetrics, refetchInterval: 5000 });",
        referencias=["TanStack Query", "SWR"],
    ),
    AntiPatron(
        id="AP-013",
        nombre="Secrets en codigo fuente",
        dominio=Dominio.SEGURIDAD,
        severidad=Severidad.CRITICA,
        sintomas=[
            "API_KEY = 'sk-...' en el codigo",
            "Credenciales hardcodeadas en configuracion",
            "Archivos .env commiteados al repo",
        ],
        premisa_mayor="Toda credencial presente en el codigo fuente es una apertura ontologica al mal que sera inevitablemente expuesta.",
        premisa_menor="Este archivo contiene una API key hardcodeada que sera publica si el repositorio es open source.",
        conclusion="Este codigo contiene secretos expuestos y debe ser purgado inmediatamente.",
        correccion="Usar variables de entorno (.env en .gitignore), secrets manager (Vault, AWS Secrets Manager), o injection de configuracion en runtime.",
        ejemplo_malo="const API_KEY = 'sk-abc123def456';",
        ejemplo_bueno="const API_KEY = process.env.API_KEY;",
        referencias=["OWASP: Secrets Management"],
    ),
    AntiPatron(
        id="AP-014",
        nombre="Botones sin estado de carga",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.MEDIA,
        sintomas=[
            "Botones que disparan acciones asincronas sin feedback visual",
            "Usuario puede hacer doble click generando requests duplicadas",
            "Falta de estado disabled durante la operacion",
        ],
        premisa_mayor="Todo elemento interactivo que ejecuta operaciones asincronas sin bloqueo temporal permite acciones duplicadas y genera estados inconsistentes.",
        premisa_menor="Este boton de pago no muestra estado de carga ni se deshabilita durante la transaccion.",
        conclusion="Este boton es ontologicamente defectuoso porque permite doble pago accidental.",
        correccion="Implementar estado loading con spinner y disabled=true durante la operacion.",
        ejemplo_malo='<button onClick={pagar}>Pagar</button>',
        ejemplo_bueno='<button onClick={pagar} disabled={loading}>{loading ? <Spinner /> : "Pagar"}</button>',
        referencias=["WCAG: Accessible buttons"],
    ),
    AntiPatron(
        id="AP-015",
        nombre="useState para estado derivable",
        dominio=Dominio.FRONTEND,
        severidad=Severidad.MEDIA,
        sintomas=[
            "Multiples useState que dependen unos de otros",
            "Sincronizacion manual entre estados relacionados",
            "useEffect para mantener estados sincronizados",
        ],
        premisa_mayor="Todo estado que puede derivarse de otro estado sin necesidad de sincronizacion manual es una fuente de inconsistencias.",
        premisa_menor="Este componente usa tres useState separados y dos useEffect para mantenerlos sincronizados.",
        conclusion="Este componente esta ontologicamente fragmentado y debe ser reducido con useReducer o estado derivado.",
        correccion="Usar useReducer para estados interdependientes o calcular valores derivados en el render sin useState.",
        ejemplo_malo="const [firstName, setFirstName] = useState(''); const [lastName, setLastName] = useState(''); const [fullName, setFullName] = useState(''); useEffect(() => { setFullName(firstName + ' ' + lastName); }, [firstName, lastName]);",
        ejemplo_bueno="const [firstName, setFirstName] = useState(''); const [lastName, setLastName] = useState(''); const fullName = firstName + ' ' + lastName;",
        referencias=["React docs: You might not need an effect"],
    ),
]

ANTI_PATRONES_POR_DOMINIO = {}
for ap in ANTI_PATRONES:
    ANTI_PATRONES_POR_DOMINIO.setdefault(ap.dominio.value, []).append(ap)

ANTI_PATRONES_POR_SEVERIDAD = {}
for ap in ANTI_PATRONES:
    ANTI_PATRONES_POR_SEVERIDAD.setdefault(ap.severidad.value, []).append(ap)


def buscar_anti_patrones(termino: str) -> List[AntiPatron]:
    termino_lower = termino.lower()
    resultados = []
    for ap in ANTI_PATRONES:
        if (termino_lower in ap.nombre.lower() or
            any(termino_lower in s.lower() for s in ap.sintomas) or
            termino_lower in ap.premisa_mayor.lower() or
            termino_lower in ap.conclusion.lower() or
            any(termino_lower in r.lower() for r in ap.referencias)):
            resultados.append(ap)
    return resultados


def listar_anti_patrones(dominio: str = None, severidad: str = None) -> List[AntiPatron]:
    resultados = ANTI_PATRONES
    if dominio:
        resultados = [ap for ap in resultados if ap.dominio.value == dominio]
    if severidad:
        resultados = [ap for ap in resultados if ap.severidad.value == severidad]
    return resultados


def resumen_anti_patrones() -> str:
    lineas = ["=== CATALOGO DE ANTI-PATRONES DEL CONCILIO ===", ""]
    for dominio in Dominio:
        aps = ANTI_PATRONES_POR_DOMINIO.get(dominio.value, [])
        if aps:
            lineas.append(f"## {dominio.value.upper()} ({len(aps)})")
            for ap in aps:
                lineas.append(f"  [{ap.severidad.value.upper()}] {ap.id}: {ap.nombre}")
    return "\n".join(lineas)
