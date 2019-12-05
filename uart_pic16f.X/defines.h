#ifndef DEFINES_H
#define DEFINES_H


#define SOM   0xFD	/* start of message */
#define EOM   0xFE	/* end of message */
#define RCLK  0xC0  /* read clock */
#define SCLK  0XC1  /* set clock */
#define RTL   0XC2  /* read temperature and luminosity */
#define RPAR  0XC3  /* read parameters */
#define MMP   0XC4  /* modify monitoring period */
#define MTA   0XC5  /* modify time alarm */
#define RALA  0XC6  /* read alarms (temperature, luminosity, active/inactive) */
#define DATL  0XC7  /* define alarm temperature and luminosity */
#define AALA  0XC8  /* activate/deactivate alarms */
#define IREG  0XC9  /* information about registers (NREG, nr, iread, iwrite)*/
#define TRGC  0XCA  /* transfer registers (curr. position)*/
#define TRGI  0XCB  /* transfer registers (index) */
#define NMFL  0XCC  /* notification memory (half) full */
#define CMD_OK    0     /* command successful */
#define CMD_ERROR 0xFF  /* error in command */



#endif 
