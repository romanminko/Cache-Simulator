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
#define NUM_SETS 256
#define NUM_WAYS 1
#define LINE_SIZE 64
#define REP_POLICY 0

typedef struct cacheEntry {
	int tag;
	int valid;
  int dirty;
  int LRU;
} cacheEntry_t, *cacheEntryPtr_t;

int setup(FILE *fp, cacheEntryPtr_t cache[][NUM_WAYS]);
int breakup(char *line, cacheEntryPtr_t cache[][NUM_WAYS]);
int cacheRead(cacheEntryPtr_t cache[][NUM_WAYS]);
int cacheWrite(cacheEntryPtr_t cache[][NUM_WAYS]);
int cacheInvalidate(cacheEntryPtr_t cache[][NUM_WAYS]);
int oneBitLRU(cacheEntryPtr_t cache[][NUM_WAYS]);
int trueLRU(cacheEntryPtr_t cache[][NUM_WAYS]);
int complete();
