#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

// DEVCFG0
#pragma config DEBUG = 0b11 // no debugging
#pragma config JTAGEN = 0 // no jtag
#pragma config ICESEL = 0b11 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = 1 // no boot write protect
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 0b11 // use primary oscillator with pll
#pragma config FSOSCEN = 0 // turn off secondary oscillator
#pragma config IESO = 0 // no switching clocks
#pragma config POSCMOD = 0b10 // high speed crystal mode
#pragma config OSCIOFNC = 1 // disable secondary osc
#pragma config FPBDIV = 0b00 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = 0b11 // do not enable clock switch
#pragma config WDTPS = 0b10100 // use slowest wdt
#pragma config WINDIS = 1 // wdt no window mode
#pragma config FWDTEN = 0 // wdt disabled
#pragma config FWDTWINSZ = 0b11 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = 0b001 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = 0b111 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = 0b001 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = 0b001 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = 0 // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = 0 // allow multiple reconfigurations
#pragma config IOL1WAY = 0 // allow multiple reconfigurations
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // USB BUSON controlled by USB module

#define TIME 40000
#define CS LATAbits.LATA4

void spi_init(void);
unsigned char spi_io(unsigned char o);
void write_dac(unsigned int, unsigned int);


int main() {

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int sine_wave[100];
    unsigned int ramp_wave[100];
    double temp;


    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    TRISAbits.TRISA4 = 0; // set LED to an output pin
    TRISBbits.TRISB4 = 1; // set pushbutton to an input pin 

    LATAbits.LATA4 = 1; // set LED on
    
    spi_init();

    __builtin_enable_interrupts();


    for(i=0; i<100; i++){ //10Hz sine wave
        temp = (255.0/2.0) + (255.0/2.0)*sin(2*3.14*(i/100.0));
        sine_wave[i] = temp;
    }
   
    for(j=0; j<100; j++){ //10Hz triangle wave
        temp = (j/100.0)*255.0;
        ramp_wave[j] = temp;
    }
    
    i=0;
    while(1) {
        _CP0_SET_COUNT(0);

        while(_CP0_GET_COUNT() < 48000000/2/1000){
            ;
        }

        _CP0_SET_COUNT(0);

        write_dac(1, sine_wave[i]);
        write_dac(2, ramp_wave[i]);
        
        i++;

        if (i == 100){
            i = 0;
        }   
    }
}


void spi_init(void){
	CS = 1;
    SPI1CON = 0;
    SPI1BUF;  
    SPI1BRG = 1;
// clear overflow bit
    SPI1CONbits.CKE = 1; 
    SPI1CONbits.MSTEN = 1;    
    SPI1CONbits.ON = 1;

    TRISAbits.TRISA4 = 0; //CS wire to A4 (Pin 12)

    RPA1Rbits.RPA1R = 0b0011; 
}

unsigned char spi_io(unsigned char o) {
  SPI4BUF = o;
  while(!SPI4STATbits.SPIRBF) { // wait to receive the byte
  }
  return SPI4BUF;
}

void write_dac(unsigned int channel, unsigned int voltage){
    unsigned char b1 = 0, b2 = 0; 
    
    CS = 0;

    channel = (channel<<4 | 0b0111); 
    b1 = (channel<<7 | voltage>>4); 
    b2 = voltage<<4; 
    
    spi_io(b1);
    spi_io(b2);
    CS = 1; 
}