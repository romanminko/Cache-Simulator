#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

//parameters given
#define NUM_SETS 8
#define NUM_WAYS 1
#define LINE_SIZE 64
#define REP_POLICY 1

typedef struct cacheEntry {
	int tag;
	int index;
	int valid;
  int dirty;
  int LRU;
} cacheEntry_t, *cacheEntryPtr_t;

cacheEntryPtr_t* createCache();
int parser(FILE *fp);
int breakup(char *line, cacheEntryPtr_t *cache);
int cacheRead(cacheEntryPtr_t *cache, cacheEntryPtr_t cacheEntry);
int cacheWrite(cacheEntryPtr_t *cache, cacheEntryPtr_t cacheEntry);
int cacheInvalidate(cacheEntryPtr_t *cache, cacheEntryPtr_t cacheEntry);
int complete();
