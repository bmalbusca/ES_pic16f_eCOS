#define NREG        30
#define PMON        5
#define TALA        3
#define ALAT        25
#define ALAL        2
#define ALAF        0
#define CLKH        0
#define CLKM        0

// #define ON          1        // Estavam a dar erros. Why? \_(?)_/
// #define OFF         0

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"
#include "eeprom_rw.h"

/* Interrupt Handlers */
void h_clock(void);
void h_precisionclock(void);

void sw1_EXT(void);

/* UI Leds */
void mod1_LED(void);
void mod2_LED(void);
void mod3_LED(void);
void mod4_LED(void);
void LED_bin(unsigned int value);
void all_LED(void);

/* Other */
unsigned char Read_S1();
unsigned char Read_S2();
void update_clock(void);
unsigned int ADC_read(void);
void represent_led(unsigned char val, unsigned char high_digit);

//

volatile unsigned char clkh, clkm, seg;
volatile unsigned int ms;
volatile unsigned char last5s, last1m;                      // 'last' variables are counters named with their default values; pmon counter default value is 5s; so 'last5s'
volatile __bit half; // h_clock()
volatile unsigned char duty_cycle, last3000ms;              // 'last3000ms' is used for alarm PWM
volatile unsigned char delta10ms_2, delta10ms;              // debouncing s1 and s2
volatile __bit s1_pressed, s2_pressed;
volatile __bit alarm;

__bit config_mode;
unsigned char select_mode;

unsigned char nreg, nreg_pt;
unsigned char nreg_init;
unsigned char ring_byte[5];

__bit running;
unsigned char pmon;
unsigned char temp, alat;
unsigned char lum_bin, alal;
unsigned char alaf, tala;
unsigned char aux, aux1;

void main(void)
{
    /* Variable Initilialization */
    running = 1;
    half = 0; // h_clock()
    duty_cycle = 0; // h_precisionclock() -- pwm
    
    clkh = CLKH; clkm = CLKM; seg = 0;
    last5s = 0;
    last1m = 0;
    last3000ms = 0;
    delta10ms = 0;
    delta10ms_2 = 0;
    
    s1_pressed = 0;
    s2_pressed = 0;

    nreg = (unsigned char) (EE_FST + 5 * NREG >= EE_RECV ? EE_SIZE : 5 * NREG);
    nreg_pt = 0;
    nreg_init = 0;

    pmon = PMON;
    alarm = 0;
    alaf = 2;
    tala = TALA;
    tala = (tala*1000 << 10);
    duty_cycle = 0;
    temp = 0;
    alat = ALAT;
    lum_bin = 0;
    alal = (__bit) ALAL;

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

    /* System Initilialization */
    SYSTEM_Initialize();
    I2C_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    /* Set of Interrupt Handlers */
    TMR0_SetInterruptHandler(h_precisionclock);
    TMR1_SetInterruptHandler(h_clock);
    INT_SetInterruptHandler(sw1_EXT);

    while (running)
    {
        
        if(pmon) {
            INTERRUPT_GlobalInterruptDisable();
            
            if(last5s >= pmon) {
                last5s = 0;
               INTERRUPT_GlobalInterruptEnable();

                /* Temperature Sensor Aquisition */
                NOP();
                temp = tsttc();
                NOP();

                /* Light Sensor Aquisition */
                lum_bin = (ADC_read() >> 8);
                LED_bin(lum_bin); // (UI)

                /* Check for Alarms */
                if((/*(temp > alat) ||*/ (lum_bin > alal)) && alaf) {
                    
                    alarm = 1;}
                
                
                if(alarm){
                    duty_cycle++;
                    PWM6_LoadDutyValue(duty_cycle); // tala is #period_ms/1024, so in #period_ms should ocurr 1024 increments of duty_cycle
                }
                
                    
                    
                
                /* Write If Changed */
                if (temp != read_ring(nreg_pt, nreg, nreg_init, 0, 3) || lum_bin != read_ring(nreg_pt, nreg, nreg_init, 0, 4)) {
                    INTERRUPT_GlobalInterruptDisable();
                    ring_byte[0] = clkh;
                    ring_byte[1] = clkm;
                    ring_byte[2] = seg;
                    INTERRUPT_GlobalInterruptEnable();
                    ring_byte[3] = temp;
                    ring_byte[4] = lum_bin;
                    push_ring(&nreg_pt, nreg, &nreg_init, ring_byte);

                    /* Write Recovery Parameters */
                    DATAEE_WriteByte(EE_RECV + 4, nreg_pt);
                    cksum_w();
                }
            }
            else
                INTERRUPT_GlobalInterruptEnable();
        }

        /* Update Clock in EEPROM*/
        INTERRUPT_GlobalInterruptDisable();
        if (last1m >= 1) {
            /* Write Recovery Parameters */
            last1m = 0;
            aux = clkh;
            aux1 = clkm;
            INTERRUPT_GlobalInterruptEnable();
            DATAEE_WriteByte(EE_RECV + 1, aux);
            DATAEE_WriteByte(EE_RECV + 2, aux1);
            cksum_w();
        }
        else
            INTERRUPT_GlobalInterruptEnable();
    }
}

/*******************************************
 *
 *  INTERRUPT HANDLERS
 *
 *******************************************/
void h_clock(void)
{
    IO_RA7_Toggle();                        // Clock Activity (UI) T = 1 s
    if(!half) {
        update_clock();
        half = 1;
    }
    else {
        half = 0;
    }
}

void h_precisionclock(void)
{
    if(alarm) {
        if(last3000ms <= tala) {
            //duty_cycle++;
            //PWM6_LoadDutyValue(duty_cycle); // tala is #period_ms/1024, so in #period_ms should ocurr 1024 increments of duty_cycle
        }
        else{
            PWM6_LoadDutyValue(1023);
        }
       
    }
    else {
        duty_cycle = 0;
        last3000ms = 0;
    }
    
    /* Debouncing of Switch 2 */
    if(LATCbits.LATC5) {
        if (!delta10ms_2) {
            delta10ms_2 = ms;
            s2_pressed = 1;
        } else if ((delta10ms_2 - ms >= 10) || (delta10ms_2 - ms <= -10)) { // debouncing
            delta10ms_2 = 0;
        }
    }
    
    ms++;
    last3000ms++;
}

void sw1_EXT(void)
{
    if(!delta10ms) {
        delta10ms = ms;
        s1_pressed = 1;
        //select_mode++;
        alarm = 0;
    }
    else if((delta10ms - ms >= 10) || (delta10ms - ms <= -10)) {  // debouncing
        delta10ms = 0;
    }
}

/*******************************************
 *
 *  UI LEDS
 *
 *******************************************/
void mod1_LED(void)
{
    LATA = 0;
    PWM6_LoadDutyValue(0);
    IO_RA7_SetHigh();
}

void mod2_LED(void)
{
    LATA = 0;
    PWM6_LoadDutyValue(1023);
}

void mod3_LED(void)
{
    LATA = 0;
    PWM6_LoadDutyValue(0);
    IO_RA5_SetHigh();
}

void mod4_LED(void)
{
    LATA = 0;
    PWM6_LoadDutyValue(0);
    IO_RA4_SetHigh();
}

/*******************************************
 *  Func: LED_bin
 *  Desc: Convert voltage into 2 bit levels
 *  Obs: Assign the bit code to RA5 and RA4
 *******************************************/
void LED_bin(unsigned int value)
{

    IO_RA4_LAT =  (value & 1);
    IO_RA5_LAT =  (value >> 1);
}

/*******************************************
 *  Func: all_LED
 *  Desc: Blink all LEDs
 *******************************************/
void all_LED(void)
{

       IO_RA7_SetHigh();
        __delay_ms(100);
       IO_RA7_SetLow();
       __delay_ms(100);
       //RA6_SetHigh();
       PWM6_LoadDutyValue(1023);
        __delay_ms(100);
       //RA6_SetLow();
        PWM6_LoadDutyValue(0);
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
 *
 *  OTHER
 *
 *******************************************/

unsigned char Read_S1()
{
    unsigned char val;
    INTERRUPT_GlobalInterruptDisable();
    val = s1_pressed;
    s1_pressed = 0;
    INTERRUPT_GlobalInterruptEnable();
    return val;
}

unsigned char Read_S2()
{
    unsigned char val;
    INTERRUPT_GlobalInterruptDisable();
    val = s1_pressed;
    s1_pressed = 0;
    INTERRUPT_GlobalInterruptEnable();
    return val;    
}

void update_clock(void) {
    seg++;
    last5s++;
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
 *  Func: ADC_read
 *  Desc: Read the RB4 port
 *  Obs:  ADC()
 *******************************************/
unsigned int ADC_read(void)
{
   ADCC_StartConversion(channel_ANA0);
    while(!ADCC_IsConversionDone()){
        __delay_ms(1);
    }

    return ADCC_GetConversionResult();
}

/*******************************************
 *  Func: represent_led
 *  Desc: Represents a varibale in 4 LEDS
 *  Obs:  high_digit is 0/1 indicates
 *  if most significant 4 bits are to
 *  be displayed or if zero
 *  the 4 less significant bits. 
 *******************************************/
void represent_led(unsigned char val, unsigned char high_digit)
{
    unsigned char aux = val;
    if(val > 99)
        return;
    if(high_digit)
        aux = val / 10;
    LATAbits.LATA4 = aux >> 3;
    LATAbits.LATA5 = aux >> 2;
    LATAbits.LATA6 = aux >> 1;
    LATAbits.LATA7 = aux >> 0; 
}