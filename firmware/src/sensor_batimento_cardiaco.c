#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"

// Definição de pinos
#define ADC_READ_PIN 26
#define REDLed 10
#define IRLed 11

// Definição de váriaveis globais

/*
    De acordo com o site:https://nav.dasa.com.br/blog/frequencia-cardiaca ,
a frequência cardiaca máxima de uma pessoa (em bpm) pode ser estimada a partir da equação
220 - <idade>, assim será considerada como frequência de leitura 220 bpm, que convertendo
para período em ms, utilizando T(ms)=60000/(f(bpm)), onde f(bpm) é a frequência cardiaca 
em batimentos por minuto, que para 220 bpm, resulta em um período de 272 ms, para garantir
margem de leitura será utilizado 200 ms.
*/

/*
    Outras fontes relevantes quanto à taxas de frequência cardíaca normal, em repouso, são
https://www.einstein.br/n/vida-saudavel/frequencia-cardiaca-como-medir-e-o-valor-ideal-em-cada-idade
https://www.tuasaude.com/frequencia-cardiaca/
*/

const int CALLBACK_TIME = 200;
#define measures 10 // Número de medidas para tirar a média da frequência cardíaca
const int measure_time = 50;

// Função chamada pelo timer 
bool timer_callback(struct repeating_timer *t){

    adc_run(true); // Inicia uma conversão single-shot
    
    return true;
}

/*void __isr adc_isr(void){
    uint16_t raw = adc_fifo_get(); // Valor digital entre 0 e 4095
    irq_clear(ADC_IRQ_FIFO);

    adc_run(false); // Para a amostragem até o próximo timer
}*/


int main()
{
    stdio_init_all();
    sleep_ms(2000); ////////

    // Inicializa os pinos
    gpio_init(REDLed);
    gpio_set_dir(REDLed, GPIO_OUT);
    gpio_put(REDLed, 0); // Seta a saída para LOW

    gpio_init(IRLed);
    gpio_set_dir(IRLed, GPIO_OUT);
    gpio_put(IRLed, 0); // Seta a saída para LOW

    // Configuração do ADC
    adc_init(); // Habilita o ADC
    adc_gpio_init(ADC_READ_PIN); // Habilita o pino 26
    adc_select_input(0); // Configura porta do ADC no canal 0

    // Configuração do FIFO
    /*adc_fifo_setup(true, false, 1, false, false);
    adc_irq_set_enabled(true);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_isr);
    irq_set_enabled(ADC_IRQ_FIFO, true);*/

    // Cria um timer de repetição
    struct repeating_timer timer;

    // Cria um timer que chama a função timer_callback
    //add_repeating_timer_ms(CALLBACK_TIME, timer_callback, NULL, &timer);

    // Criação das variáveis
    bool rising = false;
    uint n = 0;

    while (true) {
        
        // Grava o tempo atual
        int64_t ms = to_ms_since_boot(get_absolute_time());

        float reader = 0;

        // Tira uma média de leitura do adc ao longo de 20 ms
        do{
            reader += adc_read();
            n++;
        }
        while(to_ms_since_boot(get_absolute_time()) < ms + measure_time);
        reader /= n;

        reader = reader*3.3f/4095;

        //printf("timer: %i\n", ms);
        printf("%f\n", reader);

        sleep_ms(50);

    }
}
