#include "threads.h"

cyg_mutex_t global;
int nread = 0;
unsigned int commandbus[FIXED_MBBUFFER];
int cb_index = 0;

void thread_create(thread_info* ti, int flag_defaults)
{
    if(!flag_defaults) {
        ti->pri = (cyg_addrword_t) DEFAULT_PRI;
        ti->data = (cyg_addrword_t) 0;
        ti->ssize = DEFAULT_STACKSZ;
    }

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

/*
    -- EXAMPLE, how to use commandbus --

    // write
    commandbus = writeCommand('c', 5);
    commandbus[1] = 1;
    commandbus[2] = 2;
    commandbus[3] = 3;
    commandbus[4] = 4;
    commandbus[5] = 5;

    // read
    printf(">> %c\n", getName(commandbus));
    for(j = 1; j <= getArgc(commandbus); j++)
    {
        printf("> %d\n", (int) commandbus[j]);
    }
*/

unsigned int* writeCommand(unsigned char name, unsigned short int argc)
{
    commandbus[cb_index % FIXED_MBBUFFER] = ((unsigned int) name << 24) + (unsigned int) argc;
    cb_index += argc + 1;
    return commandbus;
}

char getName(unsigned int* pt)
{
    return (char) (pt[0] >> 24);
}

unsigned short int getArgc(unsigned int* pt)
{
    return (unsigned short int) (pt[0] & 0x00FF);
}
