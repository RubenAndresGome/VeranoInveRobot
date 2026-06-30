#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
 #include "SPIFFS.h" // ESP32 only
 #include "driver/ledc.h"

/* PROYECTO DE CONTROL
   Programa de control de motores para ESP32-S3-Wroom
   Controla dos motores DC mediante puente H
*/

// Pines de potenciómetros para control de motores (ADC1)
const int potenciometro[3] = {4,5,6};

// Pines de DIP Switch
const int dipSwitchPins[8] = {7, 15, 16, 17, 18, 8, 3, 46};

// Pines de LEDs
const int ledPins[4] = {19,20,21,47};


const int pwmPines[2]={1,2};
// Pines de motores DC (H-Bridge)
const int motor1Pins[3] = {pwmPines[0], 42, 41};   // PWM, IN1, IN2
const int motor2Pins[3] = {pwmPines[1], 40, 39}; // PWM, IN1, IN2

// Resolución y frecuencia de PWM (no configurable sin ledcSetup)
const int pwmResolution = 8; // 8 bits: 0-255
class MotorContinua {
private:
    int pinEnable;
    int pinesDireccion[2];
    int pinPotenciometro;
    int velocidad = 0;
    bool direccion = false;
    bool estado = false;
    int pwmChannel;  // Canal PWM para cada motor
    //bool direccionAntigua=false,nuevaDireccion=false;

public:
    MotorContinua(int pEnable, int pDir1, int pDir2, int pPot, int channel)
        : pinEnable(pEnable), pinPotenciometro(pPot), pwmChannel(channel) {
        pinesDireccion[0] = pDir1;
        pinesDireccion[1] = pDir2;
    }

    void iniciar() {
        pinMode(pinEnable, OUTPUT);
        pinMode(pinesDireccion[0], OUTPUT);
        pinMode(pinesDireccion[1], OUTPUT);

        // Configuración del canal PWM
        ledcSetup(pwmChannel, 5000, pwmResolution);  // 5 kHz, 8 bits de resolución
        ledcAttachPin(pinEnable, pwmChannel);        // Asocia el canal al pin
    }

    void setVelocidad() {
        velocidad = constrain(leerPotenciometro(pinPotenciometro), 0, 255);
        ledcWrite(pwmChannel, velocidad);  // Control de la velocidad con PWM
    }

    void encender() {
        estado = true;
        actualizarDireccion();
        setVelocidad();
    }

    void apagar() {
        estado = false;
        digitalWrite(pinesDireccion[0], LOW);
        digitalWrite(pinesDireccion[1], LOW);
        ledcWrite(pwmChannel, 0);  // Apagar el PWM
    }
     

    void cambiarDireccion(bool dir) {
        if(dir != direccion){
            apagadoTemporal();//solo se detiene un tiempo si hay cambio de direccion
        }
        direccion = dir;
       
        actualizarDireccion();//se actualiza direccion 
    }

    int getVelocidad(){
        return velocidad;
    }

private:
    int leerPotenciometro(int pinPot) {
        int valorPotenciometro = analogRead(pinPot);  // Lectura del ADC
        return map(valorPotenciometro, 0, 4095, 0, 255);  // Mapear a rango de 0-255
    }

    void apagadoTemporal() {
        //Esto se realiza para evitar saltos del motor 
        //al cambiar bruscamente de velocidad 
        //no se modifica el state por que solo 
        //es un estado de transiccion
        digitalWrite(pinesDireccion[0], LOW);
        digitalWrite(pinesDireccion[1], LOW);
        ledcWrite(pwmChannel, 0);  // Apagar el PWM
        delay(250);//delay pequenio
    }

    void actualizarDireccion() {
        if (estado) {
            if (direccion) {
                digitalWrite(pinesDireccion[0], HIGH);
                digitalWrite(pinesDireccion[1], LOW);
            } else {
                digitalWrite(pinesDireccion[0], LOW);
                digitalWrite(pinesDireccion[1], HIGH);
            }
        }
    }
};


// Creación de instancias de motores
// Creación de instancias de motores con diferentes canales PWM
MotorContinua motor1(motor1Pins[0], motor1Pins[1], motor1Pins[2], potenciometro[0], 0);
MotorContinua motor2(motor2Pins[0], motor2Pins[1], motor2Pins[2], potenciometro[1], 1);

void configurarPines() {
    // Inicializar motores
    motor1.iniciar();
    motor2.iniciar();

    // Configurar pines de LEDs
    for (int pin : ledPins) {
        pinMode(pin, OUTPUT);
    }

    // Configurar pines de potenciómetros
    for (int pin : potenciometro) {
        pinMode(pin, INPUT);
    }

    // Configurar pines de DIP Switch
    for (int pin : dipSwitchPins) {
        pinMode(pin, INPUT_PULLDOWN);
    }
}

void controlarLeds(bool encender) {
    for (int pin : ledPins) {
        //aqui aplicamos la operacion NOT, para leds con compuerta not
        if(!encender){
            digitalWrite(pin, HIGH);
        }else{
            digitalWrite(pin, LOW);
        }
        
    }
}

void controlarMotor(MotorContinua& motor, int enableDip, int dirDip) {
    if (digitalRead(enableDip) == HIGH) {
        Serial.println("Iniciando motor");
        bool direccion = digitalRead(dirDip) == HIGH;
        motor.cambiarDireccion(direccion);
        if (direccion==true){
           Serial.println("Direccion 1"); 
        }else{
            Serial.println("Direccion 2");
        }
        motor.encender();  //se reenciende motor
        motor.setVelocidad();   //actualiza l avelocidad de nuevo
        Serial.print("Velocidad motor: ");
        Serial.println(motor.getVelocidad());
    } else {
        motor.apagar();
    }
}

void setup() {
    Serial.begin(115200);
    configurarPines();
    Serial.println("Iniciando control de motores...");
}
bool estadoLeds=false;
void loop() {

for (int i = 0; i < 8; i++) {
        Serial.print("DIP ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(digitalRead(dipSwitchPins[i]));
    }
    delay(100);

    if (digitalRead(dipSwitchPins[7]) == LOW) {
        motor1.apagar();
        motor2.apagar();
        controlarLeds(false);
        Serial.println("PARO DE EMERGENCIA ACTIVADO");
        estadoLeds=false;
        controlarLeds(estadoLeds);
        while (digitalRead(dipSwitchPins[7]) == LOW) {
        delay(100); // Espera hasta que se libere
        }
        return;
    }
    
    // Controlar ambos motores
    controlarMotor(motor1, dipSwitchPins[0], dipSwitchPins[1]);
    controlarMotor(motor2, dipSwitchPins[2], dipSwitchPins[3]);

    // Control de LEDs
     estadoLeds = (digitalRead(dipSwitchPins[5]) == HIGH);
    controlarLeds(estadoLeds);

    delay(10);
}
