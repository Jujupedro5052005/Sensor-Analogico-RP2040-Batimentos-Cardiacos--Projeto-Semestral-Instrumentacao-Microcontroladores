#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// =========================
// CONFIG
// =========================

#define SENSOR_PIN 26          // GPIO26 = ADC0
#define ADC_INPUT 0

// =========================
// VARIÁVEIS GLOBAIS
// =========================

float filtered = 0.0f;
float baseline = 0.0f; // Baseline age como uma estimativa lenta do sinal lido

bool beatDetected = false;

uint32_t lastBeatTime = 0;

float bpm = 0.0f; // BPM instântaneo
float smoothBpm = 0.0f; // BPM filtrado

// =========================
// MAIN
// =========================

int main() {

    stdio_init_all();

    // =========================
    // ADC INIT
    // =========================

    adc_init();

    adc_gpio_init(SENSOR_PIN);

    adc_select_input(ADC_INPUT);

    while (true) {

        // =========================
        // READ SENSOR
        // =========================

        uint16_t raw = adc_read();

        // Filtro passa-baixa exponencial
        baseline = 0.99f * baseline + 0.01f * raw;

        // Remoção de offset DC
        float signal = raw - baseline;

        // Amplificação digital de sinal
        signal *= 10.0f;

        // Recentralização do sinal (4096/2)
        signal += 2048.0f;

        // Filtro exponencial a fim de suavizar o sinal
        filtered =
            0.92f * filtered +
            0.08f * signal;

        // Threshold; valor minimo sob qual um sinal pode ser considerado um batimento
        float threshold = 2300.0f;

        // =========================
        // DETECÇÂO DE BATIMENTO
        // =========================

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Se o sinal pode ser um batimento e não estamos já em um pico
        if (filtered > threshold && !beatDetected) {

            beatDetected = true; // Marca que o sinal é um batimento

            // Computa variação de tempo
            uint32_t delta = now - lastBeatTime;

            // Intervalo válido para a frequência cardiaca 
            // 40 < BPM < 200
            if (delta > 300 && delta < 1500) {

                bpm = 60000.0f / delta; // Conversão para min^-1

                // Filtro exponencial para suavizar a saída do BPM
                smoothBpm =
                    0.85f * smoothBpm +
                    0.15f * bpm;
            }

            lastBeatTime = now; // Atualiza o tempo
        }

        // Reseta o detector de pico
        if (filtered < threshold - 100.0f) {
            beatDetected = false;
        }

        // =========================
        // SAÍDA SERIAL
        // =========================

        printf("%.2f,%.2f,%.2f,1000,3000\n",
               filtered,
               smoothBpm,
               bpm);

        sleep_ms(10);
    }
}