#include "mem.h"

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

/*
    ******************
    LOCAL MEMORY calls
    ==================
*/

void mem_init(void)
{
    iwrite = 0; iread = 0; ring_filled = 0;
    cyg_semaphore_init(&rs_localmem, 1);
    cyg_semaphore_init(&rs_rwf, 1);
}

void pushMemReg(char *buffer, unsigned int size)
{
    if(!size)
        return;
    if(size % 5)
        size = size - (size % 5);
    pushMem(buffer, size);
}

/*
    This function puts data in local memory in push like way,
    because caller doesn't control where to write.
*/
void pushMem(char *buffer, unsigned int size)
{
    int k = 0;
    if(!size)
        return;

    AskWrite(&rs_localmem);
    AskWrite(&rs_rwf);
    for(; k < size; iwrite = (iwrite + 1) % MEM_BYTES, k++)
    {
        localmem[iwrite] = buffer[k];
    }
    if(!iwrite)
        ring_filled = 1;
    FreeWrite(&rs_rwf);
    FreeWrite(&rs_localmem);
}

/*
    This function outputs data in local memory in a pop like way,
    because caller doesn't control where to read and
    once data has taken it can't be read again with this function.
*/
char* popMem(unsigned int* size)
{
    if(iread == iwrite)
        return NULL;

    char* buffer;
    int k = 0;

    AskWrite(&rs_rwf);
    *size = RINGDELTA(iread, iwrite - 1);
    buffer = (char*) malloc((*size)*sizeof(char));

    AskRead(&rs_localmem);
    for(; k < *size; iread = (iread + 1) % MEM_BYTES, k++)
    {
        buffer[k] = localmem[iread];
    }
    FreeRead(&rs_localmem);

    FreeWrite(&rs_rwf);
    return buffer;
}

/*
    This functions exists to allow reading positions
*/
char getMemP(unsigned int _pos)
{
    char aux_c;
    int pos = ring_filled ? (_pos % MEM_BYTES) : (_pos % iwrite);

    AskRead(&rs_localmem);
    aux_c = localmem[pos];
    FreeRead(&rs_localmem);
    return aux_c;
}

/*
    This functions exists to allow reading ranges
    returns a buffer with size "size" from older to the
    most recent resgisters
*/
char* getMem(unsigned int from_i, unsigned int to_j, unsigned int* size)
{
    if(from_i == to_j) return NULL; // use getMemP

    unsigned int k = 0, i, j;
    char *buffer;

    i = from_i;
    j = to_j;

    getValidIndexes(&i, &j, size);
    if(!(i || j))
        return NULL;

    buffer = (char*) malloc((*size)*sizeof(char));
    if(buffer == NULL)
        return NULL;

    AskRead(&rs_localmem);
    for(; k < *size; i = (i + 1) % *size, k++)
        buffer[k] = localmem[i];
    FreeRead(&rs_localmem);
    return buffer;
}

void getValidRegIndexes(unsigned int* from_regi, unsigned int* to_regj, unsigned int* size)
{
    unsigned int i = REGISTER_SIZE*(*from_regi), j = REGISTER_SIZE*(*to_regj) + (REGISTER_SIZE - 1), s;
    getValidIndexes(&i, &j, &s);
    *from_regi = i/5;
    *to_regj = j/5;
    *size = s/5;
}

/*
    Validates memory indexes, adjusting them if position requested wasn't
    writen or if index was higher than buffer size
*/
void getValidIndexes(unsigned int* from_i, unsigned int* to_j, unsigned int* size)
{
    int mem_size, delta, i, j;

    AskRead(&rs_rwf);
    delta = DELTA(*from_i, *to_j);
    i = NORMLZ(*from_i);
    i = OVERREAD(i);
    j = NORMLZ(i + delta);
    j = OVERREAD(j);
    *size = RINGDELTA(i, j) + 1;
    FreeRead(&rs_rwf);
}
