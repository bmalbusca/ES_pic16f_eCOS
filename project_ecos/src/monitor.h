#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "threads.h"
#include "comando.h"

void monitor (cyg_addrword_t data);
void cmd_sos (int argc, char **argv);

struct 	command_d{
    void  (*cmd_fnct)(int, char**);
    char*	cmd_name;
    char cmd_code;
    char  min_args;
    char*	cmd_help;
};

#endif
