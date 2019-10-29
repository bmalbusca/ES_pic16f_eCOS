#define NREG        30          // Ring Buffer Size
#define PMON        5
#define TALA        3
#define ALAT        25
#define ALAL        2
#define ALAF        0
#define CLKH        0
#define CLKM        0
#define WORD_MG     0xAB        // Magic Word
#define NRECOV      5           // nº of parameters for recovery

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"

void cksum_w(void);
unsigned char cksum(void);
unsigned char read_ring(unsigned char index, unsigned char subindex);
void push_ring(void);
void update_clock(void);
unsigned char tsttc (void);

/*Interrupt Handlers*/
volatile __bit half = 0;
void h_clock(void) {
    IO_RA7_Toggle(); // Clock Activity (UI) T = 1 s
    if(!half) {
        update_clock();
        half = 1;
    }
    else {
        half = 0;
    }
}

__eeprom unsigned char buffer[NREG*5];
__eeprom unsigned char recovery_data[NRECOV]; // order {word_mg, clkh, clkm, nreg_pt, ck}

/* EEPROM adress space
 * 0xF000 to 0xF0FF     
 * 256B -- 256 x 1B  
 */

volatile unsigned char clkh, clkh_aux;
volatile unsigned char clkm, clkm_aux;
volatile unsigned char seg, seg_aux;
volatile unsigned char last5s, last1m;
__bit configuration_mode;
unsigned char nreg, nreg_pt;
__bit nreg_init;
unsigned char ring_byte[5];
unsigned char pmon;
__bit running;
unsigned char temp, lum;
unsigned char last5s_aux, last1m_aux;
unsigned int convertedValue;
unsigned int duty_cycle;
unsigned int task_schedule;
unsigned char value = 0;

void main(void)
{
    clkh = CLKH;
    clkm = CLKM;
    seg = 0;
    last5s = 0;
    last1m = 0;
    configuration_mode = 0;
    nreg = 5*NREG;
    nreg_pt = 0;
    nreg_init = 0;
    pmon = PMON;
    running = 1;
    temp = 0;
    lum = 0;
    convertedValue = 0;
    duty_cycle = 25;
    task_schedule = 0;
    
    /* Recover Parameters */
    if(recovery_data[0] == WORD_MG) {
        if(recovery_data[NRECOV - 1] == cksum()) {
            clkh = recovery_data[1];
            clkm = recovery_data[2];
            nreg_pt = recovery_data[3];
        }
    }

    /* Write Recovery Parameters */
    recovery_data[0] = WORD_MG;
    cksum_w();
    
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    TMR1_SetInterruptHandler(h_clock);

    i2c1_driver_open();
    I2C_SCL = 1;
    I2C_SDA = 1;
    WPUC3 = 1;
    WPUC4 = 1;
       
    while (running)
    {
        INTERRUPT_GlobalInterruptDisable();
        clkh_aux = clkh;
        clkm_aux = clkm;
        seg_aux = seg;
        last5s_aux = last5s;
        last1m_aux = last1m;
        INTERRUPT_GlobalInterruptEnable();
        
        if(configuration_mode) {
            /*User UI -- Config Mode*/
        }
        
        if(pmon) {
            if(last5s_aux >= 5) {
                                
                /* Temperature Sensor Aquisition */
                NOP();
                // temp = tsttc
                temp++;
                NOP();
                
                /* Light Sensor Aquisition */
                   DATAEE_WriteByte(0xF000, 0xAA);
                   DATAEE_WriteByte(0xF001, 0xAA);
                   //push_ring();
                /* Write If Changed */
                if (temp != read_ring(0, 3) || lum != read_ring(0, 4)) {
                    ring_byte[0] = clkh_aux;
                    ring_byte[1] = clkm_aux;
                    ring_byte[2] = seg_aux;
                    ring_byte[3] = temp;
                    ring_byte[4] = lum;
                    push_ring();
                    DATAEE_WriteByte(buffer, 0xAA);
                    /* Write Recovery Parameters */
                    recovery_data[3] = nreg_pt;
                    cksum_w();
                }
                
                INTERRUPT_GlobalInterruptDisable();
                last5s_aux = 0;
                INTERRUPT_GlobalInterruptEnable();
            }
        }
        
        /* Update Clock in EEPROM*/
        if (last1m >= 1) {

            /* Write Recovery Parameters */
            recovery_data[1] = clkh_aux;
            recovery_data[2] = clkm_aux;
            cksum_w();
    
            INTERRUPT_GlobalInterruptDisable();
            last1m = 0;
            INTERRUPT_GlobalInterruptEnable();
        }              
    }
}

void cksum_w()
{
    recovery_data[NRECOV - 1] = cksum();
}

unsigned char cksum()
{
    unsigned char res = 0;
    for(unsigned char i = 0; i < NRECOV; i ++) {
        res += recovery_data[i];
    }
    return res;
}
    
unsigned char read_ring(unsigned char index, unsigned char subindex)
{
    unsigned char absindex;
    unsigned char i = 0;
    
    if(index <= NREG && nreg_init) {
        absindex = 5 * index + subindex;
        if(absindex <= nreg_pt)
            i = nreg_pt - absindex;
        else
            i = nreg - (nreg_pt - absindex);
    }
    else 
        return 0;
    
    return buffer[i];
}

void push_ring()
{   unsigned char address = 0xF000;
    if(!nreg_init) nreg_init = 1;
    for(unsigned char i = 0; i < 5; i++) {
        //if(nreg_pt >= nreg) nreg_pt = 0;
         
        DATAEE_WriteByte(address + nreg_pt, 0xAC);
        nreg_pt ++;
    }
}

unsigned char aux;
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

unsigned char tsttc (void)
{
	unsigned char value;
    
    do{
        IdleI2C();
        StartI2C(); IdleI2C();
    
        WriteI2C(0x9a | 0x00); IdleI2C();
        WriteI2C(0x01); IdleI2C();
        RestartI2C(); IdleI2C();
        WriteI2C(0x9a | 0x01); IdleI2C();
        value = ReadI2C(); IdleI2C();
        NotAckI2C(); IdleI2C();
        StopI2C();
    } while (!(value & 0x40));

	IdleI2C();
	StartI2C(); IdleI2C();
	WriteI2C(0x9a | 0x00); IdleI2C();
	WriteI2C(0x00); IdleI2C();
	RestartI2C(); IdleI2C();
	WriteI2C(0x9a | 0x01); IdleI2C();
	value = ReadI2C(); IdleI2C();
	NotAckI2C(); IdleI2C();
	StopI2C();

	return value;
}