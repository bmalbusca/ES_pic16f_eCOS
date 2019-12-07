/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Maio 2008
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cyg/io/io.h>
#include "main.h"
#include "threads.h"

Cyg_ErrNo err;
cyg_io_handle_t serH;

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
    err = cyg_io_write(serH, argv[1], &n);
    printf("io_write err=%x, n=%d str=%s\n", err, n, argv[1]);
  }
  else {
    n = 10;
    err = cyg_io_write(serH, "123456789", &n);
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
    err = cyg_io_write(serH, bufw, &n);
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
  err = cyg_io_read(serH, bufr, &n);
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
  err = cyg_io_read(serH, bufr, &n);
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
    err = cyg_io_lookup("/dev/ser1", &serH);
  else err = cyg_io_lookup("/dev/ser0", &serH);
  printf("lookup err=%x\n", err);
}

/*-------------------------------------------------------------------------+
|
|   PROJECT FUNCTIONS
|
+--------------------------------------------------------------------------*/
void cmd_pr(int argc, char **argv)
{
    int arg, i = 1, k = 1;
    char *cmd_in, *cmd_out;

    cmd_out = writeCommand(USER_STATISTICS, argc);
    for(; i <= 2; i++, k += 3) {
        // range, a pair of H:M:S
        arg = strtol(argv[i], NULL, 10);
        arg = (arg >= 0 && arg <= 23) ? arg : 0;
        setArg(cmd_out, k, arg);
        arg = strtol(argv[i], NULL, 10);
        arg = (arg >= 0 && arg <= 59) ? arg : 0;
        setArg(cmd_out, k + 1, arg);
        arg = strtol(argv[i], NULL, 10);
        arg = (arg >= 0 && arg <= 59) ? arg : 0;
        setArg(cmd_out, k + 2, arg);
    }
    cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
    do {
        cmd_in = cyg_mbox_get(user.mbox_h);
    } while(getName(cmd_in) != USER_STATISTICS);

    if(getArgc(cmd_in) < 6)
        return;

    cyg_mutex_lock(&rs_stdin);
    printf("\n***************************\n\tResults\n***************************\n");
    printf("TEMPERATURE\nmax = %d \tmin = %d \tmean = %d\n", getArg(cmd_in, 1), getArg(cmd_in, 2), getArg(cmd_in, 3));
    printf("LUMINOSITY\nmax = %d \tmin = %d \tmean = %d\n", getArg(cmd_in, 4), getArg(cmd_in, 5), getArg(cmd_in, 6));
    cyg_mutex_unlock(&rs_stdin);
}

void cmd_lr(int argc, char **argv)
{
    unsigned int size, i, j;
    char *buffer;

    i = strtol(argv[2], NULL, 10);
    size = strtol(argv[1], NULL, 10);
    j = i + size;
    buffer = getMem(&i, &j, &size);

    if(!(size % 5))
        return;

    printf("\n***************************\n\tLocal Memory from 0 to %d\n***************************\n", argc);
    for(; i < j; i += 5) {
        cyg_mutex_lock(&rs_stdin);
        printf("%d %d %d %d %d\n", buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3], buffer[i + 4]);
        cyg_mutex_unlock(&rs_stdin);
    }

    free(buffer);
}
