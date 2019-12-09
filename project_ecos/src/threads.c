#include "threads.h"

char commandbus[SIZE_CB];
cyg_mutex_t global;
int nread = 0;
int cb_index = 0;

void thread_create(thread* ti, int flag_defaults)
{
    ti->data = (cyg_addrword_t) 0;
    ti->ssize = DEFAULT_STACKSZ;

    ti->sp = (void *) malloc(ti->ssize*sizeof(char));
    if(ti->sp == NULL) return;
    cyg_thread_create(ti->pri, ti->f, ti->data, ti->name, ti->sp, ti->ssize, &(ti->h), &(ti->t));
    cyg_mbox_create(&(ti->mbox_h), &(ti->mbox));
}

void AskRead(cyg_sem_t* s)
{
    cyg_mutex_lock(&global);
    nread++;
    if(nread == 1) cyg_semaphore_wait(s);
    cyg_mutex_unlock(&global);
}

void FreeRead(cyg_sem_t* s)
{
    cyg_mutex_lock(&global);
    nread--;
    if(nread == 0) cyg_semaphore_post(s);
    cyg_mutex_unlock(&global);
}

void AskWrite(cyg_sem_t* s)
{
    cyg_semaphore_wait(s);
}

void FreeWrite(cyg_sem_t* s)
{
    cyg_semaphore_post(s);
}

char* writeCommand(char name, short int argc)
{
    short int aux;

    aux = cb_index % SIZE_CB;
    commandbus[aux] = name;
    commandbus[(cb_index + 1) % SIZE_CB] = (char) argc;
    cb_index += argc + 2;

    return (commandbus + aux);
}

char getName(char *pt)
{
    return *pt;
}

short int getArgc(char* pt)
{
    return (short int) commandbus[(pt - commandbus + 1) % SIZE_CB];
}

//arg = 1 -> retorna o PRIMEIRO arg
short int getArg(char* pt, int arg)
{
    if((arg < 1) || (arg > getArgc(pt))) return 0;
    return (short int) commandbus[(pt - commandbus + 1 + arg) % SIZE_CB];
}

void setArg(char* pt, int arg, int val)
{
    if((arg < 1) || (arg > getArgc(pt))) return;
    commandbus[(pt - commandbus + 1 + arg) % SIZE_CB] = val;
}
