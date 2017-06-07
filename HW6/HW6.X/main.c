#include<xc.h> 
#include<sys/attribs.h> 
#include<stdio.h> 
#include"PICconfig.h"
#include"ILI9163C.h"
#include"LCD.h"

#define DELAYTIME 4000000 // 40000 yields 0.001 s delay time when using Core Timer
#define STRLEN 19   // maximum number of characters per string

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
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(BLACK);
    
    char iter;
    char msg[STRLEN];
    float fps = 1.0;

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    while(1) {
        for (iter=0; iter < 101; iter++) {
            _CP0_SET_COUNT(0);
            
            sprintf(msg,"Hello Weiyuan %d!",iter);
            LCD_drawString(18, 32, msg, YELLOW);
            
            LCD_drawBar(15,70,iter,RED,WHITE);
            
            sprintf(msg,"FPS: %5.2f",fps);
            LCD_drawString(35,100,msg,CYAN);
            
           fps = 24000000.0/_CP0_GET_COUNT();

            while(!PORTBbits.RB4) { // button is on GP7, so shift 7 bits to the right.
                LATAbits.LATA4 = 0;
            }
            
            while(_CP0_GET_COUNT() < (48000000/2/5)) {} 
        }
        LCD_clearScreen(BLACK); 
    }
    return 0;
}