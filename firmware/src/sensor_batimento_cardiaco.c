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
    #define ADC_INPUT 0
    #define SDA_INPUT 4
    #define SCL_INPUT 5

    #define FILTER_WINDOW 2 
    #define FINGER_THRESHOLD 1200
    #define MIN_SAMPLES 50
    #define PEAK_WINDOW 50
    #define SAMPLE_RATE 100
    #define BUFFER_WINDOW 50

    // =========================
    // VARIÁVEIS GLOBAIS
    // =========================

    float filtered = 0.0f; // filtered age como a frequência cardiaca suavizada
    float baseline = 0.0f; // baseline age como uma estimativa lenta do sinal lido

    bool beatDetected = false; // Detecta se o sinal está em um pico

    uint32_t lastBeatTime = 0; // Valor para contagem da frequêncai cardiaca 

    float bpm = 0.0f; // BPM instântaneo
    float smoothBpm = 0.0f; // BPM filtrado

    float filter_buffer[FILTER_WINDOW] = {0};
    int filter_idx = 0;

    float threshold = FINGER_THRESHOLD;

    float buffer[BUFFER_WINDOW] = {0};
    int buffer_idx = 0;

    int valid_count = 0;
    bool has_finger = false;

    int sample_counter = 0;

    float candidate_peak_value = 0;
    int candidate_peak_idx = 0;

    int samples_since_candidate = 0;

    float bpm_history[5] = {0};
    int bpm_hist_idx = 0;
    int bpm_hist_count = 0;

    int last_peak_idx = 0;


    repeating_timer_t timer; // Timer para interrupção

    uint8_t oled_buf[SSD1306_BUF_LEN]; // Cria buffer para o OLED

    ssd1306_render_area_t area={ // Cria área de renderização
        .start_col = 0,
        .end_col = 127,
        .start_page = 0,
        .end_page = SSD1306_NUM_PAGES -1
    };

    static float prev_filtered = 0.0f;
    static uint32_t last_peak = 0;
    static float peak_est = 0.0f;

    void add_bpm(float new_bpm){
        // Função para suavização da medida do bpm
        bpm_history[bpm_hist_idx] = new_bpm; // Atualiza o histórico de bpm

        // Atualiza o indice e "anda" na lista
        bpm_hist_idx = (bpm_hist_idx + 1) % 5; 

        if(bpm_hist_count < 5){bpm_hist_count++;}

        float sum = 0;

        // Faz a soma dos bpm guardados no histórico
        for(int i=0; i < bpm_hist_count; i++){sum += bpm_history[i];}

        smoothBpm = sum / bpm_hist_count; // Média simples do bpm
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

        float filtered = 0; // reinicializa o valor filtrado de raw

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

        float moving_average = 0;

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

        // Dado que signal[1] é um máximo local o considera um candidato a pico do 
        // sinal do batimento cardíaco
        if(signal[1] > signal[0] &&
           signal[1] > signal[2])
        {
            if(signal[1] > candidate_peak_value)
            {
                candidate_peak_value = signal[1];
                candidate_peak_idx = sample_counter - 1;
            }
        }
        
        samples_since_candidate++;

        // =========================
        // Final da janela
        // =========================

        if(samples_since_candidate >= PEAK_WINDOW)
        {
            if(candidate_peak_value >= threshold)
            {
                if(last_peak_idx >= 0)
                {
                    int delta_samples = candidate_peak_idx - last_peak_idx;

                    float delta_seconds = (float)delta_samples / SAMPLE_RATE; 
                    if(delta_seconds > 0)
                    {
                        float bpm_candidate = 60.0f / delta_seconds;

                        if(bpm_candidate >= 30 &&
                           bpm_candidate <= 200)
                           {
                            bpm = bpm_candidate;

                            add_bpm(bpm_candidate);

                            printf("BPM= %.1f\n", smoothBpm);


                           }
                    }
                }

                last_peak_idx = candidate_peak_idx;

            }

            // Reinicia os valores para próxima janela de análise
            candidate_peak_value = 0;
            candidate_peak_idx = 0;
            samples_since_candidate = 0;

        }

        sample_counter++;

        printf("%u\n", raw);
    }

    void adc_fifo_handler(void){ // Função chamada pelo fifo
        while(!adc_fifo_is_empty()){
            // Lê o sinal da fila
            uint16_t raw = adc_fifo_get();

            // Processa o sinal lido
            process_sample(raw);
        }
    }

    bool sample_timer_callback(repeating_timer_t *t){ // Função chamada pelo timer
        // Aciona a leitura de dados
        //adc_run(true);

        uint16_t raw = adc_read();
        process_sample(raw);

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
            "BPM %d",
            (int)smoothBpm
        );

        ssd1306_draw_text(
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

        //adc_fifo_setup( // Inicializa a fila
        //    true,
        //    false,
        //    1,
        //    false,
        //    false
        //);

        // =========================
        // CONFIG INTERRUPÇÂO
        // =========================

        //irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_fifo_handler);
    //
        //adc_irq_set_enabled(true);
    //
        //irq_set_enabled(ADC_IRQ_FIFO,true);
        
        add_repeating_timer_ms(10, sample_timer_callback, NULL, &timer);
        
        while (true) {

            // =========================
            // SAÍDA SERIAL & EXIBIÇÂO
            // =========================

            update_display(); // Atualiza o display

            //printf("%.2f,%.2f,%.2f,1000,3000\n",
            //       filtered,
            //       smoothBpm,
            //       bpm);
        }
    }