// ═══════════════════════════════════════════════════════════════════════
// WEB_SERVER.CPP — Implementación del Servidor Web (AsyncWebServer)
// ═══════════════════════════════════════════════════════════════════════
// Arquitectura: Polling /status cada 500ms desde UserControlGUI.cpp
// Comandos: POST /cmd con application/x-www-form-urlencoded
// Cola: Circular buffer estático (evita fragmentación heap de std::queue)
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "web_server.h"
#include "config.h"
#include "comandos.h"
#include "config_manager.h"
#include "UserControlGUI.h"

// ─────────────────────────────────────────────────────────────────────
// VARIABLES GLOBALES
// ─────────────────────────────────────────────────────────────────────

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

// Cola de comandos circular estática (máx COLA_MAX_COMANDOS pendientes)
#define COLA_MAX_COMANDOS 10
static PuntoRuta colaComandos[COLA_MAX_COMANDOS];
static int colaHead = 0;
static int colaTail = 0;
static int colaCount = 0;
portMUX_TYPE muxComandos = portMUX_INITIALIZER_UNLOCKED;

// Mutex para telemetria web
portMUX_TYPE muxTelWeb = portMUX_INITIALIZER_UNLOCKED;

// Track de clientes conectados
int clientesSSE = 0;

// Bandera de reset solicitado desde WebSocket (consultada desde main loop)
volatile bool wsResetRequested = false;

// Variables para forzar FSM (Modo Debug)
volatile bool wsFsmOverrideRequest = false;
char wsForcedFsmState[16] = "";
portMUX_TYPE muxFsmOverride = portMUX_INITIALIZER_UNLOCKED;

// Variables globales para la telemetría del polling (protegidas por muxTelWeb)
float tel_distancia = 0;
float tel_angulo = 0;
long tel_pulsosIzq = 0;
long tel_pulsosDer = 0;
int tel_pwmIzq = 0;
int tel_pwmDer = 0;
char tel_estado[16] = "INIT";
float tel_posX = 0;
float tel_posY = 0;
float tel_orientacion = 0;
float tel_pidCorr = 0;
float tel_distRestante = 0;
float tel_targetDist = 0;
int tel_targetVel = 0;

// ─────────────────────────────────────────────────────────────────────
// FUNCIONES AUXILIARES DE COLA CIRCULAR
// ─────────────────────────────────────────────────────────────────────

static bool cola_push(PuntoRuta cmd) {
    portENTER_CRITICAL(&muxComandos);
    bool ok = false;
    if (colaCount < COLA_MAX_COMANDOS) {
        colaComandos[colaTail] = cmd;
        colaTail = (colaTail + 1) % COLA_MAX_COMANDOS;
        colaCount++;
        ok = true;
    }
    portEXIT_CRITICAL(&muxComandos);
    return ok;
}

static bool cola_pop(PuntoRuta* cmd) {
    portENTER_CRITICAL(&muxComandos);
    bool ok = false;
    if (colaCount > 0) {
        *cmd = colaComandos[colaHead];
        colaHead = (colaHead + 1) % COLA_MAX_COMANDOS;
        colaCount--;
        ok = true;
    }
    portEXIT_CRITICAL(&muxComandos);
    return ok;
}

static bool cola_has_data() {
    portENTER_CRITICAL(&muxComandos);
    bool tiene = (colaCount > 0);
    portEXIT_CRITICAL(&muxComandos);
    return tiene;
}

// ─────────────────────────────────────────────────────────────────────
// ENDPOINT /status — Telemetría para Polling (JSON con ArduinoJson)
// ─────────────────────────────────────────────────────────────────────
static void handle_status(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["clientes"] = clientesSSE;
    doc["connected"] = (WiFi.softAPgetStationNum() > 0);
    
    portENTER_CRITICAL(&muxTelWeb);
    float d_dist = tel_distancia, d_ang = tel_angulo;
    long d_pI = tel_pulsosIzq, d_pD = tel_pulsosDer;
    int d_pwmI = tel_pwmIzq, d_pwmD = tel_pwmDer;
    char d_est[16]; strcpy(d_est, tel_estado);
    float d_pX = tel_posX, d_pY = tel_posY, d_ort = tel_orientacion;
    float d_pid = tel_pidCorr, d_rest = tel_distRestante, d_tgt = tel_targetDist;
    int d_tVel = tel_targetVel;
    portEXIT_CRITICAL(&muxTelWeb);
    
    JsonObject telem = doc["telemetria"].to<JsonObject>();
    telem["distancia"] = d_dist;
    telem["angulo"] = d_ang;
    telem["pulsosIzq"] = d_pI;
    telem["pulsosDer"] = d_pD;
    telem["pwmIzq"] = d_pwmI;
    telem["pwmDer"] = d_pwmD;
    telem["estado"] = d_est;
    telem["posX"] = d_pX;
    telem["posY"] = d_pY;
    telem["orientacion"] = d_ort;
    telem["pidCorr"] = d_pid;
    telem["distRestante"] = d_rest;
    telem["targetDist"] = d_tgt;
    telem["targetVel"] = d_tVel;
    
    String json;
    serializeJson(doc, json);
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("X-Frame-Options", "SAMEORIGIN");
    request->send(response);
}

// ─────────────────────────────────────────────────────────────────────
// ENDPOINT /cmd — Recibir comando manual (URL-encoded)
// ─────────────────────────────────────────────────────────────────────
static void handle_cmd(AsyncWebServerRequest *request) {
    String tipo = request->hasParam("tipo", true) ? request->getParam("tipo", true)->value() : "";
    float valor1 = request->hasParam("valor1", true) ? request->getParam("valor1", true)->value().toFloat() : 0;
    int valor2 = request->hasParam("valor2", true) ? request->getParam("valor2", true)->value().toInt() : 0;
    String extra1 = request->hasParam("extra1", true) ? request->getParam("extra1", true)->value() : "";
    String extra2 = request->hasParam("extra2", true) ? request->getParam("extra2", true)->value() : "";
    
    Serial.printf("[WEB] Comando: %s, v1: %.1f, v2: %d", tipo.c_str(), valor1, valor2);
    if (extra1.length() > 0) Serial.printf(", x1: %s", extra1.c_str());
    if (extra2.length() > 0) Serial.printf(", x2: %s", extra2.c_str());
    Serial.println();
    
    if (tipo == "config" && extra1.length() > 0 && extra2.length() > 0) {
        config_manager_set_segmento_max(valor1);
        config_manager_set_vel_pct(valor2);
        config_manager_set_pausa_ms((unsigned long)extra1.toFloat());
        config_manager_set_error_cm(extra2.toFloat());
        
        web_server_send_log("Config segmentación aplicada");
        request->send(200, "text/plain", "CONFIG OK");
        return;
    }
    
    if (tipo == "avance") {
        prepararComandoAvance(valor1, valor2);
        web_server_send_log("Comando avance procesado");
        request->send(200, "text/plain", "OK");
        return;
    } else if (tipo == "giro") {
        prepararComandoGiro(valor1, valor2);
        web_server_send_log("Comando giro procesado");
        request->send(200, "text/plain", "OK");
        return;
    } else if (tipo == "estop" || tipo == "parada") {
        PuntoRuta cmd = {0, 0, 0, 0};
        cmd.distancia = -999;
        if (!web_server_enqueue_command(cmd)) {
            request->send(503, "text/plain", "QUEUE FULL");
            return;
        }
        web_server_send_log("Comando parada procesado");
        request->send(200, "text/plain", "OK");
        return;
    }
    request->send(400, "text/plain", "BAD REQUEST");
}

// ─────────────────────────────────────────────────────────────────────
// Helpers de BODY para peticiones asíncronas
// ─────────────────────────────────────────────────────────────────────
static void handle_body_accumulation(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!request->_tempObject) {
        request->_tempObject = new String();
    }
    String* body = (String*)request->_tempObject;
    if (index == 0 && total > 0) {
        body->reserve(total);  // Pre-allocate — avoids heap fragmentation
    }
    body->concat((const char*)data, len);
}

// ─────────────────────────────────────────────────────────────────────
// ENDPOINT /ruta — Recibir ruta programada
// ─────────────────────────────────────────────────────────────────────
static void handle_ruta(AsyncWebServerRequest *request) {
    if (!request->_tempObject) {
        request->send(400, "text/plain", "EMPTY BODY");
        return;
    }
    String body = *(String*)request->_tempObject;
    delete (String*)request->_tempObject;
    request->_tempObject = NULL;

    Serial.printf("[WEB] Ruta recibida (%d bytes): %s\n", body.length(), body.c_str());
    
    int encolados = 0;
    int inicio = 0;
    while (inicio < (int)body.length() && encolados < COLA_MAX_COMANDOS) {
        int finSep = body.indexOf(';', inicio);
        if (finSep == -1) finSep = body.length();
        String par = body.substring(inicio, finSep);
        par.trim();
        
        int coma = par.indexOf(',');
        if (coma > 0 && coma < (int)par.length() - 1) {
            float dist = par.substring(0, coma).toFloat();
            float ang = par.substring(coma + 1).toFloat();
            int vel = 150;
            
            PuntoRuta cmd;
            if (abs(dist) > 0) {
                cmd.distancia = dist;
                cmd.angulo = 0;
                cmd.velocidad = vel;
                cmd.duracion = (unsigned long)(abs(dist) / 5.0f * 1000) + 2000;
            } else if (abs(ang) > 0) {
                cmd.distancia = 0;
                cmd.angulo = ang;
                cmd.velocidad = vel;
                cmd.duracion = 5000;
            } else {
                inicio = finSep + 1;
                continue;
            }
            
            if (cola_push(cmd)) {
                encolados++;
            } else {
                break; // Cola llena
            }
        }
        inicio = finSep + 1;
    }
    
    String msg = "Ruta: " + String(encolados) + " waypoints encolados";
    web_server_send_log(msg.c_str());
    request->send(200, "text/plain", msg);
}

// ─────────────────────────────────────────────────────────────────────
// ENDPOINT /api — Comandos JSON y rutinas (ArduinoJson)
// ─────────────────────────────────────────────────────────────────────
static void handle_api(AsyncWebServerRequest *request) {
    if (!request->_tempObject) {
        request->send(400, "text/plain", "EMPTY BODY");
        return;
    }
    String body = *(String*)request->_tempObject;
    delete (String*)request->_tempObject;
    request->_tempObject = NULL;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    
    if (err) {
        JsonDocument resp;
        resp["error"] = "JSON inválido";
        resp["detalle"] = err.c_str();
        String json;
        serializeJson(resp, json);
        request->send(400, "application/json", json);
        return;
    }
    
    String tipo = doc["tipo"] | "";
    JsonDocument respuesta;
    respuesta["status"] = "ok";
    
    if (tipo == "rutina") {
        JsonArray pasos = doc["pasos"];
        int encolados = 0;
        for (JsonObject paso : pasos) {
            String accion = paso["accion"] | "";
            if (accion == "avanzar") {
                float dist = paso["distancia"] | 0.0f;
                int vel = paso["velocidad"] | 150;
                PuntoRuta cmd;
                cmd.distancia = dist;
                cmd.angulo = 0;
                cmd.velocidad = vel;
                cmd.duracion = (unsigned long)(abs(dist) / 5.0f * 1000) + 2000;
                if (web_server_enqueue_command(cmd)) encolados++;
            } else if (accion == "girar") {
                float ang = paso["angulo"] | 0.0f;
                int vel = paso["velocidad"] | 150;
                PuntoRuta cmd;
                cmd.distancia = 0;
                cmd.angulo = ang;
                cmd.velocidad = vel;
                cmd.duracion = 5000;
                if (web_server_enqueue_command(cmd)) encolados++;
            } else if (accion == "parar") {
                PuntoRuta cmd;
                cmd.distancia = -999;
                if (web_server_enqueue_command(cmd)) encolados++;
            }
        }
        respuesta["encolados"] = encolados;
        web_server_send_log(("Rutina: " + String(encolados) + " pasos").c_str());
    }
    else if (tipo == "config") {
        if (doc["segmento_max"].is<float>()) {
            config_manager_set_segmento_max(doc["segmento_max"]);
        }
        if (doc["vel_pct"].is<float>()) {
            config_manager_set_vel_pct(doc["vel_pct"]);
        }
        if (doc["pausa_ms"].is<unsigned long>()) {
            config_manager_set_pausa_ms(doc["pausa_ms"]);
        }
        if (doc["error_cm"].is<float>()) {
            config_manager_set_error_cm(doc["error_cm"]);
        }
        web_server_send_log("Config segmentación recibida vía JSON");
        respuesta["config"] = "segmentos";
    }
    else if (tipo == "comando") {
        String accion = doc["accion"] | "";
        PuntoRuta cmd = {0, 0, 0, 0};
        if (accion == "avanzar") {
            cmd.distancia = doc["distancia"] | 0.0f;
            cmd.velocidad = doc["velocidad"] | 150;
        } else if (accion == "girar") {
            cmd.angulo = doc["angulo"] | 0.0f;
            cmd.velocidad = doc["velocidad"] | 150;
        } else if (accion == "estop") {
            cmd.distancia = -999;
        }
        cmd.duracion = 5000;
        if (!web_server_enqueue_command(cmd)) {
            respuesta["status"] = "error";
            respuesta["error"] = "queue_full";
            String json;
            serializeJson(respuesta, json);
            request->send(503, "application/json", json);
            return;
        }
        respuesta["comando"] = accion;
    }
    else {
        respuesta["error"] = "tipo desconocido";
        respuesta["status"] = "error";
        String json;
        serializeJson(respuesta, json);
        request->send(400, "application/json", json);
        return;
    }
    
    String json;
    serializeJson(respuesta, json);
    request->send(200, "application/json", json);
}

// ─────────────────────────────────────────────────────────────────────
// ENDPOINT /raw — Comandos serial raw estilo G-code
// ─────────────────────────────────────────────────────────────────────
static void handle_raw(AsyncWebServerRequest *request) {
    String raw = "";
    if (request->method() == HTTP_POST && request->_tempObject) {
        raw = *(String*)request->_tempObject;
        delete (String*)request->_tempObject;
        request->_tempObject = NULL;
    } else if (request->hasParam("cmd")) {
        raw = request->getParam("cmd")->value();
    }
    
    raw.trim();
    if (raw.length() == 0) {
        request->send(400, "text/plain", "EMPTY");
        return;
    }
    
    Serial.printf("[RAW] Comando: %s\n", raw.c_str());
    char tipo = raw.charAt(0);
    
    if (tipo == 'A' || tipo == 'a') {
        int coma = raw.indexOf(',');
        if (coma > 1) {
            float dist = raw.substring(1, coma).toFloat();
            int vel = raw.substring(coma + 1).toInt();
            if (dist > 0 && dist <= 500 && vel >= 50 && vel <= 255) {
                prepararComandoAvance(dist, vel);
                request->send(200, "text/plain", "OK");
                return;
            }
        }
        request->send(400, "text/plain", "BAD DIST/VEL");
        return;
    }
    
    if (tipo == 'G' || tipo == 'g') {
        int coma = raw.indexOf(',');
        if (coma > 1) {
            float ang = raw.substring(1, coma).toFloat();
            int vel = raw.substring(coma + 1).toInt();
            if (abs(ang) > 0 && abs(ang) <= 360 && vel >= 50 && vel <= 255) {
                prepararComandoGiro(ang, vel);
                request->send(200, "text/plain", "OK");
                return;
            }
        }
        request->send(400, "text/plain", "BAD ANG/VEL");
        return;
    }
    
    if (tipo == 'P' || tipo == 'p') {
        PuntoRuta cmd = {0, 0, 0, 0};
        cmd.distancia = -999;
        web_server_enqueue_command(cmd);
        request->send(200, "text/plain", "STOP");
        return;
    }
    
    if (tipo == 'E' || tipo == 'e') {
        PuntoRuta cmd = {0, 0, 0, 0};
        cmd.distancia = -999;
        web_server_enqueue_command(cmd);
        request->send(200, "text/plain", "ESTOP");
        return;
    }
    
    if (tipo == 'R' || tipo == 'r') {
        request->send(200, "text/plain", "RESET");
        return;
    }
    
    if (tipo == '?') {
        request->send(200, "text/plain", "STATUS");
        return;
    }
    
    request->send(400, "text/plain", "UNKNOWN CMD");
}

// ─────────────────────────────────────────────────────────────────────
// WEBSOCKET — Event Handler
// ─────────────────────────────────────────────────────────────────────
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        clientesSSE++;
        Serial.printf("[WS] Cliente conectado: %u (total: %d)\n", client->id(), clientesSSE);
        // Enviar bienvenida
        client->text("{\"tipo\":\"sys\",\"msg\":\"Conectado al robot\"}");
    } else if (type == WS_EVT_DISCONNECT) {
        clientesSSE--;
        if (clientesSSE < 0) clientesSSE = 0;
        Serial.printf("[WS] Cliente desconectado: %u\n", client->id());
    } else if (type == WS_EVT_DATA) {
        // Comando recibido via WebSocket
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            char msgBuf[512];
            size_t copyLen = (len < sizeof(msgBuf)-1) ? len : sizeof(msgBuf)-1;
            memcpy(msgBuf, data, copyLen);
            msgBuf[copyLen] = 0;
            const char* msg = msgBuf;
            
            if (strcmp(msg, "PING") == 0) {
                client->text("{\"tipo\":\"pong\"}");
            }
            else if (strcmp(msg, "RESET") == 0) {
                portENTER_CRITICAL(&muxComandos);
                colaHead = 0; colaTail = 0; colaCount = 0;
                portEXIT_CRITICAL(&muxComandos);
                wsResetRequested = true;
                Serial.println("[WS] RESET: cola limpiada, reset solicitado");
            }
            else if (strncmp(msg, "MV:", 3) == 0) {
                char dir = msg[3];
                int pwm = atoi(msg + 5);
                setMotorsManual(dir, pwm);
            }
            else if (strncmp(msg, "FSM:", 4) == 0) {
                portENTER_CRITICAL(&muxFsmOverride);
                strncpy(wsForcedFsmState, msg + 4, sizeof(wsForcedFsmState)-1);
                wsForcedFsmState[sizeof(wsForcedFsmState)-1] = '\0';
                wsFsmOverrideRequest = true;
                portEXIT_CRITICAL(&muxFsmOverride);
                Serial.printf("[WS] Forzando FSM a: %s\n", msg + 4);
            }
            else if (strncmp(msg, "RUTA:", 5) == 0) {
                // Convertir ruta absoluta a comandos de avance y giro en cola circular
                float X_prev = tel_posX;
                float Y_prev = tel_posY;
                float Heading_prev = tel_orientacion; // Radianes
                
                // Vaciar cola de comandos previa de forma segura
                portENTER_CRITICAL(&muxComandos);
                colaHead = 0;
                colaTail = 0;
                colaCount = 0;
                portEXIT_CRITICAL(&muxComandos);
                
                // Crear una copia de buffer porque strtok modifica el original
                char buf[512];
                strncpy(buf, msg + 5, sizeof(buf));
                buf[sizeof(buf) - 1] = 0;
                
                char* saveptr;
                char* token = strtok_r(buf, ";", &saveptr);
                int encolados = 0;
                while (token != NULL && encolados < COLA_MAX_COMANDOS - 2) {
                    float targetX, targetY;
                    if (sscanf(token, "X%fY%f", &targetX, &targetY) == 2) {
                        float dx = targetX - X_prev;
                        float dy = targetY - Y_prev;
                        float dist = sqrt(dx*dx + dy*dy);
                        
                        if (dist > 0.1f) {
                            float target_heading = atan2(dy, dx); // Radianes
                            float diff_ang = (target_heading - Heading_prev) * (180.0f / PI); // Grados
                            
                            // Normalizar entre [-180, 180]
                            while (diff_ang > 180.0f) diff_ang -= 360.0f;
                            while (diff_ang < -180.0f) diff_ang += 360.0f;
                            
                            // Si hay diferencia angular, encolar giro
                            if (abs(diff_ang) > 1.0f) {
                                PuntoRuta cmdGiro;
                                cmdGiro.distancia = 0;
                                cmdGiro.angulo = diff_ang;
                                cmdGiro.velocidad = 130;
                                cmdGiro.duracion = 5000;
                                cola_push(cmdGiro);
                            }
                            
                            // Encolar avance
                            PuntoRuta cmdAvance;
                            cmdAvance.distancia = dist;
                            cmdAvance.angulo = 0;
                            cmdAvance.velocidad = 150;
                            cmdAvance.duracion = (unsigned long)(dist / 5.0f * 1000) + 2000;
                            cola_push(cmdAvance);
                            
                            X_prev = targetX;
                            Y_prev = targetY;
                            Heading_prev = target_heading;
                            encolados++;
                        }
                    }
                    token = strtok_r(NULL, ";", &saveptr);
                }
                Serial.printf("[WS] Ruta cargada: %d waypoints encolados\n", encolados);
            }
            else if (strcmp(msg, "ESTOP") == 0) {
                // Parada de emergencia
                PuntoRuta pcmd = {0, 0, 0, 0};
                pcmd.distancia = -999;
                
                portENTER_CRITICAL(&muxComandos);
                colaHead = 0;
                colaTail = 0;
                colaCount = 0;
                colaComandos[colaTail] = pcmd;
                colaTail = (colaTail + 1) % COLA_MAX_COMANDOS;
                colaCount = 1;
                portEXIT_CRITICAL(&muxComandos);
                Serial.println("[WS] E-STOP!");
            }
        }
        else if (info->final && info->index == 0 && info->len == len && info->opcode == WS_BINARY) {
            // Binary route: uint16 count + N x PuntoRuta (direct memcpy, O(1))
            if (len >= sizeof(uint16_t)) {
                uint16_t count;
                memcpy(&count, data, sizeof(uint16_t));
                
                size_t expected = sizeof(uint16_t) + count * sizeof(PuntoRuta);
                if (len >= expected && count > 0 && count <= COLA_MAX_COMANDOS) {
                    portENTER_CRITICAL(&muxComandos);
                    colaHead = 0; colaTail = 0; colaCount = 0;
                    for (uint16_t i = 0; i < count && colaCount < COLA_MAX_COMANDOS; i++) {
                        PuntoRuta cmd;
                        memcpy(&cmd, data + sizeof(uint16_t) + i * sizeof(PuntoRuta), sizeof(PuntoRuta));
                        colaComandos[colaTail] = cmd;
                        colaTail = (colaTail + 1) % COLA_MAX_COMANDOS;
                        colaCount++;
                    }
                    portEXIT_CRITICAL(&muxComandos);
                    Serial.printf("[WS] Ruta binaria: %d waypoints\n", count);
                }
            }
        }
    }
}

// ── FSM state to numeric (for binary protocol) ──
uint8_t estadoToUint8(const char* estado) {
    if (strcmp(estado, "INIT") == 0) return 0;
    if (strcmp(estado, "CALIBRATING") == 0) return 1;
    if (strcmp(estado, "IDLE") == 0) return 2;
    if (strcmp(estado, "ADVANCING") == 0) return 3;
    if (strcmp(estado, "TURNING") == 0) return 4;
    if (strcmp(estado, "BRAKING") == 0) return 5;
    if (strcmp(estado, "ESTOP") == 0) return 6;
    if (strcmp(estado, "MANUAL") == 0) return 7;
    return 2; // default: IDLE
}

// PUSH telemetry to all WebSocket clients (binary, 0 heap alloc)
static void ws_push_telemetry() {
    if (clientesSSE == 0) return;
    
    TelemetryBinary bin;
    bin.version = 0x01;
    
    portENTER_CRITICAL(&muxTelWeb);
    bin.estado = estadoToUint8(tel_estado);
    bin.distancia = tel_distancia;
    bin.angulo = tel_angulo;
    bin.pulsosIzq = tel_pulsosIzq;
    bin.pulsosDer = tel_pulsosDer;
    bin.pwmIzq = (uint16_t)tel_pwmIzq;
    bin.pwmDer = (uint16_t)tel_pwmDer;
    bin.posX = tel_posX;
    bin.posY = tel_posY;
    bin.orientacion = tel_orientacion;
    bin.pidCorr = tel_pidCorr;
    bin.distRestante = tel_distRestante;
    bin.targetDist = tel_targetDist;
    bin.targetVel = (uint16_t)tel_targetVel;
    bin.padding = 0;
    portEXIT_CRITICAL(&muxTelWeb);
    
    ws.binaryAll((uint8_t*)&bin, sizeof(TelemetryBinary));
}

// ─────────────────────────────────────────────────────────────────────
// INICIALIZACIÓN DEL SERVIDOR WEB
// ─────────────────────────────────────────────────────────────────────

void web_server_init() {
    Serial.println("\n[WEB] Iniciando modo Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    WiFi.setHostname("robot");
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    String wifiSsid = config_manager_get_wifi_ssid();
    String wifiPass = config_manager_get_wifi_password();
    bool apOk = WiFi.softAP(wifiSsid.c_str(), wifiPass.c_str());
    if (!apOk) {
        Serial.println("[WEB] ERROR: no se pudo crear el Access Point");
    }
    
    IPAddress ip = WiFi.softAPIP();
    Serial.printf("[WEB] Red: %s\n", wifiSsid.c_str());
    Serial.printf("[WEB] IP fija: http://%s/\n", ip.toString().c_str());
    Serial.println("[WEB] Conectese a la red desde su telefono y abra esa URL");
    
    if (MDNS.begin("robot")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[WEB] mDNS: http://robot.local/");
    } else {
        Serial.println("[WEB] mDNS no disponible");
    }
    
    auto serveIndex = [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", get_html_page());
    };

    server.on("/", HTTP_GET, serveIndex);
    server.on("/index.html", HTTP_GET, serveIndex);
    server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
    });
    server.on("/status", HTTP_GET, handle_status);
    
    // Rutas POST sencillas (application/x-www-form-urlencoded)
    server.on("/cmd", HTTP_POST, handle_cmd);
    
    // Rutas POST con payload (texto raw o JSON)
    server.on("/ruta", HTTP_POST, handle_ruta, NULL, handle_body_accumulation);
    server.on("/api", HTTP_POST, handle_api, NULL, handle_body_accumulation);
    
    // Rutas RAW (puede ser GET o POST con payload)
    server.on("/raw", HTTP_POST, handle_raw, NULL, handle_body_accumulation);
    server.on("/raw", HTTP_GET, handle_raw);
    
    server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html><body style='background:#0f172a;color:#e2e8f0;font-family:sans-serif;padding:2rem;'>";
        html += "<h1>Robot Conectado</h1>";
        html += "<p>Red: <b>" + config_manager_get_wifi_ssid() + "</b></p>";
        html += "<p>IP: <b>http://" + web_server_get_ip() + "/</b></p>";
        html += "<p>mDNS: <b>http://robot.local/</b></p>";
        html += "<p>MAC: " + WiFi.softAPmacAddress() + "</p>";
        html += "<p><a href='/' style='color:#60a5fa;'>Ir al panel de control</a></p>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });
    
    // ── Captive Portal ──
    static const char CAPTIVE_HTML[] PROGMEM = "<!DOCTYPE html>"
    "<html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='0;url=/'>"
    "<title>Robot Control</title></head>"
    "<body style='background:#0f172a;color:#e2e8f0;font-family:sans-serif;padding:2rem;'>"
    "<h1>Robot Autonomo</h1>"
    "<p>Conectado al Access Point local</p>"
    "<p>El panel de control está listo.</p>"
    "<p><a href='/' style='color:#60a5fa;font-size:1.2rem;'>Abrir panel de control</a></p>"
    "</body></html>";

    auto sendCaptiveResponse = [](AsyncWebServerRequest *request, int statusCode, const String& contentType, const String& body) {
        AsyncWebServerResponse *response = request->beginResponse(statusCode, contentType, body);
        response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("X-Frame-Options", "SAMEORIGIN");
        request->send(response);
    };

    server.on("/hotspot-detect.html", HTTP_GET, [sendCaptiveResponse](AsyncWebServerRequest *request) {
        sendCaptiveResponse(request, 200, "text/html", String(CAPTIVE_HTML));
    });
    server.on("/library/test/success.html", HTTP_GET, [sendCaptiveResponse](AsyncWebServerRequest *request) {
        sendCaptiveResponse(request, 200, "text/html", String(CAPTIVE_HTML));
    });
    
    auto generate204 = [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(204);
        response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        response->addHeader("Pragma", "no-cache");
        request->send(response);
    };
    server.on("/generate_204", HTTP_GET, generate204);
    server.on("/connectivitycheck.gstatic.com/generate_204", HTTP_GET, generate204);
    
    server.on("/ncsi.txt", HTTP_GET, [sendCaptiveResponse](AsyncWebServerRequest *request) {
        sendCaptiveResponse(request, 200, "text/plain", "Microsoft NCSI");
    });
    
    // DNS local
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->_tempObject) {
            delete (String*)request->_tempObject;
            request->_tempObject = NULL;
        }
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(200, "text/html", get_html_page());
        }
    });
    
    // WebSocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    
    server.begin();
    Serial.println("[WEB] Servidor asíncrono iniciado en puerto 80");
    Serial.printf("[WEB] WebSocket en ws://%s/ws\n", WiFi.softAPIP().toString().c_str());
}

static unsigned long tUltimoWSPush = 0;

void web_server_loop() {
    dnsServer.processNextRequest();
    ws.cleanupClients();
    
    // Push telemetria a 10Hz via WebSocket
    unsigned long ahora = millis();
    if (ahora - tUltimoWSPush > 100) {
        tUltimoWSPush = ahora;
        ws_push_telemetry();
    }
}

void web_server_stop() {
    ws.closeAll();
    dnsServer.stop();
    MDNS.end();
    WiFi.disconnect(true);
    Serial.println("[WEB] Servidor detenido");
}

bool web_server_is_connected() {
    return WiFi.softAPgetStationNum() > 0;
}

String web_server_get_ip() {
    return WiFi.softAPIP().toString();
}

void web_server_send_telemetry(float distancia, float angulo, 
                                long pulsosIzq, long pulsosDer, 
                                int pwmIzq, int pwmDer, 
                                const char* estado,
                                float posX, float posY, float orientacion,
                                float pidCorr, float distRestante,
                                float targetDist, int targetVel) {
    portENTER_CRITICAL(&muxTelWeb);
    tel_distancia = distancia;
    tel_angulo = angulo;
    tel_pulsosIzq = pulsosIzq;
    tel_pulsosDer = pulsosDer;
    tel_pwmIzq = pwmIzq;
    tel_pwmDer = pwmDer;
    strncpy(tel_estado, estado, sizeof(tel_estado) - 1);
    tel_estado[sizeof(tel_estado) - 1] = 0;
    tel_posX = posX;
    tel_posY = posY;
    tel_orientacion = orientacion;
    tel_pidCorr = pidCorr;
    tel_distRestante = distRestante;
    tel_targetDist = targetDist;
    tel_targetVel = targetVel;
    portEXIT_CRITICAL(&muxTelWeb);
}

void web_server_send_log(const char* mensaje) {
    Serial.printf("[WEB] Log: %s\n", mensaje);
}

bool web_server_has_command() {
    return cola_has_data();
}

PuntoRuta web_server_get_command() {
    PuntoRuta cmd = {0, 0, 0, 0};
    cola_pop(&cmd);
    return cmd;
}

bool web_server_enqueue_command(PuntoRuta cmd) {
    return cola_push(cmd);
}

void web_server_broadcast(const char* msg) {
    if (ws.count() > 0) {
        ws.textAll(msg);
    }
}
