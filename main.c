/**
 * @file main.c
 * @author Ajay Varghese (Team C)
 * @date 21 March 2022
 * @brief The program aims to set-up i2c slave communication on the Raspberry
 * Pi Pico so that it can interface with I2C master controllers to send data
 * when requested of it. 
 * 
 * There are two event loops set up using interrupts. One loop for the UART
 * data and one for I2C communication. The I2C interrupt has the higher priority.
 * 
 * When new data is received from the Particulates sensor it is read into a buffer
 * where we search for the two start byte identifiers. Once they are found, the 
 * data is read sequentially until the required bits are extracted and then 
 * processed. Once processed the data is written to the respective registars.
 * 
 * At any point the I2C master can request data from the data array. This is
 * handled using another interrupt, where by depending on which register it wants
 * access to, the correct data is extracted from the 16 bit register and sent.
 * 
 * @note [WARNING] I2C communication procoal sends data in 8 bit blocks
 * this device has registars 16 bits wide therfore the data MUST be sent
 * and read using 2 blocks. If this is not adhered to you will LOCK UP
 * the I2C bus and must preform a hard reboot
 * 
 * This is part of a collection of programs and codes for the 21WSD001 Team
 * Project run by Loughborough  University
*/


/* *************** [INCLUDES] *************** */

// Standard C library's to use standard C functions and types
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

// Standard Pi Pico library to initialise Pico's Peripherals 
#include "pico/stdlib.h"

// library to implement I2C communications from Pico SDK
#include "hardware/i2c.h"

// library to handle interrupts from I2C reads and writes
#include "hardware/irq.h"

// library to communicate over UART from Pico to Geiger
#include "hardware/uart.h"



/* *************** [CONFIGURATION VARIABLES] *************** */ 



// Uncomment this line to enable debugging statements onto USB UART - MUST ENABLE IN MAKEFILE
//#define DEBUGGING
#ifndef PICO_DEFAULT_LED_PIN
    #warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
#endif


/* *************** [I2C CONFIGURATION VARIABLES START] *************** */



// The Address of the I2C Peripheral
#define I2C_ADDR 0x2D   // [WARNING] can only take values between 0x08 - 0x77

// The Specific GP Pins on the Pi Pico to enable I2C on
#define SDA_SLAVE 4
#define SCL_SLAVE 5

// Data Array to hold the information from the registers
// 0 -> The i2c address of the device itself
// 1 -> The data
volatile uint16_t data[4] = {I2C_ADDR, 0, 0, 0};

// Register Variable to Read/Write data too
volatile uint8_t register_accessed;



/* *************** [I2C CONFIGURATION VARIABLES END] *************** */




/* *************** [UART CONFIGURATION VARIABLES START] *************** */



#define UART_RX 17
#define UART_TX 16
#define UART_ID uart0



/* *************** [UART CONFIGURATION VARIABLES END] *************** */



/* *************** [EXECTUTION CODE] *************** */ 



/**
 * @brief This interrupt is called whenever the i2c0 bus is accessed by the
 * master. It handles what to do on reads and writes as well as resetting
 * interrupt registers
 * 
 */
void i2c0_irq_handler() {

    // Get interrupt status
    volatile uint32_t status = i2c0->hw->intr_stat;

    // Check to see if we have received data from the I2C master (Will always be called on R/W)
    if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {

        // Parsing the register the master is trying to access
        register_accessed = ((i2c0->hw->data_cmd) << 24) >> 24;

        #ifdef DEBUGGING
            printf("Write acessed %02x\n", register_accessed);
        #endif

        /* 
            As this device will not need to update any internal registars no writes
            will be done. If we did want to write to an internal register to e.g
            setup configurations for this i2c device we would write that code
            below here.
        */
        
    }

    // Check to see if the I2C master is requesting data from us
    if (status & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {

        #ifdef DEBUGGING
            printf("Read Accessed On Register %02x\n", register_accessed);
        #endif

        volatile uint16_t invalid_data = 0;

        switch (register_accessed)
        {   
            // Register Zero contains the address of the device
            case 0:
                i2c_write_raw_blocking(i2c0, &data[register_accessed], 2);

                #ifdef DEBUGGING
                    printf("Data Sent -> %02x\n", data[register_accessed]);
                #endif

                break;
            // Register One Contains the Data from the device (PM1)
            case 1:
                i2c_write_raw_blocking(i2c0, &data[register_accessed], 2);

                #ifdef DEBUGGING
                    printf("Data Sent -> %02x\n", data[register_accessed]);
                #endif

                break;
            // Register Two Contains the Data from the device (PM2.5)
            case 2:
                i2c_write_raw_blocking(i2c0, &data[register_accessed], 2);

                #ifdef DEBUGGING
                    printf("Data Sent -> %02x\n", data[register_accessed]);
                #endif

                break;
            // Register Three Contains the Data from the device (PM10)
            case 3:
                i2c_write_raw_blocking(i2c0, &data[register_accessed], 2);

                #ifdef DEBUGGING
                    printf("Data Sent -> %02x\n", data[register_accessed]);
                #endif

                break;
            // If An incorrect Register Is Accessed the output is the invalid_data variable
            default:

                i2c_write_raw_blocking(i2c0, &invalid_data, 2);

                #ifdef DEBUGGING
                    printf("Data Sent -> %02x\n", invalid_data);
                #endif

                #ifdef DEBUGGING
                    printf("Incorrect Register\n");
                #endif

                break;
        }

        // Clear the interrupt
        i2c0->hw->clr_rd_req;
    }
}



/**
 * @brief This interrupt is called whenever data is avaliable on the UART Bus.
 * It is used to read the data from the Particulates sensor and then update the 
 * data array in memory. As you can make no guarantees on what point the
 * UART data is at, it reads data into a buffer and only when the
 * buffer matches the line format expected from the sensor will the data be 
 * processed
 * 
 */
void uart_isq_handler() {

    volatile uint8_t Data_Start[2] = {0, 0};
    volatile uint8_t Start_Bit_Pattern[2] = {66, 77};
    volatile uint16_t PM1, PM2_5, PM10 = 0;
    volatile uint8_t PM1_Pattern[2], PM2_5_Pattern[2], PM10_Pattern[2];

    // Checks if there is Data avaliable on the the RX port
    while (uart_is_readable(UART_ID)) 
    {   
        // Read two bytes into the Data Start Buffer
        Data_Start[0] = uart_getc(UART_ID);
        Data_Start[1] = uart_getc(UART_ID);

        // If we have read in the start identifier into the Data_Start buffer
        if ((Data_Start[0] == Start_Bit_Pattern[0] && Data_Start[1] == Start_Bit_Pattern[1]))
        {
            // Blink LED on Data Received
            if (gpio_get_out_level(LED_PIN)  == true)
            {
                gpio_put(LED_PIN, 0);
            }
            else
            {
                gpio_put(LED_PIN, 1);
            }
            
            

            // The next 16 bits are not relevent
            uart_getc(UART_ID);
            uart_getc(UART_ID);

            // The next 16 bits are for PM1
            PM1_Pattern[0] = uart_getc(UART_ID);
            PM1_Pattern[1] = uart_getc(UART_ID);

            // Combining the two 8 bit data to a full 16 bit value
            PM1 = (PM1_Pattern[0] << 8) | PM1_Pattern[1];

            // Writing to the data register
            data[1] = PM1;

            // The next 16 bits are for PM2.5
            PM2_5_Pattern[0] = uart_getc(UART_ID);
            PM2_5_Pattern[1] = uart_getc(UART_ID);

            // Combining the two 8 bit data to a full 16 bit value
            PM2_5 = (PM2_5_Pattern[0] << 8) | PM2_5_Pattern[1];

            // Writing to the data register
            data[2] = PM2_5;

            // The next 16 bits are for PM10
            PM10_Pattern[0] = uart_getc(UART_ID);
            PM10_Pattern[1] = uart_getc(UART_ID);

            // Combining the two 8 bit data to a full 16 bit value
            PM10 = (PM10_Pattern[0] << 8) | PM10_Pattern[1];

            // Writing to the data register
            data[3] = PM10;

            #ifdef DEBUGGING
                printf("PM1 = %i PM2.5 = %i PM10 = %i\n", PM1, PM2_5, PM10);
            #endif
        }
    }

}


/* *************** [Main Loop] *************** */ 



int main() {

    // Initialize GPIO and Debug Over USB UART - MUST ENABLE IN MAKEFILE
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /* *************** [I2C CONFIGURATION START] *************** */

    // Initializing the I2C0 Controller on the Pi Pico
    i2c_init(i2c0, 100000);

    // Setting the I2C0 Controller as a I2C Slave
    i2c_set_slave_mode(i2c0, true, I2C_ADDR);

    // Enabling I2C Mode on Pins GP4 and GP5
    gpio_set_function(SDA_SLAVE, GPIO_FUNC_I2C);
    gpio_set_function(SCL_SLAVE, GPIO_FUNC_I2C);

    // Enabling the internal Pull Up resistors for I2C to work
    gpio_pull_up(SDA_SLAVE);
    gpio_pull_up(SCL_SLAVE);

    // Enable the interrupts on i2c0 controller on access
    i2c0->hw->intr_mask = (I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS);

    // Set up the interrupt handler function to call on an interrupt
    irq_set_exclusive_handler(I2C0_IRQ, i2c0_irq_handler);

    // Enable I2C interrupts on the NVIC
    irq_set_enabled(I2C0_IRQ, true);
    irq_set_priority(I2C0_IRQ, 0);

    /* *************** [I2C CONFIGURATION END] *************** */


    /* *************** [UART CONFIGURATION START] *************** */

    // Initialise UART 0 which is connected to the geiger counter
    uart_init(UART_ID, 9600);

    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(UART_RX, GPIO_FUNC_UART);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, 0);

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART0_IRQ, uart_isq_handler);
    irq_set_priority(UART0_IRQ, 10);
    irq_set_enabled(UART0_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    /* *************** [UART CONFIGURATION END] *************** */


    // Printing a Debugging statement to the USB UART
    #ifdef DEBUGGING
        printf("I2C and UART Set-Up and Active\n");
    #endif

    // Loop forever doing nothing
    while (true) {
        tight_loop_contents();
    }

    return 0;
}


