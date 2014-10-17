/* AUM - A small-state CSPRNG reflecting the Zen of cipher design
   AUM is an AUM with an 8,16,32 or 64+4-word internal state
   AUM may be seeded with a 256-, 512-, 1024- or 2048-bit key
   AUM Copyright C.C.Kayne 2014, GNU GPL V.3, cckayne@gmail.com
   AUM is inspired by Bob Jenkins' PRNG insights (Public Domain).
*/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "aum.h"

//#define TEST
//#define VERBOSE

// AUM defines

//#define AUM8
#define AUM16
//#define AUM32
//#define AUM64

/* internal state parameters */
#ifdef AUM8
#define STSZ 8
#define NAME "AUM8"
#endif
#ifdef AUM16
#define STSZ 16
#define NAME "AUM16"
#endif
#ifdef AUM32
#define STSZ 32
#define NAME "AUM32"
#endif
#ifdef AUM64
#define STSZ 64
#define NAME "AUM64"
#endif
#define STM1 STSZ-1
#define STBYTES STSZ*4
#define STBITS 128+STBYTES*8

/* 2**32/phi, where phi is the golden ratio */
#define GOLDEN   0x9e3779b9

static u4 state[STSZ], rsl[STSZ], b, c, d, e, rcnt=0;

#ifdef TEST
static void statepeek(void);
#endif


static void aum(void) {
	u4 i;
	for (i=0; i<STSZ; i++) {
		e = state[d & STM1];
		state[d & STM1] = b ^ c;
		b = c - d;
		c = d + e;
		rsl[i] = d = e + state[i];
	}
	#ifdef TEST
	statepeek();
	#endif
}


// obtain a AUM pseudo-random value in [0..2**32]
u4 aum_Random(void) {
	u4 r = rsl[rcnt];
	++rcnt;
	if (rcnt==STSZ) {
		aum();
		rcnt = 0;
	}
	return r;
}


void aum_Reset(void) {
	register u4 i,r;
	rcnt = 0;
	b = c = d = e = GOLDEN;
	for (i=0; i<STSZ; i++) { state[i]=GOLDEN; rsl[i]=GOLDEN; }
}


// seed AUM with a 2048-bit block of 4-byte words (Bob Jenkins method) 
void aum_SeedW(char *seed, int rounds)
{
	register u4 i,l;
	char s[STBYTES*2];
	l=strlen(seed);
	if (l>STBYTES) l=STBYTES;
	memset(s,0,l+1);
	/* truncate seed to state-size if necessary */
	for (i=0; i<l; i++) s[i] = seed[i];
	aum_Reset();
	memcpy((char *)state, (char *)s, l);
	aum();
	for (i=0; i<rounds; i++) aum_Random();  
}


// AUM # of bits internal state
u4 aum_StateBits(void) {
	return STBITS;
}


// AUM expected cycle length
u4 aum_Cycle(void) {
	return (STBITS+1)/2;
}


// AUM maximum key length (bits)
u4 aum_KeyLength(void) {
	return STBYTES*8;
}


// AUM Name
char* aum_Name(void) {
	return NAME;
}


#ifdef TEST
static u4 bcnt=0;
void testinit(u4 val) {
	register u4 i;
	rcnt = 0;
	b = c = d = e = val;
	for (i=0; i<STSZ; i++) { state[i]=val; rsl[i]=0; }
}
static void statepeek(void) {
	register u4 i;
	++bcnt;
	printf("%3u) aum%u...\n",bcnt,STSZ);
	for (i=0; i<STSZ; i++) {
		#ifdef VERBOSE
		printf("state %3u: %11u %c %02X  | rsl %3u: %11u %c %02X\n",
			i,state[i],state[i] % 26 + 'A',state[i] & 255,
			i,rsl[i],rsl[i] % 26 + 'A',rsl[i] & 255);
		#endif
	}
	printf("       b = %11u %c %02X\n       c = %11u %c %02X\n       d = %11u %c %02X\n       e = %11u %c %02X\n",
		b,b % 26+'A',b & 255,c,c % 26+'A',c & 255,d,d % 26+'A',d & 255,e,e % 26+'A',e & 255);
}
#endif
