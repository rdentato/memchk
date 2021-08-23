#include <stdio.h>
#include <stdlib.h>
#include "memchk.h"

int main(int argc, char *argv[])
{
  char *s;

  s = malloc(10);
  // s[10] = '\0'; // Buffer overflow!
  free(s);
}