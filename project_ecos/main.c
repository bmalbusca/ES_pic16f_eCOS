#include <cyg/kernel/kapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "threads.h"

#define STACKSZ 4096

void dummy(cyg_addrword_t data); // to debug
void proc_F(cyg_addrword_t data);
void user_F(cyg_addrword_t data);

// resources
cyg_mutex_t rs_stdin;
cyg_sem_t   rs_acc;
int         acc;
cyg_sem_t   rs_localmem;
int         localmem[128];

// thread
cyg_thread      proc_T;
cyg_handle_t    proc_H;
char            proc_S[STACKSZ];
void*           proc_SP = (void*) proc_S;
cyg_addrword_t  proc_D = (cyg_addrword_t) NULL;

cyg_thread      user_T;
cyg_handle_t    user_H;
char            user_S[STACKSZ];
void*           user_SP = (void*) user_S;
cyg_addrword_t  user_D = (cyg_addrword_t) NULL;

void cyg_user_start(void)
{
    int pri;
    cyg_semaphore_init(&rs_acc, 1);
    cyg_semaphore_init(&rs_localmem, 1);
    cyg_mutex_init(&rs_stdin);
    pri = cyg_thread_get_priority(cyg_thread_self());
    cyg_thread_create(pri + 1, dummy, proc_D, "Processing", proc_SP, STACKSZ, &proc_H, &proc_T);
    cyg_thread_create(pri + 2, dummy, user_D, "User", user_SP, STACKSZ, &user_H, &user_T);

    cyg_thread_resume(proc_H);
    cyg_thread_resume(user_H);
}

void dummy(cyg_addrword_t data)
{

}

void proc_F(cyg_addrword_t data)
{
    AskRead(&rs_acc);
    acc ++;
    FreeRead(&rs_acc);
}

void user_F(cyg_addrword_t data)
{
    int aux;
    AskRead(&rs_acc);
    aux = acc;
    FreeRead(&rs_acc);

    cyg_mutex_lock(&rs_stdin);
    printf("%d\n", aux);
    cyg_mutex_unlock(&rs_stdin);
}
