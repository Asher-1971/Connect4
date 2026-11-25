#include <stdio.h>
#include <string.h>

#include "saveload.h"
#include "rules.h"

#define SAVE_MAGIC    "C4SV"
#define SAVE_VERSION  3
#define HEADER_SIZE   31   /* every field before the board */
#define MAX_FILE_SIZE (HEADER_SIZE + MAX_SIZE * MAX_SIZE + 4)

