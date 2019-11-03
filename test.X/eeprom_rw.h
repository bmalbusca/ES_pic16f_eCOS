#ifndef EEPROM_RW
#define EEPROM_RW

#define WORD_MG     0xAB        // "magic word"

#define EE_FST      0xF000      // ring buffer starts here
#define EE_LST      0xF0FF      // last byte has the checksum
#define EE_SIZE     256
#define EE_RECV     EE_LST - 10 // adresses reserved to recovery parameters

/* EEPROM adress space
 * 0xF000 to 0xF0FF     
 * 256B -- 256 x 1B  
 */

void recoverData();
void push_ring(unsigned char* nreg_pt, unsigned char nreg, unsigned char* nreg_init, unsigned char* ring_byte);
unsigned char read_ring(unsigned char nreg_pt, unsigned char nreg, unsigned char nreg_init, unsigned char index, unsigned char subindex);
void cksum_w();
unsigned char cksum();
void reset_recv();

#endif