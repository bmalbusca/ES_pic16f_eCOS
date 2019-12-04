#ifndef PROC_H
#define PROC_H
#include <stdio.h>
#include "threads.h"

void checkThresholds(char*(*popMem)(unsigned int*), int alat, int alal, cyg_mutex_t* rs_stdin);
void calcStatistics(char*(*getMem)(unsigned int,unsigned int), int maxsize, int* max, int* min, int* mean, char range[6]);
void find(char* buffer, int *i, int *j, int lowbound, int upbound);
int f_calc(char* buffer, int i, int j, int k);

#endif
