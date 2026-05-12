#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"

// Definição de pinos
#define ADC_READ_PIN 26

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

const int measures = 10; // Número de medidas para tirar a média da frequência cardíaca

// Função chamada pelo timer 
bool timer_callback(struct repeating_timer *t){

    adc_run(true); // Inicia uma conversão single-shot
    
    return true;
}

void __isr adc_isr(void){
    uint16_t raw = adc_fifo_get(); // Valor digital entre 0 e 4095
    irq_clear(ADC_IRQ_FIFO);

    adc_run(false); // Para a amostragem até o próximo timer
}


int main()
{
    stdio_init_all();

    // Configuração do ADC
    adc_init(); // Habilita o ADC
    adc_gpio_init(ADC_READ_PIN); // Habilita o pino 26
    adc_select_input(0); // Configura porta do ADC no canal 0

    // Configuração do FIFO
    adc_fifo_setup(true, false, 1, false, false);
    adc_irq_set_enabled(true);
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_isr);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    // Cria um timer de repetição
    struct repeating_timer timer;

    // Cria um timer que chama a função timer_callback
    add_repeating_timer_ms(CALLBACK_TIME, timer_callback, NULL, &timer);

    while (true) {
        tight_loop_contents();
    }
}
