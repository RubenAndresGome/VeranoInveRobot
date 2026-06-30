#pragma once
// ═══════════════════════════════════════════════════════════════════════
// COMANDOS.H — Preprocesamiento y segmentación de comandos
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>

// Preparar avance con segmentación automática (si > SEGMENTO_MAX_CM)
void prepararComandoAvance(float distanciaTotal, int velocidad);

// Preparar giro (encola directamente)
void prepararComandoGiro(float angulo, int velocidad);

// Control Manual Joystick
void setMotorsManual(char dir, int pwm);
extern volatile unsigned long tUltimoComandoManual;

// Variables compartidas de segmentación (protegidas por portMUX)
extern bool avanceSegmentadoActivo;
extern int segmentosRestantes;
extern portMUX_TYPE muxSegmentacion;
