#ifndef AUM_H_
#define AUM_H_

typedef unsigned long int u4;
 
// obtain an AUM pseudo-random value in [0..2**32]
u4  aum_Random(void);
// seed AUM with a 2048-bit block of 4-byte words (Bob Jenkins method) 
void aum_SeedW(char *seed, int rounds);
// reset/initialize AUM
void aum_Reset(void);
// AUM # of bits internal state
u4 aum_StateBits(void);
// AUM expected cycle length
u4 aum_Cycle(void);
// AUM maximum key length (bits)
u4 aum_KeyLength(void);
// AUM Name
char* aum_Name(void);

#endif
