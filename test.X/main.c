/* 
 * 
 *	ALERT - MUDEI A FREQ DO TIMER 1 PARA FICAR MAIS RAPIDO
 * 		NECESSARIO ALTERAR 
 *
 * 	NOTE - KEYWORD PARA ALTERACOES 
*/

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "eeprom_rw.h"

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
#define MIN_TIME -1

volatile int value = 0;
void handler_clock_hms(void);
void copyto_EEPROM(void);
void LED_bin(unsigned int value);
void all_LED(void);
unsigned int ADC_read(void);
void mod1_LED(void);
void mod2_LED(void);
void mod3_LED(void);
void mod4_LED(void);
        
volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg;


volatile unsigned char bounce_time = 0;
volatile unsigned char set_mode = 0;
volatile unsigned char select_mode = 0;
volatile unsigned char config_mode = OFF;
volatile unsigned char alarm = 0;

unsigned int convertedValue = 0;
unsigned int duty_cycle = 0;

unsigned int level_bin = 0;
unsigned int lum_threshold = 0;

unsigned char nreg = NREG;
unsigned char nreg_pt;
unsigned char pmon = PMON;
unsigned char tala = TALA;


/*******************************************
 *  Desc: External Interrupt
 *******************************************/
void sw1_EXT(void){
    
						// NOTE Add here a simple delay 
    if (bounce_time - seg <= MIN_TIME){ 	// NOTE Debouncing SW - WE should use other timer or higher freq
        if (alarm == ON){               	// Turn off the alarm 
            alarm = OFF;
            RA6_SetLow();
            PWM6_LoadDutyValue(OFF); 
        }
        else{
            if(!IO_RB4_GetValue()){
               
                if(config_mode == OFF){
                   config_mode = ON; 			// NOTE after changing to Configure mode disable the EXT interrupt and only check if is pressed at main loop		
                   select_mode = ON;
	 		
		}					// for not overloading the interrupt vector ISR            
               }
            }    
        }
        
        bounce_time = seg;
    }
    
    


/*******************************************
 *  Desc: Timer 0 interrupt
 *******************************************/
void ISR_3s(void){

    if (lum_threshold){     //check if we still have a issue
        PWM6_LoadDutyValue(1023);
        alarm = ON;
        
    }
    
    TMR0_StopTimer();
    PIE0bits.TMR0IE = 0;
    
}

void main(void){
    SYSTEM_Initialize();
    TMR0_SetInterruptHandler(ISR_3s);
    TMR1_SetInterruptHandler(handler_clock_hms);
    INT_SetInterruptHandler(sw1_EXT);
    
    unsigned int task_schedule = 0;
    
    recoverData();
  
    PWM6_LoadDutyValue(OFF);
    alarm = OFF ;
    IO_RB4_SetPullup();                                 //Help debouncing 

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
  
    while (1)
    {   
        
        //SLEEP();
        //NOP();

        //task_schedule = seg;	// NOTE should copy the volatile to  temporary copies - race condition 
                                // In this case we should use the ring buffer to resolve the Read-Write race condition 
	
	//if (seg >= 5){  // NOTE  - ONly debug
                do{
                    if(!config_mode){
  
                        convertedValue = ADC_read();
                        level_bin = (convertedValue >> 8);
                        LED_bin(level_bin);                
                        lum_threshold = (level_bin > ALAL);

                        if(lum_threshold){		// In average we only do 2 comparisons
                            if(alarm == SET){           // If alarm is SET the LED blinks 
                                duty_cycle +=1 ;   
                                PWM6_LoadDutyValue(duty_cycle);
                            }
                            else if(alarm == OFF){
                                PIE0bits.TMR0IE = 1;    // Turn on the 3s counter - NOTE Maintain this state NON Low Power mode 
                                TMR0_StartTimer();	// NOTE - after the alarm was SET we are able to continue the reading and writing to EEPROM at the same time counting the 3s 
                                alarm = SET ;  		// NOTE - Please follow with your kind attention to all details above
                            }
                        }
                        else{
                            if(alarm == SET){		// If everything is OK turn off the alarm 
                                PWM6_LoadDutyValue(OFF);
                                alarm = OFF ;
                            }
                        }

                     }
                    else if(config_mode == ON){	// NOTE Do all your field selection routines here 
                        			// NOTE write a do while until the Config mode OFF
                      all_LED();		// NOTE Check if SW1 and SW2 was pressed - Disable EXT Interrupt

		   //config_mode = SET;			// NOTE Do this in the main loop
           /* while(select_mode){ 
		    		
			if(!IO_RB4_GetValue()){		  
                   		select_mode +=1; 
                    switch(select_mode){			// NOTE this should be on main loop
                        	case 1: mod1_LED();break;
                        	case 2: mod2_LED();break;
                        	case 3: mod3_LED();break;
                        	case 4: mod4_LED();break;
                        	default:select_mode =0; config_mode = OFF;alarm = SET;	// NOTE Enable EXT interrupt or at that moment when the pic is moving to normal operation
                        	break;

                            }
                        }
                
                     } */
                  }
		


                }while(1);    // NOTE maintain this state - ONly debug
           
        //}
        
       
        
  
    }
}



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
       //RA6_SetHigh();
       PWM6_LoadDutyValue(1023);
        __delay_ms(100);        
       //RA6_SetLow();
        PWM6_LoadDutyValue(OFF);
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
 *  Func: ADC_read
 *  Desc: Read the RB4 port
 *  Obs:  ADC()
 *******************************************/

unsigned int ADC_read(void){
    
    ADCC_StartConversion(channel_ANA0);
    while(!ADCC_IsConversionDone()){
        __delay_ms(1);
    }
                        
    return ADCC_GetConversionResult();
}

void handler_clock_hms(void){
    if(!config_mode){
        IO_RA7_Toggle();
    }
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


void mod1_LED(void){
    LATA = 0;
    PWM6_LoadDutyValue(OFF);
    IO_RA7_SetHigh();
     
}

void mod2_LED(void){
    LATA = 0;
    PWM6_LoadDutyValue(1023);    
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
