#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"i2c_master_noint.h" // I2C library

// DEVCFG0
#pragma config DEBUG = 0b11 // no debugging
#pragma config JTAGEN = 0 // no jtag
#pragma config ICESEL = 0b11 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = 1 // no boot write protect
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 0b011 // use primary oscillator with pll
#pragma config FSOSCEN = 0 // turn off secondary oscillator
#pragma config IESO = 0 // no switching clocks
#pragma config POSCMOD = 0b10 // high speed crystal mode
#pragma config OSCIOFNC = 1 // free up secondary osc pins
#pragma config FPBDIV = 0b00 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = 0b11 // do not enable clock switch
#pragma config WDTPS = 0b10100 // slowest wdt
#pragma config WINDIS = 1 // no wdt window -- are this comment and the following one reversed?
#pragma config FWDTEN = 0 // wdt off by default -- see note on previous comment
#pragma config FWDTWINSZ = 0b11 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = 0b001 // divide input clock to be in range 4-5MHz: div by 2
#pragma config FPLLMUL = 0b111 // multiply clock after FPLLIDIV, 24x
#pragma config FPLLODIV = 0b001 // divide clock after FPLLMUL to get 48MHz: div by 2
#pragma config UPLLIDIV = 0b001 // divider (2x) for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = 0 // USB clock on

// DEVCFG3
#pragma config USERID = 0xFFFF // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = 0 // allow multiple reconfigurations
#pragma config IOL1WAY = 0 // allow multiple reconfigurations
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // USB BUSON controlled by USB module

#define DELAYTIME 4000000 // 40000 yields 0.001 s delay time when using Core Timer
#define CS LATAbits.LATA0  // chip select pin
#define SLAVE_ADDR 0x20 // I2C hardware address of MCP23008

void initExpander() {
    i2c_master_setup(); // setup I2C2 at 100 kHz

    // init GP0-3 as outputs
    i2c_master_start(); // START bit
    i2c_master_send(SLAVE_ADDR << 1);  // hardware address; RW (lsb) = 0, indicates write
    i2c_master_send(0x00);  // specify address to write to: 0x00 = IODIR
    i2c_master_send(0xF0);  // send value byte to address specified above
                        // a value of 0xF0 makes GP0-3 outputs and 4-7 inputs.
    i2c_master_stop();  // STOP bit
}

void setExpander(char pin, char level) { // write to expander
    i2c_master_start();
    i2c_master_send(SLAVE_ADDR << 1); // hardware address and write bit
    i2c_master_send(0x0A); // LAT register = 0x0A
    i2c_master_send(level << pin); // write "level" (hi/lo) to "pin"
    i2c_master_stop();
}

char getExpander() {
    char level;
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1)); // hardware address and write bit
    i2c_master_send(0x09);  // PORT register = 0x09
    i2c_master_restart(); // this line is REALLY important!
    i2c_master_send((SLAVE_ADDR << 1) | 1); // hardware address and read bit
    level = i2c_master_recv(); // receive a byte from the slave
    i2c_master_ack(1); // send NACK to slave
    i2c_master_stop();
    return level;
}

int main() {
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // Turn off AN2 and AN3 pins (make B2 and B3 available for I2C)
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA0 = 0; // pin 0 of Port A is CS (chip select) (output)
    TRISAbits.TRISA1 = 1; // pin 1 of Port A is SDO1 (output)
    TRISAbits.TRISA4 = 0; // Pin 4 of Port A is LED1 (output)
    TRISBbits.TRISB4 = 1; // Pin 4 of Port B is USER button (input)
    LATAbits.LATA4 = 1; // Turn LED2 ON

    initExpander();
    setExpander(0,1); // turn on LED connected to GP0

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT() < 48000000) {} // wait 1 second
    while(1) {
        LATAbits.LATA4 = 1; // turn on LED1; USER button is low (FALSE) if pressed.
        setExpander(0,0); // turn off LED connected to GP0 of MCP23008
        while((getExpander()>>7)) { // button is on GP7, so shift 7 bits to the right.
            setExpander(0,1); // turn on LED connected to GP7 of MCP23008
        }
    }
    return 0;
}