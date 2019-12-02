#include "proc.h"

// macros
#define DELTA(X,Y) X > Y ? X - Y : Y - X

void checkThresholds(char* localmem, int iread, int iwrite, int alat, int alal, cyg_sem_t* rs_localmem, cyg_mutex_t* rs_stdin)
{
    int j, end = DELTA(iread, iwrite);
    char buffer[5];
    short int alarm;

    for(j = 0; j < end; j+=5) {

        AskRead(rs_localmem);
        memcpy(buffer, localmem + j, 5);
        FreeRead(rs_localmem);

        alarm = ((((int) buffer[3]) > alat) || (((int) buffer[4]) > alal));

        if(alarm) {
            cyg_mutex_lock(rs_stdin);
            if(((int) buffer[3]) >= alat)
                printf("[ALARM] Temperature threshold reached (T = %d) at %d:%d:%d\n",
                       (int) buffer[3], (int) buffer[0], (int) buffer[1], (int) buffer[2]);

            if(((int) buffer[4]) >= alal)
                printf("[ALARM] Luminosity threshold reached (L = %d) at %d:%d:%d\n",
                       (int) buffer[5], (int) buffer[0], (int) buffer[1], (int) buffer[2]);
            cyg_mutex_unlock(rs_stdin);
        }
    }
}

void calcStatistics(char* localmem, int iwrite, int mem_filled, int* max, int* min, int* mean, unsigned int range[6])
{


}

void find(char* localmem, int iwrite, int mem_filled, int *start_i, int* end_i, unsigned int range[6])
{

}
