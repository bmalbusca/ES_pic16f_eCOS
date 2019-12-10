#ifndef DEFINES_H
#define DEFINES_H

/*
    PIC COMMUNICATION PROTOCOL
*/

#define SOM         ':'	    /* start of message */
#define EOM         '?'     /* end of message */

#define RCLK        0xC0    /* read clock */
#define SCLK        0xC1    /* set clock */
#define RTL         0xC2    /* read temperature and luminosity */
#define RPAR        0xC3    /* read parameters */
#define MMP         0xC4    /* modify monitoring period */
#define MTA         0xC5    /* modify time alarm */
#define RALA        0xC6    /* read alarms (temperature, luminosity, active/inactive) */
#define DATL        0xC7    /* define alarm temperature and luminosity */
#define AALA        0xC8    /* activate/deactivate alarms */
#define IREG        0xC9    /* information about registers (NREG, nr, iread, iwrite)*/
#define TRGC        0xCA    /* transfer registers (curr. position)*/
#define TRGI        0xCB    /* transfer registers (index) */

#define NMFL        0xCC    /* notification memory (half) full */

//Defines para args

#define CMD_OK      0       /* command successful */
#define CMD_ERROR   0xFF    /* error in command */

/*const char cmd_names[NUM_NAMES]={
    SOM, EOM, RCLK, SCLK, RTL, RPAR, MMP, MTA, RALA, DATL, AALA, IREG, TRGC, TRGI, NMFL, CMD_OK, CMD_ERROR
};*/

/*
    THREAD COMMUNICATION PROTOCOL
    {x} -- means it corresponds to command x from page 2 of Project_part2.pdf
*/

#define RX_TRANSFERENCE             'a' // used by (proc, user, rx) to transfere registers to local memory
#define USER_STATISTICS             'b' // {pr} used by (proc, user) to send statistics to user
#define USER_MODIFY_PERIOD_TRANSF   'c' // {mpt} used by (proc, user) to change a proc variable (period_transference)
#define USER_CHANGE_THRESHOLDS      'd' // {dttl} used by (proc, user) to change thresholds used in processing registers
#define PROC_CHECK_PERIOD_TRANSF    'e' // {cpt} used by (proc, user) to send to user the period of transference
#define PROC_CHECK_THRESHOLDS       'f' // {cttl} used by (proc, user) to send to user the thresholds
#define ERROR_MEMEMPTY              'u' // local memory is empty

#endif
