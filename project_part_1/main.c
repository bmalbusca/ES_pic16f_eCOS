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
#include "I2C/i2c.h" 

#define NREG 30
#define PMON 5
#define TALA 3
#define ALAT 100
#define ALAL 2
#define ALAF 0
#define CLKH 0
#define CLKM 0

#define ON 1
#define OFF 0
#define SET 2

#define FIELD 0
#define SUBFIELD 1

#define NONE -1
#define MIN_TIME -1

#define TIMER_MS_RESET 200
#define DEBNC_TIME 2     //200ms

typedef struct Subfield_Info_Struct{
    unsigned char(*limit)(void);
    void(*reconstruct_subfield)(unsigned char);
}Subfield_Info;



/*******************************************
 *  Desc: Protos
 *******************************************/

volatile int value = 0;
void handler_clock_hms(void);
void handler_clock_ms(void);
void copyto_EEPROM(void);
void LED_bin(unsigned int value);
void all_LED(void);
unsigned int ADC_read(void);
void recAlarm(unsigned char _value);
void recMinutes(unsigned char _value);
void recHour(unsigned char _value);
unsigned char limitTempThreshUnits(void);
unsigned char limitHoursUnits(void);
void recTempThresh(unsigned char _value);
void recLumThresh(unsigned char _value);

void mod1_LED(void);
void mod2_LED(void);
void mod3_LED(void);
void mod4_LED(void);

unsigned char checkDebounceSW1();
unsigned char checkDebounceSW2();
void representLed(unsigned char val);

void clock_field(void);
void config_routine(void);
void selectSubfield(void);
void increment_subfield();
void getSubfieldInfo(void);
void mode_select_LED();

/*******************************************
 *  Desc: Global Variables
 *******************************************/

volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg;
volatile unsigned char clkms = 0;

unsigned int hours_tens =0; // 0 -23
unsigned int hours_units =0;
unsigned int min_tens =0; //0 -59
unsigned int min_units =0;

unsigned char nreg_init;
unsigned char ring_byte[5];

unsigned char last_ms = 0;
unsigned char last_ms2 = 0;
volatile unsigned char last5s =0, last1m=0;

unsigned int mode_field_subfield[2]= {NONE,NONE};
volatile unsigned char set_mode = 0;
volatile unsigned char config_mode = OFF;

volatile unsigned char alarm = 0;

unsigned char temp;
unsigned char alaf;

unsigned int convertedValue = 0;
unsigned int duty_cycle = 0;

unsigned int lum_bin = 0;
unsigned int lum_threshold = 0;

unsigned char nreg = NREG;
unsigned char nreg_pt;
unsigned char pmon = PMON;
unsigned char tala = TALA;

Subfield_Info subfield_info = {.limit = NULL, .reconstruct_subfield = NULL};

const unsigned char num_subfields[4] = {4, 1, 2, 1};
unsigned char temp_thresh = ALAT;
unsigned char lum_thresh = ALAL;


/*******************************************
 *  Desc: External Interrupt
 *******************************************/
void sw1_EXT(void){


    if(checkDebounceSW1()){
        if (alarm == ON){               	// Turn off the alarm 
            alarm = OFF;
            RA6_SetLow();
            PWM6_LoadDutyValue(OFF); 
            last_ms = clkms;
        }
        else{
            if(!IO_RB4_GetValue()){

                if(config_mode == OFF){
                    config_mode = ON; 			// NOTE after changing to Configure mode disable the EXT interrupt and only check if is pressed at main loop		



                }					// for not overloading the interrupt vector ISR            
            }
        }
        last_ms = clkms;
    }

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

/*******************************************
 *  Desc: Main fucntion
 *******************************************/


void main(void){

    SYSTEM_Initialize();
    TMR0_SetInterruptHandler(ISR_3s);
    TMR1_SetInterruptHandler(handler_clock_hms);
    INT_SetInterruptHandler(sw1_EXT);
    TMR2_SetInterruptHandler(handler_clock_ms);

    //variables initialization
    
    unsigned char t5s =0;
    unsigned char aux, aux1;

    nreg = (unsigned char) (EE_FST + 5 * NREG >= EE_RECV ? EE_SIZE : 5 * NREG);
    nreg_pt = 0;
    nreg_init = 0;
    alaf = 0;
    temp = 0;
    lum_bin = 0;
    lum_threshold = 0;
    duty_cycle = 0;
    set_mode= OFF;
    
    //I2C 
    i2c1_driver_open();
    I2C_SCL = 1;
    I2C_SDA = 1;
    WPUC3 = 1;
    WPUC4 = 1;

    //recover data from EEPROM
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

        SLEEP();                //sleep mode
        NOP();

        PIE4bits.TMR1IE = 0;    //race condition - disable interrupt
        t5s = last5s;
        PIE4bits.TMR1IE = 1;

        if(t5s >= pmon){ 
            PIE4bits.TMR1IE = 0;
            last5s =0; 
            PIE4bits.TMR1IE = 1;  

            do{       
                if(!config_mode){

                    convertedValue = ADC_read();        //read potentiometer

                    lum_bin = (convertedValue >> 8);    //convert into 4 levels

                    LED_bin(lum_bin);                   //LED representation


                    NOP();
                    temp = tsttc();                     //I2C read
                    NOP();


                    lum_threshold = (lum_bin > ALAL || temp > ALAT  ) & alaf;   //detect alarm 
                       // Push to Ring buffer
                    if (temp != read_ring(nreg_pt, nreg, nreg_init, 0, 3) || lum_bin != read_ring(nreg_pt, nreg, nreg_init, 0, 4)) {

                        PIE4bits.TMR1IE = 0;        
                        ring_byte[0] = clkh;
                        ring_byte[1] = clkm;
                        ring_byte[2] = seg;
                        PIE4bits.TMR1IE = 1; 
                        ring_byte[3] = temp;
                        ring_byte[4] = lum_bin;
                        push_ring(&nreg_pt, nreg, &nreg_init, ring_byte);

                        DATAEE_WriteByte(EE_RECV + 4, nreg_pt);
                        cksum_w();
                    }



                    if(lum_threshold){
                        if(alarm == SET){           //if alarm is set ON you need to press SW1 ON 
                            duty_cycle +=1 ;   
                            PWM6_LoadDutyValue(duty_cycle);
                        }
                        else if(alarm == OFF){
                            PIE0bits.TMR0IE = 1;    // We exceed the threshold  check the 3 sec 
                            TMR0_StartTimer();
                            alarm = SET ;  
                        }
                    }
                    else{
                        if(alarm == SET){
                            PWM6_LoadDutyValue(OFF);
                            alarm = OFF ;
                        }
                    }

                }

                else if(config_mode == ON){

                    EXT_INT_InterruptDisable(); 
                    config_routine();
                    EXT_INT_InterruptEnable(); 
                }

                __delay_ms(10);

            }while(alarm == SET);

            PIE4bits.TMR1IE = 0;    
            if (last1m >= 1) {
                /* Write Recovery Parameters */
                last1m = 0;
                aux = clkh;
                aux1 = clkm;
                PIE4bits.TMR1IE = 1;
                DATAEE_WriteByte(EE_RECV + 1, aux);
                DATAEE_WriteByte(EE_RECV + 2, aux1);
                cksum_w();

            }else{
                PIE4bits.TMR1IE = 1;}

        }
    }

}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


 void config_routine(void){
    
    unsigned int select_mode =0;
    last_ms = clkms;
    last_ms2 = clkms;
    
    do{
        if(mode_field_subfield[FIELD] == NONE && select_mode == 0){  			
            all_LED();
        } 

        if(!IO_RB4_GetValue()){		  
            if(checkDebounceSW1()){

                select_mode +=1;

                switch(select_mode){			
                    case 1: mod1_LED();break;
                    case 2: mod2_LED();break;
                    case 3: mod3_LED();break;
                    case 4: mod4_LED();break;
                    default: select_mode = 0; config_mode = OFF; alarm = OFF;	// NOTE Enable EXT interrupt or at that moment when the pic is moving to normal operation
                    break;

                }   
            }
                
            last_ms = clkms;
        }
                

        if(!IO_RC5_GetValue()){
            if(checkDebounceSW2()){                
                mode_field_subfield[FIELD] = select_mode;
                mode_select_LED();      // notice the select was done
                
                
                selectSubfield();
                
            }
            last_ms2 = clkms;
        }
        
        __delay_ms(2);
        
    }while(config_mode == ON);  
    
    mode_field_subfield[FIELD] = NONE;
    mode_field_subfield[SUBFIELD] = NONE;
    
}
/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

void selectSubfield(void){ // o clock tem 4 subfields
    unsigned int  subfield = 1;

    if(mode_field_subfield[FIELD] == 1){
        //FALTA disable int clock
    }//FALTA resto dos ints

    do{
        if(!IO_RB4_GetValue()){
            if(checkDebounceSW1()){
                subfield +=1;
            }
            last_ms = clkms;
        }
                
        switch(subfield){			// Apenas faz display do LED
            case 1: mod1_LED();break;
            case 2: mod2_LED();break;
            case 3: mod3_LED();break;
            case 4: mod4_LED();break;
            default: 
            break;

        }   
        

        if(!IO_RC5_GetValue()){             // Select operation
            if(checkDebounceSW2()){
                mode_field_subfield[SUBFIELD] = subfield;
                getSubfieldInfo();
                increment_subfield();
            }
            last_ms2 = clkms;
        }
        
        __delay_ms(2);
        
    }while(subfield <= num_subfields[mode_field_subfield[FIELD]]);  

    if(mode_field_subfield[FIELD] == 1){
        //FALTA enable int clock
    }//FALTA resto dos ints
    
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/
void getSubfieldInfo(void){
    switch(mode_field_subfield[FIELD]){
        case 1: 
            switch(mode_field_subfield[SUBFIELD]){
                case 1:                             //Hour tens
                    subfield_info.limit = (unsigned char(*)(void))2;
                    subfield_info.reconstruct_subfield = &recHour;
                break;
                case 2:                             //Hour units
                    subfield_info.limit = &limitHoursUnits;
                    subfield_info.reconstruct_subfield = &recHour;
                break;
                case 3:                             //minutes tens
                    subfield_info.limit = (unsigned char(*)(void))5;
                    subfield_info.reconstruct_subfield = &recMinutes;
                break;
                case 4:                             //minutes units
                    subfield_info.limit = (unsigned char(*)(void))9;
                    subfield_info.reconstruct_subfield = &recMinutes;
                break;
            }
        break;
        //------------------------
        case 2:
            subfield_info.limit = (unsigned char(*)(void))1;
            subfield_info.reconstruct_subfield = &recAlarm;
        break;
        //------------------------
        case 3: 
            switch(mode_field_subfield[SUBFIELD]){
                case 1:                             //Temperature Thresh tens
                    subfield_info.limit = (unsigned char(*)(void))5;
                    subfield_info.reconstruct_subfield = &recTempThresh;
                break;
                case 2:                             //Temperature Thresh units
                    subfield_info.limit = &limitTempThreshUnits;
                    subfield_info.reconstruct_subfield = &recTempThresh;
                break;
            }
        break;
        //------------------------
        case 4: 
            subfield_info.limit = (unsigned char(*)(void))3;
            subfield_info.reconstruct_subfield = &recLumThresh;
        break;
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


void recLumThresh(unsigned char _value){
    lum_thresh = _value;
}

void recTempThresh(unsigned char _value){
    if(mode_field_subfield[SUBFIELD] == 1){     //Se tivermos a mudar temp thresh tens
        
        temp_thresh = _value + (temp_thresh % 10);            //tens + units
        
    }else{                                      //Se tivermos a mudar temp units
        temp_thresh = (temp_thresh / 10) + _value;            //tens + units
    }
}


void recAlarm(unsigned char _value){
    alarm = _value;
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


void recMinutes(unsigned char _value){
    if(mode_field_subfield[SUBFIELD] == 3){     //Se tivermos a mudar Minutes tens
        
        clkm = _value + (clkm % 10);            //tens + units
        
    }else{                                      //Se tivermos a mudar Minutes units
        clkm = (clkm / 10) + _value;            //tens + units
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


void recHour(unsigned char _value){
    if(mode_field_subfield[SUBFIELD] == 1){     //Se tivermos a mudar Hour tens
        
        clkh = _value + (clkh % 10);            //tens + units
        
    }else{                                      //Se tivermos a mudar Hour units
        clkh = (clkh / 10) + _value;            //tens + units
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


unsigned char limitTempThreshUnits(){
     if((temp_thresh / 10) == 5){       //Se os tens das horas são 2, o limite de unidades é 3 
        return 0;
    }else{
        return 9;
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


unsigned char limitHoursUnits(){
    //n é preciso disable do interrupt do clock
    if((clkh / 10) == 2){       //Se os tens das horas são 2, o limite de unidades é 3 
        return 3;
    }else{
        return 9;
    }
}


/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


void increment_subfield(void){  //funcao universal para todos os subfields 
    
    unsigned char exit = 0;
    unsigned char _value = 0;

    PWM6_LoadDutyValue(0);

          
    while(exit == 0){
            
        if(!IO_RC5_GetValue()){
            if(checkDebounceSW2()){
               
                _value++;
                
                 representLed(_value);
               
            }
            last_ms2 = clkms;
        }

        if(!IO_RB4_GetValue()){		  
            if(checkDebounceSW1()){
                exit = 1;
            }
            last_ms = clkms;
        }
    }
    subfield_info.reconstruct_subfield(_value);
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

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

void handler_clock_hms(void){

    if(!config_mode){
        IO_RA7_Toggle();
    }

    last5s++;
    seg++;
    if(seg >= 60) {
        clkm++;
        last1m++;
        seg = 0;
        if(clkm >= 60) {
            clkh++;
            clkm = 0;
        }
    }

}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

void handler_clock_ms(void){   
    clkms++;

    if(clkms > TIMER_MS_RESET){
        clkms = 0;
    }
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

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

unsigned char checkDebounceSW1(){
    PIE4bits.TMR1IE = 0;

    if(clkms - last_ms < 0){       // clkms deu reset

        if ((TIMER_MS_RESET - last_ms)+ clkms > DEBNC_TIME ){
            last_ms = clkms;
            PIE4bits.TMR1IE = 1;
            return 1;
        }
    }

    if(clkms - last_ms < DEBNC_TIME){
        return 0;
        PIE4bits.TMR1IE = 1;
    }else{
        last_ms = clkms;
        PIE4bits.TMR1IE = 1;
        return 1;
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


unsigned char checkDebounceSW2(){
    //Fazer disable interrupt clkms

    if(clkms - last_ms2 < 0){       // clkms deu reset

        if ((TIMER_MS_RESET - last_ms2)+ clkms > DEBNC_TIME ){
            last_ms2 = clkms;
            return 1;
        }
    }

    if(clkms - last_ms2 < DEBNC_TIME){
        return 0;
    }else{
        last_ms2 = clkms;
        return 1;
    }
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/

void mode_select_LED(){


    PWM6_LoadDutyValue(1023);
    IO_RA4_SetHigh();
    __delay_ms(500);
    IO_RA5_SetHigh();
    __delay_ms(500);

    IO_RA4_SetLow();
    IO_RA5_SetLow();


}

void recover(){

    /* Recover Parameters */
    if(DATAEE_ReadByte(EE_RECV) == WORD_MG) {
        if(DATAEE_ReadByte(EE_LST) == cksum()) {
            clkh = DATAEE_ReadByte(EE_RECV + 1);
            clkm = DATAEE_ReadByte(EE_RECV + 2);
            nreg = DATAEE_ReadByte(EE_RECV + 3);
            nreg_pt = DATAEE_ReadByte(EE_RECV + 4);
            pmon = DATAEE_ReadByte(EE_RECV + 5);
            tala = DATAEE_ReadByte(EE_RECV + 6);
        }
    }

    reset_recv();

    /* Write Recovery Parameters */
    DATAEE_WriteByte(EE_RECV, WORD_MG);
    DATAEE_WriteByte(EE_RECV + 3, nreg);
    DATAEE_WriteByte(EE_RECV + 5, pmon);
    DATAEE_WriteByte(EE_RECV + 6, tala);
    cksum_w();



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
    
    aux = val >> 2;
    
    LATAbits.LATA4 = aux >> 3;
    LATAbits.LATA5 = aux >> 2 & 1;
    LATAbits.LATA6 = (aux /10) >>1 & 1;
    LATAbits.LATA7 = (aux /100); 
}