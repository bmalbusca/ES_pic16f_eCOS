#define NREG        30
#define PMON        5
#define TALA        3
#define ALAT        25
#define ALAL        2
#define ALAF        0
#define CLKH        0
#define CLKM        0
#define WORD_MG     0xAB        // "magic word"

#define EE_FST      0xF000      // ring buffer starts here
#define EE_LST      0xF0FF      // last byte has the checksum
#define EE_SIZE     256
#define EE_RECV     EE_LST - 10 // adresses reserved to recovery parameters

/* EEPROM adress space
 * 0xF000 to 0xF0FF     
 * 256B -- 256 x 1B  
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"

void reset_recv(void);
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

volatile unsigned char clkh, clkh_aux;
volatile unsigned char clkm, clkm_aux;
volatile unsigned char seg, seg_aux;
volatile unsigned char last5s, last1m;

__bit configuration_mode;

unsigned char nreg, nreg_pt;
__bit nreg_init;
unsigned char ring_byte[5];

__bit running;
unsigned char pmon;
unsigned char temp, lum;
unsigned char aux, aux1;

void main(void)
{
    clkh = CLKH; clkm = CLKM; seg = 0;
    last5s = 0; last1m = 0;
    
    configuration_mode = 0;
    
    nreg = EE_FST + 5 * NREG >= EE_RECV ? EE_SIZE : 5 * NREG;
    nreg_pt = 0;
    nreg_init = 0;
    
    pmon = PMON;
    running = 1;
    temp = 0;
    lum = 0;
    
    /* Recover Parameters */
    if(DATAEE_ReadByte(EE_RECV) == WORD_MG) {
        if(DATAEE_ReadByte(EE_LST) == cksum()) {
            clkh = DATAEE_ReadByte(EE_RECV + 1);
            clkm = DATAEE_ReadByte(EE_RECV + 2);;
            nreg_pt = DATAEE_ReadByte(EE_RECV + 3);;
        }
    }

    reset_recv();
    
    /* Write Recovery Parameters */
    DATAEE_WriteByte(EE_RECV, WORD_MG);
    cksum_w();

    __delay_ms(16000);
    
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    TMR1_SetInterruptHandler(h_clock);

    /* I2C Start */
    i2c1_driver_open();
    I2C_SCL = 1;
    I2C_SDA = 1;
    WPUC3 = 1;
    WPUC4 = 1;
       
    while (running)
    {
        if(configuration_mode) {
            /*User UI -- Config Mode*/
        }
        
        if(pmon) {
            INTERRUPT_GlobalInterruptDisable();
            if(last5s >= pmon) {
                last5s = 0;
               INTERRUPT_GlobalInterruptEnable();
                                
                /* Temperature Sensor Aquisition */
                NOP();
                temp++; // temp = tsttc
                NOP();
                
                /* Light Sensor Aquisition */

                /* Write If Changed */
                if (temp != read_ring(0, 3) || lum != read_ring(0, 4)) {
                    INTERRUPT_GlobalInterruptDisable();
                    ring_byte[0] = clkh;
                    ring_byte[1] = clkm;
                    ring_byte[2] = seg;
                    INTERRUPT_GlobalInterruptEnable();
                    ring_byte[3] = temp;
                    ring_byte[4] = lum;
                    push_ring();

                    /* Write Recovery Parameters */
                    DATAEE_WriteByte(EE_RECV + 3, nreg_pt);
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

void reset_recv() {
    for(unsigned int i = EE_RECV; i <= EE_LST; i ++)
        DATAEE_WriteByte(i, 0);
}

void cksum_w()
{
    DATAEE_WriteByte(EE_LST, cksum());
}

unsigned char cksum()
{
    unsigned char res = 0;
    for(unsigned int i = EE_RECV; i < EE_LST; i ++) {
        res += DATAEE_ReadByte(i);
    }
    return res;
}
    
unsigned char read_ring(unsigned char index, unsigned char subindex)
{
    unsigned char absindex;
    unsigned char i = 0;
    
    if(index <= nreg && nreg_init) {
        absindex = 5 * index + subindex;
        if(absindex <= nreg_pt)
            i = nreg_pt - 5 + absindex;
        else
            i = nreg - (nreg_pt - 5 + absindex);
    }
    else 
        return 0;
    
    return DATAEE_ReadByte(EE_FST + i);
}

void push_ring()
{   if(!nreg_init) nreg_init = 1;
    for(unsigned char i = 0; i < 5; i++) {
        if(nreg_pt >= nreg) nreg_pt = 0;
        DATAEE_WriteByte(EE_FST + nreg_pt, ring_byte[i]);
        nreg_pt ++;
    }
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