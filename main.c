/*
Program Name:		Single Level Cache Simulator
Programmers:		Roman Minko, Tyler Hull, Andrew Denzer
Date:			2/20/2020
Purpose:		Final project for ECE 485/585
*/

#include "cache.h"

int main(int argc, char *argv[])
{
	printf("starting\n\n");
	FILE *fp;

	// Open trace file
	fp = fopen(argv[1], "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	// Parse file and do all operations
//	cacheEntryPtr_t* cache = createCache();

cacheEntryPtr_t cache[NUM_SETS][NUM_WAYS];
for (int i = 0; i < NUM_SETS; i++) {
	for (int k = 0; k < NUM_WAYS; k++) {
		cache[i][k] = (cacheEntryPtr_t) malloc(sizeof(cacheEntry_t));
		if (!cache[i][k]) {
			printf("malloc failed\n");
		}
	}
}

	setup(fp, cache);
	fclose(fp);
	complete();

	return 0;
}
