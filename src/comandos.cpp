// ═══════════════════════════════════════════════════════════════════════
// COMANDOS.CPP — Preprocesamiento de comandos con segmentación automática
// ═══════════════════════════════════════════════════════════════════════

#include "comandos.h"
#include "config.h"
#include "web_server.h"

// ═══════════════════════════════════════════════════════════════════════
// FUNCIÓN INTERNA — Segmentar avance largo en N tramos
// ═══════════════════════════════════════════════════════════════════════
static void encolarAvanceSegmentado(float distanciaTotal, int velocidad) {
    int velSegmento = (int)(velocidad * FACTOR_VELOCIDAD_SEGMENTO);
    if (velSegmento < 50) velSegmento = 50;
    
    float restante = distanciaTotal;
    int count = 0;
    
    while (restante > 0.5f && count < SEGMENTOS_DISPONIBLES) {
        float segDist = (restante > SEGMENTO_MAX_CM) ? SEGMENTO_MAX_CM : restante;
        
        PuntoRuta cmd;
        cmd.distancia = segDist;
        cmd.angulo = 0;
        cmd.velocidad = (int)velSegmento;
        cmd.duracion = (unsigned long)(segDist / 5.0f * 1000) + TIMEOUT_AVANCE_EXTRA_MS;
        
        web_server_enqueue_command(cmd);
        
        restante -= segDist;
        count++;
    }
    
    // FIX [HIGH] Proteger escritura con spinlock (race condition fix)
    portENTER_CRITICAL(&muxSegmentacion);
    segmentosRestantes = count;
    avanceSegmentadoActivo = (count > 0);
    portEXIT_CRITICAL(&muxSegmentacion);
    
    Serial.print("[SEG] Avance segmentado: ");
    Serial.print(count);
    Serial.print(" tramos de max ");
    Serial.print(SEGMENTO_MAX_CM, 0);
    Serial.print(" cm @ ");
    Serial.print((int)velSegmento);
    Serial.println(" PWM");
}

// ═══════════════════════════════════════════════════════════════════════
// COMANDOS PÚBLICOS
// ═══════════════════════════════════════════════════════════════════════

void prepararComandoAvance(float distancia, int velocidad) {
    if (distancia > SEGMENTO_MAX_CM) {
        encolarAvanceSegmentado(distancia, velocidad);
    } else {
        PuntoRuta cmd;
        cmd.distancia = distancia;
        cmd.angulo = 0;
        cmd.velocidad = (int)(velocidad * FACTOR_VELOCIDAD_SEGMENTO);
        cmd.duracion = (unsigned long)(distancia / 5.0f * 1000) + TIMEOUT_AVANCE_EXTRA_MS;
        web_server_enqueue_command(cmd);
        portENTER_CRITICAL(&muxSegmentacion);
        avanceSegmentadoActivo = false;
        segmentosRestantes = 0;
        portEXIT_CRITICAL(&muxSegmentacion);
    }
}

void prepararComandoGiro(float angulo, int velocidad) {
    // NOTA: Giros NO aplican FACTOR_VELOCIDAD_SEGMENTO (80%).
    // Necesitan velocidad plena para torque instantáneo y precisión angular.
    PuntoRuta cmd;
    cmd.distancia = 0;
    cmd.angulo = angulo;
    cmd.velocidad = velocidad;
    cmd.duracion = TIMEOUT_GIRO_MS;
    web_server_enqueue_command(cmd);
}
