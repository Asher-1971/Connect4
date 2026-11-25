#include <stdio.h>
#include <string.h>

#include "saveload.h"
#include "rules.h"

#define SAVE_MAGIC    "C4SV"
#define SAVE_VERSION  3
#define HEADER_SIZE   31   /* every field before the board */
#define MAX_FILE_SIZE (HEADER_SIZE + MAX_SIZE * MAX_SIZE + 4)

/* CRC-32 (the ZIP/PNG standard): long division of the data by the
 polynomial 0xEDB88320, using XOR instead of subtraction.
 The remainder is the checksum. */
uint32_t save_crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            }
            else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}
