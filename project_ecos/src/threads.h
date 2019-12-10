#ifndef THREADS_H
#define THREADS_H
#include <cyg/kernel/kapi.h>
#include <stdlib.h>

#define LOW_PRI 11
#define DEFAULT_PRI 10
#define HIGH_PRI 9
#define MAX_PRI 8

#define DEFAULT_STACKSZ 4096

// mailbox pointers point to an element of an array with this size (commandbus size)
#define SIZE_CB  64

#define DELAY 50
#define SHORT_DELAY 10

// macros
#define __DELAY() (cyg_thread_delay(DELAY + (rand() % SHORT_DELAY)))
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

cyg_mutex_t rs_stdout;
char stdout_buff[1024], *stdout_buff_pt;

void thread_create(thread* ti);

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
