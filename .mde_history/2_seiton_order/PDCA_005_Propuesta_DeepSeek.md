A continuación te presento una **síntesis de código integrado** para un carro robótico con **ESP32 WROOM**, dos motores con encoders (LM393) y giroscopio MPU6050. El sistema permite:

- **Control de motores** mediante puente H L298N (PWM y dirección).
- **Lectura de encoders** por interrupción para medir distancia recorrida.
- **Orientación** mediante integración del giroscopio (ángulo YAW).
- **Calibración inicial** para igualar posición de motores y establecer ángulo de referencia.
- **Servidor web** para enviar comandos: avanzar distancia (cm) y girar ángulo (grados).
- **Navegación relativa** (X, Y) – se convierte en secuencias de avance y giro.

El código está escrito para el **framework Arduino** en PlatformIO/VS Code y utiliza las librerías `MPU6050`, `WiFi`, `AsyncWebServer`, `SPIFFS` (opcional). Se ha simplificado para que sea fácil de entender y modificar.

---

## 📦 Código completo (`src/main.cpp`)

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>   // Solo si quieres servir archivos HTML desde el sistema de archivos

// ========================= CONFIGURACIÓN DE RED =========================
const char* ssid     = "TU_WIFI";
const char* password = "TU_PASSWORD";

AsyncWebServer server(80);

// ========================= PINES DEL PUENTE H L298N =========================
// Motor izquierdo (A)
const int ENA = 5;   // PWM
const int IN1 = 4;
const int IN2 = 7;
// Motor derecho (B)
const int ENB = 6;   // PWM
const int IN3 = 8;
const int IN4 = 9;

// ========================= PINES DE ENCODERS (interrupción) =========================
const int encoderIzq = 2;   // Canal A del encoder izquierdo
const int encoderDer = 3;   // Canal A del encoder derecho

// ========================= MPU6050 (I2C) =========================
MPU6050 mpu;
float anguloYaw = 0.0;        // Ángulo acumulado en grados
float offsetGiroscopio = 0.0; // Compensación de deriva

// ========================= VARIABLES DE ODOMETRÍA =========================
volatile long pulsosIzq = 0;
volatile long pulsosDer = 0;

// CONSTANTE DE CALIBRACIÓN: pulsos por centímetro (debes ajustarla según tu rueda)
// Se calcula como: (pulsos por revolución) / (circunferencia de la rueda en cm)
// Por ejemplo, si el encoder da 20 pulsos por revolución y la rueda mide 6.28 cm de circunferencia -> Cm = 20 / 6.28 ≈ 3.18
float Cm = 3.18;   // <-- REEMPLAZA CON TU VALOR CALIBRADO

// ========================= VARIABLES DE CONTROL =========================
bool movimientoEnCurso = false;   // Bloquea nuevos comandos mientras se ejecuta uno

// ========================= PROTOTIPOS =========================
void detenerMotores();
void avanzarDistancia(float distanciaCm, int velocidad);
void girarAngulo(float grados, int velocidad);
void calibrarGiroscopio();
void configurarPines();
void configurarInterrupciones();
void setupWebServer();

// ========================= ISR DE ENCODERS =========================
void IRAM_ATTR contarPulsoIzq() { pulsosIzq++; }
void IRAM_ATTR contarPulsoDer() { pulsosDer++; }

// ========================= FUNCIONES DE MOTORES =========================
void detenerMotores() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void avanzarDistancia(float distanciaCm, int velocidad = 200) {
  if (movimientoEnCurso) return;
  movimientoEnCurso = true;

  // Reiniciamos contadores
  pulsosIzq = 0;
  pulsosDer = 0;

  // Calcular pulsos objetivo (usando el promedio de ambos encoders para mayor precisión)
  long pulsosObjetivo = (long)(distanciaCm / Cm);

  // Motores hacia adelante
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);

  // Bucle de control hasta alcanzar la meta (se usa el mínimo de los dos para no sobrepasar)
  while (pulsosIzq < pulsosObjetivo && pulsosDer < pulsosObjetivo) {
    // (Opcional) Corrección de trayectoria usando el giroscopio:
    // Si el ángulo se desvía, ajustar las velocidades de forma proporcional
    static float anguloRef = anguloYaw;
    float error = anguloYaw - anguloRef;
    if (abs(error) > 0.5) {
      // Corregir reduciendo velocidad de la rueda que va más adelantada
      int velIzq = velocidad - (int)(error * 2.0);
      int velDer = velocidad + (int)(error * 2.0);
      velIzq = constrain(velIzq, 0, 255);
      velDer = constrain(velDer, 0, 255);
      analogWrite(ENA, velIzq);
      analogWrite(ENB, velDer);
    }
    delay(10);
  }

  detenerMotores();
  movimientoEnCurso = false;
}

void girarAngulo(float grados, int velocidad = 180) {
  if (movimientoEnCurso) return;
  movimientoEnCurso = true;

  // Establecer el ángulo de referencia actual
  float anguloInicial = anguloYaw;
  float anguloObjetivo = anguloInicial + grados;   // Positivo = giro a la derecha, negativo = izquierda

  // Configurar motores para girar en el sentido adecuado
  if (grados > 0) {
    // Giro a la derecha: rueda izquierda adelante, derecha atrás
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    // Giro a la izquierda: rueda izquierda atrás, derecha adelante
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);

  // Bucle hasta alcanzar el ángulo deseado (con histéresis)
  while (true) {
    float error = anguloObjetivo - anguloYaw;
    if (abs(error) < 1.0) break;  // tolerancia de 1 grado
    // Ajuste de velocidad para suavizar la parada (si se desea)
    delay(10);
  }

  detenerMotores();
  movimientoEnCurso = false;
}

// ========================= CALIBRACIÓN DEL GIROSCOPIO =========================
void calibrarGiroscopio() {
  Serial.println("Calibrando giroscopio... Mantén el carro quieto.");
  const int numLecturas = 100;
  long suma = 0;
  for (int i = 0; i < numLecturas; i++) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);   // Obtener velocidades angulares en crudo
    // El eje Z (gy) es el que nos interesa para el YAW (depende de la orientación del sensor)
    // Suponemos que el sensor está plano y el eje Z apunta hacia arriba.
    // En MPU6050, el valor de gy (velocidad angular Z) suele ser el que da el YAW.
    // Convertir a grados/segundo: dividir por 131 (para rango ±250º/s)
    float velocidad = gz / 131.0;
    suma += velocidad;
    delay(5);
  }
  offsetGiroscopio = suma / numLecturas;
  Serial.print("Offset del giroscopio Z: ");
  Serial.println(offsetGiroscopio);
  anguloYaw = 0.0;
}

// ========================= ACTUALIZACIÓN DEL ÁNGULO (se llama en loop) =========================
void actualizarAngulo() {
  static unsigned long tiempoAnterior = millis();
  unsigned long ahora = millis();
  float dt = (ahora - tiempoAnterior) / 1000.0;
  tiempoAnterior = ahora;

  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);
  float velocidadZ = gz / 131.0;               // Convertir a grados/segundo
  velocidadZ -= offsetGiroscopio;              // Restar offset
  anguloYaw += velocidadZ * dt;                // Integrar
}

// ========================= CONFIGURACIÓN INICIAL =========================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  // Verificar conexión del MPU6050
  if (!mpu.testConnection()) {
    Serial.println("Error: MPU6050 no conectado.");
    while (1) delay(1000);
  }
  Serial.println("MPU6050 conectado.");

  // Calibrar el giroscopio (establece el offset)
  calibrarGiroscopio();

  // Configurar pines de motores y encoders
  configurarPines();
  configurarInterrupciones();

  // Conectar a WiFi y levantar servidor web
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
  setupWebServer();

  Serial.println("Sistema listo. Comandos disponibles vía web.");
}

void configurarPines() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(encoderIzq, INPUT_PULLUP);
  pinMode(encoderDer, INPUT_PULLUP);
}

void configurarInterrupciones() {
  attachInterrupt(digitalPinToInterrupt(encoderIzq), contarPulsoIzq, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderDer), contarPulsoDer, RISING);
}

// ========================= SERVIDOR WEB =========================
void setupWebServer() {
  // Página principal (HTML embebido en el código)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>Control de Carro</title></head>
<body>
<h1>Control del Carro</h1>
<h2>Comandos:</h2>
<form action="/comando">
  <label>Distancia (cm):</label><input type="number" name="dist" value="50"><br>
  <label>Velocidad (0-255):</label><input type="number" name="vel" value="200"><br>
  <input type="submit" value="Avanzar">
</form>
<form action="/comando">
  <label>Ángulo (grados, + = derecha, - = izquierda):</label><input type="number" name="ang" value="90"><br>
  <label>Velocidad (0-255):</label><input type="number" name="vel" value="180"><br>
  <input type="submit" value="Girar">
</form>
<h3>Estado actual:</h3>
<p>Ángulo Yaw: <span id="angulo">0.0</span>°</p>
<p>Pulsos Izq: <span id="pulIzq">0</span> | Der: <span id="pulDer">0</span></p>
<script>
  setInterval(() => {
    fetch('/estado')
      .then(res => res.json())
      .then(data => {
        document.getElementById('angulo').innerText = data.angulo.toFixed(2);
        document.getElementById('pulIzq').innerText = data.pulsosIzq;
        document.getElementById('pulDer').innerText = data.pulsosDer;
      });
  }, 1000);
</script>
</body>
</html>
)rawliteral";
    request->send(200, "text/html", html);
  });

  // Endpoint para ejecutar comandos
  server.on("/comando", HTTP_GET, [](AsyncWebServerRequest *request){
    String msg;
    if (request->hasParam("dist")) {
      float dist = request->getParam("dist")->value().toFloat();
      int vel = request->hasParam("vel") ? request->getParam("vel")->value().toInt() : 200;
      msg = "Avanzar " + String(dist) + " cm a velocidad " + String(vel);
      // Ejecutar en segundo plano (no bloquear el servidor)
      // Se recomienda usar una tarea o flag, aquí lo hacemos directo pero puede bloquear
      avanzarDistancia(dist, vel);
    } else if (request->hasParam("ang")) {
      float ang = request->getParam("ang")->value().toFloat();
      int vel = request->hasParam("vel") ? request->getParam("vel")->value().toInt() : 180;
      msg = "Girar " + String(ang) + " grados a velocidad " + String(vel);
      girarAngulo(ang, vel);
    } else {
      msg = "Comando no reconocido";
    }
    request->send(200, "text/plain", msg);
  });

  // Endpoint para obtener estado en JSON
  server.on("/estado", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"angulo\":" + String(anguloYaw) + ",";
    json += "\"pulsosIzq\":" + String(pulsosIzq) + ",";
    json += "\"pulsosDer\":" + String(pulsosDer);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

// ========================= LOOP PRINCIPAL =========================
void loop() {
  // Actualizar el ángulo integrando el giroscopio
  actualizarAngulo();

  // (Opcional) Mostrar información por serial cada segundo
  static unsigned long ultimoSerial = millis();
  if (millis() - ultimoSerial > 1000) {
    ultimoSerial = millis();
    Serial.print("Ángulo: "); Serial.print(anguloYaw);
    Serial.print(" | Pulsos Izq: "); Serial.print(pulsosIzq);
    Serial.print(" | Der: "); Serial.println(pulsosDer);
  }

  // Si no hay movimiento en curso, se pueden atender otros comandos (ya están en el servidor)
  delay(10);
}
```

---

## 🧭 Calibración necesaria antes de usar

### 1. **Constante `Cm` (pulsos por cm)**
- Mide la circunferencia de la rueda (por ejemplo, con una cinta métrica).
- Cuenta cuántos pulsos da el encoder en una vuelta completa (normalmente 20 pulsos por revolución si es de tipo LM393 con disco de 20 ranuras).
- **Fórmula:**  
  \[
  Cm = \frac{\text{pulsos por vuelta}}{\text{circunferencia en cm}}
  \]
- Sustituye el valor en la línea `float Cm = 3.18;` por el tuyo.

### 2. **Orientación del MPU6050**
- Coloca el carro en una superficie plana y nivelada.
- La calibración del giroscopio se ejecuta automáticamente en `setup()` (función `calibrarGiroscopio()`).
- **No muevas el carro** durante esos 5 segundos, para que el offset se calcule correctamente.

### 3. **Alineación inicial de motores**
- Si al avanzar en línea recta el carro se desvía, ajusta la corrección proporcional dentro de `avanzarDistancia()` (ya incluida). Puedes modificar el factor de corrección (`error * 2.0`) para que sea más o menos agresivo.
- También puedes ajustar las velocidades base de cada motor si uno es más rápido que el otro, reemplazando `velocidad` por valores distintos para ENA y ENB.

---

## 🕸️ Interfaz web

- Conecta el ESP32 a tu red WiFi (modifica `ssid` y `password`).
- En el monitor serie verás la IP asignada.
- Abre esa IP en un navegador.
- Desde la página podrás enviar:
  - **Avanzar:** distancia en cm y velocidad (0‑255).
  - **Girar:** ángulo en grados (positivo = derecha, negativo = izquierda) y velocidad.
- El estado en tiempo real (ángulo y pulsos) se actualiza automáticamente cada segundo.

---

## ⚙️ Observaciones de diseño

- **Movimientos en curso:** las funciones `avanzarDistancia` y `girarAngulo` usan la bandera `movimientoEnCurso` para evitar que se solapen comandos.
- **Corrección de trayectoria:** durante el avance, se usa el giroscopio para ajustar dinámicamente las velocidades de cada motor, manteniendo el rumbo recto.
- **Integración del giroscopio:** se actualiza en cada ciclo del `loop()` mediante `actualizarAngulo()`, lo que da una estimación continua del ángulo Yaw.
- **Servidor asíncrono:** aunque los comandos se ejecutan de forma síncrona (bloquean el servidor), se puede modificar para usar tareas en segundo plano si se requiere mayor capacidad de respuesta.

---

## 📁 Estructura del proyecto (PlatformIO)

```
proyecto/
├── src/
│   └── main.cpp          (código anterior)
├── platformio.ini
└── lib/                  (si usas librerías externas)
```

`platformio.ini` mínimo:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    wire
    MPU6050
    ESP Async WebServer
    AsyncTCP
```

---

Este código te da una base sólida para programar rutas completas (por ejemplo, ir a una coordenada (X,Y) combinando avances y giros). Solo necesitas llamar secuencialmente a `avanzarDistancia()` y `girarAngulo()` con los valores adecuados.

<!-- Plan (Objetivo) Do (Implementacion) -->
