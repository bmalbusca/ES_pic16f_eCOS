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
    COMMANDBUS -- EXAMPLE

    unsigned int* cmd_in;
    unsigned int* cmd_out;

    // Asking for Stats to Proc
    // writes to ring buffer (bytes separated by |):
    cmd_out = writeCommand('s', 6);
    //    s|0|0|6->
    cmd_out[1] = 0;
    cmd_out[2] = 0;
    cmd_out[3] = 0;
    cmd_out[4] = 23;
    cmd_out[5] = 59;
    cmd_out[6] = 59;
    // ->|0|0|0|0->
    // ->|0|0|0|0->
    // ->|0|0|0|0->
    // ->|0|0|1|7->
    // ->|0|0|3|B->
    // ->|0|0|3|B
    cyg_mbox_tryput(proc.mbox_h, cmd_out);

    while(1) {
        cyg_mutex_lock(&rs_stdin);
        printf("<us>\n");
        cyg_mutex_unlock(&rs_stdin);

        __DELAY();
    }
*/

unsigned int* writeCommand(unsigned char name, unsigned short int argc)
{
    unsigned int* aux;
    commandbus[cb_index % FIXED_MBBUFFER] = ((unsigned int) name << 24) + (unsigned int) argc;
    aux = commandbus + (cb_index % FIXED_MBBUFFER);
    cb_index += argc + 1;

    return aux;
}

char getName(unsigned int* pt)
{
    return (char) (pt[0] >> 24);
}

unsigned short int getArgc(unsigned int* pt)
{
    return (unsigned short int) (pt[0] & 0x00FF);
}
