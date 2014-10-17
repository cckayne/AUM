/* poptrial.c
Confirm that the bit-populations of consecutive 32-bit words emitted by a PRNG
over several trials converges to an expected normal distribution for i.i.d random variables.
	Usage  : >poptrial <from # trials> <to # trials> <rng> {<seed>}
	Example: >poptrial 24 34 1 "my seed"
	(perform 11 trials from 2**24 to 2**34 using AUM on seed "my seed")
*/
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <math.h>
#include "cktype.h"
#include "csprng.h"

//#define TEST 
#define VERBOSE

#define	MODU 32
#define	MODM1 MODU-1
#define MODP1 MODU+1

enum CSPRNG	rng = AUM;
const char *RNGs[10] = { "ISAAC", "AUM", "IOTA", "MOTE8", "MOTE16", "MOTE32", "MOTE64", "BB128", "BB256", "BB512" };

static ub4	r,m;
long int 	i,j;
static char	seed[MAXK] = "Monte Carlo Mod";
static ub8	q,qi;
static ub8	totals[MODP1];		
double		prob[MODP1];
double		expect[MODP1];
double		probtot,range;
double		chi,chis[MODP1];
char		values[MODP1];


// count the bits in a number (35 instructions maximum)
static ub4 bitCount(register ub4 val) { 
    register ub4 v,c=0;
	v = val;
	while (v > 0) { 
		c++; v = v >> 1;
	}
	return c;
}

// obtain the 1-bit population of a 32-bit quantity (20 instructions)
static ub4 pop0(register ub4 x) {
   x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
   x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
   x = (x & 0x0000FFFF) + ((x >>16) & 0x0000FFFF);
   return x;
}

// obtain the 1-bit population of a 32-bit quantity (15 instructions)
static ub4 pop1(register ub4 x) {
   x = x - ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x + (x >> 4)) & 0x0F0F0F0F;
   x = x + (x >> 8);
   x = x + (x >> 16);
   return x & 0x0000003F;
}


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


// Maximum (mode) from a set of probabilities (returns array index)
static ub4 MaxP(ub4 mins,ub4 maxs, double *p) {
	register ub4 i,max;
	double m = 0.0;
	for (i=mins; i<=maxs; i++)
		if (p[i]>m) { m=p[i]; max=i; }
	return max;
}	

	
// Minimum from a set of probabilities (returns array index)
static ub4 MinP(ub4 mins,ub4 maxs, double *p) {
	register ub4 i,min;
	double m = 1.0;
	for (i=mins; i<=maxs; i++)
		if (p[i]<m) { m=p[i]; min=i; }
	return min;
}	


// Mean of a set of probabilities
static double Mean(ub4 mins,ub4 maxs, double *p) {
	register i;
	double t = 0.0;
	double dv= maxs-mins+1;
	for (i=mins; i<=maxs; i++) t += p[i];
	return t/dv;
}


// Standard deviation of a set of probabilities
static double Sigma(ub4 minx,ub4 maxs, double *p, int bessel) {
	register ub4 i;
	double  dv=maxs-minx+1;
	double	m, t=0.0;
	double d[MODP1];
	// sample mean
	m = Mean(minx,maxs,p);
	for (i=minx; i<=maxs; i++) 
			d[i] = pow((p[i]-m),2);
	// variance 
	for (i=minx; i<=maxs; i++) t += d[i];
	// Bessel correction for sample variance
	if (bessel)
		m = t/(dv-1);
	else
		m = t/dv;
	return sqrt(m);
}


// Variance of a set of probabilities
static double Variance(ub4 minx,ub4 maxs, double *p, int bessel) {
	register ub4 i;
	double  dv=maxs-minx+1;
	double	m, t=0.0;
	double d[MODP1];
	// sample mean
	m = Mean(minx,maxs,p);
	for (i=minx; i<=maxs; i++) 
			d[i] = pow((p[i]-m),2);
	// variance 
	for (i=minx; i<=maxs; i++) t += d[i];
	// Bessel correction for sample variance
	if (bessel)
		return t/(dv-1);
	else
		return t/dv;
}


// chi-square of a set of probabilities
static double ChiSquare(ub4 minx, ub4 maxs, double *p, double *e, int norm) {
	ub4 i;
	double dv=maxs-minx+1;
	double dof=maxs-minx;
	double m, ch=0.0;
	// expected mean
	m = Mean(minx,maxs,e);
	for (i=minx;i<=maxs;i++)
			ch += pow((p[i]-m),2)/m;
	if (norm) ch = (ch-dof)/(3*sqrt(dof));
	return ch;
}


// Normalized chi-square given expected and observed variance
static double Chi2(ub4 minx, ub4 maxs, double varobs, double varexp) {
	double dof=maxs-minx;
	// m-1 degrees of freedom
	return ((varexp-varobs)-dof)/(3*sqrt(dof));
}


// Expected probabilities for a normal distribution of maxbits
static void PhiExpect( double *pc, ub4 maxbits)
{
  ub8 i,j,k;
  for (i=0; i<=maxbits; ++i) {
    pc[i] = 0.0;
  }
  for (i=0; i<=maxbits; ++i) {
    k = 1;
    for (j=1; j<=i; ++j) {
      k = (k * (maxbits+1-j)) / j;
    }
    pc[i] += ldexp((double)k,-32);
  }
}


int main(int argc, char *argv[]) {
	ub4 tots=0;
	ub4 ts, r, bts, div;
	ub4 q1 = 24;
	ub4 q2 = 28;
	double totSigma=0.0;
	double totRange=0.0;
	double sigma,sigmaE,range,rangeE,mSigma,mRange;
	// timer
	time_t a,z;
	// rounds
	qi = pow(2,q1);
	// check the command line
	if (argc>=2) q1 = atoi(argv[1]);
	if (argc>=3) q2 = atoi(argv[2]);
	if (argc>=4) rng= atoi(argv[3]) % 10;
	if (argc>=5) strcpy(seed,argv[4]);
	
	#ifdef TEST
	#ifdef __TINYC__
		puts("Tiny C");
	#endif
	#ifdef __WATCOMC__
		puts("Open Watcom C");
	#endif
	#ifdef _MSC_VER
		puts("Microsoft Visual C");
	#endif
	#ifdef __GNUC__
		puts("GNU C");
	#endif
	#if __STDC_VERSION__ >= 199901L
		puts("C99 supported.");
	#endif
	#endif

	printf("POP: ");

	div = q2-q1+1;
	printf("%d %s trial sets in [2**%d..2**%d]\n",div,RNGs[rng],q1,q2);
		
	puts("Trial      Range        Sigma         Time");
	puts("------------------------------------------");
	
	// compute the expected probabilities for a normal distribution
	PhiExpect(expect, MODU);
	sigmaE = Sigma(0,MODU,expect,FALSE);
	rangeE = expect[MaxP(0,MODU,expect)]-expect[MinP(0,MODU,expect)];
	
	// conduct specified # of trials
	for (j=q1; j<=q2; j++) {
		memset(totals,0,sizeof totals);
		probtot = 0.0;
		qi = pow(2,j);
		rSeed(rng,seed,rStateSize(rng)*7);
		
		time(&a);
		// trial #
		for (q=0; q<qi; q++) { 
			r=pop1(rRandom(rng));
			totals[r]++;
		}
		time(&z);
		ts=(size_t)(z-a);
		tots+=ts;
		
		// collate experimental observations
		for (i=0; i<=MODU; i++) {
			// actual probabilities
			prob[i]=(double)totals[i]/qi;
			// probtot holds total of probabilities - it should converge to 1.0
			probtot=probtot+prob[i];
			// collect value-names & decide output format
			values[i] = i;
		}
		// calculate and display results for this trial
		sigma = Sigma(0,MODU,prob,FALSE);
		chis[i] = ChiSquare(0,MODU,prob,expect,FALSE);
		range = prob[MaxP(0,MODU,prob)]-prob[MinP(0,MODU,prob)];
		totSigma+=sigma; totRange+=range;
		printf("2**%2d    %1.7f    %1.7f      %3d s\n",j,range,sigma,ts);
	}
	puts("------------------------------------------");
	// compute mean of observed sigmas and ranges
	mSigma = (double)totSigma/div; mRange = (double)totRange/div;
	printf("Mean  :  %1.7f    %1.7f     %4d s\n",mRange,mSigma,tots);
	printf("Expect:  %1.7f    %1.7f          \n",rangeE,sigmaE);
	#ifdef VERBOSE
	printf("\nBits    P (actual)        P (expect)    \n");
	printf("------------------------------------------\n");
	for (i=0; i<=MODU; i++) 
	  printf("%3d)   %1.11f     %1.11f\n",values[i],prob[i],expect[i]);
	printf("-------------------------------------------\n");
	#endif
	#ifdef TEST
	printf("%1.11f\n",Chi2(0,MODU,sigma,sigmaE));
	printf("%1.11f\n",ChiSquare(0,MODU,prob,expect,FALSE));
	printf("%1.11f\n",ChiSquare(0,MODU,expect,expect,FALSE));
	#endif
	return 0;
}
