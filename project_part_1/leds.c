
#include "leds.h"
#include <xc.h>
#include "mcc_generated_files/mcc.h"

#define OFF 0
#define MAX 1023
#define RA6_High PWM6_LoadDutyValue(MAX)
#define RA6_Low PWM6_LoadDutyValue(OFF)


/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/



void all_LED(void){

    IO_RA7_SetHigh();
    __delay_ms(100);      
    IO_RA7_SetLow();
    __delay_ms(100); 
    RA6_High;
    __delay_ms(100);        
    RA6_Low;
    __delay_ms(100);        
    IO_RA5_SetHigh();       
    __delay_ms(100);       
    IO_RA5_SetLow();
    __delay_ms(100); 
    IO_RA4_SetHigh();
    __delay_ms(100); 
    IO_RA4_SetLow();       



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




/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


void mod1_LED(void){
    LATA = 0;
    PWM6_LoadDutyValue(OFF);
    IO_RA7_SetHigh();

}

void mod2_LED(void){
    LATA = 0;
    RA6_High;    
}

void mod3_LED(void){
    LATA = 0;
    PWM6_LoadDutyValue(OFF);
    IO_RA5_SetHigh();

}

void mod4_LED(void){
    LATA = 0;
    PWM6_LoadDutyValue(OFF);
    IO_RA4_SetHigh();
}



/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

void mode_select_LED(){


    RA6_High;
    IO_RA4_SetHigh();
    __delay_ms(500);
    IO_RA5_SetHigh();
    __delay_ms(500);

    IO_RA4_SetLow();
    IO_RA5_SetLow();


}

void mode_LED(unsigned char subfield){
        switch(subfield){			// Apenas faz display do LED
            case 1: mod1_LED();break;
            case 2: mod2_LED();break;
            case 3: mod3_LED();break;
            case 4: mod4_LED();break;
            default: 
            break;
        }   
        
}


/*******************************************
 *  Func: represent_led
 *  Desc: Represents a varibale in 4 LEDS
 *  Obs:  high_digit is 0/1 indicates
 *  if most significant 4 bits are to
 *  be displayed or if zero
 *  the 4 less significant bits. 
 *******************************************/
void representLed(unsigned char val)
{
    unsigned char aux = val;
    if(val > 99)
        return;
    
    LATAbits.LATA7 = aux >> 3;
    PWM6_LoadDutyValue(((aux >> 2) & 1)*MAX);
    LATAbits.LATA5 = aux >> 1;
    LATAbits.LATA4 = aux & 1; 
}