

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

#define ON 1
#define OFF 0
#define SET 2

volatile int value = 0;
void handler_clock_hms(void);
void copyto_EEPROM(void);
void LED_bin(unsigned int value);
        
volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg;

volatile unsigned char alarm = 0; 

unsigned int convertedValue = 0;
unsigned int duty_cycle = 0;

unsigned int level_bin = 0;
unsigned int lum_threshold = 0;



void sw1_EXT(void){
    
    alarm = OFF;
    PWM6_LoadDutyValue(OFF); 
    
    
}

void ISR_3s(void){
    
    
    
    if (lum_threshold){     //check if we still have a issue
        PWM6_LoadDutyValue(1023);
        alarm = ON;
        
    }
    
    TMR0_StopTimer();
    PIE0bits.TMR0IE = 0;
    
}

void main(void)
{
 
    
    SYSTEM_Initialize();
    TMR0_SetInterruptHandler(ISR_3s);
    TMR1_SetInterruptHandler(handler_clock_hms);
    INT_SetInterruptHandler(sw1_EXT);
    
    unsigned int task_schedule = 0;
    
  
    PWM6_LoadDutyValue(OFF);
    alarm = OFF ;
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
    
    while (1)
    {   
        
        SLEEP();
        NOP();
        
       
        
        task_schedule = seg;
        //if (seg >= 5){
        
            do{
                ADCC_StartConversion(channel_ANA0);
                while(!ADCC_IsConversionDone()){
                    __delay_ms(1);
                }
                convertedValue = ADCC_GetConversionResult();
                
                level_bin = (convertedValue >> 8);
                LED_bin(level_bin);
                lum_threshold = (level_bin > ALAL);
                
                if (alarm == SET && lum_threshold ){    //if alarm is set ON you need to press SW1 ON   
                    duty_cycle +=1 ;   
                    PWM6_LoadDutyValue(duty_cycle);
                }
                else if (alarm == SET && !(lum_threshold)) { //
                    PWM6_LoadDutyValue(OFF);
                    alarm = OFF ;
                   
                }
                else if (alarm == OFF && lum_threshold){
                    PIE0bits.TMR0IE = 1;
                    TMR0_StartTimer();
                    alarm = SET ;
                }
                
                
            }while(1);    //maintain this state 
            
        //}
        
       
        
  
    }
}

/*******************************************
 *  Func: LED_bin
 *  Desc: Convert voltage into 2 bit levels
 *  Obs: Assign the bit code to RA5 and RA4
 *******************************************/

void LED_bin(unsigned int value){
    
    IO_RA4_LAT =  (value & 1);
    IO_RA5_LAT =  (value >>1);
  
}

void handler_clock_hms(void){
    IO_RA7_Toggle();
   
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
