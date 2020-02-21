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
#define SETS 256
#define SET_ASS 8
#define CLINE_SIZE 64
#define REP_POLICY 1

int parser(FILE *fp);
int breakup(char *line);
int switchInstruction(int instruct, int address);

int complete();
