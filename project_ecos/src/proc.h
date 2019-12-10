#ifndef PROC_H
#define PROC_H
#include <stdio.h>
#include "threads.h"
#include "mem.h"

typedef struct {
    int min;
    int max;
    int mean;
} stats;

void checkThresholds(int alat, int alal);
char calcStatistics(stats* temp, stats* lum, char range[6]);
void find(char* buffer, unsigned int size, unsigned int *i, unsigned int *j, int lowbound, int upbound);
int f_calc(char* buffer, unsigned int i, unsigned int j, int k);

#endif
