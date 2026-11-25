#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "rules.h"
#include "saveload.h"
#include "terminal.h"

#define TEST_FILE "test_saveload.bin"

int main()
{
  /* CRC-32 gives the official check value. */
    assert(save_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u);
    
}