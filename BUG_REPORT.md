# 🐛 REPORTE DE BUGS Y MEJORAS - Proyecto Carro Autónomo

**Fecha de Análisis:** 2026-06-27  
**Versión Analizada:** main (HEAD)  
**Estado:** 7 bugs identificados, 3 críticos

---

## 🔴 BUGS CRÍTICOS

### 1. **[CRITICAL] Race Condition en Cálculo de Odometría**
- **Archivo:** `src/main.cpp` - Función `tickAvance()`
- **Línea:** ~440
- **Problema:** La orientación global se actualiza DESPUÉS de usar el valor anterior para calcular heading:
  ```cpp
  float heading = orientacionGlobal + (angZ - angPrevio) * (PI / 180.0f);  // usa viejo orientacionGlobal
  orientacionGlobal += (angZ - angPrevio) * (PI / 180.0f);  // LUEGO lo actualiza
  posX += deltaDist * cos(heading);
  ```
  Esto causa que `posX` y `posY` se calculen incorrectamente, acumulando error de dead reckoning.
- **Severidad:** CRÍTICA - Odometría global completamente fallida
- **Fix:** Usar `orientacionGlobal` en lugar de `heading` para el cálculo

### 2. **[CRITICAL] Division/Margin Calculation Bug**
- **Archivo:** `src/main.cpp` - Función `tickAvance()`
- **Línea:** ~420
- **Problema:** 
  ```cpp
  float margenDinamico = (objetivo.distancia < MARGEN_ERROR_CM * 5) 
                         ? objetivo.distancia * 0.05f 
                         : MARGEN_ERROR_CM;
  float umbralLlegada = objetivo.distancia - margenDinamico;
  if (umbralLlegada < 0) umbralLlegada = 0;  // ← FIX intenta reparar pero llega tarde
  ```
  Si `objetivo.distancia = 1.0 cm`, entonces `margenDinamico = 0.05 cm`, `umbralLlegada = 0.95 cm`. 
  Con encoders discretos, puede nunca alcanzar este umbral exacto.
- **Severidad:** CRÍTICA - Comandos cortos se atascan
- **Fix:** Usar margen mínimo absoluto para distancias muy cortas

### 3. **[CRITICAL] Security - WiFi Password Hardcoded**
- **Archivo:** `include/config.h`
- **Línea:** ~65
- **Problema:** Credenciales WiFi visibles en código fuente:
  ```cpp
  #define WIFI_PASSWORD   "<PASSWORD_ANTERIOR>"
  ```
- **Severidad:** CRÍTICA - Red física comprometida, credenciales expuestas
- **Fix:** Usar config_manager con NVS, nunca en #define

---

## 🟠 BUGS ALTOS

### 4. **[HIGH] Race Condition en Segmentación**
- **Archivo:** `src/main.cpp` - Loop + `src/comandos.cpp`
- **Problema:** Variable `avanceSegmentadoActivo` accedida desde `main.cpp` (línea 768) y `comandos.cpp` (línea 34) sin spinlock consistente:
  ```cpp
  // comandos.cpp - SIN SPINLOCK al escribir
  avanceSegmentadoActivo = (count > 0);
  
  // main.cpp - CON SPINLOCK al leer
  portENTER_CRITICAL_ISR(&muxSegmentacion);
  bool segmentado = avanceSegmentadoActivo;
  ```
  La escritura desde `encolarAvanceSegmentado()` no está protegida.
- **Severidad:** ALTA - Corrupción de datos en ejecución segmentada
- **Fix:** Proteger TODAS las escrituras con spinlock

### 5. **[HIGH] Telemetry Data Race**
- **Archivo:** `src/main.cpp` 
- **Problema:** Estructura `telemetria` es escrita desde motor functions pero leída desde web_server sin sincronización:
  ```cpp
  // main.cpp - Escribe sin protección
  telemetria.pwmIzq = pwmIzq;
  
  // web_server.cpp - Lee telemetria global sin lock
  tel_pwmIzq = telemetria.pwmIzq;
  ```
- **Severidad:** ALTA - Datos de telemetría inconsistentes/corruptos
- **Fix:** Usar volatile + atomic operations o mutex

### 6. **[HIGH] Watchdog Manual Mode - Unsigned Math**
- **Archivo:** `src/main.cpp` - Loop STATE_MANUAL
- **Línea:** ~773
- **Problema:**
  ```cpp
  if (millis() - tUltimoComandoManual > 500)  // ← millis() es unsigned long
  ```
  Si `tUltimoComandoManual > millis()` (wrapping), la resta puede ser incorrecta.
- **Severidad:** ALTA - Timeout falso después de ~49 días uptime
- **Fix:** Usar `(unsigned long)(millis() - tUltimoComandoManual) > 500`

---

## 🟡 BUGS MEDIOS

### 7. **[MEDIUM] Inconsistent IMU Calibration State**
- **Archivo:** `src/main.cpp` - Función `leerIMU()`
- **Línea:** ~334
- **Problema:**
  ```cpp
  void leerIMU(float dt) {
      if (!imuCal.calibrated) return;  // ← Sale si no calibrado
      // Pero angX, angY, angZ pueden tener valores previos residuales
  ```
  Si IMU falla durante calibración, `imuCal.calibrated` queda false pero offsets son 0.
- **Severidad:** MEDIA - IMU no calibrado = mediciones erráticas
- **Fix:** Validar explícitamente estado de calibración

---

## 📋 RECOMENDACIONES DE ACTUALIZACIÓN

### Modernizaciones Necesarias:

1. **Agregar C++17 Features:**
   - `std::optional<float>` para valores que pueden no ser válidos
   - `std::atomic<bool>` para variables volátiles compartidas
   - Smart pointers (aunque limitado en embedded)

2. **Mejorar Seguridad:**
   - Cifrar/hashear credenciales WiFi en NVS
   - Agregar autenticación token en endpoints /api
   - HTTPS en lugar de HTTP (requiere certificado)

3. **Mejorar Robustez:**
   - Filtro de Kalman en lugar de complementario para IMU
   - Checksum en cola de comandos
   - Watchdog timer externo (WDT del MCU)

4. **Mejorar Telemetría:**
   - Buffer circular para datos, no variables globales
   - Timestamp en cada lectura
   - Validación de rango antes de serializar JSON

5. **Testing:**
   - Unit tests en `test/` con mocks de hardware
   - Fuzzing de endpoints HTTP
   - Simulación de timeout/reset

---

## ✅ BUGS DOCUMENTADOS PARA ARREGLARSE

**Prioridad:** 1. CRITICAL (bugs 1-3), 2. HIGH (bugs 4-6), 3. MEDIUM (bug 7)

| Bug | Severidad | Status | Fix Time |
|-----|-----------|--------|----------|
| Odometría | 🔴 | TODO | 15 min |
| Margen Dist | 🔴 | TODO | 10 min |
| WiFi Pwd | 🔴 | TODO | 20 min |
| Segmentación Race | 🟠 | TODO | 15 min |
| Telemetry Race | 🟠 | TODO | 25 min |
| Watchdog Math | 🟠 | TODO | 10 min |
| IMU State | 🟡 | TODO | 10 min |

**Tiempo Total Estimado:** ~105 minutos

