#ifndef THREADS_H
#define THREADS_H
#include <cyg/kernel/kapi.h>
#include <stdlib.h>

/*
    THREAD COMMUNICATION PROTOCOL
    to do ACK send the same command back with no arguments
    {x} -- means it corresponds to command x from page 2 of Project_part2.pdf
*/

#define RX_TRANSFERENCE             0 // used by (proc, user, rx) to transfere registers to local memory
#define USER_STATISTICS             1 // {pr} used by (proc, user) to send statistics to user
#define USER_MODIFY_PERIOD_TRANSF   2 // {mpt} used by (proc, user) to change a proc variable (period_transference)
#define USER_CHANGE_THRESHOLDS      3 // {dttl} used by (proc, user) to change thresholds used in processing registers
#define PROC_CHECK_PERIOD_TRANSF    4 // {cpt} used by (proc, user) to send to user the period of transference
#define PROC_CHECK_THRESHOLDS       5 // {cttl} used by (proc, user) to send to user the thresholds

#define RC


#define LOW_PRI 11
#define DEFAULT_PRI 10
#define HIGH_PRI 9
#define MAX_PRI 8

#define DEFAULT_STACKSZ 4096

// mailbox pointers point to an element of an array with this size (commandbus size)
#define SIZE_CB  64

#define DELAY 100
#define SHORT_DELAY 20

// macros
#define __DELAY() (cyg_thread_delay(DELAY + (rand() % (DELAY >> 2))))
#define __SHORT_DELAY() (cyg_thread_delay(SHORT_DELAY))

typedef struct {
    cyg_addrword_t  pri;
    void(*f)(cyg_addrword_t data);
    cyg_thread      t;
    cyg_handle_t    h;
    void*           sp;
    int             ssize;
    cyg_addrword_t  data;
    cyg_mbox        mbox;
    cyg_handle_t    mbox_h;
    char            name[16];
} thread;

thread proc, user, rx, tx;
cyg_mutex_t rs_stdin;
char stdin_buff[1024], *stdin_buff_pt;

void thread_create(thread* ti, int flag_defaults);
void AskRead(cyg_sem_t* s);
void FreeRead(cyg_sem_t* s);
void AskWrite(cyg_sem_t* s);
void FreeWrite(cyg_sem_t* s);
char* writeCommand(char name, short int argc);
char getName(char *pt);
short int getArgc(char* pt);
short int getArg(char* pt, int arg);
void setArg(char* pt, int arg, int val);

#endif
