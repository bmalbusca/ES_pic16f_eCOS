#ifndef COMANDO_H
#define COMANDO_H

void cmd_sair (int argc, char **argv);
void cmd_test (int argc, char** argv);
void cmd_ems (int argc, char **argv);
void cmd_emh (int argc, char **argv);
void cmd_rms (int argc, char **argv);
void cmd_rmh (int argc, char **argv);
void cmd_ini(int argc, char **argv);

// our commands:
void cmd_irl(int argc, char **argv);
void cmd_lr(int argc, char **argv);
void cmd_dr(int argc, char **argv);
void cmd_mpt(int argc, char **argv);
void cmd_cpt(int argc, char **argv);
void cmd_dttl(int argc, char **argv);
void cmd_cttl(int argc, char **argv);
void cmd_pr(int argc, char **argv);

#endif
