#define NREG        30          // Ring Buffer Size --  49 will fill adresses until 0x70F4
#define PMON        5
#define TALA        3
#define ALAT        25
#define ALAL        2
#define ALAF        0
#define CLKH        0
#define CLKM        0
#define ADDR_RING   0x7000      // EEPROM ring buffer head
#define ADDR_RCVR   0x70F4      // EEPROM recovery values

/* EEPROM adress space
 * 0x7000 to 0x70FF     
 * 256B -- 256 x 1B  
 */

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"

void update_clock(void);
void acquire_sensor_lum(void);
unsigned char tsttc (void);
void push_ring5(unsigned char data);

/*Interrupt Handlers*/
void h_clock(void) {
    IO_RA7_Toggle(); // Clock Activity (UI)
    update_clock();
}

volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg;
volatile bit configuration_mode;
volatile unsigned char nreg_pt;

void main(void)
{
    clkh = CLKH;
    clkm = CLKM;
    seg = 0;
    configuration_mode = 0;
    nreg_pt = 0;
    
    bit running = 1;
    unsigned char temp, lum;
    unsigned char last = 0;
    
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
        if(PMON) {
            if(!last) last = seg;
            else if((seg - last >= PMON) || (seg - last <= -PMON)) {

                /* Temperature Sensor Aquisition */
                NOP();
                temp = tsttc();     		
                NOP();

                /* Light Sensor Aquisition */
                acquire_sensor_lum();

                /* Write If Changed */
                if(temp != DATAEE_ReadByte(ADDR_RING + 3) || lum != DATAEE_ReadByte(ADDR_RING + 4))
                    push_ring5(clkh, clkm, seg, temp, lum);
                
                /* Write Recovery Parameters */
                
                last = 0;
            }
        }
        
        if(configuration_mode) {
            /*User UI -- Config Mode*/
        }
    }
}

void push_ring5(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned char byte4)
{
    if(nreg_pt >= NREG) nreg_pt = 0;
    else nreg_pt ++;
    DATAEE_WriteByte(ADDR_RING + nreg_pt + 0, byte0);
    DATAEE_WriteByte(ADDR_RING + nreg_pt + 1, byte1);
    DATAEE_WriteByte(ADDR_RING + nreg_pt + 2, byte2);
    DATAEE_WriteByte(ADDR_RING + nreg_pt + 3, byte3);
    DATAEE_WriteByte(ADDR_RING + nreg_pt + 4, byte4);
}

void update_clock(void) {
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