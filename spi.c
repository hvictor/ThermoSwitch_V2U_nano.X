#include "spi.h"
#include <xc.h>

void spi_configure()
{
    /****************************************************
     * SPI bus SPI1 Pin Configuration
     * This procedure configures the bus for shared
     * operation of TFT and SDCARD
     * 
     * Master Out:  MOSI    = SDO1 = RB11 = pin 22
     * Clock:       SCLK    = pin 25
     * TFT Chip Select: CS  = RB10 = pin 21
     * SD Chip Select:  CS  = RB15 = pin 26
     ****************************************************/
    
    SPI1CONbits.ON = 0;     // Turn off SPI1 before configuring
    SPI1CONbits.MSTEN = 1;  // Enable Master mode
    SPI1CONbits.CKP = 1;    // Clock signal is active low, idle state is high
    SPI1CONbits.CKE = 0;    // Data is shifted out/in on transition from idle (high) state to active (low) state
    SPI1CONbits.SMP = 1;    // Input data is sampled at the end of the clock signal
    SPI1CONbits.MODE16 = 0; // Do not use 16-bit mode
    SPI1CONbits.MODE32 = 0; // Do not use 32-bit mode (combines with the above line to activate 8-bit mode)
    SPI1BRG = 0x0000;       // (BRG DISARMED) Set Baud Rate Generator to 0
    SPI1CONbits.ENHBUF = 0; // Disables Enhanced Buffer mode
    SPI1CONbits.ON = 1;     // Configuration is done, turn on SPI1 peripheral
   
    // Peripheral Pin Select
    SDI1R = 0b0100;         // RB8  = SDI1 (MISO)
    RPB11R = 0b0011;        // RB11 = SDO1 (MOSI)
    
    // Input/Output selection
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB11 = 0;  // MOSI         = RB11  => Output
    TRISBbits.TRISB10 = 0;  // TFT CS       = RB10  => Output
    TRISBbits.TRISB15 = 0;  // SDCARD CS   = RB15  => Output
    
    // TFT Data/Command selection
    TRISBbits.TRISB13 = 0;  // DC = Pin 24  => Output
    
    /* End SPI Configuration */
}