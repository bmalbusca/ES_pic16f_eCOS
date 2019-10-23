#define NREG    30
#define PMON    5
#define TALA    3
#define ALAT    25
#define ALAL    2
#define ALAF    0
#define CLKH    0
#define CLKM    0

#define EEAddr  0x7000        // EEPROM starting address


#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"

void update_clock(void);
void moveto_EEPROM(void);
void read_EEPROM(void);

/*Handlers*/
void h_clock(void){
    IO_RA7_Toggle(); // Clock Activity (UI)
    update_clock();
    moveto_EEPROM();
}
/**/

/*Main loop*/
unsigned char tsttc (void);
void acquire_sensor_lum(void);
/**/

volatile unsigned char clkh = CLKH;
volatile unsigned char clkm = CLKM;
volatile unsigned char seg = 0;
bit configuration_mode = 0;

void main(void)
{
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
        if(!last) last = seg;
        else if((seg - last >= PMON) || (seg - last <= -PMON)) {

            /* Temperature Sensor Aquisition */
            NOP();
            temp = tsttc();     		
            NOP();
        
            /* Light Sensor Aquisition */
            acquire_sensor_lum();
            
            /* Write Results */
            if(temp != read_EEPROM() || lum != read_EEPROM()) {
                DATAEE_WriteByte(EEAddr, clkh);
            }
            
            last = 0;
        }
        if(configuration_mode) {
            /*User UI -- COnfig Mode*/
        }
    }
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

void moveto_EEPROM(void) {
    
}

void read_EEPROM(void) {
    
}