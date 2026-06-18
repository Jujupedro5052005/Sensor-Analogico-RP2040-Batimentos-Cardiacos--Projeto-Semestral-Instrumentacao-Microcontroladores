#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// =========================
// CONFIG
// =========================

#define SENSOR_PIN 26       // GPIO26 = ADC0
#define BUZZER_PIN 22       // GPIO22 = Buzzer
#define ADC_INPUT 0         // Canal para o ADC
#define SDA_INPUT 4         // Pino SDA para comunicação I2C
#define SCL_INPUT 5         // Pino SCL para comunicação I2C

#define FILTER_WINDOW 2         // # de amostras usadas para suavização inicial do sinal
#define FINGER_THRESHOLD 1100    // Valor minímo vindo do ADC para considerar a presença do dedo no leitor
#define MIN_SAMPLES 50          // # minímo de amostras com sinal no intervalo determinado para consider a presença do dedo
#define PEAK_WINDOW 50          // Tamanho da janela de análise para detecção de pico
#define SAMPLE_RATE 100         // Taxa de amostras coletadas
#define BUFFER_WINDOW 50        // # de amostras utilzadas na média móvel do threshold
#define BPM_HIST_COUNT 5        // # de amostras utilizadas na suavização do valor do bpm

// =========================
// VARIÁVEIS GLOBAIS
// =========================

float prev_filtered = 0.0f; // filtered age como a sinal suavizada

float bpm = 0.0f; // BPM instântaneo
float smoothBpm = 0.0f; // BPM filtrado

float filter_buffer[FILTER_WINDOW] = {0}; // Buffer para suavização do sinal lido
int filter_idx = 0; // Indíce do filter_buffer

float threshold = FINGER_THRESHOLD; // Valor minímo que o sinal deve ter para ser considerado um pico

float buffer[BUFFER_WINDOW] = {0}; // Buffer para cálculo da média móvel do threshol
int buffer_idx = 0; // Indíce do buffer

int valid_count = 0; // # de amostras em que a condição da presença do dedo foi satisfeita
bool has_finger = false; // Guarda se o dedo foi detectado

int sample_counter = 0; // Guarda o número de amostras tratadas

float candidate_peak_value = 0; // Candidato a pico mais alto da janela de amostra
int candidate_peak_idx = 0; // # da amostra do ca

int samples_since_candidate = 0; // # de amostras desde o último candidato a pico

float bpm_history[BPM_HIST_COUNT] = {0}; // Buffer para cálculo da média aritimérica dos bpm detectados
int bpm_hist_idx = 0; // Indíce do bpm_history

int last_peak_idx = 0; // # da amostra do último pico detectado
    
repeating_timer_t timer; // Timer para interrupção

uint8_t oled_buf[SSD1306_BUF_LEN]; // Cria buffer para o OLED

ssd1306_render_area_t area={ // Cria área de renderização na tela do OLED
    .start_col = 0,
    .end_col = 127,
    .start_page = 0,
    .end_page = SSD1306_NUM_PAGES -1
};


void add_bpm(float new_bpm){
    // Função para suavização da medida do bpm
    bpm_history[bpm_hist_idx] = new_bpm; // Atualiza o histórico de bpm

    // Atualiza o indice e "anda" na lista
    bpm_hist_idx = (bpm_hist_idx + 1) % 5; 

    float bpm_sum = 0; // Reinicia bpm_sum

    // Faz a soma dos bpm guardados no histórico
    for(int i=0; i < BPM_HIST_COUNT; i++){bpm_sum += bpm_history[i];}

    smoothBpm = bpm_sum / BPM_HIST_COUNT; // Média aritmética do bpm
}

void process_sample(uint16_t raw){
    // Função para tratamento do sinal

    // Guarda o sinal lido para detecção de máximo local
    static float signal[3] = {0};

    // =========================
    // Filtro média móvel
    // =========================

    filter_buffer[filter_idx] = raw; // Atualiza a lista dos dados lidos
    filter_idx = (filter_idx + 1) % FILTER_WINDOW; // Atualiza o índice

    float filtered = 0; // Reinicializa o valor filtrado de raw

    // Calcula o valor filtrado do sinal lido
    for(int i=0; i < FILTER_WINDOW; i++){
        filtered += filter_buffer[i];
    }
    filtered /= FILTER_WINDOW;

    // =========================
    // Threshold móvel
    // =========================

    // Atualiza a lista dos valores que serão usados no threshold móvel
    buffer[buffer_idx] = filtered; 
    buffer_idx = (buffer_idx + 1) % BUFFER_WINDOW; // Atualiza o índice

    float moving_average = 0; // Reinicia moving_average

    // Calcula o threshold móvel
    for(int i=0; i < BUFFER_WINDOW; i++){moving_average+=buffer[i];}
    moving_average/=BUFFER_WINDOW;
    threshold = moving_average + 100;

    // =========================
    // Detecção de dedo
    // =========================

    if(filtered > FINGER_THRESHOLD && filtered < 4095){
        valid_count++;
    }
    else{
        valid_count = 0;
    }

    // Sinal deve estar dentro de um intervalo determinado por tempo suficiente
    // para que seja considerado a presença do dedo
    has_finger = (valid_count >= MIN_SAMPLES);

    // =========================
    // Atualiza signal
    // =========================

    signal[0] = signal[1];
    signal[1] = signal[2];
    signal[2] = filtered;
        
    // =========================
    // Máximo local
    // =========================

    // Dado que signal[1] é um máximo local ele é considera um candidato a pico do 
    // sinal do batimento cardíaco
    if(signal[1] > signal[0] &&
       signal[1] > signal[2])
    {
        if(signal[1] > candidate_peak_value) 
        {   // Caso signal[1] seja um máximo local ele vira um candidato a pico
            candidate_peak_value = signal[1];
            candidate_peak_idx = sample_counter - 1;
        }
    }
        
    samples_since_candidate++;

    // =========================
    // Final da janela
    // =========================

    if(samples_since_candidate >= PEAK_WINDOW)
    {   // Verifica se chegou no fim da janela de análise
        // para então fazer a detecção de pico
        if(candidate_peak_value >= threshold)
        {   // Verifica se o candidato a pico supera o threshold móvel
            if(last_peak_idx >= 0)
            {
                // Calculo intervalo entre o pico atual e o anterio 
                // no dominio de # de amostras
                int delta_samples = candidate_peak_idx - last_peak_idx;

                // Converte delta do dominio de amostras para o dominio de tempo
                float delta_seconds = (float)delta_samples / SAMPLE_RATE; 
                if(delta_seconds > 0)
                {
                    // Converte delta em bpm
                    float bpm_candidate = 60.0f / delta_seconds;

                    if(bpm_candidate >= 30 &&
                       bpm_candidate <= 200)
                       {    // Verifica se bpm tem um valor válido

                        gpio_put(BUZZER_PIN, 1); // Liga o buzzer indicando detecção de pico

                        bpm = bpm_candidate;

                        add_bpm(bpm); // Suaviza o sinal de bpm

                        printf("BPM= %.1f\n", smoothBpm); // print para debug


                        }
                }
            }

            last_peak_idx = candidate_peak_idx; // Atualiza o indíce de pico detectado

        }

        // Reinicia os valores para próxima janela de análise
        candidate_peak_value = 0;
        candidate_peak_idx = 0;
        samples_since_candidate = 0;

    }

    sample_counter++; // Atualiza o # de amostras lidas

    gpio_put(BUZZER_PIN, 0); // Desativa o buzzer

    printf("%u\n", raw); // Print para debug
} 

bool sample_timer_callback(repeating_timer_t *t){ // Função chamada pelo timer
    // Aciona a leitura de dados

    uint16_t raw = adc_read(); // Lê o sinal do adc
    process_sample(raw); // Chama a função que trata e interpreta o sinal

    return true;
}

void update_display(void){
    // Função para atualizar o display

    char bpm_text[32];

    ssd1306_clear(oled_buf); // Limpa o texto do OLED

    ssd1306_draw_text( // Escreve no OLED
        oled_buf,
        0,
        0,
        "HEART RATE"
    );

    sprintf( // Converte numero em texto
        bpm_text,
        "BPM %d\nDedo %s",
        (int)smoothBpm,
        has_finger ? "Detectado" : "Ausente"
    );

    ssd1306_draw_text( // Escreve o texto no display
        oled_buf,
        0,
        16,
        bpm_text
    );

    ssd1306_render_full( // Atualiza efetivamente o display
        oled_buf,
        &area
    );

}

// =========================
// MAIN
// =========================

int main() {

    stdio_init_all();

    // =========================
    // BUZZER INIT
    // =========================

    gpio_init(BUZZER_PIN); // Inicializa o pino que aciona o buzzer
    gpio_set_dir(BUZZER_PIN, GPIO_OUT); // Define o pino como saída
    gpio_put(BUZZER_PIN, 0); // Inicia o código com ele desligado

    // =========================
    // I2C INIT
    // =========================

    i2c_init(i2c_default, 400 * 1000); // Inicializa o i2c

    gpio_set_function(SDA_INPUT, GPIO_FUNC_I2C); // Liga o GPIO4 ao SDA
    gpio_set_function(SCL_INPUT, GPIO_FUNC_I2C); // Liga o GPIO5 ao SCL

    gpio_pull_up(SDA_INPUT); // Aciona resistores de pull-up internos
    gpio_pull_up(SCL_INPUT); // Aciona resistores de pull-up internos

    // =========================
    // SSD1306 INIT
    // =========================

    ssd1306_init(); // Inicializa o SSD1306

    ssd1306_calc_area(&area); // Calcula qual área do OLED será usada

    // =========================
    // ADC INIT
    // =========================

    adc_init(); // Inicializa o adc

    adc_gpio_init(SENSOR_PIN); // Lê o sinal pelo SENSOR_PIN

    adc_select_input(ADC_INPUT); // Seleciona o canal ADC_INPUT

    adc_set_clkdiv(16000); // Clock divider para prevenir flood pelo ADC

    // =========================
    // CONFIG INTERRUPÇÂO
    // =========================

    // Define o timer para leitura e tratamento do sinal
    add_repeating_timer_ms(10, sample_timer_callback, NULL, &timer);
        
    while (true) {

        // =========================
        // EXIBIÇÂO
        // =========================

        update_display(); // Atualiza o display

    }
}