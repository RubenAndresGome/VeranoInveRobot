#pragma once
// ═══════════════════════════════════════════════════════════════════════
// CONFIG.H — Configuración Central del Carro Robot Autónomo
// ═══════════════════════════════════════════════════════════════════════
// Parámetros físicos y de control (calibración del robot).
// 
// ⚠️  PIN MAPPING se ha movido a hal_pins.h y hal_pins.cpp para permitir
//     migración fácil entre microcontroladores.
// ═══════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────
// CONSTANTES FÍSICAS DEL ROBOT
// ─────────────────────────────────────────────────────────────────────

#define RANURAS_ENCODER           20
#define DIAMETRO_RUEDA_CM         6.5f     // ← MEDIR CON VERNIER
#define CM_POR_PULSO              (PI * DIAMETRO_RUEDA_CM / RANURAS_ENCODER)
#define DISTANCIA_ENTRE_RUEDAS_CM 15.0f    // ← MEDIR centro a centro

// ─────────────────────────────────────────────────────────────────────
// PARÁMETROS DE CONTROL
// ─────────────────────────────────────────────────────────────────────

// PWM
#define PWM_FRECUENCIA   40000 // Hz (40kHz evita zumbido audible en motores DC)
#define PWM_RESOLUCION   8     // bits (0-255)

// PID diferencial (para avance recto)
#define PID_KP  1.0f
#define PID_KI  0.1f
#define PID_KD  0.0f
#define MAX_PID_INTEGRAL  500.0f  // Anti-windup: clamp integral term

// Rampa de desaceleración
#define DISTANCIA_DECEL_CM  15.0f   // Comienza a desacelerar a 15cm del objetivo
#define FACTOR_MIN_DECEL    0.25f   // PWM mínimo durante desaceleración (25%)

// ─────────────────────────────────────────────────────────────────────
// MOVIMIENTO SEGMENTADO (evita error acumulativo en distancias largas)
// ─────────────────────────────────────────────────────────────────────
#define SEGMENTO_MAX_CM          50.0f   // Distancia máxima por segmento
#define SEGMENTOS_DISPONIBLES    20      // Máx segmentos que se pueden encolar
#define FACTOR_VELOCIDAD_SEGMENTO 0.8f   // 80% de velocidad para control de inercia
#define TIEMPO_PAUSA_SEGMENTO_MS 400     // Pausa entre segmentos (absorbe inercia)
#define MARGEN_ERROR_CM          2.0f    // Tolerancia: detener cuando distProm >= objetivo - margen

// Umbral de giro
#define UMBRAL_GIRO_GRADOS  2.0f    // Detener giro cuando |error| < 2°

// Timeouts de seguridad
#define TIMEOUT_AVANCE_EXTRA_MS  5000   // Tiempo extra sobre el estimado
#define TIMEOUT_GIRO_MS          10000  // Máximo 10s para cualquier giro
#define TIMEOUT_FRENADO_MS       2000   // Máximo 2s para detenerse

// ─────────────────────────────────────────────────────────────────────
// PARÁMETROS DE IMU
// ─────────────────────────────────────────────────────────────────────

#define IMU_CALIBRATION_SAMPLES  500    // Lecturas para calcular offset
#define IMU_GYRO_SENSITIVITY     131.0f // LSB/(°/s) para rango ±250°/s
#define COMPLEMENTARY_ALPHA      0.98f  // Peso del giroscopio en filtro complementario

// ─────────────────────────────────────────────────────────────────────
// PARÁMETROS DE COMUNICACIÓN
// ─────────────────────────────────────────────────────────────────────

// Credenciales WiFi — definidas en credentials.h (gitignored)
// Template publico: credentials_template.h
// Runtime: config_manager_set_wifi_ssid() / config_manager_set_wifi_password()
#include "credentials.h"
#define SERIAL_BAUD     115200

// Telemetría SSE
#define SSE_INTERVAL_MS  100   // Enviar telemetría cada 100ms
