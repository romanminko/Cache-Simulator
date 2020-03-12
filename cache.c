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

  cacheRWI(cache, instruct);

	return 1;
}


int cacheRWI(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], int instruct) {
  switch (instruct) {
    case 0 : readCount++; break;
    case 1 : writeCount++; break;
    case 2 : invalidateCount++; break;
  }

  // check for cache hit
	for (int k = 0; k < NUM_WAYS; k++) {

    // hit found
		if (cache[newIndex][k]->valid == 1 && cache[newIndex][k]->tag == newTag) {

      // invalidate hit
      if (instruct == 2) {

        // writeback if dirty
        if (cache[newIndex][k]->dirty == 1) {
          writebackCount++;
        }

        cache[newIndex][k]->valid = 0;
        cache[newIndex][k]->tag = 0;
        cache[newIndex][k]->dirty = 0;

        // update true LRU
        if (REP_POLICY == 0) {
          for (int i = 0; i < NUM_WAYS; i++) {
            if ((cache[newIndex][i]->valid == 1) && (cache[newIndex][i]->LRU > cache[newIndex][k]->LRU))
              cache[newIndex][i]->LRU--;
          }
        }

        cache[newIndex][k]->LRU = 0;

        return 0;
      } // end invalidate hit

      hitCount++;

      // if write, set dirty bit
      if (instruct == 1)
			    cache[newIndex][k]->dirty = 1;

			// update 1-bit LRU
			if (REP_POLICY == 1)
				cache[newIndex][k]->LRU = 1;

      // update true LRU
      if (REP_POLICY == 0) {
				for (int i = 0; i < NUM_WAYS; i++) {
					if ((cache[newIndex][i]->valid == 1) && (cache[newIndex][i]->LRU < cache[newIndex][k]->LRU))
						cache[newIndex][i]->LRU++;
				}
				cache[newIndex][k]->LRU = 0;
			}

			return 0;
    }
	}

  // invalidate miss
  if (instruct == 2)
    return 0;

  // cache miss
  missCount++;

	// check for invalid line to store new entry
	for (int k = 0; k < NUM_WAYS; k++) {

    // if invalid line found
		if (cache[newIndex][k]->valid == 0) {
			cache[newIndex][k]->tag = newTag;
			cache[newIndex][k]->valid = 1;

      // if write, set dirty bit
      if (instruct == 1)
			   cache[newIndex][k]->dirty = 1;

			// update 1-bit LRU
			if (REP_POLICY == 1) {
        cache[newIndex][k]->LRU = 1;
			}

      // update true LRU
      if (REP_POLICY == 0) {
				for (int i = 0; i < NUM_WAYS; i++) {
					if (cache[newIndex][i]->valid == 1)
						cache[newIndex][i]->LRU++;
				}
				cache[newIndex][k]->LRU = 0;
			}

			return 0;
		}
	} // end invalid check

  // if miss and no open lines, evict appropriate line
  if (REP_POLICY == 1)
    oneBitEvict(cache, instruct);

  if (REP_POLICY == 0)
    trueLRUEvict(cache, instruct);

} // end cacheRWI

int oneBitEvict(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], int instruct) {

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

	// find first entry whose LRU is 0 and update LRU bit to 1
	for (int k = 0; k < NUM_WAYS; k++) {
		if (cache[newIndex][k]->LRU == 0) {

				cache[newIndex][k]->LRU = 1;

				cache[newIndex][k]->tag = newTag;

				evictionCount++;

				// check dirty bit and writeback if it is high
				if (cache[newIndex][k]->dirty == 1) {
					writebackCount++;
				}

        // update dirty bit appropriately
        if (instruct == 0)
          cache[newIndex][k]->dirty = 0;
        if (instruct == 1)
          cache[newIndex][k]->dirty = 1;

			return 0;
		}
	}
	printf("1-bit LRU Error\n");
} // end oneBitEvict

int trueLRUEvict(cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS], int instruct) {

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

      // writeback if dirty
			if (cache[newIndex][k]->dirty == 1) {
				writebackCount++;
			}

      // update dirty bit appropriately
			if (instruct == 0)
				cache[newIndex][k]->dirty = 0;
			if (instruct == 1)
				cache[newIndex][k]->dirty = 1;

			return 0;
		}
	}
	printf("True LRU Error\n");
} // end trueLRUEvict


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
