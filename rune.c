#include "rune.h"

#define E_EMPTY_INPUT "empty input"
#define E_INVALID_LENGTH "invalid length"
#define E_INVALID_CONTINUATION_BYTE "invalid continuation byte"
#define E_OVERLONG "overlong"
#define E_INVALID_UTF8 "invalid utf8"
#define E_TOO_BIG "too big"
#define E_SURROGATE_HALVES "surrogate halves"

DecodeResult decode_rune(const uint8_t* b, size_t len) {
    DecodeResult result = {0, 0, NULL};
    
    if (!b || len == 0) {
        result.error = E_EMPTY_INPUT;
        return result;
    }

    uint8_t b0 = b[0];

    // ascii
    if (b0 < 0x80) {
        if (len > 1 && (b[1] & 0xC0) == 0x80) {
            result.error = E_INVALID_LENGTH;
            return result;
        }
        result.r = (Rune)b0;
        result.size = 1;
        return result;
    }
    
    if ((b0 & 0xE0) == 0xC0) {  // 2 byte character
        if (len < 2) {
            result.error = E_INVALID_LENGTH;
            return result;
        }
        if (len > 2 && (b[2] & 0xC0) == 0x80) {
            result.error = E_INVALID_LENGTH;
            return result;
        }

        uint8_t b1 = b[1];
        if ((b1 & 0xC0) != 0x80) {
            result.error = E_INVALID_CONTINUATION_BYTE;
            return result;
        }

        result.size = 2;
        result.r = ((Rune)(b0 & 0x1F) << 6) |
                   ((Rune)(b1 & 0x3F));

        if (result.r < 0x80) {
            result.error = E_OVERLONG;
            return result;
        }
    } else if ((b0 & 0xF0) == 0xE0) {  // 3 byte character
        if (len < 3) {
            result.error = E_INVALID_LENGTH;
            return result;
        }
        if (len > 3 && (b[3] & 0xC0) == 0x80) {
            result.error = E_INVALID_LENGTH;
            return result;
        }

        uint8_t b1 = b[1];
        uint8_t b2 = b[2];

        if (b0 == 0xE0 && b1 < 0xA0) {
            result.error = E_OVERLONG;
            return result;
        }

        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) {
            result.error = E_INVALID_CONTINUATION_BYTE;
            return result;
        }

        result.size = 3;
        result.r = ((Rune)(b0 & 0x0F) << 12) |
                   ((Rune)(b1 & 0x3F) << 6) |
                   ((Rune)(b2 & 0x3F));
    } else if ((b0 & 0xF8) == 0xF0) {  // 4 byte character
        if (len < 4) {
            result.error = E_INVALID_LENGTH;
            return result;
        }
        if (len > 4 && (b[4] & 0xC0) == 0x80) {
            result.error = E_INVALID_LENGTH;
            return result;
        }

        uint8_t b1 = b[1];
        uint8_t b2 = b[2];
        uint8_t b3 = b[3];

        if (b0 == 0xF0 && b1 < 0x90) {
            result.error = E_OVERLONG;
            return result;
        }

        if ((b1 & 0xC0) != 0x80 || 
            (b2 & 0xC0) != 0x80 || 
            (b3 & 0xC0) != 0x80) {
            result.error = E_INVALID_CONTINUATION_BYTE;
            return result;
        }

        result.size = 4;
        result.r = ((Rune)(b0 & 0x07) << 18) |
                   ((Rune)(b1 & 0x3F) << 12) |
                   ((Rune)(b2 & 0x3F) << 6) |
                   ((Rune)(b3 & 0x3F));
    } else {
        result.error = E_INVALID_UTF8;
        return result;
    }

    // check surrogate halves and too big values
    if (result.r >= 0xD800 && result.r <= 0xDFFF) {
        result.error = E_SURROGATE_HALVES;
        return result;
    }
    if (result.r > 0x10FFFF) {
        result.error = E_TOO_BIG;
        return result;
    }

    return result;
}