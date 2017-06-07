#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>

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

#define CS LATAbits.LATA0 

void spi_init(void);
char spi_io(unsigned char);
void write_dac(unsigned int, unsigned int);

void spi_init(void){
    // re-assign pins
    RPA1Rbits.RPA1R = 0b0011; 
    CS = 1;                 
    SPI1CONbits.ON = 0;        
    SPI1BUF;                  
    SPI1BRG = 0x1;              
    SPI1CONbits.MSTEN = 1;    
    SPI1STATbits.SPIROV = 0;  
    SPI1CONbits.CKP = 1;      
    SPI1CONbits.CKE = 1;       
    SPI1CONbits.ON = 1;        
}

char spi_io(unsigned char o) {    
  SPI1BUF = o;             
  while (!SPI1STATbits.SPIRBF) {
      ;              
  }
  return SPI1BUF;
}

void write_dac(unsigned int channel, unsigned int value) {
    unsigned char l;
    unsigned char m;
    
    CS = 0; 
    l = value << 4;
    m = ((channel << 7)|(0b111 << 4))|(value >> 4);
    spi_io(m);
    spi_io(l);
    CS = 1; 
}

int main() {
    unsigned int sine_wave[100];
    unsigned int ramp_wave[200];
    double s_tmp, r_tmp;
    int i,j;
    __builtin_disable_interrupts();

    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    
    BMXCONbits.BMXWSDRM = 0x0;
    INTCONbits.MVEC = 0x1;
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA0 = 0; 
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA4 = 0; 
    TRISBbits.TRISB4 = 1; 
    LATAbits.LATA4 = 1; 
    
   spi_init();
    
    for(i=0; i<100; i++){
        s_tmp = (255.0/2.0) + (255.0/2.0)*sin(2*M_PI*(i/100.0));
        sine_wave[i] = s_tmp;
    }
   
    for(j=0; j<200; j++){ 
        r_tmp = (j/200.0)*255.0;
        ramp_wave[j] = r_tmp;
    }

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    i = 0;
    while(1) {
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 10000) {}
        write_dac(0, sine_wave[100]); 
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 10000) {}
        write_dac(1, ramp_wave[200]);
        i++;
        if (i==200){
         i = 0;   
        }
    }
    return 0;
}