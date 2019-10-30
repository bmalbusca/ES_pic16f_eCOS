

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include <xc.h>
#include "mcc_generated_files/mcc.h"

#define NREG 30
#define PMON 5
#define TALA 3
#define ALAT 25
#define ALAL 2
#define ALAF 0
#define CLKH 0
#define CLKM 0

volatile int value = 0;
void handler_clock_hms(void);
void copyto_EEPROM(void);
void LED_bin(unsigned int value);
        
volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg;

unsigned int convertedValue = 0;
unsigned int duty_cycle = 25;

void sw1_EXT(void){
    
    //IO_RA5_Toggle(); 
    
}

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    TMR1_SetInterruptHandler(handler_clock_hms);
    INT_SetInterruptHandler(sw1_EXT);
    
    unsigned int task_schedule = 0;
    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();
     //IO_RA4_SetLow(); 
    while (1)
    {   
        
        SLEEP();
        NOP();
        
        //IO_RA4_Toggle();
        
        task_schedule = seg;
        //if (seg >= 5){
        
            do{
                ADCC_StartConversion(channel_ANA0);
                while(!ADCC_IsConversionDone()){
                    __delay_ms(1);
                }
                convertedValue = ADCC_GetConversionResult();
                
                //IO_RA6_LAT = 1 << convertedValue;
                duty_cycle =  convertedValue;
                
                LED_bin(convertedValue);
                
                if (duty_cycle < 50){   //set zero 
                    duty_cycle = 0;
                }
                
                
                PWM6_LoadDutyValue(duty_cycle);
                
            }while(duty_cycle > 1);    //maintain this state 
            
        //}
        
       
        
  
    }
}

/*******************************************
 *  Func: LED_bin
 *  Desc: Convert voltage into 2 bit levels
 *  Obs: Assign the bit code to RA5 and RA4
 *******************************************/

void LED_bin(unsigned int value){
    
    IO_RA4_LAT =  (value >> 8) & 1;
    IO_RA5_LAT =  ((value >> 8)>>1);
  
}
void handler_clock_hms(void){
    IO_RA7_Toggle();
    //PIE4 |= (1<< TMR1IE);
    seg++;
    if(seg >= 60) {
        clkm++;
        seg = 0;
        if(clkm >= 60) {
            clkh++;
            clkm = 0;
        }
    }
    
}

void copyto_EEPROM(void) {
    
}
