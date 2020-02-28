#include "cache.h"

int readCount = 0;
int writeCount = 0;
int invalidateCount = 0;
int hitCount = 0;
int missCount = 0;
int evictionCount = 0;
int writebackCount = 0;
int byteSelectBits;
int indexBits;
int tagBits;

// cacheEntryPtr_t* createCache() {
// 	static cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS];
// 	for (int i = 0; i < NUM_SETS; i++) {
// 		for (int k = 0; k < NUM_WAYS; k++) {
// 			cache[i][k] = (cacheEntryPtr_t) malloc(sizeof(cacheEntry_t));
// 			if (!cache[i][k]) {
// 				printf("malloc failed\n");
// 			}
// 			printf("valid: %d\ndirty %d\nLRU %d\ntag %d\nindex %d\n", cache[i][k]->valid, cache[i][k]->dirty, cache[i][k]->LRU, cache[i][k]->tag, cache[i][k]->index);
// 		}
// 	}
// 	byteSelectBits = log2(LINE_SIZE);
// 	indexBits = log2(NUM_SETS);
// 	tagBits = 32 - indexBits - byteSelectBits;
// 	printf("number of byte select bits: %d\n", byteSelectBits);
// 	printf("number of index bits: %d\n", indexBits);
// 	printf("number of tag bits: %d\n", tagBits);
// 	return cache;
// }

int parser(FILE *fp) {
	static cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS];
	for (int i = 0; i < NUM_SETS; i++) {
		for (int k = 0; k < NUM_WAYS; k++) {
			cache[i][k] = (cacheEntryPtr_t) malloc(sizeof(cacheEntry_t));
			if (!cache[i][k]) {
				printf("malloc failed\n");
			}
		}
	}
	byteSelectBits = log2(LINE_SIZE);
	indexBits = log2(NUM_SETS);
	tagBits = 32 - indexBits - byteSelectBits;
	printf("number of byte select bits: %d\n", byteSelectBits);
	printf("number of index bits: %d\n", indexBits);
	printf("number of tag bits: %d\n", tagBits);

	for (int i = 0; i < NUM_SETS; i++) {
		for (int k = 0; k < NUM_WAYS; k++) {
		}
	}
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		if (!breakup(line, cache))
		{
			return -1;
		}
	}
}

int breakup(char *line, cacheEntryPtr_t cache[][NUM_WAYS]) {
	char *token;
	int instruct;
	int address;

	int index;
	int tag;

	token = strtok(line, " ");
	instruct = (int)strtol(token, NULL, 16);

	token = strtok(NULL, " ");
	address = (int)strtol(token, NULL, 16);

	cacheEntryPtr_t cacheEntry = (cacheEntryPtr_t) malloc(sizeof(cacheEntry_t));
	if (!cacheEntry) {
		printf("Malloc failed\n");
		return -1;
	}

	int indexMask = NUM_SETS - 1;
	cacheEntry->index = ((address >> byteSelectBits) & indexMask);
	int tagMask = 1;
	for (int i = 0; i < tagBits; i++)	// 2^tagBits
		tagMask = 2*tagMask;
	tagMask = tagMask - 1;
	cacheEntry->tag = ((address >> indexBits + byteSelectBits) & tagMask);
	cacheEntry->valid = 1;

	if (instruct == 0) {
		cacheRead(cache, cacheEntry);
	}
	else if (instruct == 1) {
		cacheWrite(cache, cacheEntry);
	}
	else
		cacheInvalidate(cache, cacheEntry);

	return 1;
}

int cacheRead(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], cacheEntryPtr_t cacheEntry) {
	readCount++;
	// look for hit
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[cacheEntry->index][k]->valid == 1 && cache[cacheEntry->index][k]->tag == cacheEntry->tag) {
			//if (cache[cacheEntry->index][k]->tag == cacheEntry->tag) {
				hitCount++;
				if (REP_POLICY == 1)
					cache[cacheEntry->index][k]->LRU = 1;
				// ELSE TRUE LRU
				return 0;
			//}
		}
	}
	// if miss
	missCount++;
	// check for invalid line to store new entry in
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[cacheEntry->index][k]->valid == 0) {
			cache[cacheEntry->index][k]->tag = cacheEntry->tag;
			cache[cacheEntry->index][k]->valid = 1;
			if (REP_POLICY == 1)
				cache[cacheEntry->index][k]->LRU = 1;
			// ELSE TRUE LRU
			return 0;
		}
	}
	// if no open lines, use replacement policy
	if (REP_POLICY == 1) {
			oneBitLRU(cache, cacheEntry);
	}
	else {
		// call TRUE LRU function
	}
	return 0;
}

int cacheWrite(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], cacheEntryPtr_t cacheEntry) {
	writeCount++;
	// check for cache hit
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[cacheEntry->index][k]->valid == 1 && cache[cacheEntry->index][k]->tag == cacheEntry->tag) {
				hitCount++;
				cache[cacheEntry->index][k]->dirty = 1;
				if (REP_POLICY == 1)
					cache[cacheEntry->index][k]->LRU = 1;
				// ELSE TRUE LRU
				return 0;
		}
	}
	// if miss
	missCount++;
	// check for invalid line to store new entry
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[cacheEntry->index][k]->valid == 0) {
			cache[cacheEntry->index][k]->tag = cacheEntry->tag;
			cache[cacheEntry->index][k]->valid = 1;
			cache[cacheEntry->index][k]->dirty = 1;
			if (REP_POLICY == 1)
				cache[cacheEntry->index][k]->LRU = 1;
			// ELSE TRUE LRU
			return 0;
		}
	}
	// if no open lines, use replacement policy
	if (REP_POLICY == 1) {
			oneBitLRU(cache, cacheEntry);
	}
	else {
		// call TRUE LRU function
	}
	return 0;
}

int cacheInvalidate(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], cacheEntryPtr_t cacheEntry) {
	return 0;
}

int oneBitLRU(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], cacheEntryPtr_t cacheEntry) {
	// check if all LRU bits in the set are high
	int LRUcheck = 0;
	for (int k = 0; k < NUM_WAYS; k++) {
		printf("LRU[%d]: %d\n", k, cache[cacheEntry->index][k]->LRU);
		if (cache[cacheEntry->index][k]->LRU == 1) {
			LRUcheck++;
		}
	}
	printf("LRUcheck: %d\n", LRUcheck);
	// if all LRU bits are high, reset all LRU bits in the set
	if (LRUcheck == NUM_WAYS) {
		for (int k = 0; k < NUM_WAYS; k++) {
			cache[cacheEntry->index][k]->LRU = 0;
		}
	}
	// evict first entry whose LRU is 0
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[cacheEntry->index][k]->LRU == 0) {
			cache[cacheEntry->index][k]->tag = cacheEntry->tag;
			cache[cacheEntry->index][k]->LRU = 1;
			evictionCount++;
			printf("WB DIRTY: %d\n", cache[cacheEntry->index][k]->dirty);
			if (cache[cacheEntry->index][k]->dirty == 1) {
				writebackCount++;
			}
			return 0;
		}
	}
}



int complete() {
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
	int accessCount = (readCount + writeCount + invalidateCount);
	float hitRatio = (((float)(hitCount) / ((float)(readCount + writeCount))) * 100.0);
	float missRatio = (100.0 - hitRatio);

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
}
