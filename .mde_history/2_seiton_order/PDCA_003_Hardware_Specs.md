# Especificaciones de Hardware y Pin Mapping — ESP32-S3-DevKitC-1

> [!NOTE]
> Basado en el pinout del ESP32-S3-DevKitC-1 de Espressif y el encoder infrarrojo LM393 con disco de 20 ranuras.

---

## 1. Encoder Infrarrojo LM393 — Especificaciones

| Parámetro | Valor |
|---|---|
| **Chip comparador** | LM393 |
| **Tipo de sensor** | Optoacoplador ranurado (infrarrojo) |
| **Canales** | 1 (solo velocidad, **sin dirección**) |
| **Ranuras del disco** | 20 |
| **Salida** | Digital — HIGH cuando la ranura pasa, LOW cuando el diente bloquea |
| **Voltaje de operación** | 3.3V – 5V (compatible directo con ESP32-S3 a 3.3V) |
| **Pines del módulo** | VCC, GND, D0 (salida digital), A0 (salida analógica — no usamos) |

### Cálculo de `CM_POR_PULSO`

```
CM_POR_PULSO = (π × DIAMETRO_RUEDA_CM) / RANURAS_DISCO
             = (π × D) / 20
```

| Diámetro de rueda | CM_POR_PULSO | Pulsos para 1 metro |
|---|---|---|
| 3 cm | 0.471 cm | 212 pulsos |
| 4 cm | 0.628 cm | 159 pulsos |
| 5 cm | 0.785 cm | 127 pulsos |
| 6 cm | 0.942 cm | 106 pulsos |
| 6.5 cm | 1.021 cm | 98 pulsos |
| 7 cm | 1.100 cm | 91 pulsos |

> [!IMPORTANT]
> **Medir el diámetro exacto de la rueda** (con vernier si es posible) y usar la tabla para confirmar tu `CM_POR_PULSO`. El valor `Cm = 0.319` del código original implica un diámetro de ~2.03 cm, lo cual es demasiado pequeño para una rueda — probablemente se midió empíricamente con error, o el disco tiene más ranuras. **Recomendación: recalibrar midiendo una distancia conocida (1m) y contando pulsos.**

### Limitación Crítica: Sin Detección de Dirección

El encoder LM393 de 1 canal **no puede detectar si la rueda gira hacia adelante o hacia atrás**. Solo cuenta pulsos.

**Implicaciones:**
- Si el robot patina y retrocede, los pulsos se suman en vez de restarse → error de odometría
- Para la FSM, esto es aceptable si solo avanzamos y giramos en el lugar (nunca retroceso)
- La dirección de movimiento la sabemos por el comando que enviamos al puente H

```cpp
// Como no hay cuadratura, la ISR siempre suma
void IRAM_ATTR isrEncoderIzq() {
    pulsosIzq++;  // Siempre incrementa — asumimos dirección del motor
}
```

---

## 2. ESP32-S3-DevKitC-1 — Pines a Evitar

### Pines de Strapping (NO USAR para I/O general)

| Pin | Función de Strapping | Riesgo |
|---|---|---|
| **GPIO0** | Boot mode select | Si está LOW al arranque, entra en modo descarga |
| **GPIO3** | JTAG signal source | Puede interferir con el boot |
| **GPIO45** | VDD_SPI voltage | Afecta voltaje de flash SPI |
| **GPIO46** | Boot mode / Log output | Afecta configuración de boot |

### Pines Ocupados por USB (si usas USB nativo)

| Pin | Función |
|---|---|
| **GPIO19** | USB D- |
| **GPIO20** | USB D+ |

### Pines de Flash SPI Interno (NUNCA usar)

| Pines | Función |
|---|---|
| **GPIO26–32** | Flash/PSRAM SPI (en módulos WROOM con PSRAM) |

### ADC2 no funciona con WiFi activo

| ADC2 Pins (NO usar para lectura analógica con WiFi) |
|---|
| GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13, GPIO14 |

> [!WARNING]
> En el código original del proyecto, los potenciómetros estaban en pines `{4, 5, 6}` que son **ADC2**. Con WiFi activo esos pines **no pueden leer valores analógicos**. Para el nuevo proyecto esto no importa (no usamos potenciómetro), pero es bueno saberlo.

---

## 3. Pin Mapping Propuesto

### Asignación para el Proyecto de Navegación Autónoma

```cpp
// ═══════════════════════════════════════════════════════
// PIN MAPPING — ESP32-S3-DevKitC-1
// ═══════════════════════════════════════════════════════

// --- PUENTE H (L298N) ---
// Usar pines LEDC-capable (todos los GPIO del ESP32-S3 soportan PWM)
const int PIN_ENA = 15;    // PWM Motor Izquierdo (Enable A)
const int PIN_IN1 = 16;    // Dirección Motor Izquierdo
const int PIN_IN2 = 17;    // Dirección Motor Izquierdo
const int PIN_ENB = 18;    // PWM Motor Derecho (Enable B)
const int PIN_IN3 = 8;     // Dirección Motor Derecho
const int PIN_IN4 = 9;     // Dirección Motor Derecho (reubicado de GPIO3 por strapping)
// NOTA: Evitar GPIO0, GPIO45, GPIO46 (strapping)
//       Evitar GPIO19, GPIO20 (USB)
//       Evitar GPIO26-32 (flash SPI)

// --- ENCODERS (LM393) ---
// Cualquier GPIO soporta interrupciones en ESP32-S3
const int PIN_ENC_IZQ = 4;   // D0 del encoder izquierdo
const int PIN_ENC_DER = 5;   // D0 del encoder derecho

// --- IMU (MPU6050 via I2C) ---
// I2C es remapeable a cualquier GPIO en ESP32-S3
const int PIN_SDA = 6;     // I2C Data
const int PIN_SCL = 7;     // I2C Clock
// Dirección I2C del MPU6050: 0x68 (AD0=LOW) o 0x69 (AD0=HIGH)

// --- LED de estado (opcional) ---
const int PIN_LED_STATUS = 48;  // LED RGB integrado en el DevKit
// GPIO38 = RGB_LED en algunos DevKitC-1 (verificar tu versión)
```

### Diagrama de Conexiones

```
ESP32-S3-DevKitC-1          L298N Puente H
┌──────────────┐           ┌──────────────┐
│         GP15 ├───────────┤ ENA (PWM)    │
│         GP16 ├───────────┤ IN1          │──── Motor
│         GP17 ├───────────┤ IN2          │──── Izquierdo
│              │           │              │
│         GP18 ├───────────┤ ENB (PWM)    │
│          GP8 ├───────────┤ IN3          │──── Motor
│          GP9 ├───────────┤ IN4          │──── Derecho
│              │           │              │
│          GND ├───────────┤ GND          │ ← GND COMÚN
└──────────────┘           └──────────────┘
                                 │
                            12V-24V DC
                           (alimentación
                            de motores)

ESP32-S3                   Encoder LM393 (×2)
┌──────────────┐           ┌──────────────┐
│          GP4 ├───────────┤ D0 (izq)     │
│          GP5 ├───────────┤ D0 (der)     │
│         3V3  ├───────────┤ VCC (ambos)  │
│          GND ├───────────┤ GND (ambos)  │
└──────────────┘           └──────────────┘

ESP32-S3                   MPU6050
┌──────────────┐           ┌──────────────┐
│          GP6 ├───────────┤ SDA          │
│          GP7 ├───────────┤ SCL          │
│         3V3  ├───────────┤ VCC          │ ← 3.3V directo
│          GND ├───────────┤ GND          │
└──────────────┘           └──────────────┘
```

> [!TIP]
> **Los pines propuestos están en la parte izquierda del DevKit**, lo que facilita el cableado en protoboard. Los pines de la derecha (GPIO33–GPIO48) quedan libres para expansión futura (sensores ultrasónicos, servo, etc.).

---

## 4. Configuración en Código

### Setup Completo de Hardware

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <MPU6050.h>

// ═══════════════════════════════════════════════════════
// CONSTANTES FÍSICAS DEL ROBOT
// ═══════════════════════════════════════════════════════
const int   RANURAS_ENCODER     = 20;
const float DIAMETRO_RUEDA_CM   = 6.5;  // ← MEDIR CON VERNIER
const float CM_POR_PULSO        = (PI * DIAMETRO_RUEDA_CM) / RANURAS_ENCODER;
const float DISTANCIA_ENTRE_RUEDAS_CM = 15.0;  // ← MEDIR centro a centro

// ═══════════════════════════════════════════════════════
// VARIABLES DE ENCODER (volatile + protegidas)
// ═══════════════════════════════════════════════════════
volatile long pulsosIzq = 0;
volatile long pulsosDer = 0;

// En ESP32, usamos spinlock en vez de noInterrupts()
portMUX_TYPE muxEncoder = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isrEncoderIzq() {
    portENTER_CRITICAL_ISR(&muxEncoder);
    pulsosIzq++;
    portEXIT_CRITICAL_ISR(&muxEncoder);
}

void IRAM_ATTR isrEncoderDer() {
    portENTER_CRITICAL_ISR(&muxEncoder);
    pulsosDer++;
    portEXIT_CRITICAL_ISR(&muxEncoder);
}

// Lectura segura desde loop() (fuera de ISR)
long leerPulsos(volatile long *contador) {
    portENTER_CRITICAL(&muxEncoder);
    long copia = *contador;
    portEXIT_CRITICAL(&muxEncoder);
    return copia;
}

void resetPulsos() {
    portENTER_CRITICAL(&muxEncoder);
    pulsosIzq = 0;
    pulsosDer = 0;
    portEXIT_CRITICAL(&muxEncoder);
}

// ═══════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);

    // --- Motores ---
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);

    // PWM por hardware (API v3.x del Arduino ESP32 Core)
    ledcAttach(PIN_ENA, 5000, 8);  // 5kHz, 8-bit resolution
    ledcAttach(PIN_ENB, 5000, 8);

    // --- Encoders ---
    pinMode(PIN_ENC_IZQ, INPUT_PULLUP);
    pinMode(PIN_ENC_DER, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_IZQ), isrEncoderIzq, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DER), isrEncoderDer, RISING);

    // --- IMU ---
    Wire.begin(PIN_SDA, PIN_SCL);  // ESP32 permite reasignar I2C
    // ... inicializar MPU6050 ...

    // --- WiFi ---
    WiFi.softAP("CarroRobot", "12345678");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());  // Siempre 192.168.4.1

    Serial.println("Sistema listo.");
    Serial.print("CM_POR_PULSO = ");
    Serial.println(CM_POR_PULSO, 4);
}
```

### Nota sobre `IRAM_ATTR` y `portMUX`

En ESP32 (a diferencia de Arduino Uno):
- Las ISR **deben** estar en IRAM (`IRAM_ATTR`), no en flash
- `long` (32 bits) **es** atómico en ESP32 (Xtensa de 32 bits), pero usamos `portMUX` de todas formas porque es buena práctica y protege contra race conditions en acceso multi-core
- `noInterrupts()`/`interrupts()` funciona pero no es thread-safe en dual-core — `portMUX` sí lo es

---

## 5. Procedimiento de Calibración del Encoder

### Paso 1: Medir diámetro de rueda
```
Con vernier: medir en milímetros, convertir a cm
Ejemplo: 65.0 mm → 6.5 cm
```

### Paso 2: Calcular CM_POR_PULSO teórico
```
CM_POR_PULSO = π × 6.5 / 20 = 1.021 cm/pulso
```

### Paso 3: Validar empíricamente
```
1. Marcar punto de inicio en el suelo
2. Comando: avanzar hasta contar 98 pulsos (teórico = 1m)
3. Medir distancia real recorrida con cinta métrica
4. Si midió 95 cm → CM_POR_PULSO_real = 95.0 / 98 = 0.969 cm/pulso
5. Actualizar la constante
```

### Paso 4: Verificar ambas ruedas
```
Repetir paso 3 para cada rueda por separado.
Si difieren > 5%, hay que usar CM_POR_PULSO individual por rueda.
```


<!-- Plan (Objetivo) Do (Implementacion) -->
