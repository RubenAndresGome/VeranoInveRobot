// ═══════════════════════════════════════════════════════════════════════
// HAL_PINS.CPP — Implementación de Hardware Abstraction Layer
// ═══════════════════════════════════════════════════════════════════════
// Variables globales explícitas de todos los pines con inicialización
// centralizada para facilitar migración entre microcontroladores.
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <Wire.h>
#include "hal_pins.h"
#include "config.h"

// ─────────────────────────────────────────────────────────────────────
// INICIALIZACIÓN DE VARIABLES GLOBALES DE PINES
// ─────────────────────────────────────────────────────────────────────
// OBJETIVO: Centralizar la configuración de hardware en un solo lugar
// para permitir cambios rápidos sin buscar #define esparcidos por el código

// ┌─────────────────────────────────────────────────────────────────┐
// │ MOTOR DRIVER - L298N (Puente H)                                 │
// │ Pines elegidos según disponibilidad en ESP32-S3-DevKitC-1       │
// │ Requerimientos: ENA/ENB = PWM (timers), IN*/IN2 = GPIO simple   │
// └─────────────────────────────────────────────────────────────────┘

uint8_t PIN_ENA = 15;    // LEDC Timer para PWM Motor Izquierdo
                         // Rango 0-255, frecuencia 5000 Hz, resolución 8 bits
                         // ⚠️ Verificar que no esté asignado a UART/I2C/SPI en plataforma destino

uint8_t PIN_IN1 = 16;    // Control de dirección Motor Izquierdo (bit 0)
                         // Paired con PIN_IN2 para H-bridge:
                         //   IN1=HIGH, IN2=LOW  → Motor avanza
                         //   IN1=LOW,  IN2=HIGH → Motor retrocede
                         //   IN1=LOW,  IN2=LOW  → Motor frena (regenerativo)

uint8_t PIN_IN2 = 17;    // Control de dirección Motor Izquierdo (bit 1)
                         // GPIO simple, sin requisitos de velocidad

uint8_t PIN_ENB = 18;    // LEDC Timer para PWM Motor Derecho
                         // Mismo rango/frecuencia que PIN_ENA

uint8_t PIN_IN3 = 8;     // Control de dirección Motor Derecho (bit 0)
                         // Mismo esquema H-bridge que IN1/IN2

uint8_t PIN_IN4 = 9;     // Control de dirección Motor Derecho (bit 1)

// Canales PWM asignados (ESP32 tiene 16 canales LEDC disponibles)
uint8_t PWM_CHANNEL_ENA = 0;  // Canal 0 para motor izquierdo
uint8_t PWM_CHANNEL_ENB = 1;  // Canal 1 para motor derecho

// ┌─────────────────────────────────────────────────────────────────┐
// │ ENCODERS - Realimentación de velocidad                          │
// │ Tipo: Óptico LM393 (disco 20 ranuras)                           │
// │ Requerimiento: GPIO con soporte para interrupciones externas     │
// └─────────────────────────────────────────────────────────────────┘

uint8_t PIN_ENC_IZQ = 4;  // Salida D0 del módulo LM393 (izquierdo)
                          // Configuración: INPUT_PULLUP, disparo en RISING
                          // Cálculo: pulsosCount → distancia mediante odometría
                          // Sincronización: Critical section (spinlock) en ISR

uint8_t PIN_ENC_DER = 5;  // Salida D0 del módulo LM393 (derecho)
                          // Idem PIN_ENC_IZQ (independiente, mismo sensor type)

// ┌─────────────────────────────────────────────────────────────────┐
// │ IMU - Medición de orientación (aceleración + rotación)          │
// │ Tipo: MPU6050 (6DoF: 3-axis acelerómetro + 3-axis giroscopio)   │
// │ Protocolo: I2C (dirección 0x68, velocidad estándar ~400kHz)     │
// └─────────────────────────────────────────────────────────────────┘

uint8_t PIN_SDA = 6;     // I2C Data (SDA) → comunicación datos con MPU6050
                         // Bidireccional, open-drain, requiere pull-up (~4.7kΩ)
                         // En ESP32: configuración automática en Wire.begin()

uint8_t PIN_SCL = 7;     // I2C Clock (SCL) → sincronización I2C
                         // Generado por master (ESP32), idem pull-up

// ┌─────────────────────────────────────────────────────────────────┐
// │ LEDS - Indicadores de estado (realimentación visual)            │
// │ Tipo: GPIO simple HIGH/LOW, sin requisitos de frecuencia        │
// │ Consumo: ~20mA típico por LED a 3.3V (ESP32 máx ~40mA por pin) │
// └─────────────────────────────────────────────────────────────────┘

uint8_t PIN_LED_IDLE = 38;    // LED Verde: Robot ocioso (no requiere acción)
                              // Parpadeo opcional implementable en loop()

uint8_t PIN_LED_MOVING = 39;  // LED Azul: Robot en movimiento
                              // Encendido durante STATE_ADVANCING/STATE_TURNING

uint8_t PIN_LED_ERROR = 40;   // LED Rojo: Error de seguridad o E-Stop
                              // Encendido cuando estado = STATE_ESTOP

// ┌─────────────────────────────────────────────────────────────────┐
// │ SEGURIDAD - Paro de Emergencia (E-Stop)                         │
// │ Tipo: Pushbutton digital, PULL_DOWN (activo HIGH)               │
// │ Requerimiento: Respuesta rápida → usar ISR si posible           │
// └─────────────────────────────────────────────────────────────────┘

uint8_t PIN_ESTOP = 41;  // Botón de paro de emergencia
                         // Configuración: INPUT_PULLDOWN, lectura periódica en main loop
                         // Si HIGH → detener todo (motorDetener) y pasar a STATE_ESTOP
                         // Debounce: implementar con filtro en software (100-200ms)

// ─────────────────────────────────────────────────────────────────────
// FUNCIÓN DE INICIALIZACIÓN - hal_pins_init()
// ─────────────────────────────────────────────────────────────────────
// Propósito: Configurar todas las direcciones GPIO y parámetros iniciales
// Llamada: Una sola vez en setup(), ANTES de usar cualquier pin
// 
// Orden importante:
// 1. Motor driver outputs (IN1-IN4 = LOW, PWM = 0)
// 2. Encoder inputs + interrupts
// 3. IMU setup (Wire.begin con pines explícitos)
// 4. LED outputs
// 5. E-Stop input + debounce setup

void hal_pins_init() {
    // ─────────────────────────────────────────────────────────────
    // 1. MOTOR DRIVER - Configurar outputs, inicializar en PARADO
    // ─────────────────────────────────────────────────────────────
    
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    
    // Configurar PWM en ESP32 (LEDC)
    ledcSetup(PWM_CHANNEL_ENA, PWM_FRECUENCIA, PWM_RESOLUCION);
    ledcSetup(PWM_CHANNEL_ENB, PWM_FRECUENCIA, PWM_RESOLUCION);
    ledcAttachPin(PIN_ENA, PWM_CHANNEL_ENA);
    ledcAttachPin(PIN_ENB, PWM_CHANNEL_ENB);
    
    // Estado inicial: motores detenidos (PWM = 0, dirección = LOW)
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, LOW);
    ledcWrite(PWM_CHANNEL_ENA, 0);
    ledcWrite(PWM_CHANNEL_ENB, 0);
    
    // ─────────────────────────────────────────────────────────────
    // 2. ENCODERS - Configurar inputs e interrupts
    // ─────────────────────────────────────────────────────────────
    
    pinMode(PIN_ENC_IZQ, INPUT_PULLUP);
    pinMode(PIN_ENC_DER, INPUT_PULLUP);
    
    // Nota: attachInterrupt() se realiza en main.cpp con la ISR específica
    // (aquí solo inicializamos pines, no handlers, para separar concerns)
    
    // ─────────────────────────────────────────────────────────────
    // 3. IMU - Configurar I2C con pines explícitos
    // ─────────────────────────────────────────────────────────────
    
    Wire.begin(PIN_SDA, PIN_SCL, 400000);  // I2C a 400 kHz estándar
    
    // ─────────────────────────────────────────────────────────────
    // 4. LEDS - Configurar outputs, inicializar en APAGADO
    // ─────────────────────────────────────────────────────────────
    
    pinMode(PIN_LED_IDLE, OUTPUT);
    pinMode(PIN_LED_MOVING, OUTPUT);
    pinMode(PIN_LED_ERROR, OUTPUT);
    
    digitalWrite(PIN_LED_IDLE, LOW);
    digitalWrite(PIN_LED_MOVING, LOW);
    digitalWrite(PIN_LED_ERROR, LOW);
    
    // ─────────────────────────────────────────────────────────────
    // 5. E-STOP - Configurar input, pull-down activo
    // ─────────────────────────────────────────────────────────────
    
    pinMode(PIN_ESTOP, INPUT_PULLDOWN);
    
    // Nota: Debounce implementado en main loop si es necesario
}
