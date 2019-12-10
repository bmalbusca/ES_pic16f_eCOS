#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "threads.h"
#include "mem.h"

#include "proc.h" // proc tasks
#include "monitor.h" // user interface


#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define RINGDELTA(X,Y)  ((Y) >= (X) ? (Y) - (X) + 1 : LM_SIZE - (X) + (Y) + 1) // X,Y are indexes
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

void cyg_user_start(void)
{
    iwrite = 0; iread = 0; ring_filled = 0;

    cyg_semaphore_init(&rs_localmem, 1);
    cyg_semaphore_init(&rs_rwf, 1);
    cyg_mutex_init(&rs_stdin);
    stdin_buff_pt = stdin_buff;
    usart_init();
    thread_init();

    cyg_thread_resume(proc.h);
    cyg_thread_resume(user.h);
    cyg_thread_resume(rx.h);
    cyg_thread_resume(tx.h);
}

// On configuration... (not working)
void usart_init(){
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
    proc.pri = (cyg_addrword_t) HIGH_PRI;
    user.pri = (cyg_addrword_t) DEFAULT_PRI;
    rx.pri = (cyg_addrword_t) HIGH_PRI;
    tx.pri = (cyg_addrword_t) HIGH_PRI;

    thread_create(&proc);
    thread_create(&user);
    thread_create(&rx);
    thread_create(&tx);
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
        cyg_mutex_lock(&rs_stdin);
        printf("PROC\n");
        cyg_mutex_unlock(&rs_stdin);

        now = cyg_current_time();

        if(period_transference) {
            if(DELTA(now, last) > period_transference) {
                cmd_out = writeCommand(TRGC, 0);
                cyg_mbox_tryput(rx.mbox_h, (void*) cmd_out);
                last = now;
            }
        }

        cmd_in = (char*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                case TRGC:
                    checkThresholds(popMem, alat, alal);
                    break;
                case USER_STATISTICS:
                    for(k = 0; k < 6; k ++) {
                        range[k] = getArg(cmd_in, k + 1);
                    }
                    returns = calcStatistics(getMem, LM_SIZE, &temp, &lum, range);
                    if(returns) {
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

void rx_F(cyg_addrword_t data){
    char *cmd_out, *cmd_in;

    /*
    char buffer[30] = {(char)  0, (char) 30, (char) 27, (char) 30, (char)  1,
                       (char)  0, (char) 32, (char) 10, (char) 20, (char)  2,
                       (char)  1, (char) 21, (char)  9, (char) 60, (char)  0,
                       (char)  5, (char) 19, (char) 59, (char) 50, (char)  0,
                       (char) 13, (char) 59, (char) 59, (char) 10, (char)  1,
                       (char) 23, (char) 59, (char) 59, (char) 20, (char)  1}; // for testing
    */

    char buffer_rx[10];

    int num_bytes = 7;

    //pushMem(buffer, 30);



    while(1){

        cmd_in = (char*)cyg_io_read(serial_h, (void*)buffer_rx, &num_bytes);        //7 = nº bytes a serem lidos

        /*if(cmd_in != NULL){
            switch (getName(cmd_in)) {
                case TRGC:
                    cmd_out = writeCommand(TRGC, 0);
                    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
                    break;
            }
        }
        */

        __DELAY();

        }*/
        printf("%s\n", buffer_rx);
    }
}

void tx_F(cyg_addrword_t data){

    while(1){
        cyg_mutex_lock(&rs_stdin);
        printf("TX\n");
        cyg_mutex_unlock(&rs_stdin);

        __DELAY();
    }
}
