#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "proc.h" // proc tasks
#include "monitor.h" // user interface

#define LM_REGISTERS 128
#define LM_SIZE (5*LM_REGISTERS)
#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define RINGDELTA(X,Y)  ((Y) >= (X) ? (Y) - (X) : LM_SIZE - (X) + (Y)) // X,Y are indexes

void usart_init (void);
void thread_init(void);
void proc_F     (cyg_addrword_t data);
void user_F     (cyg_addrword_t data);
void rx_F       (cyg_addrword_t data);
void tx_F       (cyg_addrword_t data);
void pushMem    (char* buffer, unsigned int size);
char* popMem    (unsigned int* size);
char getMemP    (unsigned int pos);
char* getMem    (unsigned int* from_i, unsigned int* to_j, unsigned int* buffer_size);

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
    RESOURCES
*/

char        localmem[LM_SIZE];          // 1 register (5 bytes): h | m | s | T | L
int         iread = 0, iwrite = 0;      // only proc should change iread, it should be updated after reading (iread = iwrite)
cyg_sem_t   rs_localmem;
cyg_sem_t   rs_irw;                     // iread, iwrite

// flags
char ring_filled = 0;                   // only change once -- needs thread write protection?

/*
    USART
*/

cyg_io_handle_t usart_h;
char* usart_n = "/dev/sr0"; // name
cyg_serial_info_t serial_i; // struct with configs for usart

/*
*
*   ECOS runs this on START (user side inits)
*
*/

void cyg_user_start(void)
{
    cyg_semaphore_init(&rs_localmem, 1);
    cyg_semaphore_init(&rs_irw, 1);
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
void usart_init() {
    int err, len;

    serial_i.baud =  CYGNUM_SERIAL_BAUD_9600;
    serial_i.stop = CYGNUM_SERIAL_STOP_1;
    serial_i.parity = CYGNUM_SERIAL_PARITY_NONE;
    serial_i.word_length = CYGNUM_SERIAL_WORD_LENGTH_8;
    serial_i.flags = CYG_SERIAL_FLAGS_RTSCTS;

    err = cyg_io_lookup(usart_n, &usart_h);
    if(err == ENOERR) {
        printf("[IO:\"%s\"] Detected\n", usart_n);
        //cyg_io_set_config(usart_h, key, (const void*) , &len);
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
    user.pri = (cyg_addrword_t) LOW_PRI;
    rx.pri = (cyg_addrword_t) DEFAULT_PRI;
    tx.pri = (cyg_addrword_t) DEFAULT_PRI;

    thread_create(&proc, 0);
    thread_create(&user, 0);
    thread_create(&rx, 0);
    thread_create(&tx, 0);
}

/*
    ************
    THREAD calls
    ============
    user thread has its function on "monitor.c"
*/

void proc_F(cyg_addrword_t data)
{
    int now, last = 0;
    char *cmd_out, *cmd_in;
    char returns;
    unsigned short int argc = 0;

    stats temp, lum;
    char range[6] = {00,00,00,04,20,00};

    int period_transference = 5;
    int tht = alat, thl = alal; // Temperature and Luminosity thresholds reserved to proc

    while(1) {
        now = cyg_current_time();

        if(period_transference) {
            if(DELTA(now, last) > period_transference) {
                // transfere, ask to transfere
                cmd_out = writeCommand(TX_TRANSFERENCE, 0);
                cyg_mbox_tryput(tx.mbox_h, (void*) cmd_out);
                last = now;
            }
        }

        cmd_in = (char*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                case TX_TRANSFERENCE:
                    checkThresholds(popMem, alat, alal);
                    break;
                case USER_STATISTICS:
                    returns = calcStatistics(getMem, LM_SIZE, &temp, &lum, range);
                    if(!returns) break;
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

void rx_F(cyg_addrword_t data)
{
    /*
    char _buf[32];
    int len = 32;
    void* buf;
    buf = (void*) _buf;
    */

    while(1) {
        /*
        cyg_mutex_unlock(&rs_stdin);
        cyg_io_write(usart_h, buf, &len);
        __DELAY();
        cyg_io_read(usart_h, buf, &len);
        cyg_mutex_lock(&rs_stdin);
        printf("%s\n", _buf);
        cyg_mutex_unlock(&rs_stdin);
        */

        __DELAY();
    }
}

// function for transference thread
void tx_F(cyg_addrword_t data)
{
    char *cmd_out, *cmd_in;
    char buffer[30] = {(char) 23, (char) 59, (char) 59, (char) 20, (char)  1,
                       (char)  0, (char) 30, (char) 27, (char) 30, (char)  1,
                       (char)  0, (char) 32, (char) 10, (char) 20, (char)  2,
                       (char)  1, (char) 21, (char)  9, (char) 60, (char)  0,
                       (char)  5, (char) 19, (char) 59, (char) 50, (char)  0,
                       (char) 13, (char) 59, (char) 59, (char) 10, (char)  1}; // for testing

    pushMem(buffer, 30);

    while(1) {
        cmd_in = (char*) cyg_mbox_tryget(tx.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                case TX_TRANSFERENCE:
                    cmd_out = writeCommand(TX_TRANSFERENCE, 3);
                    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
                    break;
            }
        }

        __DELAY();
    }
}


/*
    ******************
    LOCAL MEMORY calls
    ==================
*/

/*
    This function puts data in local memory in push like way,
    because caller doesn't control where to write.
*/
void pushMem(char* buffer, unsigned int size)
{
    int k = 0;
    if(size % 5)
        return;
    AskWrite(&rs_localmem);
    AskWrite(&rs_irw);
    for(; k < size; iwrite = (iwrite + 1) % LM_SIZE, k++)
    {
        localmem[iwrite] = buffer[k];
    }
    FreeWrite(&rs_irw);
    FreeWrite(&rs_localmem);
    if(!iwrite)
        ring_filled = 1;
}

/*
    This function outputs data in local memory in a pop like way,
    because caller doesn't control where to read and
    once data has taken it can't be read again with this function.
*/
char* popMem(unsigned int* size)
{
    char* buffer;
    int k = 0;

    AskWrite(&rs_irw);
    *size = RINGDELTA(iread, iwrite);
    buffer = (char*) malloc((*size)*sizeof(char));

    AskRead(&rs_localmem);
    for(; k < *size; iread = (iread + 1) % LM_SIZE, k++)
    {
        buffer[k] = localmem[iread];
    }
    FreeRead(&rs_localmem);
    FreeWrite(&rs_irw);
    return buffer;
}

/*
    This functions exists to allow reading positions
*/
char getMemP(unsigned int _pos)
{
    char aux_c;
    int pos = ring_filled ? (_pos % LM_SIZE) : (_pos % iwrite);

    AskRead(&rs_localmem);
    aux_c = localmem[pos];
    FreeRead(&rs_localmem);
    return aux_c;
}

/*
    This functions exists to allow reading ranges
*/
char* getMem(unsigned int* from_i, unsigned int* to_j, unsigned int* buffer_size)
{
    if(*from_i == *to_j)
        return NULL; // use getMemP()
    unsigned int k = 0, i, j, s;
    char *buffer;
    s = ring_filled ? LM_SIZE : iwrite;
    if(!s)
        return NULL;
    i = *from_i % s;
    j = *to_j >= s ? (s - 1) : *to_j; // j "saturates" if ring buffer "end" was reached

    *buffer_size = j - i + 1;
    buffer = (char*) malloc((*buffer_size)*sizeof(char));
    if(buffer == NULL)
        return NULL;

    *from_i = i;
    *to_j = j;

    AskRead(&rs_localmem);
    for(; i < j; i++, k++)
        buffer[k] = localmem[i];
    FreeRead(&rs_localmem);
    return buffer;
}
