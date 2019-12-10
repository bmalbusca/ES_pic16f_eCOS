#ifndef MEM_H
#define MEM_H

#define NRBUF 100
#define REGISTER_SIZE 5
#define MEM_BYTES REGISTER_SIZE*NRBUF

char        localmem[MEM_BYTES];        // 1 register (5 bytes): h | m | s | T | L
int         iread, iwrite;              // only proc should change iread, it should be updated after reading (iread = iwrite)
char        ring_filled;
cyg_sem_t   rs_localmem;
cyg_sem_t   rs_rwf;                     // read, write, flags (iread, iwrite, ring_filled)

void pushMem(char* buffer, unsigned int size);
char* popMem(unsigned int* size);
char getMemP(unsigned int _pos);
char* getMem(unsigned int from_i, unsigned int to_j, unsigned int* size);
char getValidIndexes(unsigned int* from_i, unsigned int* to_j, unsigned int* buffer_size);

#endif
