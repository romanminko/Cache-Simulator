#include "cache.h"

int cache[NUM_SETS][NUM_WAYS];
int readCount = 0;
int writeCount = 0;
int invalidateCount = 0;
int missCount = 0;
int evictionCount = 0;
int writebackCount = 0;

int parser(FILE *fp)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1)
	{
		if (!breakup(line))
		{
			return -1;
		}
	}
}

int breakup(char *line)
{
	char *token;
	int instruct[2];
	int address;
	int byteSelectBits;
	int indexBits;
	int tagBits;
	int index;
	int tag;

	token = strtok(line, " ");
	instruct[0] = (int)strtol(token, NULL, 16);

	token = strtok(NULL, " ");
	instruct[1] = (int)strtol(token, NULL, 16);
//	switchInstruction(instruct[0], instruct[1]);
	address = instruct[1];
	byteSelectBits = log2(LINE_SIZE);
	indexBits = log2(NUM_SETS/NUM_WAYS);
	tagBits = 32 - indexBits - byteSelectBits;
	printf("number of byte select bits: %d\n", byteSelectBits);
	printf("number of index bits: %d\n", indexBits);
	printf("number of tag bits: %d\n", tagBits);

	index = ((instruct[1] >> byteSelectBits) & (NUM_SETS/NUM_WAYS - 1));
	int tagMask = 1;
	for (int i = 0; i < tagBits; i++)
		tagMask = 2*tagMask;
	tag = ((instruct[1] >> indexBits + byteSelectBits) & (tagMask - 1));
	printf("address: %x\n", address);
	printf("index: %x\n", index);
	printf("tag: %x\n", tag);

	return 1;
}

int switchInstruction(int instruct, int address)
{
	switch (instruct)
	{
		// Read
		case 0:
			printf("\nInstruction: 0\n");

			break;

		// Write
		case 1:
			printf("\nInstruction: 1\n");

			break;

		// Invalidate
		case 2:
			printf("\nInstruction: 2\n");

			break;

		default:
			return -1;
			break;
	}
}

int complete()
{
	printf("\n===========================================================\n");

	// Print parameters
	printf("\nCACHE PARAMETERS:");
	printf("\nNumber of NUM_sets: %d", NUM_SETS);
	printf("\nAssociativity: %d", NUM_WAYS);
	printf("\nCache line size: %d", LINE_SIZE);

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
