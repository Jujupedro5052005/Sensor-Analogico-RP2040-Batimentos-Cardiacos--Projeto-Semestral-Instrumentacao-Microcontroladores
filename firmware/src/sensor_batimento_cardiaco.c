#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// =========================
// CONFIG
// =========================

#define SENSOR_PIN 26          // GPIO26 = ADC0
#define ADC_INPUT 0

// =========================
// GLOBAL VARIABLES
// =========================

float filtered = 0.0f;
float baseline = 0.0f;

bool beatDetected = false;

uint32_t lastBeatTime = 0;

float bpm = 0.0f;
float smoothBpm = 0.0f;

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

    sleep_ms(2000);

    while (true) {

        // =========================
        // READ SENSOR
        // =========================

        uint16_t raw = adc_read();

        // baseline tracking
        baseline = 0.99f * baseline + 0.01f * raw;

        // remove DC
        float signal = raw - baseline;

        // amplify
        signal *= 10.0f;

        // center signal
        signal += 2048.0f;

        // smoothing filter
        filtered =
            0.92f * filtered +
            0.08f * signal;

        // threshold
        float threshold = 2300.0f;

        // =========================
        // BEAT DETECTION
        // =========================

        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (filtered > threshold && !beatDetected) {

            beatDetected = true;

            uint32_t delta = now - lastBeatTime;

            // valid BPM range
            if (delta > 300 && delta < 1500) {

                bpm = 60000.0f / delta;

                // smooth BPM
                smoothBpm =
                    0.85f * smoothBpm +
                    0.15f * bpm;
            }

            lastBeatTime = now;
        }

        // reset detector
        if (filtered < threshold - 100.0f) {
            beatDetected = false;
        }

        // =========================
        // SERIAL OUTPUT
        // =========================

        printf("%.2f,%.2f,1000,3000\n",
               filtered,
               smoothBpm);

        sleep_ms(10);
    }
}