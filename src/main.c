#include "libs/funcs.c"
#include "libs/types.h"
#include <stdio.h>
#include <string.h>
#include <libs/cJSON.h>
#include <libs/types.h>

int main(int argc, char* argv[])
{
      // arg 0 is the app q
      if (argc > 1 && strcmp(argv[1], "-help") == 0) {
            printf("-help \n");
            printf("-S <name>\n");
            printf("-Syu update app\n");
      }
}
