#include "proc.h"

// macros
#define DELTA(X,Y) ((X) >= (Y) ? (X) - (Y) : (Y) - (X))

void checkThresholds(char*(*popMem)(unsigned int*), int alat, int alal, cyg_mutex_t* rs_stdin)
{
    char* buffer;
    short int alarm_T, alarm_L;
    unsigned int size;
    int j = 0;

    buffer = popMem(&size);

    for(; j < size; j+=5) {
        alarm_T = ((int) buffer[j+3]) >= alat;
        alarm_L = ((int) buffer[j+4]) >= alal;
        if(alarm_T || alarm_L) {
            cyg_mutex_lock(rs_stdin);
            if(alarm_T)
                printf("[ALARM] Temperature threshold reached (T = %d) at %d:%d:%d\n",
                       (int) buffer[j+3], (int) buffer[j], (int) buffer[j+1], (int) buffer[j+2]);
            else
                printf("[ALARM] Luminosity threshold reached (L = %d) at %d:%d:%d\n",
                       (int) buffer[j+4], (int) buffer[j], (int) buffer[j+1], (int) buffer[j+2]);
            cyg_mutex_unlock(rs_stdin);
        }
    }

    free(buffer);
}

void calcStatistics(char*(*getMem)(unsigned int,unsigned int), int maxsize, int* max, int* min, int* mean, char range[6])
{
    int i, j;
    char* buffer;
    buffer = getMem(0, maxsize);
    find(buffer, &i, &j, f_calc(range,0,1,2), f_calc(range,3,4,5));

    // statistics

    free(buffer);
}

void find(char* buffer, int *i, int *j, int lowbound, int upbound)
{
    int size = *j - *i, calc;
    int k, distL, distU;
    int bestL = size, bestU = size;

    for(k = 0; k < size; k ++) {
        calc = f_calc(buffer, k, k+1, k+2);
        distL = calc - lowbound;
        distU = upbound - calc;
        if(distL > 0 && distL < bestL) *i = k;
        if(distU > 0 && distU < bestU) *j = k;
    }

    printf("%d %d\n", *i, *j);
}

int f_calc(char* buffer, int i, int j, int k)
{
    return ((int) buffer[i])*60 + ((int) buffer[j])*100 + ((int) buffer[k]);
}
