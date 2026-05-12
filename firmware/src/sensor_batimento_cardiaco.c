/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

*/

#include <stdio.h>
#include "pico/stdlib.h"

#define LED  25

int main1() { // bem baixo nivel -> acessando diretamente os registradores

    // variavel de 32 bits inteira que pode ser modificada a qualquer momento pelo codigo
    // coloco o * para ser um ponteiro
    // o valor é o base adress de "2.19.6.1. IO - User Bank"
    // escolho o GPIO_25_CTRL com endereço de offset 0x0CC
    volatile uint32_t *io_bank0_gpio_25_ctrl = (uint32_t *)0x400140CC;

    // variavel com o SIO base de "2.3.1.7. List of Registers"
    // permite habilitar como saida (diretiva de mascara) -> usamos o output enable set
    // perceba que output enable set eh  GPIO_OE_SET = 0x024
    // eu somo esse valor no base address 0xd0000000 dos registradores SIO_BASE
    volatile uint32_t *sio_gpio_oe_set = (uint32_t *)(0xd0000000+0x024);

    // vou usar o XOR para ficar invertendo (efeito de blink)
    volatile uint32_t *sio_gpio_out_xor = (uint32_t *)0xD000001C;

    // configurar o pino 25 com SIO (coloca 5 no FUNCSEL)
    *io_bank0_gpio_25_ctrl = 5;

    // colocar o GP25 como saida
    // uso operação de deslocamento para direita '<<'
    // se eu fizer o shift 25 vezes, ele para o bit 25, sem precisar calcular na mao
    *sio_gpio_oe_set = (1 << 25);

    while(1){ // LOOP
        // mudar o estado do pino 25
        *sio_gpio_out_xor = (1 << 25);

        // gasta ciclos de clocks
        sleep_ms(1000);

        // no SDK tenho funcoes de alto nivel que abstraem ainda mais, como digitalWrite
        // nao preciso ir no registrador
        // documento "raspberry-pi-pico-c-sdk.pdf" 
    }
}

int main2() { // começo a usar o struct do .h para substituir os enderecos

    // variavel com o SIO base de "2.3.1.7. List of Registers"
    // permite habilitar como saida (diretiva de mascara)
    volatile uint32_t *sio_gpio_oe_set = (uint32_t *)0xD0000024;

    // vou usar o XOR para ficar invertendo (efeito de blink)
    volatile uint32_t *sio_gpio_out_xor = (uint32_t *)0xD000001C;

    // OU: fabricante me fornece um constante que ja esta alocada no endereco 
    // é um struct que já esta no BANK0_BASE, o endereco que copiei antes
    // ja tem todos os endereços no struct, ai nao preciso fazer o offset a partir do endereco de base do banco 0
    // '->' é acessar dentro do struct o io
    // assim, nao preciso mais de *io_bank0_gpio_25_ctrl
    io_bank0_hw->io[LED].ctrl = GPIO_FUNC_SIO;

    // colocar o GP25 como saida
    // uso operação de deslocamento para direita '<<'
    // se eu fizer o shift 25 vezes, ele para o bit 25, sem precisar calcular na mao
    *sio_gpio_oe_set = (1 << 25);

    while(1){ // LOOP

        // OU: posso usar o toggle
        sio_hw->gpio_togl = (1 << LED);

        // gasta ciclos de clocks
        sleep_ms(4000);

        // no SDK tenho funcoes de alto nivel que abstraem ainda mais, como digitalWrite
        // nao preciso ir no registrador
        // documento "raspberry-pi-pico-c-sdk.pdf" 
    }
}

int main3(){ // usando SDK -> fica mais limpo de entender
    // substitui o uso do SIO
    gpio_init(LED);

    // configurar como saida (direcao)
    // recebe numero do IO e bool para definir entrada ou saida
    // substitui a diretiva de mascara
    gpio_set_dir(LED, true);

    while(1){
        // mascara XOR
        gpio_xor_mask(1 << LED);

        // delay de 200 ms
        sleep_ms(200);
    }
}

int main(){
    main3();
}
