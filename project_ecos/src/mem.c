/*
    ******************
    LOCAL MEMORY calls
    ==================
*/

/*
    This function puts data in local memory in push like way,
    because caller doesn't control where to write.
*/
void pushMem(char* buffer, unsigned int size)
{
    int k = 0;
    if(size % 5)
        return;
    AskWrite(&rs_localmem);
    AskWrite(&rs_rwf);
    for(; k < size; iwrite = (iwrite + 1) % LM_SIZE, k++)
    {
        localmem[iwrite] = buffer[k];
    }
    FreeWrite(&rs_rwf);
    FreeWrite(&rs_localmem);
    if(!iwrite) {
        AskRead(&rs_rwf);
        ring_filled = 1;
        FreeRead(&rs_rwf);
    }
}

/*
    This function outputs data in local memory in a pop like way,
    because caller doesn't control where to read and
    once data has taken it can't be read again with this function.
*/
char* popMem(unsigned int* size)
{
    char* buffer;
    int k = 0;

    AskWrite(&rs_rwf);
    *size = RINGDELTA(iread, iwrite);
    buffer = (char*) malloc((*size)*sizeof(char));

    AskRead(&rs_localmem);
    for(; k < *size; iread = (iread + 1) % LM_SIZE, k++)
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
    int pos = ring_filled ? (_pos % LM_SIZE) : (_pos % iwrite);

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

    if(!getValidIndexes(&i, &j, size))
        return NULL;

    cyg_mutex_lock(&rs_stdin);
    printf("!!!\n");
    cyg_mutex_unlock(&rs_stdin);

    buffer = (char*) malloc((*size)*sizeof(char));
    if(buffer == NULL)
        return NULL;

    AskRead(&rs_localmem);
    for(; i != j; i++, k++)
        buffer[k] = localmem[i % *size];
    FreeRead(&rs_localmem);
    return buffer;
}

/*
    Validates memory indexes, adjusting them if position requested wasn't
    writen or if index was higher than buffer size
*/
char getValidIndexes(unsigned int* from_i, unsigned int* to_j, unsigned int* size)
{
    int lm_size, delta, aux_j, _iwrite, _iread, _ring_filled, i, j;
    i = *from_i; j = *to_j;

    cyg_mutex_lock(&rs_stdin);
    printf("HERE\n");
    cyg_mutex_unlock(&rs_stdin);

    AskRead(&rs_rwf);
    _iwrite = iwrite;
    _iread = iread;
    _ring_filled = ring_filled;
    FreeRead(&rs_rwf);

    delta = j - i;
    lm_size = _ring_filled ? LM_SIZE : _iwrite;
    if(delta > lm_size) delta = lm_size - 1;
    if(!lm_size || delta <= 0) {
        *size = 0;
        return 0;
    }
    i = (_iwrite + i) % lm_size;
    aux_j = (iwrite + *from_i + delta) % lm_size;
    j = (aux_j >= _iread) && (aux_j < _iwrite) ? _iread - 1 : aux_j;

    *from_i = (unsigned int) i;
    *to_j = (unsigned int) j;
    *size = RINGDELTA(*from_i, *to_j);

    printf("%d %d\n", i ,j);
    return 1; // success
}
