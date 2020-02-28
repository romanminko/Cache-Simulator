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
#define LINE_SIZE 256
#define REP_POLICY 1

int parser(FILE *fp);
int breakup(char *line);
int switchInstruction(int instruct, int address);

int complete();
