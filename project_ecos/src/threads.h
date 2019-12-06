#ifndef THREADS_H
#define THREADS_H
#include <cyg/kernel/kapi.h>
#include <stdlib.h>

// to use other vals then defaults, flag_defaults = 1
#define DEFAULT_PRI 10
#define DEFAULT_STACKSZ 4096

// mailbox pointers point to an element of an array with this size (commandbus size)
#define SIZE_CB  64

#define DELAY 100

// macros
#define __DELAY(X) (cyg_thread_delay(DELAY + (rand() % (DELAY << (X)))))

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
} thread_info;

void thread_create(thread_info* ti, int flag_defaults);
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
