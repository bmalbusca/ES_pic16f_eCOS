/* 
 * 
 *  ALERT - MUDEI A FREQ DO TIMER 1 PARA FICAR MAIS RAPIDO
 *      NECESSARIO ALTERAR 
 *
 *  NOTE - KEYWORD PARA ALTERACOES 
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "eeprom_rw.h"
#include "I2C/i2c.h"
#include "leds.h" 
#include "defines.h"
#include "uartusr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define DEBNC_TIME 15


#define UART_BUFF_SIZE 16


/*******************************************
 *  Desc: Protos
 *******************************************/

volatile int value = 0;
void handler_clock_hms(void);
void handler_clock_ms(void);
void copyto_EEPROM(void);

unsigned int ADC_read(void);
void recAlarm(unsigned char _value);
void recMinutes(unsigned char _value);
void recHour(unsigned char _value);
unsigned char limitTempThreshUnits(void);
unsigned char limitHoursUnits(void);
void recTempThresh(unsigned char _value);
void recLumThresh(unsigned char _value);
unsigned char increment_subfield(unsigned char limit, unsigned init_val);
void parse_message(char * message, uint8_t max_size);

unsigned char checkDebounceSW1();
unsigned char checkDebounceSW2();

void clock_field(void);
void config_routine(void);
void selectSubfield(void);
void getSubfieldInfo(void);
void save_recovery_params(void);
void check_thresholds(uint8_t threshold);

typedef unsigned char udch;
void ring_buffer (udch * _ring_byte, udch clock_h, udch  clock_m, udch  clock_s,udch tem, udch lum);



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

char str_rc[UART_BUFF_SIZE]="\0";
char str_snd[UART_BUFF_SIZE]="hello\0";

unsigned int convertedValue = 0;
unsigned int duty_cycle = 0;

unsigned int lum_bin = 0;
unsigned int threshold = 0;

unsigned char temp;
unsigned char alaf;
unsigned char nreg = NREG;
unsigned char nreg_pt;
unsigned char pmon = PMON;
unsigned char tala = TALA;

const unsigned char num_subfields[5] = {0,4, 1, 2, 1};
unsigned char temp_thresh = ALAT;
unsigned char lum_thresh = ALAL;


/*******************************************
 *  Desc: External Interrupt
 *******************************************/
void sw1_EXT(void){

    if(checkDebounceSW1()){
        if (alarm == ON){                   // Turn off the alarm 
            alarm = OFF;
            PWM6_LoadDutyValue(OFF); 
            last_ms = clkms;
        }
        else{
            if(config_mode == OFF){
                config_mode = ON;           // NOTE after changing to Configure mode disable the EXT interrupt and only check if is pressed at main loop        
            }                   // for not overloading the interrupt vector ISR            
        }
    }
}




/*******************************************
 *  Desc: Timer 0 interrupt
 *******************************************/
void ISR_3s(void){

    if (threshold){     //check if we still have a issue
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
    
    unsigned char t5s =0, t1m =0;
    

    nreg = (unsigned char) (EE_FST + 5 * NREG >= EE_RECV ? EE_SIZE : 5 * NREG);
    nreg_pt = 0;
    nreg_init = 0;
    alaf = 1;
    temp = 0;
    lum_bin = 0;
    threshold = 0;
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

        //SLEEP();                //sleep mode
        NOP();

         if(EUSART_is_rx_ready()){
            if (read_str_UART(str_rc, UART_BUFF_SIZE)>1){
                parse_message(str_rc, UART_BUFF_SIZE);  
            }
             
         }

    
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
                            
                               // Push to Ring buffer
                            if (temp != read_ring(nreg_pt, nreg, nreg_init, 0, 3) || lum_bin != read_ring(nreg_pt, nreg, nreg_init, 0, 4)) {
                                ring_buffer (ring_byte,clkh, clkm, seg, temp,lum_bin);
                                push_ring(&nreg_pt, nreg, &nreg_init, ring_byte);
                                DATAEE_WriteByte(EE_RECV + 4, nreg_pt);
                                cksum_w();
                            }


                            threshold = (lum_bin > lum_thresh || temp > temp_thresh  ) & alaf;   //detect alarm 
                            check_thresholds(threshold);
                            

                    }

                    else if(config_mode == ON){

                        EXT_INT_InterruptDisable(); 
                        config_routine();
                        EXT_INT_InterruptEnable(); 
                        DATAEE_WriteByte(EE_RECV + 5, temp_thresh);
                        DATAEE_WriteByte(EE_RECV + 6, lum_thresh);
                    }

                    __delay_ms(10);

            }while(alarm == SET);
            
        }
   
       PIE4bits.TMR1IE = 0;
       t1m = last1m;
       PIE4bits.TMR1IE = 1;
       
       if (t1m  >= 1) {
         save_recovery_params();
       }
    
    }

}



void check_thresholds(uint8_t threshold){
    
    if(threshold){
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
                    default: select_mode = 0; config_mode = OFF; alarm = OFF;   // NOTE Enable EXT interrupt or at that moment when the pic is moving to normal operation
                    break;

                }   
            }
        }
                

        if(!IO_RC5_GetValue()){
            if(checkDebounceSW2()){                
                mode_field_subfield[FIELD] = select_mode;
                mode_select_LED();      // notice the select was done  
                selectSubfield();
            }
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

 
    do{
        if(!IO_RB4_GetValue()){
            if(checkDebounceSW1()){
                subfield +=1;
            }

        }
                
        mode_LED( subfield);

        if(!IO_RC5_GetValue()){             // Select operation
            if(checkDebounceSW2()){
                mode_field_subfield[SUBFIELD] = subfield;
                getSubfieldInfo();
                all_LED();                  // indication of subfield selection 
            }

        }
        
        __delay_ms(2);
        
    }while(subfield <= num_subfields[mode_field_subfield[FIELD]]);  

     
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/
void getSubfieldInfo(void){

    unsigned char h_tens, h_units, m_tens, m_units, temp_thresh_tens, temp_thresh_units;
    
    switch(mode_field_subfield[FIELD]){
        case 1: 
            PIE4bits.TMR1IE = 0;
            h_tens = clkh / 10;
            h_units = clkh % 10;
            m_tens = clkm / 10;
            m_units = clkm % 10;
        
            switch(mode_field_subfield[SUBFIELD]){
                case 1:                             //Hour tens
                    h_tens = increment_subfield(2, h_tens);
                    break;
                case 2:                             //Hour units
                    h_units = increment_subfield( limitHoursUnits() , h_units);
                    break;
                case 3:                             //minutes tens
                    m_tens = increment_subfield( 5 , m_tens);
                    break;
                case 4:                             //minutes units
                    m_units = increment_subfield( 9, m_units);
                        break;
        
            }
            clkh = 10*h_tens + h_units;
            clkm = 10*m_tens + m_units;
            PIE4bits.TMR1IE = 0;
        break;
        //------------------------
        case 2:
            alaf = increment_subfield( 1, alaf );   
    break;
        //------------------------
        case 3:
            temp_thresh_tens = temp_thresh / 10;
            temp_thresh_units = temp_thresh % 10;
            switch(mode_field_subfield[SUBFIELD]){
                case 1:                             //Temperature Thresh tens
                    temp_thresh_tens = increment_subfield(5, temp_thresh_tens);
                break;
                case 2:                             //Temperature Thresh units
                    temp_thresh_units = increment_subfield(limitTempThreshUnits(), temp_thresh_units);
                break;
            }
            temp_thresh = 10*temp_thresh_tens + temp_thresh_units;
        break;
        //------------------------
        case 4: 
            lum_thresh = increment_subfield(3, lum_thresh);
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


/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *  Obs: 
 *******************************************/


unsigned char limitTempThreshUnits(){
     if((temp_thresh / 10) == 5){       //Se os tens das horas s?o 2, o limite de unidades ? 3 
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
    //n ? preciso disable do interrupt do clock
    if((clkh / 10) == 2){       //Se os tens das horas s?o 2, o limite de unidades ? 3 
        return 3;
    }else{
        return 9;
    }
}

unsigned char increment_subfield(unsigned char limit, unsigned init_val){  //funcao universal para todos os subfields

    unsigned char exit = 0;
    unsigned char _value = init_val;

    PWM6_LoadDutyValue(0);
    LATA = 0;
    
    if(_value > limit) _value = 0;

    while(exit == 0){
        representLed(_value);
        if(!IO_RC5_GetValue()){
            if(checkDebounceSW2()){
                _value++;
                if(_value > limit) _value = 0;
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
return _value;  
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



void parse_message(char * message, uint8_t max_size){
    uint8_t i,h,m,s,p;
    uint8_t  T,L,a;
    char c = '0', cmd =message[1] ;
    //reply_UART_OK(cmd);
    switch (cmd)
    {
     case (char)RCLK : 
        
        PIE4bits.TMR1IE = 0;
        //printf("%c%i%i%i%c",(char)SOM,clkh,clkm,seg,(char)EOM );
        write_UART((char)SOM);
        write_UART((char)cmd);
        write_UART((char)clkh);
        write_UART((char)clkm);
        write_UART((char)seg);
        write_UART((char)EOM);
        
        PIE4bits.TMR1IE = 1;
        
        break;
    
    case (char)SCLK:
        
        if ((strlen(message)) >= 5 ){ 
            h = message[2] -'0';
            m = message[3] - '0';
            s = message[4] - '0';
            
            if (( h >= 0 && h <24) && (m >=0 && m < 60 ) && ( s>=0 && s<60) ){
                PIE4bits.TMR1IE = 0;
                clkh = h;
                clkm = m;
                seg = s;
                PIE4bits.TMR1IE = 1;
            }
            else{
                reply_UART_ERROR((char)cmd);  
            } 
        
        }
        else{
            
            reply_UART_ERROR((char)cmd);
        }
        
        break;
    case (char)RTL:
        //printf("%c%i%i%c",(char)SOM,temp,lum_bin,(char)EOM );
        write_UART((char)SOM);
        write_UART((char)cmd);
        write_UART((char)temp);
        write_UART((char)lum_bin);
        write_UART((char)EOM);
        break;
    case (char)RPAR:
        //printf("%c%i%i%c",(char)SOM,pmon,tala,(char)EOM );
        write_UART((char)SOM);
        write_UART((char)cmd);
        write_UART((char)pmon);
        write_UART((char)tala);
        write_UART((char)EOM);
        break;
    case (char)MMP:
            
            if ((strlen(message)) >= 3 ){ 
                p = message[2] -'0';
               
                if (( p >= 0 && p <100)  ){
                 pmon = p  ;  
                }
                else{
                    reply_UART_ERROR((char)cmd);  
                } 
            }
            else{
                
                reply_UART_ERROR((char)cmd);
            }
        break;
    case (char) MTA:
        break;
    case (char) RALA:
        //printf("%c%i%i%c",(char)SOM,pmon,tala,(char)EOM );
        write_UART((char)SOM);
        write_UART((char)cmd);
        write_UART((char)temp_thresh);
        write_UART((char)lum_thresh);
        write_UART((char)EOM);
        break;
    case (char)DATL:
        if ((strlen(message)) >= 4 ){ 
                T = message[2] -'0';
                L = message[3] -'0';
               
                if (( T >= 0 && T <51) && ( L >= 0 && L<4)  ){
                 temp_thresh = T ;
                 lum_thresh = L;
                }
                else{
                    reply_UART_ERROR((char)cmd);  
                } 
            }
            else{
                
                reply_UART_ERROR((char)cmd);
        }
        break;
    case (char)AALA:
        if ((strlen(message)) >= 3 ){ 
                a = message[2] -'0';
                if (( a == 0 || a == 1)){
                 alaf = a ;
                 
                }
                else{
                    reply_UART_ERROR((char)cmd);  
                } 
            }
            else{              
                reply_UART_ERROR((char)cmd);
        }
        break;
    case (char)IREG:
        break;
    case (char)TRGC:
        break;
    case (char)TRGI:
        break;
    case (char)NMFL:
        break;
    default:
        reply_UART_ERROR((char)CMD_ERROR);
        
    }

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

/* Write Recovery Parameters */
void save_recovery_params(){
        
    unsigned char aux, aux1;
    PIE4bits.TMR1IE = 0;  
    last1m = 0;
    aux = clkh;
    aux1 = clkm;
    PIE4bits.TMR1IE = 1;
    DATAEE_WriteByte(EE_RECV + 1, aux);
    DATAEE_WriteByte(EE_RECV + 2, aux1);
    cksum_w();

    
}

void ring_buffer (unsigned char * _ring_byte, unsigned char clock_h, unsigned char clock_m, unsigned char clock_s, unsigned char tem,unsigned char lum){

    PIE4bits.TMR1IE = 0;        
    _ring_byte[0] = clock_h;
    _ring_byte[1] = clock_m;
    _ring_byte[2] = clock_s;
    PIE4bits.TMR1IE = 1; 
    _ring_byte[3] = tem;
    _ring_byte[4] = lum;
}
               



