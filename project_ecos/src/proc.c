#include "proc.h"
/*
    indexes should index byte to byte
    but will be divided by 5 when necessary to represent 5 bytes
*/

// macros
#define DELTA(X,Y) ((X) >= (Y) ? (X) - (Y) : (Y) - (X))

void checkThresholds(int alat, int alal)
{
    char* buffer;
    short int alarm_T, alarm_L;
    unsigned int size;
    int j = 0;

    AskRead(&rs_rwf);
    if(!ring_filled && (iread == 0)) {
        cyg_mutex_lock(&rs_stdout);
        printf("\n\nWriting valid data...\n\nCmd> ");
        cyg_mutex_unlock(&rs_stdout);
    }
    FreeRead(&rs_rwf);

    buffer = popMem(&size);
    if(buffer == NULL)
        return;

    for(; j < size; j+=5) {
        alarm_T = ((int) buffer[j+3]) >= alat;
        alarm_L = ((int) buffer[j+4]) >= alal;
        if(alarm_T || alarm_L) {
            if(alarm_T) {
                sprintf(stdout_buff, "[ALARM] Temperature threshold reached (T = %d celsius)\t at %d:%d:%d\n",
                       (int) buffer[j+3], (int) buffer[j], (int) buffer[j+1], (int) buffer[j+2]);
                queueStdout(stdout_buff);
            }
            else {
                sprintf(stdout_buff, "[ALARM] Temperature threshold reached (L = %d [0 - 3])\t at %d:%d:%d\n",
                       (int) buffer[j+3], (int) buffer[j], (int) buffer[j+1], (int) buffer[j+2]);
                queueStdout(stdout_buff);
            }
        }
    }

    free(buffer);
}


/*
    return 0 -> statistics invalid (error)
    return 1 -> statistics valid (success)
*/
char calcStatistics(stats* temp, stats* lum, char range[6])
{
    unsigned int i = 0, j = ACTUAL_SIZE - 1, k, newsize, resized;
    char* buffer;
    int temp_sum, lum_sum;

    buffer = getMem(i, j, &newsize);
    if(buffer == NULL)
        return 0;

    find(buffer, newsize, &i, &j, f_calc(range,0,1,2), f_calc(range,3,4,5));
    resized = j - i + 1;
    temp->min = 999; temp->max = 0;
    lum->min = 999; lum->max = 0;
    temp_sum = 0; lum_sum = 0;

    for(k = i; k < j; k += 5) {
        temp_sum += buffer[k + 3];
        lum_sum += buffer[k + 4];
        if(temp->max < buffer[k + 3]) temp->max = buffer[k + 3];
        if(temp->min > buffer[k + 3]) temp->min = buffer[k + 3];
        if(lum->max < buffer[k + 4]) lum->max = buffer[k + 4];
        if(lum->min > buffer[k + 4]) lum->min = buffer[k + 4];
    }

    temp->mean = temp_sum/(resized/5);
    lum->mean = lum_sum/(resized/5);

    free(buffer);

    return 1;
}

void find(char* buffer, unsigned int size, unsigned int *i, unsigned int *j, int lowbound, int upbound)
{
    unsigned int calc, k;
    int distL, distU, bestL = 24*3600, bestU = bestL;

    for(k = 0; k < size; k += 5) {
        calc = f_calc(buffer, k, k+1, k+2);
        distL = calc - lowbound;
        distU = upbound - calc;
        if(distL >= 0 && distL <= bestL) {
            *i = k;
            bestL = distL;
        }
        if(distU >= 0 && distU <= bestU) {
            *j = k + 4;
            bestU = distU;
        }
    }
}

int f_calc(char* buffer, unsigned int i, unsigned int j, int k)
{
    return ((int) buffer[i])*3600 + ((int) buffer[j])*60 + ((int) buffer[k]);
}
