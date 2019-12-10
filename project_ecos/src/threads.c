#include "threads.h"

#define STDOUT_BUFFER_GLOBAL 1024

char commandbus[SIZE_CB];
int cb_index = 0;
char stdout_buff_global[STDOUT_BUFFER_GLOBAL], *stdout_buff_pt;

void stdout_init()
{
    stdout_buff_pt = stdout_buff;
    cyg_mutex_init(&rs_stdout);
}

void dumpStdout()
{
    cyg_mutex_lock(&rs_stdout);
    // dump strings waiting to be written
    if(*stdout_buff_global != '\0') printf("\n.\n%s", stdout_buff_global);
    *stdout_buff_global = '\0';
    stdout_buff_pt = stdout_buff_global;
    cyg_mutex_unlock(&rs_stdout);
}

void queueStdout(char* buf)
{
    if(strlen(buf) > (STDOUT_BUFFER_GLOBAL - strlen(stdout_buff_pt)))
        return;
    stdout_buff_pt = strcat(stdout_buff_pt, buf);
}

void thread_create(thread* ti)
{
    ti->data = (cyg_addrword_t) 0;
    ti->ssize = DEFAULT_STACKSZ;

    ti->sp = (void *) malloc(ti->ssize*sizeof(char));
    if(ti->sp == NULL) return;
    cyg_thread_create(ti->pri, ti->f, ti->data, ti->name, ti->sp, ti->ssize, &(ti->h), &(ti->t));
    cyg_mbox_create(&(ti->mbox_h), &(ti->mbox));
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

short int getArg(char* pt, int arg)
{
    if((arg < 1) || (arg > getArgc(pt))) return 0;
    return (short int) commandbus[(pt - commandbus + 1 + arg) % SIZE_CB];
}

//arg = 1 -> primeiro arg
void setArg(char* pt, int arg, int val)
{
    if((arg < 1) || (arg > getArgc(pt))) return;
    commandbus[(pt - commandbus + 1 + arg) % SIZE_CB] = val;
    
}
