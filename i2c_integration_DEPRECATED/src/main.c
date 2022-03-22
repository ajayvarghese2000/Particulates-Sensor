#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"

#define UART_ID uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define UART_TX_PIN 8
#define UART_RX_PIN 9

#define I2C_ADDR 0x4d

static int adc_pin = 0;

const float conversion_factor=3.3f*1000/(1<<12);

void send_i2c(message){
    uint8_t rxdata[4];
    uint8_t txdata[2];
    i2c_read_raw_blocking(i2c1, rxdata, 3);
    //sprintf(message, "Rx: %d %d %d\r\n",rxdata[0],rxdata[1],rxdata[2])
    sprintf(message, "Value: %d\r\n",rxdata[0] + (rxdata[1]<<8));

    // respond with adc value in milivolts
    uint16_t adc_value=adc_read();
    //note this will drop fractions rather than rounding dow, but close enough
    int value = (int) adc_value * conversion_factor;
    txdata[0] = value & 0xFF;
    txdata[1] = value >> 8;
    sprintf(message, "Tx: %d %d - %d\r\n", txdata[0], txdata[1], value);
    i2c_write_raw_blocking(i2c1, txdata, 2);
}

void read_data(void){

    //inp = uart_getc(UART_ID);
}

void setup(void){
    //Setup adc
    stdio_init_all();
    adc_init();

    // Set GPIO as adc
    adc_gpio_init(26);
    adc_select_input(adc_pin);

    // setup UART for debugging
    uart_init(UART_ID, 9600);

    // set TX and RX pins
    gpio_set_function(UART_TX_PIN,GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN,GPIO_FUNC_UART);
    int actual = uart_set_baudrate(UART_ID,false);
    //set UART flow controls cts/rts, we dont want these to turn them off
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID,DATA_BITS,STOP_BITS,PARITY);

    // I2C Setup
    i2c_init(i2c0, 10000);
    i2c_set_slave_mode(i2c0,true,I2C_ADDR);
    gpio_set_function(4,GPIO_FUNC_I2C);
    gpio_set_function(5,GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    uart_puts(UART_ID, "voltage-i2c on pico");
}

int main(void)
{  
    setup();

    char message[20] = "t";
    while (true){
        // recieve data from controller
        // 3 bytes recieved - byte 0 is cmd (used as lower byte) byte 2 is higher - byte 3 is 0
        if (i2c_get_read_available(i2c0) < 3){
            continue;
        } 

        send_i2c(message);

    }
/*
    printf("\n\nHello World\n");
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(1000);
    }
    */
}
