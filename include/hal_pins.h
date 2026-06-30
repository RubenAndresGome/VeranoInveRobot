#pragma once
// ═══════════════════════════════════════════════════════════════════════
// HAL_PINS.H — Hardware Abstraction Layer para Pin Mapping
// ═══════════════════════════════════════════════════════════════════════
// Encapsulación de pines en variables globales explícitas para facilitar
// migración de código entre microcontroladores (ESP32, STM32, Arduino, etc.)
//
// ESTRUCTURA:
// - Motor Driver (L298N): 6 pines (Enable PWM + Direction Control)
// - Encoders (LM393): 2 pines (Digital input, RISING edge)
// - IMU (MPU6050): 2 pines (I2C SDA/SCL)
// - Status LEDs: 3 pines (Digital output)
// - Safety (E-Stop): 1 pin (Digital input)
//
// MIGRACIÓN A OTRO MICROCONTROLADOR:
// 1. Cambiar solo los valores numéricos de los pines en esta sección
// 2. Verificar que los pines soporten los requerimientos:
//    - Motor ENA/ENB: Deben soportar PWM (LEDC en ESP32, analogWrite en AVR)
//    - Encoders: Deben soportar interrupciones externas (GPIO con EXTI)
//    - I2C: Pines dedicados o configurables en periféricos I2C
// 3. Adaptar las funciones de inicialización en hal_pins.cpp si es necesario
// ═══════════════════════════════════════════════════════════════════════

#ifdef __cplusplus
extern "C" {
#endif

// ─────────────────────────────────────────────────────────────────────
// MOTOR DRIVER - Puente H L298N (2 motores DC independientes)
// ─────────────────────────────────────────────────────────────────────
// Función: Controlar velocidad y dirección de 2 motores DC
// Protocolo: 2 pines de dirección (IN1/IN2) + 1 pin PWM (ENA) por motor
// Control: ENA/ENB modulan velocidad (0-255), IN*/IN* definen dirección
// 
// Motor Izquierdo: IN1 (HIGH)/IN2 (LOW) = Adelante, ENA = PWM de velocidad
// Motor Derecho:   IN3 (HIGH)/IN4 (LOW) = Adelante, ENB = PWM de velocidad

extern uint8_t PIN_ENA;    // PWM Motor Izquierdo Enable (0-255 velocidad)
extern uint8_t PIN_IN1;    // Motor Izquierdo Control 1 (dirección bit 0)
extern uint8_t PIN_IN2;    // Motor Izquierdo Control 2 (dirección bit 1)

extern uint8_t PIN_ENB;    // PWM Motor Derecho Enable (0-255 velocidad)
extern uint8_t PIN_IN3;    // Motor Derecho Control 1 (dirección bit 0)
extern uint8_t PIN_IN4;    // Motor Derecho Control 2 (dirección bit 1)

// Canales PWM asignados en ESP32 (0-15 disponibles en LEDC)
extern uint8_t PWM_CHANNEL_ENA;  // Canal LEDC para motor izquierdo
extern uint8_t PWM_CHANNEL_ENB;  // Canal LEDC para motor derecho

// ─────────────────────────────────────────────────────────────────────
// ENCODERS - Sensores de velocidad (LM393 Infrarrojo, disco 20 ranuras)
// ─────────────────────────────────────────────────────────────────────
// Función: Realimentación de velocidad y odometría de posición
// Protocolo: 1 pin digital por rueda, 1 flanco RISING = 1 ranura = incremento
// Tipos: Óptico (LM393), Efecto Hall, o inductivo
// 
// Cálculo de distancia: dist_cm = (pulsosCount * PI * DIAMETRO) / RANURAS
// Detección de velocidad: diferencia de pulsosCount en intervalos de tiempo
// Requerimiento: GPIO con soporte para interrupciones externas (EXTI)

extern uint8_t PIN_ENC_IZQ;  // Encoder Motor Izquierdo (entrada digital)
extern uint8_t PIN_ENC_DER;  // Encoder Motor Derecho (entrada digital)

// ─────────────────────────────────────────────────────────────────────
// SENSORES - IMU (MPU6050 vía I2C, 6DoF: acelerómetro + giroscopio)
// ─────────────────────────────────────────────────────────────────────
// Función: Medición de orientación (ángulo de giro) y aceleración
// Protocolo: I2C (SDA/SCL, dirección 0x68 por defecto)
// Ventajas: Baja latencia, datos continua rate, complementar con odometría
// 
// Giroscopio rango: ±250°/s (sensibilidad 131 LSB/°/s)
// Acelerómetro rango: ±16g (sensibilidad 2048 LSB/g)
// Usar filtro complementario para fusionar datos giroscópicos + aceleración

extern uint8_t PIN_SDA;  // I2C Data Line (comunicación bidireccional)
extern uint8_t PIN_SCL;  // I2C Clock Line (sincronización)

// ─────────────────────────────────────────────────────────────────────
// LEDS DE ESTADO - Indicadores visuales de máquina de estados
// ─────────────────────────────────────────────────────────────────────
// Función: Realimentación visual del estado operacional del robot
// Protocolo: GPIO digital output, HIGH = LED encendido
// 
// IDLE:   Verde → Robot en reposo, listo para comandos
// MOVING: Azul  → En movimiento (avance o giro)
// ERROR:  Rojo  → Error / E-Stop activado / timeout de seguridad

extern uint8_t PIN_LED_IDLE;    // LED Verde: Robot listo (STATE_IDLE)
extern uint8_t PIN_LED_MOVING;  // LED Azul: En movimiento (ADVANCING/TURNING)
extern uint8_t PIN_LED_ERROR;   // LED Rojo: Error o E-Stop

// ─────────────────────────────────────────────────────────────────────
// SEGURIDAD - E-Stop (Paro de Emergencia)
// ─────────────────────────────────────────────────────────────────────
// Función: Detener inmediatamente el robot ante situación de riesgo
// Protocolo: GPIO digital input con PULL_DOWN, activo HIGH
// Behavior: Cualquier lectura HIGH → cambiar a STATE_ESTOP y detener motores
// 
// Requerimiento: Respuesta rápida (idealmente con ISR) para seguridad

extern uint8_t PIN_ESTOP;  // Botón de paro de emergencia (entrada digital)

// ─────────────────────────────────────────────────────────────────────
// FUNCIÓN DE INICIALIZACIÓN DE PINES
// ─────────────────────────────────────────────────────────────────────
// Llamar una sola vez en setup() para configurar todas las direcciones GPIO
// y parámetros iniciales (INPUT/OUTPUT, PULL_UP/PULL_DOWN, etc.)

void hal_pins_init();

#ifdef __cplusplus
}
#endif
