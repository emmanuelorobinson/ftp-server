#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_TOKENS 100
#define tokenSeparators " \t\n"

int tokenise(char inputLine[], char * token[]);