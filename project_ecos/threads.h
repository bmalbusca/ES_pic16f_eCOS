#ifndef THREADS_H
#define THREADS_H

#include <cyg/kernel/kapi.h>

void AskRead(cyg_sem_t* s);
void FreeRead(cyg_sem_t* s);
void AskWrite(cyg_sem_t* s);
void FreeWrite(cyg_sem_t* s);

#endif
