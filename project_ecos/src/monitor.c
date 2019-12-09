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
extern void cmd_lr  (int, char** );
       void cmd_sos (int, char** );

/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/
const char TitleMsg[] = "\n\tWeather Station Control\n";
const char Group[] = "\n\t***************************\n\tBruno Figueiredo\n\t\tFrancisco Melo\n\t\t\tAndre Silva\n\t***************************\n";
const char InvalMsg[] = "\nInvalid command!";

struct 	command_d {
  void  (*cmd_fnct)(int, char**);
  char*	cmd_name;
  char  min_args;
  char*	cmd_help;
} const commands[] = {
  {cmd_sos,  "sos",     0,  " \t \t \t \thelp"},
  {cmd_sair, "sair",    0,  " \t \t \t \texit"},
  {cmd_ini,  "ini",     0,  " \t<d> \t \t \tstart devices (0/1) ser0/ser1"},
  {cmd_ems,  "ems",     0,  " \t<str> \t \t \tsend message (string)"},
  {cmd_emh,  "emh",     0,  " \t<h1> <h2> <...> \tsend message (hexadecimal)"},
  {cmd_rms,  "rms",     0,  " \t<n> \t \t \treceive message (string)"},
  {cmd_rmh,  "rmh",     0,  " \t<n> \t \t \treceive message (hexadecimal)"},
  {cmd_test, "test",    0,  " \t<arg1> <arg2> \t \ttest cmd"},
  {cmd_pr,   "pr",      6,  " \t[h1m1s1 [h2m2s2]] \tmax, min e average in specified interval"},
  {cmd_lr,   "lr",      1,  " \tn i \t \t \tlistnregisters (local memory) from indexi(0 - oldest)"}
};

#define PR 8;           //ID do comando "pr"
#define LR 9;           //ID do comando "LR"


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
    printf("%s %s\n(Type sos for help)", TitleMsg, Group);
    cyg_mutex_unlock(&rs_stdin);

    while(1) {
        __SHORT_DELAY();

        cyg_mutex_lock(&rs_stdin);
        // dump strings waiting to be written
        printf("\n%s", stdin_buff);
        *stdin_buff = '\0';
        stdin_buff_pt = stdin_buff;
        printf("\nCmd> ");
        cyg_mutex_unlock(&rs_stdin);

        // reading and parsing command line
        if ((argc = my_getline(argv, ARGVECSIZE)) > 0) {

            // to lower case
            for (p=argv[0]; *p != '\0'; *p=tolower(*p), p++);

            // find command
            for (i = 0; i < NCOMMANDS; i++)
                if (strcmp(argv[0], commands[i].cmd_name) == 0)
                    break;

            // executing commands
            if (i < NCOMMANDS) {
                if(argc >= commands[i].min_args) {
                    commands[i].cmd_fnct (argc, argv);
                }
            }
            else {
                cyg_mutex_lock(&rs_stdin);
                printf("%s", InvalMsg);
                cyg_mutex_unlock(&rs_stdin);
            }
        }
    }
}
