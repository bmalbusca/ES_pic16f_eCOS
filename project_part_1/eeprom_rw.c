#include "eeprom_rw.h"
#include "mcc_generated_files/mcc.h"

extern unsigned char clkh;
extern unsigned char clkm;
extern unsigned char nreg;
extern unsigned char nreg_pt;
extern unsigned char pmon;
extern unsigned char tala;
extern unsigned char temp_thresh;
extern unsigned char lum_thresh;

void recoverData(){
    /* Recover Parameters */
    if(DATAEE_ReadByte(EE_RECV) == WORD_MG){
        if(DATAEE_ReadByte(EE_LST) == cksum()){
            clkh = DATAEE_ReadByte(EE_RECV + 1);
            clkm = DATAEE_ReadByte(EE_RECV + 2);
            nreg = DATAEE_ReadByte(EE_RECV + 3);
            nreg_pt = DATAEE_ReadByte(EE_RECV + 4);
            temp_thresh = DATAEE_ReadByte(EE_RECV + 5);
            lum_thresh = DATAEE_ReadByte(EE_RECV + 6);
            pmon = DATAEE_ReadByte(EE_RECV + 7);
            tala = DATAEE_ReadByte(EE_RECV + 8);
        }
    }

    reset_recv();

    /* Write Recovery Parameters */
    DATAEE_WriteByte(EE_RECV, WORD_MG);
    DATAEE_WriteByte(EE_RECV + 3, nreg);
    DATAEE_WriteByte(EE_RECV + 5, temp_thresh);
    DATAEE_WriteByte(EE_RECV + 6, lum_thresh);
    DATAEE_WriteByte(EE_RECV + 7, pmon);
    DATAEE_WriteByte(EE_RECV + 8, tala);
    cksum_w();
}

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