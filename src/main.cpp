// ═══════════════════════════════════════════════════════════════════════
// MAIN.CPP — Carro Robot Autónomo con FSM No-Bloqueante
// ═══════════════════════════════════════════════════════════════════════
// Arquitectura: Finite State Machine (FSM) donde loop() NUNCA bloquea.
// Cada tick de estado es un fragmento de trabajo, no un bucle.
// 
// Basado en: new_template.txt (MPU6050 + Encoders + L298N)
// Adaptado a: ESP32-S3-DevKitC-1 con arquitectura async
// ═══════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "config.h"
#include "hal_pins.h"
#include "web_server.h"
#include "comandos.h"
#include "config_manager.h"
#include "debug_logger.h"

// ─────────────────────────────────────────────────────────────────────
// ESTADOS DE LA FSM
// ─────────────────────────────────────────────────────────────────────
enum EstadoRobot {
    STATE_INIT,
    STATE_CALIBRATING,
    STATE_IDLE,
    STATE_ADVANCING,
    STATE_TURNING,
    STATE_BRAKING,
    STATE_ESTOP,
    STATE_MANUAL
};

const char* nombreEstado(EstadoRobot e) {
    switch (e) {
        case STATE_INIT:        return "INIT";
        case STATE_CALIBRATING: return "CALIBRATING";
        case STATE_IDLE:        return "IDLE";
        case STATE_ADVANCING:   return "ADVANCING";
        case STATE_TURNING:     return "TURNING";
        case STATE_BRAKING:     return "BRAKING";
        case STATE_ESTOP:       return "ESTOP";
        case STATE_MANUAL:      return "MANUAL";
        default:                return "UNKNOWN";
    }
}

// Prototipos de FSM
void cambiarEstado(EstadoRobot nuevo);

// ─────────────────────────────────────────────────────────────────────
// ESTRUCTURAS DE DATOS
// ─────────────────────────────────────────────────────────────────────
struct Objetivo {
    float distancia;     // cm (para avance)
    float angulo;        // grados (para giro)
    int velocidad;       // PWM (0-255)
    unsigned long timeout;   // ms máximo para completar
};

struct Telemetria {
    float distanciaRecorrida;  // cm
    float anguloActual;        // grados
    long pulsosIzq;
    long pulsosDer;
    int pwmIzq;
    int pwmDer;
    EstadoRobot estado;
    float posX;                // cm (odometría global)
    float posY;                // cm
    float orientacionGlobal;   // radianes
    float pidCorr;             // Correccion PID actual
    float distRestante;        // Distancia restante del objetivo
    float targetDist;          // Distancia objetivo
    int targetVel;             // Velocidad objetivo
};

struct IMUCalibration {
    float offsetGX;
    float offsetGY;
    float offsetGZ;
    int samplesCount;
    float sumGX, sumGY, sumGZ;
    bool calibrated;
};

// ─────────────────────────────────────────────────────────────────────
// VARIABLES GLOBALES
// ─────────────────────────────────────────────────────────────────────

// FSM
EstadoRobot estado = STATE_INIT;
Objetivo objetivo = {0, 0, 0, 0};
Telemetria telemetria = {0, 0, 0, 0, 0, 0, STATE_INIT};
IMUCalibration imuCal = {0, 0, 0, 0, 0, 0, 0, false};
volatile unsigned long tUltimoComandoManual = 0;

// Timing
unsigned long tAnterior = 0;
unsigned long tInicioEstado = 0;
unsigned long tUltimoTelem = 0;
unsigned long tLoopMax = 0;

// IMU
Adafruit_MPU6050 mpu;
float angX = 0, angY = 0, angZ = 0;

// Odometría global (dead reckoning)
float posX = 0;                // cm
float posY = 0;                // cm
float orientacionGlobal = 0;   // radianes
float distPrevia = 0;          // distancia acumulada en tick anterior (para delta)
float angPrevio = 0;           // ángulo acumulado en tick anterior (para delta)

// Encoders (volatile + protegidos por spinlock)
volatile long pulsosIzq = 0;
volatile long pulsosDer = 0;
portMUX_TYPE muxEncoder = portMUX_INITIALIZER_UNLOCKED;

// PID
float pidIntegral = 0;
float pidErrorAnterior = 0;
float pidCorr = 0;           // Correccion PID calculada (para debug web)
float distRestante = 0;      // Distancia restante del objetivo actual

// E-Stop disparado desde ISR (volatile para lectura segura desde loop)
volatile bool estadoEStopDisparado = false;

// Segmentación (protegidas con portMUX para acceso desde comandos.cpp y main.cpp)
portMUX_TYPE muxSegmentacion = portMUX_INITIALIZER_UNLOCKED;
bool avanceSegmentadoActivo = false;
int segmentosRestantes = 0;

// Spinlock para acceso seguro a telemetría desde web_server y loop
portMUX_TYPE muxTelemetria = portMUX_INITIALIZER_UNLOCKED;

// ─────────────────────────────────────────────────────────────────────
// ISR — Interrupciones de Encoder y E-Stop (IRAM, thread-safe)
// ─────────────────────────────────────────────────────────────────────
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

// E-Stop ISR: respuesta inmediata a emergencia, complementa el polling en loop()
void IRAM_ATTR isrEStop() {
    // La transición del estado se maneja en loop() vía variable atómica
    // para no llamar cambiarEstado() desde ISR (no es IRAM-Safe)
    estadoEStopDisparado = true;
}

// Lectura segura desde loop()
long leerPulsosSeguro(volatile long *contador) {
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

// ─────────────────────────────────────────────────────────────────────
// MOTOR DRIVER
// ─────────────────────────────────────────────────────────────────────

// Helper: actualiza telemetria PWM con spinlock (evita duplicacion DRY)
static inline void setTelemetriaPWM(int pwmI, int pwmD) {
    portENTER_CRITICAL(&muxTelemetria);
    telemetria.pwmIzq = pwmI;
    telemetria.pwmDer = pwmD;
    portEXIT_CRITICAL(&muxTelemetria);
}

void motorDetener() {
    ledcWrite(PWM_CHANNEL_ENA, 0);
    ledcWrite(PWM_CHANNEL_ENB, 0);
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, LOW);
    setTelemetriaPWM(0, 0);
}

void motorAvanzar(int pwmIzq, int pwmDer) {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, HIGH);
    digitalWrite(PIN_IN4, LOW);
    ledcWrite(PWM_CHANNEL_ENA, constrain(pwmIzq, 0, 255));
    ledcWrite(PWM_CHANNEL_ENB, constrain(pwmDer, 0, 255));
    setTelemetriaPWM(pwmIzq, pwmDer);
}

void motorGirar(bool derecha, int pwm) {
    if (derecha) {
        digitalWrite(PIN_IN1, HIGH);
        digitalWrite(PIN_IN2, LOW);
        digitalWrite(PIN_IN3, LOW);
        digitalWrite(PIN_IN4, HIGH);
    } else {
        digitalWrite(PIN_IN1, LOW);
        digitalWrite(PIN_IN2, HIGH);
        digitalWrite(PIN_IN3, HIGH);
        digitalWrite(PIN_IN4, LOW);
    }
    ledcWrite(PWM_CHANNEL_ENA, constrain(pwm, 0, 255));
    ledcWrite(PWM_CHANNEL_ENB, constrain(pwm, 0, 255));
    setTelemetriaPWM(pwm, pwm);
}

void motorRetroceder(int pwmIzq, int pwmDer) {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, HIGH);
    ledcWrite(PWM_CHANNEL_ENA, constrain(pwmIzq, 0, 255));
    ledcWrite(PWM_CHANNEL_ENB, constrain(pwmDer, 0, 255));
    setTelemetriaPWM(pwmIzq, pwmDer);
}

void setMotorsManual(char dir, int pwm) {
    tUltimoComandoManual = millis();
    
    if (dir != 'S' && estado != STATE_MANUAL) {
        cambiarEstado(STATE_MANUAL);
    }
    
    switch (dir) {
        case 'F':
            motorAvanzar(pwm, pwm);
            break;
        case 'B':
            motorRetroceder(pwm, pwm);
            break;
        case 'L':
            motorGirar(false, pwm);
            break;
        case 'R':
            motorGirar(true, pwm);
            break;
        case 'S':
            motorDetener();
            if (estado == STATE_MANUAL) {
                cambiarEstado(STATE_IDLE);
            }
            break;
        default:
            motorDetener();
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────
// LED DE ESTADO
// ─────────────────────────────────────────────────────────────────────
void actualizarLEDs(EstadoRobot est) {
    digitalWrite(PIN_LED_IDLE, est == STATE_IDLE ? HIGH : LOW);
    digitalWrite(PIN_LED_MOVING, (est == STATE_ADVANCING || est == STATE_TURNING) ? HIGH : LOW);
    digitalWrite(PIN_LED_ERROR, est == STATE_ESTOP ? HIGH : LOW);
}

// ─────────────────────────────────────────────────────────────────────
// CAMBIO DE ESTADO
// ─────────────────────────────────────────────────────────────────────
void cambiarEstado(EstadoRobot nuevo) {
    Serial.print("[FSM] ");
    Serial.print(nombreEstado(estado));
    Serial.print(" → ");
    Serial.println(nombreEstado(nuevo));

    estado = nuevo;
    tInicioEstado = millis();
    telemetria.estado = nuevo;
    actualizarLEDs(nuevo);

    // Acciones de entrada
    switch (nuevo) {
        case STATE_BRAKING:
            motorDetener();
            break;
        case STATE_ESTOP:
            motorDetener();
            pidIntegral = 0;
            pidErrorAnterior = 0;
            web_server_broadcast("{\"tipo\":\"estop\"}");
            break;
        case STATE_ADVANCING:
            resetPulsos();
            pidIntegral = 0;
            pidErrorAnterior = 0;
            distPrevia = 0;
            angPrevio = angZ;
            break;
        case STATE_TURNING:
            angZ = 0;  // Reset ángulo integrado
            break;
        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────
// IMU — Lectura y Calibración
// ─────────────────────────────────────────────────────────────────────
bool inicializarIMU() {
    if (!mpu.begin()) {
        Serial.println("[IMU] ERROR: MPU6050 no encontrado!");
        return false;
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("[IMU] MPU6050 inicializado");
    return true;
}

void tickCalibrationIMU() {
    if (imuCal.samplesCount < IMU_CALIBRATION_SAMPLES) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        imuCal.sumGX += g.gyro.x;
        imuCal.sumGY += g.gyro.y;
        imuCal.sumGZ += g.gyro.z;
        imuCal.samplesCount++;

        // Parpadear LED durante calibración
        if (imuCal.samplesCount % 50 == 0) {
            digitalWrite(PIN_LED_MOVING, !digitalRead(PIN_LED_MOVING));
            Serial.print("[IMU] Calibrando... ");
            Serial.print(imuCal.samplesCount);
            Serial.print("/");
            Serial.println(IMU_CALIBRATION_SAMPLES);
        }
    } else if (!imuCal.calibrated) {
        imuCal.offsetGX = imuCal.sumGX / IMU_CALIBRATION_SAMPLES;
        imuCal.offsetGY = imuCal.sumGY / IMU_CALIBRATION_SAMPLES;
        imuCal.offsetGZ = imuCal.sumGZ / IMU_CALIBRATION_SAMPLES;
        imuCal.calibrated = true;

        Serial.println("[IMU] Calibración completa:");
        Serial.print("  Offset GX: "); Serial.println(imuCal.offsetGX, 6);
        Serial.print("  Offset GY: "); Serial.println(imuCal.offsetGY, 6);
        Serial.print("  Offset GZ: "); Serial.println(imuCal.offsetGZ, 6);

        digitalWrite(PIN_LED_MOVING, LOW);
        cambiarEstado(STATE_IDLE);
    }
}

void leerIMU(float dt) {
    // FIX [MEDIUM] Validar explícitamente que calibración fue exitosa
    if (!imuCal.calibrated || imuCal.samplesCount < IMU_CALIBRATION_SAMPLES) {
        return;  // No proceder si calibración incompleta
    }

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convertir a °/s con calibración
    float gx = (g.gyro.x - imuCal.offsetGX) * (180.0f / PI);  // rad/s → °/s
    float gy = (g.gyro.y - imuCal.offsetGY) * (180.0f / PI);
    float gz = (g.gyro.z - imuCal.offsetGZ) * (180.0f / PI);

    // Integrar ángulos
    angX += gx * dt;
    angY += gy * dt;
    angZ += gz * dt;

    // Filtro complementario con acelerómetro (solo para pitch/roll, no yaw)
    float accelAngleX = atan2(a.acceleration.y, a.acceleration.z) * 180.0f / PI;
    float accelAngleY = atan2(-a.acceleration.x, a.acceleration.z) * 180.0f / PI;
    angX = COMPLEMENTARY_ALPHA * angX + (1.0f - COMPLEMENTARY_ALPHA) * accelAngleX;
    angY = COMPLEMENTARY_ALPHA * angY + (1.0f - COMPLEMENTARY_ALPHA) * accelAngleY;
    // Yaw (angZ) solo se integra del gyro — el acelerómetro no lo puede medir

    telemetria.anguloActual = angZ;
}

// ─────────────────────────────────────────────────────────────────────
// TICK DE ESTADOS (Fragmentos, no bucles)
// ─────────────────────────────────────────────────────────────────────

void tickAvance(float dt) {
    unsigned long ahora = millis();

    // Timeout de seguridad
    if (ahora - tInicioEstado > objetivo.timeout) {
        Serial.println("[FSM] Timeout de avance!");
        cambiarEstado(STATE_BRAKING);
        return;
    }

    // Leer encoders
    long pI = leerPulsosSeguro(&pulsosIzq);
    long pD = leerPulsosSeguro(&pulsosDer);
    float distProm = ((pI + pD) / 2.0f) * CM_POR_PULSO;
    telemetria.distanciaRecorrida = distProm;
    telemetria.pulsosIzq = pI;
    telemetria.pulsosDer = pD;

    // ¿Llegamos? (margen de error proporcional para segmentos cortos)
    // FIX [CRITICAL] Asegurar margen mínimo incluso para distancias muy cortas
    float margenDinamico;
    if (objetivo.distancia < 5.0f) {
        // Para distancias muy cortas (< 5cm), usar 10% o 0.5cm, lo que sea mayor
        margenDinamico = max(0.5f, objetivo.distancia * 0.1f);
    } else if (objetivo.distancia < MARGEN_ERROR_CM * 5) {
        // Rango medio: usar 5% del objetivo
        margenDinamico = objetivo.distancia * 0.05f;
    } else {
        // Rango largo: usar margen fijo
        margenDinamico = MARGEN_ERROR_CM;
    }
    float umbralLlegada = objetivo.distancia - margenDinamico;
    // Verificación redundante (no debería ser necesaria)
    if (umbralLlegada < 0) umbralLlegada = 0;
    if (distProm >= umbralLlegada) {
        Serial.print("[FSM] Distancia alcanzada: ");
        Serial.print(distProm, 1);
        Serial.println(" cm");
        cambiarEstado(STATE_BRAKING);
        return;
    }

    // Odometría global: actualizar X/Y con el delta de distancia
    // FIX [CRITICAL] Usar orientacionGlobal ANTES de actualizar para cálculo de posición
    float deltaDist = distProm - distPrevia;
    if (deltaDist > 0) {
        // Calcular heading ANTES de actualizar orientación
        float deltaAnglRad = (angZ - angPrevio) * (PI / 180.0f);
        // Usar la orientación ACTUAL (no futura) para integración
        posX += deltaDist * cos(orientacionGlobal);
        posY += deltaDist * sin(orientacionGlobal);
        // AHORA actualizar orientación para próxima iteración
        orientacionGlobal += deltaAnglRad;
        distPrevia = distProm;
        angPrevio = angZ;
    }
    telemetria.posX = posX;
    telemetria.posY = posY;
    telemetria.orientacionGlobal = orientacionGlobal;
    telemetria.pidCorr = pidCorr;
    telemetria.distRestante = distRestante;
    telemetria.targetDist = objetivo.distancia;
    telemetria.targetVel = objetivo.velocidad;

    // PID diferencial: igualar velocidad de ambas ruedas
    float error = (float)(pI - pD);
    pidIntegral += error * dt;
    pidIntegral = constrain(pidIntegral, -MAX_PID_INTEGRAL, MAX_PID_INTEGRAL);
    float derivada = (dt > 0) ? (error - pidErrorAnterior) / dt : 0;
    pidErrorAnterior = error;

    pidCorr = PID_KP * error + PID_KI * pidIntegral + PID_KD * derivada;
    int pwmI = constrain(objetivo.velocidad - (int)pidCorr, 0, 255);
    int pwmD = constrain(objetivo.velocidad + (int)pidCorr, 0, 255);

    // Rampa de desaceleración en últimos DISTANCIA_DECEL_CM
    float restante = objetivo.distancia - distProm;
    distRestante = restante;
    if (restante < DISTANCIA_DECEL_CM) {
        float factor = max(FACTOR_MIN_DECEL, restante / DISTANCIA_DECEL_CM);
        pwmI = (int)(pwmI * factor);
        pwmD = (int)(pwmD * factor);
    }

    motorAvanzar(pwmI, pwmD);
}

void tickGiro(float dt) {
    unsigned long ahora = millis();

    // Timeout de seguridad
    if (ahora - tInicioEstado > TIMEOUT_GIRO_MS) {
        Serial.println("[FSM] Timeout de giro!");
        cambiarEstado(STATE_BRAKING);
        return;
    }

    // ¿Llegamos al ángulo?
    float errorAng = objetivo.angulo - angZ;
    if (abs(errorAng) < UMBRAL_GIRO_GRADOS) {
        Serial.print("[FSM] Ángulo alcanzado: ");
        Serial.print(angZ, 1);
        Serial.println("°");
        // Actualizar orientación global con angZ real (medido por IMU)
        orientacionGlobal += angZ * (PI / 180.0f);
        telemetria.posX = posX;
        telemetria.posY = posY;
        telemetria.orientacionGlobal = orientacionGlobal;
        cambiarEstado(STATE_BRAKING);
        return;
    }

    // Girar en la dirección correcta
    bool giroDerecha = (errorAng > 0);
    
    // Reducir velocidad al acercarse al ángulo objetivo
    int pwm = objetivo.velocidad;
    if (abs(errorAng) < 20.0f) {
        pwm = max(80, (int)(objetivo.velocidad * abs(errorAng) / 20.0f));
    }

    motorGirar(giroDerecha, pwm);
}

void tickFrenado() {
    unsigned long ahora = millis();
    portENTER_CRITICAL(&muxSegmentacion);
    bool segmentado = avanceSegmentadoActivo;
    portEXIT_CRITICAL(&muxSegmentacion);
    unsigned long pausaRequerida = segmentado ? TIEMPO_PAUSA_SEGMENTO_MS : 200;

    if (ahora - tInicioEstado > pausaRequerida) {
        cambiarEstado(STATE_IDLE);
    }
}

// ─────────────────────────────────────────────────────────────────────
// PROCESAMIENTO DE COMANDOS SERIAL
// ─────────────────────────────────────────────────────────────────────
// Comandos de avance/giro usan prepararComandoAvance/Giro (comandos.cpp)
// para segmentación automática. Comandos de control (P/E/R) manipulan
// el estado FSM directamente.
// ─────────────────────────────────────────────────────────────────────
void procesarComandoSerial() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() == 0) return;

    char tipo = cmd.charAt(0);
    
    switch (tipo) {
        case 'A': case 'a': {
            // Avanzar: A<distancia>,<velocidad>  ej: A100,200
            int coma = cmd.indexOf(',');
            if (coma > 1 && estado == STATE_IDLE) {
                float dist = cmd.substring(1, coma).toFloat();
                int vel = cmd.substring(coma + 1).toInt();
                if (dist > 0 && dist <= 500 && vel >= 50 && vel <= 255) {
                    Serial.print("[CMD] Avanzar ");
                    Serial.print(dist, 1);
                    Serial.print(" cm @ PWM ");
                    Serial.println(vel);
                    prepararComandoAvance(dist, vel);
                } else {
                    Serial.println("[CMD] Error: dist 1-500, vel 50-255");
                }
            } else if (estado != STATE_IDLE) {
                Serial.println("[CMD] Error: robot ocupado");
            }
            break;
        }
        case 'G': case 'g': {
            // Girar: G<angulo>,<velocidad>  ej: G90,150
            int coma = cmd.indexOf(',');
            if (coma > 1 && estado == STATE_IDLE) {
                float ang = cmd.substring(1, coma).toFloat();
                int vel = cmd.substring(coma + 1).toInt();
                if (abs(ang) > 0 && abs(ang) <= 360 && vel >= 50 && vel <= 255) {
                    Serial.print("[CMD] Girar ");
                    Serial.print(ang, 1);
                    Serial.print("° @ PWM ");
                    Serial.println(vel);
                    prepararComandoGiro(ang, vel);
                } else {
                    Serial.println("[CMD] Error: ang 1-360, vel 50-255");
                }
            }
            break;
        }
        case 'P': case 'p':
            // Parar
            Serial.println("[CMD] Parar");
            cambiarEstado(STATE_BRAKING);
            break;
        case 'E': case 'e':
            // E-Stop
            Serial.println("[CMD] E-STOP!");
            cambiarEstado(STATE_ESTOP);
            break;
        case 'R': case 'r':
            // Reset desde E-Stop
            if (estado == STATE_ESTOP) {
                Serial.println("[CMD] Reset desde E-Stop");
                cambiarEstado(STATE_IDLE);
            }
            break;
        case '?':
            // Estado actual
            Serial.print("[INFO] Estado: "); Serial.println(nombreEstado(estado));
            Serial.print("[INFO] Dist: "); Serial.print(telemetria.distanciaRecorrida, 1); Serial.println(" cm");
            Serial.print("[INFO] Ang: "); Serial.print(telemetria.anguloActual, 1); Serial.println("°");
            Serial.print("[INFO] Pulsos I/D: "); Serial.print(telemetria.pulsosIzq);
            Serial.print("/"); Serial.println(telemetria.pulsosDer);
            Serial.print("[INFO] PWM I/D: "); Serial.print(telemetria.pwmIzq);
            Serial.print("/"); Serial.println(telemetria.pwmDer);
            Serial.print("[INFO] Loop max: "); Serial.print(tLoopMax); Serial.println(" µs");
            break;
        default:
            Serial.println("[CMD] Comandos: A<dist>,<vel> | G<ang>,<vel> | P | E | R | ?");
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────
// TELEMETRÍA SERIAL (temporal, hasta implementar SSE)
// ─────────────────────────────────────────────────────────────────────
void enviarTelemetriaSerial() {
    unsigned long ahora = millis();
    if (ahora - tUltimoTelem < SSE_INTERVAL_MS) return;
    tUltimoTelem = ahora;

    // Solo enviar si estamos en movimiento (DEBUG condicional para no bloquear loop)
    if (estado == STATE_ADVANCING || estado == STATE_TURNING) {
        DEBUG_PRINT("[TELEM] ");
        DEBUG_PRINT(nombreEstado(estado));
        DEBUG_PRINT(" | dist="); DEBUG_PRINT(telemetria.distanciaRecorrida, 1);
        DEBUG_PRINT("cm | ang="); DEBUG_PRINT(telemetria.anguloActual, 1);
        DEBUG_PRINT("° | pI="); DEBUG_PRINT(telemetria.pulsosIzq);
        DEBUG_PRINT(" pD="); DEBUG_PRINT(telemetria.pulsosDer);
        DEBUG_PRINT(" | pwmI="); DEBUG_PRINT(telemetria.pwmIzq);
        DEBUG_PRINT(" pwmD="); DEBUG_PRINT(telemetria.pwmDer);
        DEBUG_PRINT(" | X="); DEBUG_PRINT(telemetria.posX, 1);
        DEBUG_PRINT(" Y="); DEBUG_PRINTLN(telemetria.posY, 1);
    }
    
    // Enviar a la web en cada intervalo, independientemente de si se mueve
    // Leer PWM bajo spinlock para consistencia con motorDriver
    portENTER_CRITICAL(&muxTelemetria);
    int telPwmI = telemetria.pwmIzq;
    int telPwmD = telemetria.pwmDer;
    portEXIT_CRITICAL(&muxTelemetria);
    
    web_server_send_telemetry(telemetria.distanciaRecorrida, telemetria.anguloActual, 
                              telemetria.pulsosIzq, telemetria.pulsosDer, 
                              telPwmI, telPwmD, 
                              nombreEstado(estado),
                              telemetria.posX, telemetria.posY, telemetria.orientacionGlobal,
                              telemetria.pidCorr, telemetria.distRestante,
                              telemetria.targetDist, telemetria.targetVel);
}

// ─────────────────────────────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);  // Esperar estabilización USB

    Serial.println("═══════════════════════════════════════════");
    Serial.println("  Carro Robot Autónomo — ESP32-S3");
    Serial.println("  FSM No-Bloqueante v1.0");
    Serial.println("═══════════════════════════════════════════");

    // ─────────────────────────────────────────────────────────────
    // HAL - Inicializar capa de abstracción de hardware (pines)
    // ─────────────────────────────────────────────────────────────
    hal_pins_init();
    Serial.println("[INIT] Hardware Abstraction Layer inicializado");

    // --- Config Manager (NVS) ---
    config_manager_init();
    Serial.println("[INIT] Config Manager inicializado");

    motorDetener();
    Serial.println("[INIT] Motores configurados");

    // --- Encoders ---
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_IZQ), isrEncoderIzq, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DER), isrEncoderDer, RISING);
    Serial.println("[INIT] Encoders configurados");

    // --- E-Stop ISR ---
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP), isrEStop, RISING);
    Serial.println("[INIT] E-Stop ISR configurado");

    // --- IMU ---
    if (inicializarIMU()) {
        cambiarEstado(STATE_CALIBRATING);
    } else {
        Serial.println("[INIT] ADVERTENCIA: Sin IMU — giros no disponibles");
        cambiarEstado(STATE_IDLE);
    }

    // --- Constantes ---
    Serial.print("[INIT] CM_POR_PULSO = ");
    Serial.println(CM_POR_PULSO, 4);
    Serial.print("[INIT] Diámetro rueda = ");
    Serial.print(DIAMETRO_RUEDA_CM, 1);
    Serial.println(" cm");
    Serial.println("[INIT] Comandos: A<dist>,<vel> | G<ang>,<vel> | P | E | R | ?");
    Serial.println("═══════════════════════════════════════════");

    // --- Web Server ---
    web_server_init();

    tAnterior = micros();
}

// ─────────────────────────────────────────────────────────────────────
// LOOP PRINCIPAL — Nunca bloquea. Tick < 5ms.
// ─────────────────────────────────────────────────────────────────────
void loop() {
    unsigned long tLoopInicio = micros();

    // ① Calcular dt
    unsigned long ahora = micros();
    float dt = (ahora - tAnterior) / 1e6f;
    tAnterior = ahora;

    // ② Leer sensores (siempre)
    leerIMU(dt);

    // ③ Verificar E-Stop (ISR flag + polling redundante)
    if (estadoEStopDisparado || (digitalRead(PIN_ESTOP) == HIGH && estado != STATE_ESTOP)) {
        if (estadoEStopDisparado) {
            estadoEStopDisparado = false; // Clear ISR flag
            Serial.println("[E-STOP] Emergencia detectada por ISR!");
        } else {
            Serial.println("[E-STOP] Botón de emergencia presionado (polling)!");
        }
        cambiarEstado(STATE_ESTOP);
    }

    // ③.5 Modo Debug FSM Override (solicitado via WebSocket)
    if (wsFsmOverrideRequest) {
        portENTER_CRITICAL(&muxFsmOverride);
        wsFsmOverrideRequest = false;
        String reqState = String(wsForcedFsmState);
        portEXIT_CRITICAL(&muxFsmOverride);
        
        EstadoRobot nuevo = estado;
        if(reqState == "INIT") nuevo = STATE_INIT;
        else if(reqState == "CALIBRATING") nuevo = STATE_CALIBRATING;
        else if(reqState == "IDLE") nuevo = STATE_IDLE;
        else if(reqState == "ADVANCING") nuevo = STATE_ADVANCING;
        else if(reqState == "TURNING") nuevo = STATE_TURNING;
        else if(reqState == "BRAKING") nuevo = STATE_BRAKING;
        else if(reqState == "ESTOP") nuevo = STATE_ESTOP;
        else if(reqState == "MANUAL") nuevo = STATE_MANUAL;
        
        if (nuevo != estado) {
            Serial.printf("[DEBUG] Forzando FSM override web: %s\n", wsForcedFsmState);
            cambiarEstado(nuevo);
        }
    }

    // ④ Ejecutar tick del estado actual
    switch (estado) {
        case STATE_INIT:
            break;
        case STATE_CALIBRATING:
            tickCalibrationIMU();
            break;
        case STATE_IDLE:
            if (web_server_has_command()) {
                PuntoRuta cmd = web_server_get_command();
                if (cmd.distancia == -999) {
                    cambiarEstado(STATE_ESTOP);
                } else if (abs(cmd.distancia) > 0) {
                    // Comando ya segmentado: ejecutar directamente
                    objetivo.distancia = cmd.distancia;
                    objetivo.velocidad = cmd.velocidad;
                    objetivo.timeout = cmd.duracion;
                    cambiarEstado(STATE_ADVANCING);
                    portENTER_CRITICAL(&muxSegmentacion);
                    if (avanceSegmentadoActivo) {
                        segmentosRestantes--;
                        if (segmentosRestantes <= 0) {
                            avanceSegmentadoActivo = false;
                            Serial.println("[SEG] Todos los segmentos completados");
                        }
                    }
                    portEXIT_CRITICAL(&muxSegmentacion);
                } else if (abs(cmd.angulo) > 0) {
                    objetivo.angulo = cmd.angulo;
                    objetivo.velocidad = cmd.velocidad;
                    portENTER_CRITICAL(&muxSegmentacion);
                    avanceSegmentadoActivo = false;
                    segmentosRestantes = 0;
                    portEXIT_CRITICAL(&muxSegmentacion);
                    cambiarEstado(STATE_TURNING);
                }
            }
            break;
        case STATE_ADVANCING:
            tickAvance(dt);
            break;
        case STATE_TURNING:
            tickGiro(dt);
            break;
        case STATE_BRAKING:
            tickFrenado();
            break;
        case STATE_ESTOP:
            if (wsResetRequested) {
                wsResetRequested = false;
                cambiarEstado(STATE_IDLE);
            }
            break;
        case STATE_MANUAL:
            // FIX [HIGH] Usar cast a unsigned long para evitar wrap-around en resta
            if ((unsigned long)(millis() - tUltimoComandoManual) > 500) {
                Serial.println("[FSM] Watchdog manual disparado: timeout joystick");
                motorDetener();
                cambiarEstado(STATE_IDLE);
            }
            break;
    }

    // ⑤ Procesar comandos serial
    procesarComandoSerial();

    // ⑥ Enviar telemetría
    enviarTelemetriaSerial();
    
    // ⑦ Handle web clients (polling de DNS local)
    web_server_loop();

    // ⑦ Medir duración del loop
    unsigned long tLoopDuracion = micros() - tLoopInicio;
    if (tLoopDuracion > tLoopMax) {
        tLoopMax = tLoopDuracion;
    }
}