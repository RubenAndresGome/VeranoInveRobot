// ═══════════════════════════════════════════════════════════════════════
// CONFIG_MANAGER.CPP — Gestión de Configuración Persistente (NVS)
// ═══════════════════════════════════════════════════════════════════════
// Implementación con Preferences.h para ESP32.
// Los valores se guardan en la partición NVS con namespace "robotcfg".
// ═══════════════════════════════════════════════════════════════════════

#include <Preferences.h>
#include "config_manager.h"
#include "config.h"

static Preferences prefs;
static bool inicializado = false;

static const char* NVS_NAMESPACE = "robotcfg";

void config_manager_init() {
    if (!inicializado) {
        prefs.begin(NVS_NAMESPACE, false); // false = read/write mode
        inicializado = true;
        
        // Log de valores actuales
        float segMax = config_manager_get_segmento_max();
        float velPct = config_manager_get_vel_pct();
        Serial.printf("[CFG] segmento_max=%.1f vel_pct=%.1f\n", segMax, velPct);
    }
}

// ─────────────────────────────────────────────────────────────────────
// SETTERS
// ─────────────────────────────────────────────────────────────────────

void config_manager_set_segmento_max(float cm) {
    if (!inicializado) config_manager_init();
    prefs.putFloat("seg_max", cm);
    Serial.printf("[CFG] segmento_max -> %.1f cm\n", cm);
}

void config_manager_set_vel_pct(float pct) {
    if (!inicializado) config_manager_init();
    prefs.putFloat("vel_pct", pct);
    Serial.printf("[CFG] vel_pct -> %.1f %%\n", pct);
}

void config_manager_set_pausa_ms(unsigned long ms) {
    if (!inicializado) config_manager_init();
    prefs.putULong("pausa_ms", ms);
    Serial.printf("[CFG] pausa_ms -> %lu ms\n", ms);
}

void config_manager_set_error_cm(float cm) {
    if (!inicializado) config_manager_init();
    prefs.putFloat("err_cm", cm);
    Serial.printf("[CFG] error_cm -> %.1f cm\n", cm);
}

void config_manager_set_wifi_ssid(const char* ssid) {
    if (!inicializado) config_manager_init();
    prefs.putString("wifi_ssid", ssid);
    Serial.printf("[CFG] wifi_ssid -> %s\n", ssid);
}

void config_manager_set_wifi_password(const char* password) {
    if (!inicializado) config_manager_init();
    prefs.putString("wifi_pass", password);
    Serial.printf("[CFG] wifi_password actualizado\n");
}

// ─────────────────────────────────────────────────────────────────────
// GETTERS (fallback a #define de config.h si NVS está vacío)
// ─────────────────────────────────────────────────────────────────────

float config_manager_get_segmento_max() {
    if (!inicializado) config_manager_init();
    return prefs.getFloat("seg_max", SEGMENTO_MAX_CM);
}

float config_manager_get_vel_pct() {
    if (!inicializado) config_manager_init();
    return prefs.getFloat("vel_pct", FACTOR_VELOCIDAD_SEGMENTO * 100.0f);
}

unsigned long config_manager_get_pausa_ms() {
    if (!inicializado) config_manager_init();
    return prefs.getULong("pausa_ms", TIEMPO_PAUSA_SEGMENTO_MS);
}

float config_manager_get_error_cm() {
    if (!inicializado) config_manager_init();
    return prefs.getFloat("err_cm", MARGEN_ERROR_CM);
}

String config_manager_get_wifi_ssid() {
    if (!inicializado) config_manager_init();
    return prefs.getString("wifi_ssid", WIFI_SSID);
}

String config_manager_get_wifi_password() {
    if (!inicializado) config_manager_init();
    return prefs.getString("wifi_pass", WIFI_PASSWORD);
}
