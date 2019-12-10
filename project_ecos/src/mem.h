#ifndef MEM_H
#define MEM_H

#include <cyg/kernel/kapi.h>
#include <stdlib.h>

#define NRBUF 100
#define REGISTER_SIZE 5
#define MEM_BYTES (REGISTER_SIZE*NRBUF)
#define ACTUAL_SIZE (ring_filled ? MEM_BYTES : iwrite)

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define NORMLZ(X)       ((X) % ACTUAL_SIZE)
#define OVERREAD(X)     ((X) > iread ? iread : (X))
#define RINGDELTA(X,Y)  (NORMLZ(X) <= NORMLZ(Y) ? NORMLZ(Y) - NORMLZ(X) : ACTUAL_SIZE - NORMLZ(X) + NORMLZ(Y))

char        localmem[MEM_BYTES];        // 1 register (5 bytes): h | m | s | T | L
int         iread, iwrite;              // only proc should change iread, it should be updated after reading (iread = iwrite)
char        ring_filled;
cyg_sem_t   rs_localmem;
cyg_sem_t   rs_rwf;                     // read, write, flags (iread, iwrite, ring_filled)

void AskRead(cyg_sem_t* s);
void FreeRead(cyg_sem_t* s);
void AskWrite(cyg_sem_t* s);
void FreeWrite(cyg_sem_t* s);
void mem_init(void);
void pushMemReg(char *buffer, unsigned int size);
void pushMem(char *buffer, unsigned int size);
char* popMem(unsigned int* size);
char getMemP(unsigned int _pos);
char* getMem(unsigned int from_i, unsigned int to_j, unsigned int* size);
void getValidRegIndexes(unsigned int* from_regi, unsigned int* to_regj, unsigned int* size);
void getValidIndexes(unsigned int* from_i, unsigned int* to_j, unsigned int* size);

#endif
