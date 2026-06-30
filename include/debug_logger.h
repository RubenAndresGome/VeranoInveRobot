#pragma once
// ═══════════════════════════════════════════════════════════════════════
// DEBUG_LOGGER.H — Log condicional para reducir bloqueo por Serial.print
// ═══════════════════════════════════════════════════════════════════════
//
// Uso:
//   DEBUG_PRINTLN("[FSM] Timeout!");
//   DEBUG_PRINT("dist="); DEBUG_PRINTLN(dist);
//
// En producción: definir DEBUG_ENABLED=0 o no definirlo.
// Para debug: agregar -DDEBUG_ENABLED=1 en build_flags de platformio.ini
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>

#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED 0
#endif

#if DEBUG_ENABLED
    #define DEBUG_INIT()      Serial.begin(SERIAL_BAUD); delay(500)
    #define DEBUG_PRINT(...)  Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
    #define DEBUG_FLUSH()     Serial.flush()
#else
    #define DEBUG_INIT()
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
    #define DEBUG_PRINTF(...)
    #define DEBUG_FLUSH()
#endif
