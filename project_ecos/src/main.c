#include <cyg/kernel/kapi.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "proc.h" // proc tasks

#define LM_REGISTERS 128
#define LM_SIZE (5*LM_REGISTERS)

// macros
#define DELTA(X,Y) X > Y ? X - Y : Y - X

void proc_F (cyg_addrword_t data);
void user_F (cyg_addrword_t data);
void rx_F   (cyg_addrword_t data);
void tx_F   (cyg_addrword_t data);
void push   (char clkh, char clkm, char seg, char Temp, char Lum);

/*
    CONFIGS
*/

// threads
short int periodic_transference_EN = 0;
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
short int    mem_filled = 0;            // 1 if ring buffer was writen fully once
cyg_mutex_t rs_stdin;
cyg_sem_t   rs_localmem;

/*
    THREADS
*/
thread_info proc, user, rx, tx;

void cyg_user_start(void)
{
    strcpy(proc.name, "Processe");
    strcpy(user.name, "User");
    strcpy(rx.name,   "Read");
    strcpy(tx.name,   "Transfere");
    proc.f = proc_F;
    user.f = user_F;
    rx.f = rx_F;
    tx.f = tx_F;

    cyg_semaphore_init(&rs_localmem, 1);
    cyg_mutex_init(&rs_stdin);
    thread_create(&proc, 0);
    thread_create(&user, 0);
    thread_create(&rx, 0);
    thread_create(&tx, 0);

    cyg_thread_resume(proc.h);
    cyg_thread_resume(user.h);
    cyg_thread_resume(rx.h);
    cyg_thread_resume(tx.h);
}

void proc_F(cyg_addrword_t data)
{
    int now, last = 0;
    unsigned int* cmd_in; // cmd_out;
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
                //cmd_out = writeCommand('t', 0); // ask Tx to transfere
                //cyg_mbox_timed_put(tx.mbox_h, (void*) cmd_out, DELAY);
            }
        }

        cmd_in = (unsigned int*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch ((char) getName(cmd_in)) {
                // Tx replied transference complete
                case 't': checkThresholds(localmem, iread, iwrite, alat, alal, &rs_localmem, &rs_stdin);
                          iread = iwrite;
                          break;
                // User asked for register statistics
                case 's': argc = getArgc(cmd_in);
                          if(argc == 6) {
                            memcpy(range, cmd_in + 1, 6);
                            calcStatistics(localmem, iwrite, mem_filled, &max, &min, &mean, range);
                          }
            }
        }

        last = now;
        __DELAY();
    }
}

void user_F(cyg_addrword_t data)
{
    unsigned int* cmd_out; // cmd_in;

    cmd_out = writeCommand('s', 6);
    cmd_out[1] = 0;
    cmd_out[2] = 0;
    cmd_out[3] = 0;
    cmd_out[4] = 23;
    cmd_out[5] = 59;
    cmd_out[6] = 59;
    cyg_mbox_tryput(proc.mbox_h, cmd_out);

    while(1) {
        __DELAY();
    }
}

void rx_F(cyg_addrword_t data)
{
    while(1) {
        __DELAY();
    }
}

void tx_F(cyg_addrword_t data)
{
    unsigned int* cmd_out; // cmd_in;
    char test;

    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<tx>\n");
        cyg_mutex_unlock(&rs_stdin);

        test = (char) (rand() % 255);
        push(test,test,test,test,test);

        cmd_out = writeCommand('t', 0); // reply to ask Tx to transfere
        cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);

        __DELAY();
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
