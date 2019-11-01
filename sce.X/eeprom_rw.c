#include "eeprom_rw.h"
#include "mcc_generated_files/mcc.h"

void push_ring(unsigned char* nreg_pt, unsigned char nreg, unsigned char* nreg_init, unsigned char* ring_byte)
{   if(!(*nreg_init)) (*nreg_init) = 1;
    for(unsigned char i = 0; i < 5; i++) {
        if((*nreg_pt) >= nreg) (*nreg_pt) = 0;
        DATAEE_WriteByte(EE_FST + (*nreg_pt), ring_byte[i]);
        (*nreg_pt)++;
    }
}

unsigned char read_ring(unsigned char nreg_pt, unsigned char nreg, unsigned char nreg_init, unsigned char index, unsigned char subindex)
{
    unsigned char absindex;
    unsigned char i = 0;
    
    if(index <= nreg && nreg_init) {
        absindex = 5 * index + subindex;
        if(absindex <= nreg_pt)
            i = nreg_pt - 5 + absindex;
        else
            i = nreg - (nreg_pt - 5 + absindex);
    }
    else 
        return 0;
    
    return DATAEE_ReadByte(EE_FST + i);
}

void cksum_w()
{
    DATAEE_WriteByte(EE_LST, cksum());
}

unsigned char cksum()
{
    unsigned char res = 0;
    for(unsigned int i = EE_RECV; i < EE_LST; i ++) {
        res += DATAEE_ReadByte(i);
    }
    return res;
}
    
void reset_recv()
{
    for(unsigned int i = EE_RECV; i <= EE_LST; i ++)
        DATAEE_WriteByte(i, 0);
}