# 📜 PLAN PDCA 013: Resolución de Bugs - Fase 1 y 2
**Estado:** PLANIFICADO (Por Hacer)  
**Fecha:** 2026-06-27  

## 1. 🎯 PLAN (Planificar)
**Objetivo:** Solucionar los 21 bugs y problemas de robustez identificados en la auditoría exhaustiva del código del carro robot ESP32-S3. Se priorizarán los 5 problemas CRÍTICOS y los 7 problemas ALTOS para garantizar la estabilidad del sistema, evitar *crashes* y asegurar el correcto funcionamiento de las características principales.

**Problemas detectados (Resumen Auditoría):**
- *Críticos:* Buffer overflow (WebSocket), Tearing de datos en telemetría (Race conditions), Envío doble de WebSocket, Errores de contexto ISR (`portENTER_CRITICAL_ISR`), Memory leak en peticiones HTTP con body.
- *Altos:* Cálculo de odometría con valores teóricos en vez de medidos, APIs de estado WiFi incompatibles con AP Mode, Hardcoding de credenciales ignorando NVS, Truncamientos silenciosos, y Errores en flujos de E-Stop y Reset.

**Justificación Arquitectónica:**
La estabilidad del servidor asíncrono y de la FSM (Finite State Machine) son pilares fundamentales para el sistema robótico. Solucionar estos fallos se alinea con la filosofía de Clean Code y resiliencia esperada para el proyecto.

## 2. 🚀 DO (Hacer)
- **Fase 1 (Críticos):**
  - **BUG-01:** Implementar buffer seguro de copiado en `ws.onEvent` en `web_server.cpp`.
  - **BUG-02:** Introducir `portMUX_TYPE muxTelWeb` para proteger las variables `tel_*`.
  - **BUG-03:** Eliminar el doble envío de telemetría (solo usar `ws_push_telemetry()` temporalizado).
  - **BUG-04:** Cambiar `portENTER_CRITICAL_ISR` a `portENTER_CRITICAL` en `main.cpp` (fuera de ISR).
  - **BUG-05:** Agregar manejo global para limpiar `_tempObject` en `server.onNotFound`.
- **Fase 2 (Altos):**
  - **BUG-06:** Modificar `tickGiro` para que `orientacionGlobal` sume `angZ` (medida IMU) y no el objetivo teórico.
  - **BUG-07/08:** Corregir `web_server_is_connected()` para verificar stations conectadas, y `web_server_get_ip()` para usar `softAPIP()`.
  - **BUG-09:** Corrección de tipos (float/int) en variables de segmento en `comandos.cpp`.
  - **BUG-10/11:** Sincronizar limpieza de E-Stop (`estadoEStopDisparado = false`) con resets remotos (WS/RAW) con handlers thread-safe.
  - **BUG-12:** Modificar `web_server_init` para utilizar las credenciales en `config_manager`.

## 3. 🔍 CHECK (Verificar)
- [ ] Compilación exitosa usando `pio run`.
- [ ] Verificación de no-caída del servidor web ante estrés.
- [ ] Validación de la correcta orientación angular integrada desde el MPU6050.
- [ ] Comprobar que E-Stop interrumpe el movimiento de forma segura y puede ser reiniciado mediante los comandos RAW/WS correspondientes.

## 4. 🔄 ACT (Actuar)
Si el plan cumple con los estándares, actualizar el documento de arquitectura y el `BUG_REPORT.md` para cerrar estos temas. Pasar este documento a la carpeta `Realizado`.


<!-- Plan (Objetivo) Do (Implementacion) -->
