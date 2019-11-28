#include "threads.h"

cyg_mutex_t global;
int nread = 0;

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
