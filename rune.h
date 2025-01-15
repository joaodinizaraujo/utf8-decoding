#ifndef RUNE_H
#define RUNE_H

#include <stdint.h>
#include <stddef.h>

typedef int32_t Rune;
typedef struct {
    Rune r;
    int size;
    const char* error;
} DecodeResult;

DecodeResult decode_rune(const uint8_t* b, size_t len);

#endif  // RUNE_H