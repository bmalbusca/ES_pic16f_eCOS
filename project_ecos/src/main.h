#ifndef MAIN_H
#define MAIN_H

#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "proc.h" // proc tasks
#include "monitor.h" // user interface

#define NRBUF 100
#define LM_SIZE (5*NRBUF)
#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define RINGDELTA(X,Y)  ((Y) >= (X) ? (Y) - (X) + 1 : LM_SIZE - (X) + (Y) + 1) // X,Y are indexes
#define ABS(X)          ((X) < 0 ? -(X) : (X))

/*
    RESOURCES
*/

char        localmem[LM_SIZE];          // 1 register (5 bytes): h | m | s | T | L
int         iread, iwrite;              // only proc should change iread, it should be updated after reading (iread = iwrite)
char        ring_filled;
cyg_sem_t   rs_localmem;
cyg_sem_t   rs_rwf;                     // iread, iwrite, ring_filled

void pushMem(char* buffer, unsigned int size);
char* popMem(unsigned int* size);
char getMemP(unsigned int _pos);
char* getMem(unsigned int from_i, unsigned int to_j, unsigned int* size);
char getValidIndexes(unsigned int* from_i, unsigned int* to_j, unsigned int* buffer_size);

#endif
