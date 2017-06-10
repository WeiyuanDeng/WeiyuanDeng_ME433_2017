#include<xc.h>   
#include<sys/attribs.h>  
#include<math.h>
#include<stdio.h>   
#include"PICconfig.h" 
#include"ILI9163C.h" 
#include"i2c_master_noint.h"
#include"polulu.h" 

#define DELAYTIME 4000000 
#define CS LATBbits.LATB7
#define STRLEN 19 

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
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(BLACK);
    
    LSM6DS33_init();
    
    unsigned char i;
    unsigned char outbuf[STRLEN];
    float fps = 1.0;
    unsigned char boxptx, boxpty;
    
    unsigned char iamwhatiam = 0;
    iamwhatiam = WhoAmI();
    
    unsigned char LSMRead[LSM_ARRAY_LEN];

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    while(1) {
        for (i=0; i < 101; i++) {
            I2C_read_multiple(IMU_ADDR, 0x20, LSMRead, LSM_ARRAY_LEN); // use length = LSM_ARRAY_LEN
            _CP0_SET_COUNT(0);
            
            drawxXLvec(62,62,((signed char) (convxXL(LSMRead)*50)),YELLOW);
            drawyXLvec(62,62,((signed char) (convyXL(LSMRead)*50)),CYAN);
            
            for(boxptx = 0;boxptx < 4;boxptx++) { 
                for(boxpty = 0;boxpty < 4;boxpty++) {
                    LCD_drawPixel(62+boxptx,62+boxpty,RED);
                }
            }
            
            fps = 24000000.0/_CP0_GET_COUNT();
            while(!PORTBbits.RB4) { // button is on GP7, so shift 7 bits to the right.
                LATAbits.LATA4 = 0;
            }
            
            while(_CP0_GET_COUNT() < (48000000/2/5)) {} // 0.2 ms delay = 5 Hz timer
        }
        LCD_clearScreen(BLACK); // use this function sparingly!
    }
    return 0;
}