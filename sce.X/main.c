#define NREG        30
#define PMON        5
#define TALA        3
#define ALAT        25
#define ALAL        2
#define ALAF        0
#define CLKH        0
#define CLKM        0

#define ON          1
#define OFF         0

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"
#include "eeprom_rw.h"

/* Interrupt Handlers */
void h_clock(void);
void sw1_EXT(void);
void sw2_EXT(void);

/* UI Leds */
void mod1_LED(void);
void mod2_LED(void);
void mod3_LED(void);
void mod4_LED(void);
void LED_bin(unsigned int value);

/* Other */
unsigned char Read_S1();
unsigned char Read_S2();
void update_clock(void);
unsigned int ADC_read(void);
unsigned int represent_led(unsigned char val, __bit two_digits, __bit high_digit);

//

volatile unsigned char clkh, clkm, seg;
volatile unsigned int ms;
volatile unsigned char last5s, last1m;                      // 'last' variables are counters named with their default values; pmon counter default value is 5s; so 'last5s'
volatile __bit half; // h_clock()
volatile __bit alarm, alal;
volatile unsigned char duty_cycle, last3000ms;              // 'last3000ms' is used for alarm PWM
volatile unsigned char delta10ms_2, delta10ms;              // debouncing s1 and s2
volatile __bit s1_pressed, s2_pressed;

__bit s2_pressed;
__bit config_mode;
unsigned char select_mode;

unsigned char nreg, nreg_pt;
unsigned char nreg_init;
unsigned char ring_byte[5];

__bit running;
__bit alarm;
unsigned char pmon;
unsigned char temp, alat;
unsigned char lum_bin;
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

    nreg = EE_FST + 5 * NREG >= EE_RECV ? EE_SIZE : 5 * NREG;
    nreg_pt = 0;
    nreg_init = 0;

    pmon = PMON;
    alarm = 0;
    alaf = 0;
    tala = (TALA*1000 << 10);
    duty_cycle = 0;
    temp = 0;
    alat = ALAT;
    lum_bin = 0;
    alal = ALAL;

    /* Recover Parameters */
    if(DATAEE_ReadByte(EE_RECV) == WORD_MG) {
        if(DATAEE_ReadByte(EE_LST) == cksum()) {
            clkh = DATAEE_ReadByte(EE_RECV + 1);
            clkm = DATAEE_ReadByte(EE_RECV + 2);
            nreg = DATAEE_ReadByte(EE_RECV + 3)
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
        if(Read_S1()) {          
            /* Alarm Dismiss */
            if(alarm == ON) {                                               // Turn off the alarm
                alarm = OFF;
                PWM6_LoadDutyValue(OFF);
                select_mode = 0;                                            // By default select_mode is incremented when s1 is pressed
            }
            /* User UI -- configuration mode */
            else{
                switch(select_mode) {                                       // select_mode is incremented in switch 1 interrupt (after debouncing)
                    case 1: mod1_LED();                                     // Clock Configuration
                            while(!Read_S1() & !Read_S2());                 // Wait for a switch to be pressed
                            if(Read_S1())
                                break;
                            represent_led(clkh, 1);                         // There are 4 subfields, one for each digit of hours and minutes -- in order to represent in 4 leds
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    clkh += 0x10;
                                    if(clkh >= 24) clkh = 0;
                            }
                            represent_led(clkh, 0);
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    clkh += 0x01;
                                    if(clkh >= 24) clkh = 0;
                            }
                            represent_led(clkh, 1);
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    clkm += 0x10;
                                    if(clkm >= 60) clkm = 0;
                            }
                            represent_led(clkm, 0);
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    clkm += 0x01;
                                    if(clkm >= 60) clkm = 0;
                            }
                            break;
                    case 2: mod2_LED();
                            while(!Read_S1() & !Read_S2());                 // Wait for a switch to be pressed
                            if(Read_S1())
                                break;
                            represent_led(alat, 1);                         // There are 2 subfields -- one for each digit of temperature alarm treshold
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    alat += 0x10;
                                    if(alat > 50) alat = 0;
                            }
                            represent_led(alat, 0);
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    alat += 0x01;
                                    if(alat > 50) alat = 0;
                            }
                            break;
                    case 3: mod3_LED();
                            while(!Read_S1() & !Read_S2());                 // Wait for a switch to be pressed
                            if(Read_S1())
                                break;
                            represent_led(alal, 0);                         // There are 1 subfield -- for one digit of luminosity alarm treshold
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    alal += 0x01;
                                    if(alal > 3) alal = 0;
                            }
                            break;
                    case 4: mod4_LED();
                            while(!Read_S1() & !Read_S2());                 // Wait for a switch to be pressed
                            if(Read_S1())
                                break;
                            represent_led(alaf, 0);                         // There are 1 subfield -- for one digit of luminosity alarm treshold
                            while(!Read_S1()) {                             // Increment process
                                if(Read_S2())
                                    alaf^1;
                            }
                            break;
                    default: select_mode = 0;
                }
            }
        }

        if(pmon) {
            INTERRUPT_GlobalInterruptDisable();
            if(last5s >= pmon) {
                last5s = 0;
               INTERRUPT_GlobalInterruptEnable();

                /* Temperature Sensor Aquisition */
                NOP();
                temp = tsttc
                NOP();

                /* Light Sensor Aquisition */
                lum_bin = (ADC_read() >> 8);
                LED_bin(lum_bin); // (UI)

                /* Check for Alarms */
                if(((temp > alat) || (lum_bin > alal)) && alaf) alarm = 1;
                
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
        if(last3000ms >= tala) {
            duty_cycle++;
            PWM6_LoadDutyValue(duty_cycle); // tala is #period_ms/1024, so in #period_ms should ocurr 1024 increments of duty_cycle
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
        select_mode++;
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
    PWM6_LoadDutyValue(OFF);
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
    PWM6_LoadDutyValue(OFF);
    IO_RA5_SetHigh();
}

void mod4_LED(void)
{
    LATA = 0;
    PWM6_LoadDutyValue(OFF);
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

unsigned int represent_led(unsigned char val, __bit high_digit)
{
    unsigned char aux, i = high_digit ? 4 : 0;
    if(val > 99) return 0;
    aux = val / 10;
    LATAbits.LATA4 = (aux >> 3 + i);
    LATAbits.LATA5 = (aux >> 2 + i);
    LATAbits.LATA6 = (aux >> 1 + i);
    LATAbits.LATA7 = (aux >> 0 + i); 
}
