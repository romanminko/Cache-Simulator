#include "cache.h"

int readCount = 0;
int writeCount = 0;
int invalidateCount = 0;
int missCount = 0;
int evictionCount = 0;
int writebackCount = 0;

int complete()
{
	printf("\n===========================================================\n");

	// Print parameters
	printf("\nCACHE PARAMETERS:");
	printf("\nNumber of sets: %d", SETS);
	printf("\nAssociativity: %d", SET_ASS);
	printf("\nCache line size: %d", CLINE_SIZE);

	if (REP_POLICY == 0)
	{
		printf("\nReplacement policy: True LRU\n");
	}
	else
	{
		printf("\nReplacement policy: 1-bit LRU\n");
	}

	// Print statistics
	int accessCount = (readCount + writeCount);
	int hitCount = ((readCount + writeCount) - missCount);
	int hitRatio = ((((float)hitCount) / ((float)(readCount + writeCount))) * 100.0);
	int missRatio = (100.0 - hitRatio);

	printf("\nCACHE STATISTICS:");
	printf("\nTotal number of cache accesses: %d", accessCount);
	printf("\nNumber of cache reads: %d", readCount);
	printf("\nNumber of cache writes: %d", writeCount);
	printf("\nNumber of invalidates: %d", invalidateCount);
	printf("\nNumber of cache hits: %d", hitCount);
	printf("\nNumber of cache misses: %d", missCount);
	printf("\nCache hit ratio: %.2f%%", hitRatio);
	printf("\nCache miss ratio: %.2f%%", missRatio);
	printf("\nNumber of evictions: %d", evictionCount);
	printf("\nNumber of writebacks: %d\n", writebackCount);
};
