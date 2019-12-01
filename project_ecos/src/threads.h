#ifndef THREADS_H
#define THREADS_H
#include <cyg/kernel/kapi.h>
#include <stdlib.h>

// to use other vals then defaults, flag_defaults = 1
#define DEFAULT_PRI 10
#define DEFAULT_STACKSZ 4096

// mailbox pointers point to an element of an array with this size
#define FIXED_MBBUFFER  64

#define DELAY 40

// macros
#define __DELAY() cyg_thread_delay(DELAY + (rand() % (DELAY << 2)))

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
unsigned int* writeCommand(unsigned char name, unsigned short int argc);
char getName(unsigned int* pt);
unsigned short int getArgc(unsigned int* pt);

#endif
