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
	cacheEntryPtr_t* cache = createCache();
	parser(fp, cache);
	fclose(fp);
	complete();

	return 0;
}
