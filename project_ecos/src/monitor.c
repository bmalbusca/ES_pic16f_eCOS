/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC),
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include "monitor.h"

struct command_d const commands[] = {
    {cmd_sos,  "sos", 0,     0,  " \t \t \t help"},
    {cmd_sair, "sair", 0,    0,  " \t \t \t exit"},
    {cmd_ini,  "ini", 0,     0,  " \t<d> \t \t start devices (0/1) ser0/ser1"},
    {cmd_ems,  "ems", 0,     0,  " \t<str> \t \t send message (string)"},
    {cmd_emh,  "emh", 0,     0,  " \t<h1> <h2> <...> \t send message (hexadecimal)"},
    {cmd_rms,  "rms", 0,     0,  " \t<n> \t \t receive message (string)"},
    {cmd_rmh,  "rmh", 0,     0,  " \t<n> \t \t receive message (hexadecimal)"},
    {cmd_test, "test", 0,    0,  " \t<arg1> <arg2> \t test cmd"},
    {cmd_irl,  "irl", 0,     0,  " \t \t \t information about local reg. (NRBUF, nr, iread, iwrite)"},
    {cmd_lr,   "lr", 0,      2,  " \tn i \t \t list n registers (local) from index i (0 - oldest)"},
    {cmd_dr,   "dr", 0,      0,  " \t \t \t delete registers (local)"},
    {cmd_cpt,  "cpt", 0,     0,  " \t \t \t check period of transference"},
    {cmd_mpt,  "mpt", 0,     1,  " \tp \t \t modify period of transference (minutes - 0 deactivate)"},
    {cmd_cttl, "cttl", 0,    0,  " \t \t \t check threshold temperature and luminosity for processing"},
    {cmd_dttl, "dttl", 0,    2,  " \tn i \t \t define threshold temperature and luminosity for proc."},
    {cmd_pr,   "pr", 0,      6,  " \t[h1m1s1 [h2m2s2]]max, min e average in specified interval"},

    {cmd_emh,  "rc", RCLK,      0,  " \t<h1> <h2> <...> \t read clock"},
    {cmd_rms,  "sc", SCLK,      3,  " \t<n> \t \t set clock (h m s)"},
    {cmd_rmh,  "rtl", RTL,      0,  " \t<n> \t \t read temperature and luminosity"},
    {cmd_test, "rp", RPAR,      0,  " \t<arg1> <arg2> \t read parameters: PMON and TALA"},
    {cmd_irl,  "mmp", MMP,      1,  " \t \t \t modify monitoring period (seconds) "},
    {cmd_lr,   "mta", MTA,      1,  " \tn i \t \t modify time alarm (seconds)"},
    {cmd_dr,   "ra", RALA,      0,  " \t \t \t read alarms: temperature, luminosity, 1/0"},
    {cmd_cpt,  "dtl", DATL,     2,  " \t \t \t define alarm (temperature and luminosity)"},
    {cmd_mpt,  "aa", AALA,      1,  " \tp \t \t activate or deactive alarms (1/0)"},
    {cmd_cttl, "ir", IREG,      0,  " \t \t \t information about registers: NREG, nr, iread, iwrite"},
    {cmd_dttl, "trc", TRGC,     1,  " \tn i \t \t transfer n registers from current iread position (n)"},
    {cmd_pr,   "tri", TRGI,     2,  " \t transfer n registers from index i (n i)"}
};

int commands_size = sizeof(commands)/sizeof(struct command_d);


/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/
const char TitleMsg[] = "\n\tWeather Station Control\n";
const char Group[] = "\n\t***************************\n\tBruno Figueiredo\n\t\tFrancisco Melo\n\t\t\tAndre Silva\n\t***************************\n";
const char InvalMsg[] = "\nInvalid command!\n";
const char NotEnoughMsg[] = "\nNot enough arguments.\n";


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
void monitor (cyg_addrword_t data)
{
    static char *argv[ARGVECSIZE+1], *p;
    int argc, i;

    cyg_mutex_lock(&rs_stdout);
    printf("%s %s\n(Type sos for help)\n", TitleMsg, Group);
    cyg_mutex_unlock(&rs_stdout);

    while(1) {
        __SHORT_DELAY();

        dumpStdout();

        /*
        cyg_mutex_lock(&rs_stdout);
        printf("\nCmd> ");
        cyg_mutex_unlock(&rs_stdout);

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
                if(argc >= commands[i].min_args + 1) {
                    commands[i].cmd_fnct(argc, argv);
                }
                else {
                    cyg_mutex_lock(&rs_stdout);
                    printf("%s", NotEnoughMsg);
                    cyg_mutex_unlock(&rs_stdout);
                }
            }
            else {
                cyg_mutex_lock(&rs_stdout);
                printf("%s", InvalMsg);
                cyg_mutex_unlock(&rs_stdout);
            }
        }
        */
    }
}
