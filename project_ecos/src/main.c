#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "proc.h" // proc tasks

#define LM_REGISTERS 128
#define LM_SIZE (5*LM_REGISTERS)
#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

/*
THREAD COMMUNICATION PROTOCOL
to do ACK send the same command back with no arguments
*/

#define TX_TRANSFERENCE t // used by (proc, user, rx) to transfere registers to local memory
#define USER_STATISTICS s // used by (proc, user) to send statistics to user

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
char* getMem    (unsigned int from_i, unsigned int to_j);

/*
    CONFIGS
*/

// threads
short int periodic_transference_EN = 1;
int period_transference = 5;

// pic
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
cyg_mutex_t rs_stdin;
cyg_sem_t   rs_localmem;
cyg_sem_t   rs_irw;                     // iread, iwrite

// flags
char ring_filled = 0;

/*
    THREADS
*/

thread_info proc, user, rx, tx;

/*
    USART
*/

cyg_io_handle_t usart_h;
char* usart_n = "/dev/sr0"; // name
cyg_serial_info_t serial_i; // struct with configs for usart

void cyg_user_start(void)
{
    cyg_semaphore_init(&rs_localmem, 1);
    cyg_semaphore_init(&rs_irw, 1);
    cyg_mutex_init(&rs_stdin);
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
    user.f = user_F;
    rx.f = rx_F;
    tx.f = tx_F;

    thread_create(&proc, 0);
    thread_create(&user, 0);
    thread_create(&rx, 0);
    thread_create(&tx, 0);
}

void proc_F(cyg_addrword_t data)
{
    int now, last = 0;
    char *cmd_out, *cmd_in;
    unsigned short int argc = 0;

    int max, min, mean;
    char range[6] = {00,00,00,05,20,00};

    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<pc>\n");
        cyg_mutex_unlock(&rs_stdin);

        now = cyg_current_time();

        if(periodic_transference_EN) {
            if(DELTA(now, last) > period_transference) {
                // transfere, ask to transfere
                cmd_out = writeCommand('t', 0);
                cyg_mbox_tryput(tx.mbox_h, (void*) cmd_out);
                last = now;
            }
        }

        cmd_in = (char*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                // transfere, replied
                case 't':
                        checkThresholds(popMem, alat, alal, &rs_stdin);

                        /* (apagar mais tarde) exemplo,
                           se quisesse ler argumentos
                        */
                        cyg_mutex_lock(&rs_stdin);
                        printf("EXAMPLE: %d %d %d\n", getArg(cmd_in, 1), getArg(cmd_in, 2), getArg(cmd_in, 3));
                        cyg_mutex_unlock(&rs_stdin);
                        break;
                // stats, asked for statistics
                case 's':
                        calcStatistics(getMem, LM_SIZE, &max, &min, &mean, range);


                        cyg_mutex_lock(&rs_stdin);
                        printf("Stats\n");
                        cyg_mutex_unlock(&rs_stdin);
                        break;
            }
        }

        __DELAY(2);
    }
}

/*
    THREADS f(x)
*/
void user_F(cyg_addrword_t data)
{
    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<us>\n");
        cyg_mutex_unlock(&rs_stdin);

        __DELAY(0);
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
        cyg_mutex_lock(&rs_stdin);
        printf("<rx>\n");
        cyg_mutex_unlock(&rs_stdin);

        /*
        cyg_mutex_unlock(&rs_stdin);
        cyg_io_write(usart_h, buf, &len);
        __DELAY();
        cyg_io_read(usart_h, buf, &len);
        cyg_mutex_lock(&rs_stdin);
        printf("%s\n", _buf);
        cyg_mutex_unlock(&rs_stdin);
        */

        __DELAY(0);
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

    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<tx>\n");
        cyg_mutex_unlock(&rs_stdin);

        cmd_in = (char*) cyg_mbox_tryget(tx.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                // transference, asked to transfere
                case 't':
                    pushMem(buffer, 30);
                    cmd_out = writeCommand('t', 3);

                    /* (apagar mais tarde) exemplo,
                       se quisesse escrever argumentos
                       (afinal não foi preciso o índice, fiz get() e set())
                    */
                    setArg(cmd_out, 1, 12);
                    setArg(cmd_out, 2, 00);
                    setArg(cmd_out, 3, 59);

                    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
                    break;
            }
        }

        __DELAY(0);
    }
}


/*
    LOCAL MEMORY f(x)
    ================
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
char* getMem(unsigned int from_i, unsigned int to_j)
{
    int k = 0, i, j, s;
    char *buffer;

    s = ring_filled ? LM_SIZE : iwrite;
    i = from_i % s;
    j = to_j >= s ? (s - 1) : to_j; // j "saturates" if ring buffer "end" was reached

    buffer = (char*) malloc((j - i)*sizeof(char));
    if(buffer == NULL)
        return NULL;

    AskRead(&rs_localmem);
    for(; i < j; i++, k++)
        buffer[k] = localmem[i];
    FreeRead(&rs_localmem);
    return buffer;
}
