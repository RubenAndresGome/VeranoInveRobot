#pragma once
// ═══════════════════════════════════════════════════════════════════════
// WEB_SERVER.H — Sistema Web para Control y Telemetría del Robot
// ═══════════════════════════════════════════════════════════════════════
// Servidor web embebido en ESP32-S3 que proporciona:
// 
// FEATURES:
// 1. Dashboard HTML/CSS sin dependencias externas para control del robot
// 2. Interfaz para programar ruta/destino (puntos de navegación)
// 3. Telemetría en tiempo real vía WebSocket (push 10Hz) + HTTP fallback
// 4. Control remoto: Avance, Giro, Parada, E-Stop
// 5. Conexión WiFi Access Point puro (modo AP, no requiere router externo)
// 
// ARQUITECTURA:
// - AsyncWebServer: Servidor web asíncrono (ESPAsyncWebServer)
// - WebSocket: ws://IP/ws para comandos y telemetría push en tiempo real
// - Polling: /status como fallback cuando WebSocket está caído
// - UI: CSS inline propio (panel-tech, zero CDN) servida desde UserControlGUI.cpp
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>

// ─────────────────────────────────────────────────────────────────────
// ESTRUCTURA DE DATOS - Comando de Ruta
// ─────────────────────────────────────────────────────────────────────
// Punto de navegación: distancia (cm) + ángulo (grados) + velocidad (PWM)

struct PuntoRuta {
    float distancia;      // cm
    float angulo;         // grados (positivo = derecha, negativo = izquierda)
    int velocidad;        // PWM (0-255)
    unsigned long duracion;  // ms esperado para completar
};

// ─────────────────────────────────────────────────────────────────────
// GESTIÓN DEL SERVIDOR WEB
// ─────────────────────────────────────────────────────────────────────

// Inicializar servidor web y conectarse a WiFi
void web_server_init();

// Loop del servidor — llamar en cada iteración de loop() (solo para DNS)
void web_server_loop();

// Detener servidor web y desconectar WiFi
void web_server_stop();

// Verificar si está conectado a WiFi
bool web_server_is_connected();

// Obtener dirección IP actual (retorna String por copia, sin dangling pointer)
String web_server_get_ip();

// Enviar datos de telemetría (almacena para polling desde /status)
void web_server_send_telemetry(float distancia, float angulo, 
                                long pulsosIzq, long pulsosDer, 
                                int pwmIzq, int pwmDer, 
                                const char* estado,
                                float posX, float posY, float orientacion,
                                float pidCorr = 0, float distRestante = 0,
                                float targetDist = 0, int targetVel = 0);

// Enviar mensajes de log a serial
void web_server_send_log(const char* mensaje);

// Verificar si hay nuevo comando en la cola
bool web_server_has_command();

// Obtener siguiente comando de ruta
PuntoRuta web_server_get_command();

// Encolar comando (para movimiento segmentado, laberinto, etc.)
// Retorna true si se encoló, false si la cola está llena
bool web_server_enqueue_command(PuntoRuta cmd);

// Bandera de reset solicitado desde WebSocket (consultada desde main loop)
extern volatile bool wsResetRequested;

// Broadcast de emergencia a todos los clientes WS
void web_server_broadcast(const char* msg);