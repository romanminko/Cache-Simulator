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
#define NUM_SETS 4096
#define NUM_WAYS 1
#define LINE_SIZE 64
#define REP_POLICY 1

typedef struct cacheEntry {
	int tag;
	int valid;
  int dirty;
  int LRU;
} cacheEntry_t, *cacheEntryPtr_t;

int setup(FILE *fp, cacheEntryPtr_t cache[][NUM_WAYS]);
int breakup(char *line, cacheEntryPtr_t cache[][NUM_WAYS]);
int cacheRWI(cacheEntryPtr_t cache[][NUM_WAYS], int instruct);
int oneBitEvict(cacheEntryPtr_t cache[][NUM_WAYS], int instruct);
int trueLRUEvict(cacheEntryPtr_t cache[][NUM_WAYS], int instruct);
int complete();
