/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Maio 2008
***************************************************************************/
#include "comando.h"

Cyg_ErrNo err;
cyg_io_handle_t serial_h;

/*-------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/
void cmd_sair (int argc, char **argv)
{
  exit(0);
}

/*-------------------------------------------------------------------------+
| Function: cmd_test - apenas como exemplo
+--------------------------------------------------------------------------*/
void cmd_test (int argc, char** argv)
{
  int i;

  /* exemplo -- escreve argumentos */
  for (i=0; i<argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}

/*-------------------------------------------------------------------------+
| Function: cmd_ems - enviar mensagem (string)
+--------------------------------------------------------------------------*/
void cmd_ems (int argc, char **argv)
{
  unsigned int n;

  if (argc > 1) {
    n = strlen(argv[1]) + 1;
    err = cyg_io_write(serial_h, argv[1], &n);
    printf("io_write err=%x, n=%d str=%s\n", err, n, argv[1]);
  }
  else {
    n = 10;
    err = cyg_io_write(serial_h, "123456789", &n);
    printf("io_write err=%x, n=%d str=%s\n", err, n, "123456789");
  }
}

/*-------------------------------------------------------------------------+
| Function: cmd_emh - enviar mensagem (hexadecimal)
+--------------------------------------------------------------------------*/
void cmd_emh (int argc, char **argv)
{
  unsigned int n, i;
  unsigned char bufw[50];

  if ((n=argc) > 1) {
    n--;
    if (n > 50) n = 50;
    for (i=0; i<n; i++)
      //      sscanf(argv[i+1], "%hhx", &bufw[i]);
      {unsigned int x; sscanf(argv[i+1], "%x", &x); bufw[i]=(unsigned char)x;}
    err = cyg_io_write(serial_h, bufw, &n);
    printf("io_write err=%x, n=%d\n", err, n);
    for (i=0; i<n; i++)
      printf("buf[%d]=%x\n", i, bufw[i]);
  }
  else {
    printf("nenhuma mensagem!!!\n");
  }
}

/*-------------------------------------------------------------------------+
| Function: cmd_rms - receber mensagem (string)
+--------------------------------------------------------------------------*/
void cmd_rms (int argc, char **argv)
{
  unsigned int n;
  char bufr[50];

  if (argc > 1) n = atoi(argv[1]);
  if (n > 50) n = 50;
  err = cyg_io_read(serial_h, bufr, &n);
  printf("io_read err=%x, n=%d buf=%s\n", err, n, bufr);
}

/*-------------------------------------------------------------------------+
| Function: cmd_rmh - receber mensagem (hexadecimal)
+--------------------------------------------------------------------------*/
void cmd_rmh (int argc, char **argv)
{
  unsigned int n, i;
  unsigned char bufr[50];

  if (argc > 1) n = atoi(argv[1]);
  if (n > 50) n = 50;
  err = cyg_io_read(serial_h, bufr, &n);
  printf("io_read err=%x, n=%d\n", err, n);
  for (i=0; i<n; i++)
    printf("buf[%d]=%x\n", i, bufr[i]);
}


/*-------------------------------------------------------------------------+
| Function: cmd_ini - inicializar dispositivo
+--------------------------------------------------------------------------*/
void cmd_ini(int argc, char **argv)
{
  printf("io_lookup\n");
  if ((argc > 1) && (argv[1][0] = '1'))
    err = cyg_io_lookup("/dev/ser1", &serial_h);
  else err = cyg_io_lookup("/dev/ser0", &serial_h);
  printf("lookup err=%x\n", err);
}

/*-------------------------------------------------------------------------+
|
|   PROJECT FUNCTIONS
|
+--------------------------------------------------------------------------*/
void cmd_irl(int argc, char **argv)
{
    unsigned int i = 0, j = NRBUF, nr, _iwrite, _iread;
    getValidIndexes(&i, &j, &nr);

    AskRead(&rs_rwf);
    _iwrite = iwrite;
    _iread = iread;
    FreeRead(&rs_rwf);

    cyg_mutex_lock(&rs_stdout);
    printf("\nNRBUF \tnr \tiread \tiwrite\n%d \t%d \t%d \t%d\n", NRBUF, nr, _iread, _iwrite);
    cyg_mutex_unlock(&rs_stdout);
}

void cmd_lr(int argc, char **argv)
{
    unsigned int i, j, size;

    i = strtol(argv[2], NULL, 10);
    size = strtol(argv[1], NULL, 10);
    j = i + size - 1;

    printMemReg(i, j);
}

void printMemReg(unsigned int from_regi, unsigned int to_regj)
{
    unsigned int i = from_regi, j = to_regj, size, mem_size;
    int u, v;

    mem_size = ACTUAL_SIZE;
    getValidRegIndexes(&i, &j, &size);

    AskRead(&rs_localmem);
    cyg_mutex_lock(&rs_stdout);
    if(!size)
        printf("\n**************************************\n\tLocal Memory is EMPTY\n**************************************\n");
    else {
        printf("\n**************************************\nLocal Memory from Register %d to %d\n**************************************\n", i, j);
        printf("HOURS \tMIN \tSEC \tTEMP \tLUM\n");
        for(u = 0; u < size; u++) {
            for(v = 0; v < REGISTER_SIZE; v++) {
                printf("%d\t", localmem[(REGISTER_SIZE*(u + i) + v) % mem_size]);
            }
            printf("\n");
        }
        printf("_____________________________________\n");
    }
    cyg_mutex_unlock(&rs_stdout);
    FreeRead(&rs_localmem);
}

void cmd_dr(int argc, char **argv)
{
    AskRead(&rs_rwf);
    ring_filled = 0;
    iwrite = 0;
    iread = iwrite;
    FreeRead(&rs_rwf);
}

void cmd_mpt(int argc, char **argv)
{
    char *cmd_out;

    cmd_out = writeCommand(USER_MODIFY_PERIOD_TRANSF, 1);
    setArg(cmd_out, 1, strtol(argv[1], NULL, 10));
    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
}

void cmd_cpt(int argc, char **argv)
{
    char *cmd_out, *cmd_in;

    cmd_out = writeCommand(PROC_CHECK_PERIOD_TRANSF, 0);
    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
    cmd_in = cyg_mbox_get(user.mbox_h);
    if(getName(cmd_in) != PROC_CHECK_PERIOD_TRANSF)
        return;

    cyg_mutex_lock(&rs_stdout);
    printf("\nPeriod of transference is %d\n", getArg(cmd_in, 1));
    cyg_mutex_unlock(&rs_stdout);
}

void cmd_dttl(int argc, char **argv)
{
    char *cmd_out;

    cmd_out = writeCommand(USER_CHANGE_THRESHOLDS, 2);
    setArg(cmd_out, 1, strtol(argv[1], NULL, 10));
    setArg(cmd_out, 2, strtol(argv[2], NULL, 10));
    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
}

void cmd_cttl(int argc, char **argv)
{
    char *cmd_out, *cmd_in;

    cmd_out = writeCommand(PROC_CHECK_THRESHOLDS, 0);
    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
    cmd_in = cyg_mbox_tryget(user.mbox_h);
    if(getName(cmd_in) != PROC_CHECK_THRESHOLDS)
        return;

    cyg_mutex_lock(&rs_stdout);
    printf("\nTemperature threshold is %d celsius\nLuminosity threshold is %d (0 - 3)\n", getArg(cmd_in, 1), getArg(cmd_in, 2));
    cyg_mutex_unlock(&rs_stdout);
}

void cmd_pr(int argc, char **argv)
{
    int arg, k;
    char *cmd_in, *cmd_out;

    cmd_out = writeCommand(USER_STATISTICS, argc);

    for(k = 0; k < 6; k += 3) {
        // range, a pair of H:M:S
        arg = strtol(argv[k + 1], NULL, 10);
        if(arg < 0 || arg > 23)
            arg = 0;
        setArg(cmd_out, k + 1, arg);

        arg = strtol(argv[k + 2], NULL, 10);
        if(arg < 0 || arg > 59)
            arg = 0;
        setArg(cmd_out, k + 2, arg);

        arg = strtol(argv[k + 3], NULL, 10);
        if(arg < 0 || arg > 59)
            arg = 0;
        setArg(cmd_out, k + 3, arg);
    }

    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
    cmd_in = cyg_mbox_get(user.mbox_h);
    if(getArgc(cmd_in) < 6 || getName(cmd_in) != USER_STATISTICS) {
        if(getName(cmd_in) == ERROR_MEMEMPTY) {
            cyg_mutex_lock(&rs_stdout);
            printf("\n<!> No registers found in local memory\n");
            cyg_mutex_unlock(&rs_stdout);
        }
        else {
            cyg_mutex_lock(&rs_stdout);
            printf("\n<!> Unknow error\n");
            cyg_mutex_unlock(&rs_stdout);
        }
    }
    else {
        cyg_mutex_lock(&rs_stdout);
        printf("\n**********************************\n\tResults\n**********************************\n");
        printf("TEMPERATURE (celsius)\nmin = %d \tmax = %d \tmean = %d\n", getArg(cmd_in, 1), getArg(cmd_in, 2), getArg(cmd_in, 3));
        printf("LUMINOSITY (0 - 3)\nmin = %d \tmax = %d \tmean = %d\n", getArg(cmd_in, 4), getArg(cmd_in, 5), getArg(cmd_in, 6));
        cyg_mutex_unlock(&rs_stdout);
    }
}
