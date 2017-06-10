/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c
  Summary:
    This file contains the source code for the MPLAB Harmony application.
  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.
Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).
You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.
SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
unsigned char outbuf[STRLEN]; // used for writing to LCD
unsigned char LSMRead[LSM_ARRAY_LEN];
// *****************************************************************************
/* Application Data
  Summary:
    Holds application data
  Description:
    This structure holds the application's data.
  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/void delay(void) {
    _CP0_SET_COUNT(0);  // reset the core timer
    while (_CP0_GET_COUNT() < DELAYTIME) {
        while(!PORTBbits.RB4) {
        LATAbits.LATA4 = 0; 
        }
    }
}



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )
  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(BLACK);
    
    LSM6DS33_init();
    
    unsigned char progress;
    float fps = 1.0;
    unsigned char boxptx, boxpty;
    
    unsigned char iamwhatiam = 0;
    iamwhatiam = WhoAmI();
    
}


void APP_Tasks ( void )
{

    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            unsigned char progress;
            float fps = 1.0;
            unsigned char boxptx, boxpty;
            for (progress=0; progress < 101; progress++) {
                I2C_read_multiple(IMU_ADDR, 0x20, LSMRead, LSM_ARRAY_LEN); 
                _CP0_SET_COUNT(0);

                LCD_drawString(45,5,"Harmony",BLUE);

                drawxXLvec(62,62,((signed char) (convxXL(LSMRead)*50)),YELLOW);
                drawyXLvec(62,62,((signed char) (convyXL(LSMRead)*50)),CYAN);

                for(boxptx=0;boxptx<4;boxptx++) {
                    for(boxpty=0;boxpty<4;boxpty++) {
                        LCD_drawPixel(62+boxptx,62+boxpty,RED);
                    }
                }

                fps = 24000000.0/_CP0_GET_COUNT();

                while(!PORTBbits.RB4) {
                    LATAbits.LATA4 = 0;
                }

                while(_CP0_GET_COUNT() < (48000000/2/5)) {} 
            }
            LCD_clearScreen(BLACK); 
            break;
        }

        default:
        {
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */