// ═══════════════════════════════════════════════════════════════════════
// TEST_SEGMENTACION.CPP — Tests unitarios para lógica de segmentación
// ═══════════════════════════════════════════════════════════════════════
// Compilar con: pio test -e native
// ═══════════════════════════════════════════════════════════════════════

#include <unity.h>
#include <math.h>

// Simular las constantes de config.h para test nativo
#define SEGMENTO_MAX_CM          50.0f
#define SEGMENTOS_DISPONIBLES    20
#define FACTOR_VELOCIDAD_SEGMENTO 0.8f
#define TIEMPO_PAUSA_SEGMENTO_MS 400
#define MARGEN_ERROR_CM          2.0f
#define TIMEOUT_AVANCE_EXTRA_MS  5000
#define TIMEOUT_GIRO_MS          10000
#define CM_POR_PULSO             1.021f

// Estructura PuntoRuta simulada (misma que en web_server.h)
typedef struct {
    float distancia;
    float angulo;
    int velocidad;
    unsigned long duracion;
} PuntoRuta;

// Contador de comandos encolados (simulación)
static int encoladosCount = 0;
static PuntoRuta ultimoCmd;

// Simulación de web_server_enqueue_command
static bool test_enqueue_called = false;
bool web_server_enqueue_command(PuntoRuta cmd) {
    test_enqueue_called = true;
    ultimoCmd = cmd;
    encoladosCount++;
    return true;
}

// Función de segmentación bajo test
static int segmentarAvance(float distanciaTotal, int velocidad) {
    float velSegmento = (int)(velocidad * FACTOR_VELOCIDAD_SEGMENTO);
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
    return count;
}

// ─────────────────────────────────────────────────────────────────────
// TESTS
// ─────────────────────────────────────────────────────────────────────

void test_distancia_menor_que_segmento_max(void) {
    encoladosCount = 0;
    test_enqueue_called = false;
    
    int count = segmentarAvance(30.0f, 150);
    
    TEST_ASSERT_EQUAL_INT(1, count);
    TEST_ASSERT_EQUAL_INT(1, encoladosCount);
    TEST_ASSERT_TRUE(test_enqueue_called);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, ultimoCmd.distancia);
}

void test_distancia_igual_a_segmento_max(void) {
    encoladosCount = 0;
    
    int count = segmentarAvance(50.0f, 150);
    
    TEST_ASSERT_EQUAL_INT(1, count);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, ultimoCmd.distancia);
}

void test_distancia_mayor_que_segmento_max(void) {
    encoladosCount = 0;
    
    int count = segmentarAvance(120.0f, 150);
    
    // 120cm / 50cm = 2 segmentos completos + 1 de 20cm = 3
    TEST_ASSERT_EQUAL_INT(3, count);
    TEST_ASSERT_EQUAL_INT(3, encoladosCount);
}

void test_distancia_exactamente_dos_segmentos(void) {
    encoladosCount = 0;
    
    int count = segmentarAvance(100.0f, 150);
    
    TEST_ASSERT_EQUAL_INT(2, count);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, ultimoCmd.distancia);
}

void test_velocidad_segmento_aplicada(void) {
    encoladosCount = 0;
    
    segmentarAvance(30.0f, 200); // velocidad 200, factor 0.8 = 160
    
    TEST_ASSERT_EQUAL_INT(160, ultimoCmd.velocidad);
}

void test_velocidad_minima_clamp(void) {
    encoladosCount = 0;
    
    segmentarAvance(30.0f, 30); // velocidad 30, factor 0.8 = 24 → clamp a 50
    
    TEST_ASSERT_EQUAL_INT(50, ultimoCmd.velocidad);
}

void test_segmentos_max_limit(void) {
    encoladosCount = 0;
    
    // 2000cm / 50cm = 40 segmentos, pero SEGMENTOS_DISPONIBLES = 20
    int count = segmentarAvance(2000.0f, 150);
    
    TEST_ASSERT_EQUAL_INT(20, count);
}

void test_duracion_segmento(void) {
    encoladosCount = 0;
    
    segmentarAvance(50.0f, 150);
    // duracion = (50/5*1000) + 5000 = 10000 + 5000 = 15000ms
    unsigned long esperada = (unsigned long)(50.0f / 5.0f * 1000) + TIMEOUT_AVANCE_EXTRA_MS;
    
    TEST_ASSERT_EQUAL_UINT(esperada, ultimoCmd.duracion);
}

// ─────────────────────────────────────────────────────────────────────
// RUNNER
// ─────────────────────────────────────────────────────────────────────

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_distancia_menor_que_segmento_max);
    RUN_TEST(test_distancia_igual_a_segmento_max);
    RUN_TEST(test_distancia_mayor_que_segmento_max);
    RUN_TEST(test_distancia_exactamente_dos_segmentos);
    RUN_TEST(test_velocidad_segmento_aplicada);
    RUN_TEST(test_velocidad_minima_clamp);
    RUN_TEST(test_segmentos_max_limit);
    RUN_TEST(test_duracion_segmento);
    
    return UNITY_END();
}
