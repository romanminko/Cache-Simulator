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
int newTag;
int newIndex;
int writeFlag = 0;
int evictFlag = 0;
int hitFlag = 0;


int setup(FILE *fp, cacheEntryPtr_t cache[][NUM_WAYS]) {
	byteSelectBits = log2(LINE_SIZE);
	indexBits = log2(NUM_SETS);
	tagBits = 32 - indexBits - byteSelectBits;
 	printf("number of byte select bits: %d\n", byteSelectBits);
	printf("number of index bits: %d\n", indexBits);
	printf("number of tag bits: %d\n", tagBits);

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

	// breakup line into instruction and address
	token = strtok(line, " ");
	instruct = (int)strtol(token, NULL, 16);

	token = strtok(NULL, " ");
	address = (int)strtol(token, NULL, 16);

	// breakup address into index and tag
	int indexMask = NUM_SETS - 1;
	newIndex = ((address >> byteSelectBits) & indexMask);
	int tagMask = 1;
	for (int i = 0; i < tagBits; i++)	// 2^tagBits
		tagMask = 2*tagMask;
	tagMask = tagMask - 1;
	newTag = ((address >> indexBits + byteSelectBits) & tagMask);

	printf("New Index: %d, New tag: %x, %d\n", newIndex, newTag, instruct);

		printf("Set %d before\n", newIndex);
		for (int k = 0; k < NUM_WAYS; k++) {
			printf("	Way %d\n", k);
			printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
		}

	if (instruct == 0) {
		cacheRead(cache);
	}
	else if (instruct == 1) {
		cacheWrite(cache);
	}
	else
		cacheInvalidate(cache);

	return 1;
}

int cacheRead(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS]) {
	readCount++;
	writeFlag = 0;
	// look for hit
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->valid == 1 && cache[newIndex][k]->tag == newTag) {
				hitCount++;
				hitFlag = 1;
				// adjust LRU bit(s)

				if (REP_POLICY == 1) {
					cache[newIndex][k]->LRU = 1;
					evictFlag = 0;
				//	oneBitLRU(cache);
				}

				if (REP_POLICY == 0) {
					for (int i = 0; i < NUM_WAYS; i++) {
						if ((cache[newIndex][i]->valid == 1) && (cache[newIndex][i]->LRU < cache[newIndex][k]->LRU))
							cache[newIndex][i]->LRU++;
					}
					cache[newIndex][k]->LRU = 0;
				}

				printf("READ HIT\n");
				printf("Set %d after\n", newIndex);
				for (int k = 0; k < NUM_WAYS; k++) {
				 	printf("	Way %d\n", k);
				 	printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
				 }
				return 0;
		}
	}

	// if cache miss
	missCount++;
	hitFlag = 0;
	// check for invalid line to store new entry in
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->valid == 0) {
			cache[newIndex][k]->tag = newTag;
			cache[newIndex][k]->valid = 1;

			// adjust LRU bit(s)
			if (REP_POLICY == 1) {
				evictFlag = 0;
				oneBitLRU(cache);
			}
			if (REP_POLICY == 0) {
				for (int i = 0; i < NUM_WAYS; i++) {
					if (cache[newIndex][i]->valid == 1)
						cache[newIndex][i]->LRU++;
				}
				cache[newIndex][k]->LRU = 0;
			}

			printf("READ MISS (Line available)\n");
			printf("Set %d after\n", newIndex);
			for (int k = 0; k < NUM_WAYS; k++) {
				printf("	Way %d\n", k);
				printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
			}
			return 0;
		}
	}
	// if miss and no open lines, use replacement policy
	if (REP_POLICY == 1) {
			printf("READ MISS (LRU)\n");
			evictFlag = 1;
			oneBitLRU(cache);
	}
	if (REP_POLICY == 0) {
		printf("READ MISS (LRU)\n");
		trueLRU(cache);
	}
	printf("Set %d after\n", newIndex);
	for (int k = 0; k < NUM_WAYS; k++) {
	printf("	Way %d\n", k);
	printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
	}
	return 0;
}


int cacheWrite(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS]) {
	writeCount++;
	writeFlag = 1;
	// check for cache hit
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->valid == 1 && cache[newIndex][k]->tag == newTag) {
				hitCount++;
				hitFlag = 1;
				cache[newIndex][k]->dirty = 1;

				// adjust LRU bit(s)

				if (REP_POLICY == 1) {
					cache[newIndex][k]->LRU = 1;
					evictFlag = 0;
					//oneBitLRU(cache);
				}
				if (REP_POLICY == 0) {
					for (int i = 0; i < NUM_WAYS; i++) {
						if ((cache[newIndex][i]->valid == 1) && (cache[newIndex][i]->LRU < cache[newIndex][k]->LRU))
							cache[newIndex][i]->LRU++;
					}
					cache[newIndex][k]->LRU = 0;
				}

				printf("WRITE HIT\n");
				printf("Set %d after\n", newIndex);
				for (int k = 0; k < NUM_WAYS; k++) {
					printf("	Way %d\n", k);
					printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
				}
				return 0;
		}
	}

	// if cache miss
	missCount++;
	hitFlag = 0;
	// check for invalid line to store new entry
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->valid == 0) {
			cache[newIndex][k]->tag = newTag;
			cache[newIndex][k]->valid = 1;
			cache[newIndex][k]->dirty = 1;

			// replacement policy
			if (REP_POLICY == 1) {
				evictFlag = 0;
				oneBitLRU(cache);
			}
			if (REP_POLICY == 0) {
				for (int i = 0; i < NUM_WAYS; i++) {
					if (cache[newIndex][i]->valid == 1)
						cache[newIndex][i]->LRU++;
				}
				cache[newIndex][k]->LRU = 0;
			}

			printf("WRITE MISS (Line available)\n");
			printf("Set %d after\n", newIndex);
			for (int k = 0; k < NUM_WAYS; k++) {
				printf("	Way %d\n", k);
				printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
			}
			return 0;
		}
	}
	// if miss and no open lines, use replacement policy
	if (REP_POLICY == 1) {
		  printf("WRITE MISS (LRU)\n");
			evictFlag = 1;
			oneBitLRU(cache);
	}
	if (REP_POLICY == 0) {
		printf("WRITE MISS (LRU)\n");
		trueLRU(cache);
	}
	printf("Set %d after\n", newIndex);
	for (int k = 0; k < NUM_WAYS; k++) {
	printf("	Way %d\n", k);
	printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
  }
	return 0;
}

int cacheInvalidate(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS]) {
	invalidateCount++;
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->valid == 1 && cache[newIndex][k]->tag == newTag) {

				if (cache[newIndex][k]->dirty == 1) {
					writebackCount++;
					printf("Invalidate Writeback\n");
				}
				cache[newIndex][k]->valid = 0;
				cache[newIndex][k]->tag = 0;
				cache[newIndex][k]->dirty = 0;

				// adjust True LRU
				if (REP_POLICY == 0) {
					for (int i = 0; i < NUM_WAYS; i++) {
						if ((cache[newIndex][i]->valid == 1) && (cache[newIndex][i]->LRU > cache[newIndex][k]->LRU))
							cache[newIndex][i]->LRU--;
					}
					//cache[newIndex][k]->LRU = 0;
				}

				cache[newIndex][k]->LRU = 0;

				printf("Invalidate HIT\n");
				printf("Set %d after\n", newIndex);
				for (int k = 0; k < NUM_WAYS; k++) {
					printf("	Way %d\n", k);
					printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
				}
				return 0;
		 }
	}
	printf("INVALIDATE MISS\n");
	printf("Set %d after\n", newIndex);
	for (int k = 0; k < NUM_WAYS; k++) {
		printf("	Way %d\n", k);
		printf("	Tag: %x, Valid: %d, Dirty: %d, LRU: %d\n", cache[newIndex][k]->tag, cache[newIndex][k]->valid, cache[newIndex][k]->dirty, cache[newIndex][k]->LRU);
	}
	return 0;
}

int oneBitLRU(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS]) {

	if (hitFlag == 0) {
		// check if all LRU bits in the set are high
		int LRUcheck = 0;
		for (int k = 0; k < NUM_WAYS; k++) {
			if (cache[newIndex][k]->LRU == 1) {
				LRUcheck++;
			}
		}
		// if all LRU bits are high, reset all LRU bits in the set
		if (LRUcheck == NUM_WAYS) {
			for (int k = 0; k < NUM_WAYS; k++) {
				cache[newIndex][k]->LRU = 0;
			}
		}
	}

	// find first entry whose LRU is 0 and update LRU bit to 1
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->LRU == 0) {

//			if (hitFlag == 0)
				cache[newIndex][k]->LRU = 1;

			// if eviction needed, replace tag
			if (evictFlag) {
				cache[newIndex][k]->tag = newTag;

				evictionCount++;

				// check dirty bit and writeback if it is high
				if (cache[newIndex][k]->dirty == 1)
					printf("LRU Writeback\n");
					writebackCount++;

				// update dirty bit appropriately
				if (writeFlag == 0)
					cache[newIndex][k]->dirty = 0;
				else
					cache[newIndex][k]->dirty = 1;
			}

			return 0;
		}
	}
	printf("1-bit LRU Error\n");
}

int trueLRU(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS]) {


	// evict entry with highest possible LRU value
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->LRU == NUM_WAYS - 1) {
			cache[newIndex][k]->tag = newTag;
			cache[newIndex][k]->LRU = 0;
			for (int i = 0; i < NUM_WAYS; i++) {
				if (i != k)
					cache[newIndex][i]->LRU++;
			}

			evictionCount++;

			if (cache[newIndex][k]->dirty == 1)
				writebackCount++;
				printf("Writeback\n");

			if (writeFlag == 0)
				cache[newIndex][k]->dirty = 0;
			else
				cache[newIndex][k]->dirty = 1;

			return 0;
		}
	}
	printf("True LRU Error\n");
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
