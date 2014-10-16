// m: Check output from the AUM CSPRNG 
#include "aum.h"
#include <stdio.h>
#include <stddef.h>
// mixing rounds: 7 recommended minimum
#define ROUNDS 7
//#define STREAM

int main(int argc, char *argv[]) {
	int i;
	char seed[2048]="Bacon";
	// pull a seed from the command line
	if (argc>=2) strcpy(seed,argv[1]);
	// print some information
	printf("CSPRNG        : %s\nKey length    : %u bits\nInternal state: %u bits\nMinimum cycle : 2**%u\nMaximum cycle : 2**%u\nAverage cycle : 2**%u\n",
		aum_Name(),aum_KeyLength(),aum_StateBits(),aum_Cycle(),aum_StateBits(),aum_StateBits()-2);
	// seed AUM and mix
	aum_SeedW(seed,8*ROUNDS);
	#ifdef STREAM
	// show some MOD 26 stream output
	for (i=0; i<672; i++) printf("%c",aum_Random() % 26 + 'A');
	printf("\n");
	#endif
	return 0;
}
