#ifndef MAIN_H
#define MAIN_H

void pushMem(char* buffer, unsigned int size);
char* popMem(unsigned int* size);
char getMemP(unsigned int _pos);
char* getMem(unsigned int* from_i, unsigned int* to_j, unsigned int* buffer_size);

#endif
