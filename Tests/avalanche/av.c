/* measure mean and maximum avalanche between consecutive values from a PRNG */
#include <stdio.h>
#include "mote8.h"
#include "bb128.h"
#include "aleph.h"
#include "aum.h"

//#define VERBOSE

/* select your RNG here */
//#define MOTE8
//#define BB128
#define AUM
//#define ALEPH

typedef unsigned long u4;


#ifdef VERBOSE
// print a binary string followed by chosen escape character
// usage: putb(sizeof(n),&n,'\n');
static void putb(size_t const size, void const * const ptr, char esc)
{
    unsigned char *b = (unsigned char*) ptr;
    register unsigned char byte;
    register int i, j;
    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    printf("%c",esc);
}
#endif

// obtain the 1-bit population of a 32-bit quantity (15 instructions)
static u4 pop1(register u4 x) {
   x = x - ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x + (x >> 4)) & 0x0F0F0F0F;
   x = x + (x >> 8);
   x = x + (x >> 16);
   return x & 0x0000003F;
}


int main(int argc, char *argv[]) {
	u4 n1 = 8; u4 n2=7; u4 n3=0;
	u4 i,p,tot=0,q=10000000;
	u4 min=32; u4 max=0;
	float mean;
	
	#ifdef MOTE8
	mote8_SeedW("Bacon",1000);
	#endif
	#ifdef BB128
	bb128_SeedW("Bacon",1000);
	#endif
	#ifdef ALEPH
	aleph_SeedW("Bacon",1000);
	#endif
	#ifdef AUM
	aum_SeedW("Bacon",1000);
	#endif
	
	for (i=0; i<q; i++) {
		#ifdef MOTE8
		n1 = mote8_Random(); n2 = mote8_Random();
		#endif
		#ifdef BB128
		n1 = bb128_Random(); n2 = bb128_Random();
		#endif
		#ifdef ALEPH
		n1 = aleph_Random(); n2 = aleph_Random();
		#endif
		#ifdef AUM
		n1 = aum_Random(); n2 = aum_Random();
		#endif
		#ifdef OM
		n1 = Om; n2 = Om;
		#endif
		// compute # of changed bits
		n3 = n1 ^ n2;
		p  = pop1(n3);
		// gather statistics
		tot += p;
		if (p>max) max=p;
		if (p<min) min=p;
		
		#ifdef VERBOSE
		putb(sizeof n1, &n1, 10);
		putb(sizeof n2, &n2, 10);
		putb(sizeof n3, &n3, 10);
		printf("%u\n",p);
		#endif
	}
	mean = (float)tot/i;
	#ifdef MOTE8
	printf("%s\nMean: %2.3f\nMax : %u\nMin : %u\n",mote8_Name(),mean,max,min);
	#endif
	#ifdef BB128
	printf("%s\nMean: %2.3f\nMax : %u\nMin : %u\n","BB128",mean,max,min);
	#endif
	#ifdef ALEPH
	printf("%s\nMean: %2.3f\nMax : %u\nMin : %u\n",aleph_Name(),mean,max,min);
	#endif
	#ifdef AUM
	printf("%s\nMean: %2.3f\nMax : %u\nMin : %u\n",aum_Name(),mean,max,min);
	#endif

	return 0;
}
