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

// macros
#define DELTA(X,Y) X > Y ? X - Y : Y - X

void proc_F     (cyg_addrword_t data);
void user_F     (cyg_addrword_t data);
void rx_F       (cyg_addrword_t data);
void tx_F       (cyg_addrword_t data);
void push       (char clkh, char clkm, char seg, char Temp, char Lum);
void usart_init (void);
void thread_init(void);

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
short int   mem_filled = 0;             // 1 if ring buffer was writen fully once
cyg_mutex_t rs_stdin;
cyg_sem_t   rs_localmem;

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
    char* cmd_in;
    char* cmd_out;
    unsigned short int argc = 0;

    int max, min, mean;
    unsigned int range[6];

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
                case 't': checkThresholds(localmem, iread, iwrite, alat, alal, &rs_localmem, &rs_stdin);

                          /* (apagar mais tarde) exemplo,
                             se quisesse ler argumentos
                          */
                          cyg_mutex_lock(&rs_stdin);
                          printf("EXAMPLE: %d %d %d\n", getArg(cmd_in, 1), getArg(cmd_in, 2), getArg(cmd_in, 3));
                          cyg_mutex_unlock(&rs_stdin);

                          iread = iwrite;
                          break;
                // stats, asked for statistics
                case 's': argc = getArgc(cmd_in);
                          cyg_mutex_lock(&rs_stdin);
                          printf("Stats\n");
                          cyg_mutex_unlock(&rs_stdin);
                          if(argc == 6) {
                            memcpy(range, cmd_in + 1, 6);
                            calcStatistics(localmem, iwrite, mem_filled, &max, &min, &mean, range);
                          }
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
    char* cmd_out;
    char* cmd_in;

    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<tx>\n");
        cyg_mutex_unlock(&rs_stdin);

        cmd_in = (char*) cyg_mbox_tryget(tx.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                // transference, asked to transfere
                case 't': push((char) 23, (char) 59, (char) 59, (char) 60, (char) 1);
                cmd_out = writeCommand('t', 3);

                /* (apagar mais tarde) exemplo,
                   se quisesse escrever argumentos
                   (afinal não foi preciso o índice, fiz get() e set())
                */
                setArg(cmd_out, 1, 12);
                setArg(cmd_out, 2, 00);
                setArg(cmd_out, 3, 59);

                cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
            }
        }

        __DELAY(0);
    }
}


/*
    LOCAL MEMORY f(x)
*/
void push(char clkh, char clkm, char seg, char Temp, char Lum)
{
    AskWrite(&rs_localmem);
    localmem[iwrite % LM_SIZE] = clkh;
    localmem[(iwrite + 1) % LM_SIZE] = clkm;
    localmem[(iwrite + 2) % LM_SIZE] = seg;
    localmem[(iwrite + 3) % LM_SIZE] = Temp;
    localmem[(iwrite + 4) % LM_SIZE] = Lum;
    FreeWrite(&rs_localmem);
    iwrite += 5;
    if(!iwrite) mem_filled = 1;
}
