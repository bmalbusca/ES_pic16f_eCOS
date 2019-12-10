#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "threads.h"
#include "mem.h"

#include "proc.h" // proc tasks
#include "monitor.h" // user interface
#include "com_pic.h"

#define DEBUG_THREADS 0
#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define ABS(X)          ((X) < 0 ? -(X) : (X))


// f(x)
void usart_init (void);
void thread_init(void);
void proc_F     (cyg_addrword_t data);
void user_F     (cyg_addrword_t data);
void rx_F       (cyg_addrword_t data);
void tx_F       (cyg_addrword_t data);

/*
    PIC CONFIGS
*/

//int nreg = 30;
//int pmon = 5;
//int tala = 3;
int alat = 50;
int alal = 2;
//unsigned short int alaf = 0;
//int clkh = 0;
//int clkm = 0;

/*
    USART
*/

extern cyg_io_handle_t serial_h;
char* usart_n = "/dev/ser0"; // name
cyg_serial_info_t serial_i; // struct with configs for usart

extern Cyg_ErrNo err;

/*
*
*   ECOS runs this on START (user side inits)
*
*/

extern int commands_in_argc[TRGI];

void cyg_user_start(void){
    commands_in_argc[RCLK] = 3;
    commands_in_argc[SCLK] = 0;
    commands_in_argc[RTL] = 2;
    commands_in_argc[RPAR] = 3;
    commands_in_argc[MMP] = 0;
    commands_in_argc[MTA] = 3;
    commands_in_argc[RALA] = 3;
    commands_in_argc[DATL] = 0;
    commands_in_argc[AALA] = 0;
    commands_in_argc[IREG] = 4;
    commands_in_argc[TRGC] = -1;
    commands_in_argc[TRGI] = -1;

    usart_init();
    mem_init();
    thread_init();
}

// On configuration... (not working)
void usart_init() {
    int err;//, len;

    serial_i.baud =  CYGNUM_SERIAL_BAUD_9600;
    serial_i.stop = CYGNUM_SERIAL_STOP_1;
    serial_i.parity = CYGNUM_SERIAL_PARITY_NONE;
    serial_i.word_length = CYGNUM_SERIAL_WORD_LENGTH_8;
    serial_i.flags = CYG_SERIAL_FLAGS_RTSCTS;

    err = cyg_io_lookup(usart_n, &serial_h);
    if(err == ENOERR) {
        printf("[IO:\"%s\"] Detected\n", usart_n);
        //cyg_io_set_config(serial_h, key, (const void*) , &len);
    }
    else if(err == -ENOENT)
        printf("[IO:\"%s\"] No such entity\n", usart_n);
    else
        printf("[IO:\"%s\"] Some error with code <%d> (lookup \'CYGONCE_ERROR_CODES_H\')\n", usart_n, err);
}

void thread_init() {

    strcpy(proc.name, "Processe");
    strcpy(user.name, "User");
    strcpy(rx.name,   "Read");
    strcpy(tx.name,   "Transfere");
    proc.f = proc_F;
    user.f = monitor; // this function is in "monitor.c"
    rx.f = rx_F;
    tx.f = tx_F;
    proc.pri = (cyg_addrword_t) DEFAULT_PRI;
    user.pri = (cyg_addrword_t) LOW_PRI;
    rx.pri = (cyg_addrword_t) DEFAULT_PRI;
    tx.pri = (cyg_addrword_t) DEFAULT_PRI;

    thread_create(&proc);
    thread_create(&user);
    thread_create(&rx);
    thread_create(&tx);

    cyg_thread_resume(proc.h);
    cyg_thread_resume(user.h);
    cyg_thread_resume(rx.h);
    cyg_thread_resume(tx.h);
}

/*
    ************
    THREAD calls
    ============
    user thread has its function on "monitor.c"
*/

void proc_F(cyg_addrword_t data)
{
    int now, last = 0, k;
    char *cmd_out, *cmd_in;
    char returns;

    stats temp, lum;
    char range[6];

    int period_transference = 5;
    int tht = alat, thl = alal; // Temperature and Luminosity thresholds reserved to proc

    while(1) {
        if (DEBUG_THREADS) {
            sprintf(stdout_buff, "<PC>\n");
            queueStdout(stdout_buff);
        }

        now = cyg_current_time();

        if(period_transference) {
            if(DELTA(now, last) > period_transference) {
                cmd_out = writeCommand(RX_TRANSFERENCE, 0);
                cyg_mbox_tryput(rx.mbox_h, (void*) cmd_out);
                last = now;
            }
        }

        cmd_in = (char*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                case RX_TRANSFERENCE:
                    checkThresholds(alat, alal);
                    break;
                case USER_STATISTICS:
                    for(k = 0; k < 6; k ++) {
                        range[k] = getArg(cmd_in, k + 1);
                    }
                    returns = calcStatistics(&temp, &lum, range);
                    if(!returns) {
                        cmd_out = writeCommand(ERROR_MEMEMPTY, 0);
                    } else {
                        cmd_out = writeCommand(USER_STATISTICS, 6);
                        setArg(cmd_out, 1, temp.min);
                        setArg(cmd_out, 2, temp.max);
                        setArg(cmd_out, 3, temp.mean);
                        setArg(cmd_out, 4, lum.min);
                        setArg(cmd_out, 5, lum.max);
                        setArg(cmd_out, 6, lum.mean);
                    }
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
                case USER_MODIFY_PERIOD_TRANSF:
                    if(getArgc(cmd_in) >= 1) {
                        period_transference = getArg(cmd_in, 1);
                    }
                    break;
                case USER_CHANGE_THRESHOLDS:
                    if(getArgc(cmd_in) >= 2) {
                        tht = getArg(cmd_in, 1);
                        thl = getArg(cmd_in, 2);
                    }
                    break;
                case PROC_CHECK_PERIOD_TRANSF:
                    cmd_out = writeCommand(PROC_CHECK_PERIOD_TRANSF, 1);
                    setArg(cmd_out, 1, period_transference);
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
                case PROC_CHECK_THRESHOLDS:
                    cmd_out = writeCommand(PROC_CHECK_THRESHOLDS, 2);
                    setArg(cmd_out, 1, tht);
                    setArg(cmd_out, 2, thl);
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
            }
        }

        __DELAY();
    }
}
