#include "main.h"

void usart_init (void);
void thread_init(void);
void proc_F     (cyg_addrword_t data);
void user_F     (cyg_addrword_t data);
void rx_F       (cyg_addrword_t data);
void tx_F       (cyg_addrword_t data);

/*
    PIC CONFIGS
*/

//int nreg = 30;
//int pmon = 5;
//int tala = 3;
int alat = 50;
int alal = 2;
//unsigned short int alaf = 0;
//int clkh = 0;
//int clkm = 0;

/*
    USART
*/

extern cyg_io_handle_t serial_h;
char* usart_n = "/dev/ser0"; // name
cyg_serial_info_t serial_i; // struct with configs for usart

extern Cyg_ErrNo err;


/*
*
*   ECOS runs this on START (user side inits)
*
*/

void cyg_user_start(void)
{
    iwrite = 0; iread = 0; ring_filled = 0;

    cyg_semaphore_init(&rs_localmem, 1);
    cyg_semaphore_init(&rs_rwf, 1);
    cyg_mutex_init(&rs_stdin);
    stdin_buff_pt = stdin_buff;
    usart_init();
    thread_init();

    cyg_thread_resume(proc.h);
    cyg_thread_resume(user.h);
    cyg_thread_resume(rx.h);
    cyg_thread_resume(tx.h);
}

// On configuration... (not working)
void usart_init(){
    int err;//, len;

    serial_i.baud =  CYGNUM_SERIAL_BAUD_9600;
    serial_i.stop = CYGNUM_SERIAL_STOP_1;
    serial_i.parity = CYGNUM_SERIAL_PARITY_NONE;
    serial_i.word_length = CYGNUM_SERIAL_WORD_LENGTH_8;
    serial_i.flags = CYG_SERIAL_FLAGS_RTSCTS;

    err = cyg_io_lookup(usart_n, &serial_h);
    if(err == ENOERR) {
        printf("[IO:\"%s\"] Detected\n", usart_n);
        //cyg_io_set_config(serial_h, key, (const void*) , &len);
    }
    else if(err == -ENOENT) 
        printf("[IO:\"%s\"] No such entity\n", usart_n);
    else
        printf("[IO:\"%s\"] Some error with code <%d> (lookup \'CYGONCE_ERROR_CODES_H\')\n", usart_n, err);
}

void thread_init() {

    strcpy(proc.name, "Processe");
    strcpy(user.name, "User");
    strcpy(rx.name,   "Read");
    strcpy(tx.name,   "Transfere");
    proc.f = proc_F;
    user.f = monitor; // this function is in "monitor.c"
    rx.f = rx_F;
    tx.f = tx_F;
    proc.pri = (cyg_addrword_t) HIGH_PRI;
    user.pri = (cyg_addrword_t) LOW_PRI;
    rx.pri = (cyg_addrword_t) DEFAULT_PRI;
    tx.pri = (cyg_addrword_t) DEFAULT_PRI;

    thread_create(&proc, 0);
    thread_create(&user, 0);
    thread_create(&rx, 0);
    thread_create(&tx, 0);
}

/*
    ************
    THREAD calls
    ============
    user thread has its function on "monitor.c"
*/

void proc_F(cyg_addrword_t data)
{
    int now, last = 0, k;
    char *cmd_out, *cmd_in;
    char returns;

    stats temp, lum;
    char range[6];

    int period_transference = 5;
    int tht = alat, thl = alal; // Temperature and Luminosity thresholds reserved to proc

    while(1) {
        now = cyg_current_time();

        if(period_transference) {
            if(DELTA(now, last) > period_transference) {
                // transfere, ask to transfere
                cmd_out = writeCommand(RX_TRANSFERENCE, 0);
                cyg_mbox_tryput(rx.mbox_h, (void*) cmd_out);
                last = now;
            }
        }

        cmd_in = (char*) cyg_mbox_tryget(proc.mbox_h);

        if(cmd_in != NULL) {
            switch (getName(cmd_in)) {
                case RX_TRANSFERENCE:
                    checkThresholds(popMem, alat, alal);
                    break;
                case USER_STATISTICS:
                    for(k = 0; k < 6; k ++) {
                        range[k] = getArg(cmd_in, k + 1);
                    }
                    returns = calcStatistics(getMem, LM_SIZE, &temp, &lum, range);
                    if(returns) {
                        cmd_out = writeCommand(ERROR_MEMEMPTY, 0);
                    } else {
                        cmd_out = writeCommand(USER_STATISTICS, 6);
                        setArg(cmd_out, 1, temp.min);
                        setArg(cmd_out, 2, temp.max);
                        setArg(cmd_out, 3, temp.mean);
                        setArg(cmd_out, 4, lum.min);
                        setArg(cmd_out, 5, lum.max);
                        setArg(cmd_out, 6, lum.mean);
                    }
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
                case USER_MODIFY_PERIOD_TRANSF:
                    if(getArgc(cmd_in) >= 1) {
                        period_transference = getArg(cmd_in, 1);
                    }
                    break;
                case USER_CHANGE_THRESHOLDS:
                    if(getArgc(cmd_in) >= 2) {
                        tht = getArg(cmd_in, 1);
                        thl = getArg(cmd_in, 2);
                    }
                    break;
                case PROC_CHECK_PERIOD_TRANSF:
                    cmd_out = writeCommand(PROC_CHECK_PERIOD_TRANSF, 1);
                    setArg(cmd_out, 1, period_transference);
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
                case PROC_CHECK_THRESHOLDS:
                    cmd_out = writeCommand(PROC_CHECK_THRESHOLDS, 2);
                    setArg(cmd_out, 1, tht);
                    setArg(cmd_out, 2, thl);
                    cyg_mbox_tryput(user.mbox_h, (void*) cmd_out);
                    break;
            }
        }

        __DELAY();
    }
}

void rx_F(cyg_addrword_t data){
    char *cmd_out, *cmd_in;

    char buffer_rx[10];

    int num_bytes = 7;

    //pushMem(buffer, 30);

    

    while(1){

        cmd_in = (char*)cyg_io_read(serial_h, (void*)buffer_rx, &num_bytes);        //7 = nº bytes a serem lidos

        /*if(cmd_in != NULL){
            switch (getName(cmd_in)) {
                case RX_TRANSFERENCE:
                    cmd_out = writeCommand(RX_TRANSFERENCE, 0);
                    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
                    break;
            }
        }*/
        printf("%s\n", buffer_rx);
    }
}

void tx_F(cyg_addrword_t data){
    char buffer_tx[50];
    char *cmd_out, *cmd_in;
    int i;
    int num_args = 0;
    int num_bytes = 5;

    while(1){
        //cmd_in = (char*)cyg_mbox_get(tx.mbox_h);            //Bloquear enquanto não houver cenas para enviar para o PIC

        buffer_tx[0] = ':';
        /*
        AskRead(&rs_rwf);           //Pedir para ler o ring buffer com inter threads

        buffer_tx[1] = getName(cmd_in);
        num_args = getArgc(cmd_in);

        for(i = 2; i < num_args + 2; i++){
            buffer_tx[i] = getArg(cmd_in, i-1);
        }
        FreeRead(&rs_rwf);          //Largar as keys para ler o ring buffer com inter threads
        */
        buffer_tx[1] = 'c';
        buffer_tx[2] = 'r';
        buffer_tx[3] = 'l';
        buffer_tx[4] = '?';
        err = cyg_io_write(serial_h, buffer_tx, &num_bytes);
        printf("Sent, err =%x\n", err);

        cyg_thread_delay(40);

        /*buffer_tx[num_args + 2] = '?';
        num_bytes = num_args + 3;
        cyg_io_write(serial_h, buffer_tx, &num_bytes)*/
    }
}


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
    unsigned int s, _iwrite, _iread, _ring_filled, aux;

    AskRead(&rs_rwf);
    _iwrite = iwrite;
    _iread = iread;
    _ring_filled = ring_filled;
    FreeRead(&rs_rwf);

    s = _ring_filled ? LM_SIZE : _iwrite + 1;
    if(!s) {
        *size = 0;
        return 0;
    }
    *from_i = (_iwrite + *from_i) % s;
    *to_j = (_iwrite + *to_j) > s ? _iwrite - 1 : *to_j;

    *size = RINGDELTA(*from_i, *to_j);

    return 1; // success
}
