/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC),
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include "monitor.h"

/*-------------------------------------------------------------------------+
| Headers of command functions
+--------------------------------------------------------------------------*/
extern void cmd_sair(int, char** );
extern void cmd_ini (int, char** );
extern void cmd_ems (int, char** );
extern void cmd_emh (int, char** );
extern void cmd_rms (int, char** );
extern void cmd_rmh (int, char** );
extern void cmd_test(int, char** );
extern void cmd_pr  (int, char** );
       void cmd_sos (int, char** );

/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/
const char TitleMsg[] = "\n Application Control Monitor (tst)\n";
const char InvalMsg[] = "\nInvalid command!";

struct 	command_d {
  void  (*cmd_fnct)(int, char**);
  char*	cmd_name;
  char  min_args;
  char*	cmd_help;
} const commands[] = {
  {cmd_sos,  "sos",     0,  "                   help"},
  {cmd_sair, "sair",    0,  "                   sair"},
  {cmd_ini,  "ini",     0,  "<d>                inicializar dispositivo (0/1) ser0/ser1"},
  {cmd_ems,  "ems",     0,  "<str>              enviar mensagem (string)"},
  {cmd_emh,  "emh",     0,  "<h1> <h2> <...>    enviar mensagem (hexadecimal)"},
  {cmd_rms,  "rms",     0,  "<n>                receber mensagem (string)"},
  {cmd_rmh,  "rmh",     0,  "<n>                receber mensagem (hexadecimal)"},
  {cmd_test, "teste",   0,  "<arg1> <arg2>      comando de teste"},
  {cmd_pr,   "pr",      6,  "[h1m1s1 [h2m2s2]]  max, min e média do intervalo de registos especificado"}
};

#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define ARGVECSIZE 10
#define MAX_LINE   50

/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/
void cmd_sos (int argc, char **argv)
{
  int i;

  printf("%s\n", TitleMsg);
  for (i=0; i<NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}

/*-------------------------------------------------------------------------+
| Function: getline        (called from monitor)
+--------------------------------------------------------------------------*/
int my_getline (char** argv, int argvsize)
{
  static char line[MAX_LINE];
  char *p;
  int argc;

  fgets(line, MAX_LINE, stdin);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for (argc=0,p=line; (*line != '\0') && (argc < argvsize); p=NULL,argc++) {
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if (p == NULL) return argc;
  }
  argv[argc] = p;
  return argc;
}

/*-------------------------------------------------------------------------+
| Function: monitor (called from main) (user thread)
+--------------------------------------------------------------------------*/
void monitor (void)
{
    static char *argv[ARGVECSIZE+1], *p;
    int argc, i;

    cyg_mutex_lock(&rs_stdin);
    printf("%s Type sos for help\n", TitleMsg);
    cyg_mutex_unlock(&rs_stdin);

    for (;;) {
        __SHORT_DELAY();

        cyg_mutex_lock(&rs_stdin);
        // dump strings waiting to be written
        printf("\n%s", stdin_buff);
        stdin_buff_pt = stdin_buff;
        printf("\nCmd> ");
        cyg_mutex_unlock(&rs_stdin);

        /* Reading and parsing command line  ----------------------------------*/
        if ((argc = my_getline(argv, ARGVECSIZE)) > 0) {
            for (p=argv[0]; (*p != '\0') || (*p != '\n'); *p=tolower(*p), p++);
            for (i = 0; i < NCOMMANDS; i++)
                if (strcmp(argv[0], commands[i].cmd_name) == 0)
                    break;
            /* Executing commands -----------------------------------------------*/
            if (i < NCOMMANDS) {
                if(argc >= commands[i].min_args)
                commands[i].cmd_fnct (argc, argv);
            }
            else {
                cyg_mutex_lock(&rs_stdin);
                printf("%s", InvalMsg);
                cyg_mutex_unlock(&rs_stdin);
            }
        } /* if my_getline */
    } /* forever */
}
