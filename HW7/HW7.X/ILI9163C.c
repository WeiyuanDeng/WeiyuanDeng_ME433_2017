#include <xc.h>
#include "ILI9163C.h"

void SPI1_init() {
    SDI1Rbits.SDI1R = 0b0100; 
    RPA1Rbits.RPA1R = 0b0011; 
    TRISBbits.TRISB7 = 0; 
    LATBbits.LATB7 = 1;
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 0;
    SPI1CON = 0; 
    SPI1BUF; 
    SPI1BRG = 1; 
    SPI1STATbits.SPIROV = 0; 
    SPI1CONbits.CKE = 1;
    SPI1CONbits.MSTEN = 1;
    SPI1CONbits.ON = 1;

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { 
    ;
  }
  return SPI1BUF;
}

void LCD_command(unsigned char com) {
    LATBbits.LATB15 = 0; 
    LATBbits.LATB7 = 0;
    spi_io(com);
    LATBbits.LATB7 = 1; 
}

void LCD_data(unsigned char dat) {
    LATBbits.LATB15 = 1;
    LATBbits.LATB7 = 0; 
    spi_io(dat);
    LATBbits.LATB7 = 1; 
}

void LCD_data16(unsigned short dat) {
    LATBbits.LATB15 = 1; 
    LATBbits.LATB7 = 0; 
    spi_io(dat>>8);
    spi_io(dat);
    LATBbits.LATB7 = 1; 
}

void LCD_init() {
    int time = 0;
    LCD_command(CMD_SWRESET);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/2) {} 

    LCD_command(CMD_SLPOUT);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/200) {} 

    LCD_command(CMD_PIXFMT);
    LCD_data(0x05);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/200) {} 

    LCD_command(CMD_GAMMASET);
    LCD_data(0x04);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {}

    LCD_command(CMD_GAMRSEL);
    LCD_data(0x01);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} 

    LCD_command(CMD_NORML);

    LCD_command(CMD_DFUNCTR);
    LCD_data(0b11111111);
    LCD_data(0b00000110);

    int i = 0;
    LCD_command(CMD_PGAMMAC);
    for (i=0;i<15;i++){
        LCD_data(pGammaSet[i]);
    }

    LCD_command(CMD_NGAMMAC);
    for (i=0;i<15;i++){
        LCD_data(nGammaSet[i]);
    }

    LCD_command(CMD_FRMCTR1);
    LCD_data(0x08);
    LCD_data(0x02);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} 

    LCD_command(CMD_DINVCTR);
    LCD_data(0x07);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {}
    LCD_command(CMD_PWCTR1);
    LCD_data(0x0A);
    LCD_data(0x02);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {}

    LCD_command(CMD_PWCTR2);
    LCD_data(0x02);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} 
    LCD_command(CMD_VCOMCTR1);
    LCD_data(0x50);
    LCD_data(99);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} 

    LCD_command(CMD_VCOMOFFS);
    LCD_data(0);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} 
    LCD_command(CMD_CLMADRS);
    LCD_data16(0x00);
    LCD_data16(_GRAMWIDTH);
    LCD_command(CMD_PGEADRS);
    LCD_data16(0x00);
    LCD_data16(_GRAMHEIGH);
    LCD_command(CMD_VSCLLDEF);
    LCD_data16(0); 
    LCD_data16(_GRAMHEIGH);
    LCD_data16(0);

    LCD_command(CMD_MADCTL); 
    LCD_data(0b00001000);

    LCD_command(CMD_DISPON);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/1000) {}

    LCD_command(CMD_RAMWR);
}

void LCD_drawPixel(unsigned short x, unsigned short y, unsigned short color) {
    LCD_setAddr(x,y,x+1,y+1);
    LCD_data16(color);
}

void LCD_setAddr(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
    LCD_command(CMD_CLMADRS);
    LCD_data16(x0);
    LCD_data16(x1);

    LCD_command(CMD_PGEADRS);
    LCD_data16(y0);
    LCD_data16(y1);

    LCD_command(CMD_RAMWR);
}

void LCD_clearScreen(unsigned short color) {
    int i;
    LCD_setAddr(0,0,_GRAMWIDTH,_GRAMHEIGH);
        for (i = 0;i < _GRAMSIZE; i++){
            LCD_data16(color);
        }
}

void LCD_drawCharacter(short x, short y, char character, short color) {
    int i;
    int j;

    if ((x < (124 - m)) && (y < (122 - m))) {
        for (i = 0; i < 5; i++) {
            for (j = 0; j < + 8; j++) {
                if (((ASCII[character - 0x20][i]) >> j) & 1) {
                    LCD_drawPixel(x + i, y + j, color);
                }
                else {
                    LCD_drawPixel(x + i, y + j, BLACK);
                }
            }
        }
    }
}

void LCD_drawString(short x, short y, char *message, short color) {
    int k = 0;

    while(message[k]){
        LCD_drawCharacter((x + 6*k), y, message[k], color);
        k++;
    }
}

void LCD_drawBar(char x, char y, char percentage, short done, short remain) {
    int barX, barY;
    for (barX=0; barX<101; barX++) {
        if (barX<percentage) {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,done);
            }
        }
        else {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,remain);
            }
        }
    }
}

void LCD_drawCharacter(short x, short y, char character, short color) {
    int i;
    int j;

    if ((x < (124 - m)) && (y < (122 - m))) {
        for (i = 0; i < 5; i++) {
            for (j = 0; j < + 8; j++) {
                if (((ASCII[character - 0x20][i]) >> j) & 1) {
                    LCD_drawPixel(x + i, y + j, color);
                }
                else {
                    LCD_drawPixel(x + i, y + j, BLACK);
                }
            }
        }
    }
}

void LCD_drawString(short x, short y, char *message, short color) {
    int k = 0;

    while(message[k]){
        LCD_drawCharacter((x + 6*k), y, message[k], color);
        k++;
    }
}

void LCD_drawBar(char x, char y, char percentage, short done, short remain) {
    int barX, barY;
    for (barX=0; barX<101; barX++) {
        if (barX<percentage) {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,done);
            }
        }
        else {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,remain);
            }
        }
    }
}