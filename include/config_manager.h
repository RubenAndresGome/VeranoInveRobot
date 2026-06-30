#pragma once
// ═══════════════════════════════════════════════════════════════════════
// CONFIG_MANAGER.H — Gestión de Configuración Persistente (NVS)
// ═══════════════════════════════════════════════════════════════════════
// Usa Preferences.h (NVS en flash) para almacenar parámetros configurables
// en runtime. Los valores se persisten entre reinicios.
//
// Fallback: si NVS está vacío, se usan los valores por defecto de config.h
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>

// Inicializar NVS y cargar configuración
void config_manager_init();

// --- Setters (guardan en NVS inmediatamente) ---
void config_manager_set_segmento_max(float cm);
void config_manager_set_vel_pct(float pct);
void config_manager_set_pausa_ms(unsigned long ms);
void config_manager_set_error_cm(float cm);
void config_manager_set_wifi_ssid(const char* ssid);
void config_manager_set_wifi_password(const char* password);

// --- Getters (retornan valor actual, de NVS o default) ---
float config_manager_get_segmento_max();
float config_manager_get_vel_pct();
unsigned long config_manager_get_pausa_ms();
float config_manager_get_error_cm();
String config_manager_get_wifi_ssid();
String config_manager_get_wifi_password();
