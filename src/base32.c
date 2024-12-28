#include "base32.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char ALPHABET[32] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                            'Y', 'Z', '2', '3', '4', '5', '6', '7'};

static bool in_the_alphabet(char c) {
  for (size_t i = 0; i < sizeof(ALPHABET); ++i) {
    if (ALPHABET[i] == c) {
      return true;
    }
  }

  return false;
}

typedef struct {
  unsigned char data[8];
  size_t bytes_consumed;
  uint8_t digits_produced;
} chunk;

// Try reading 8 digits (as chars) from the array and write the result to
// `*chunk`;
//
// This function will try reading the padding too but it does not do any
// validation.
static chunk read_chunk(char const* string, size_t length) {
  chunk chunk;
  uint8_t iDigit = 0;
  size_t i = 0;
  for (; iDigit < 8 && i < length && string[i]; ++i) {
    if (in_the_alphabet(string[i]) || string[i] == '=') {
      chunk.data[iDigit++] = string[i];
    }
  }

  chunk.digits_produced = iDigit;
  chunk.bytes_consumed = i;

  return chunk;
}

char* base32_encode(char* array, size_t size) {
  const size_t total_output_len = (size + 4) / 5 * 8;
  char* output = calloc(total_output_len + 1, 1);

  if (output == NULL) {
    perror("Memory couldn't be allocated");
    return NULL;
  }
  unsigned char last = 0;
  size_t offset = 0;
  size_t output_len = 0;
  size_t i = 0;
  unsigned char current = 0;

  while (i < size) {
    current = array[i];

    if (offset != 0) {
      // there are leftover bits from the last character, use them.
      size_t shiftForLast = 5 - offset;
      last &= 0x1F >> shiftForLast;
      size_t shiftForCurrent = 8 - (5 - offset);
      unsigned char maskForCurrent = 0xFF << shiftForCurrent;
      size_t lengthOfCurrentMask = 5 - offset;
      unsigned char out =
          (last << lengthOfCurrentMask) |
          ((unsigned char)(current & maskForCurrent) >> (shiftForCurrent));
      output[output_len++] = ALPHABET[out];
      offset = lengthOfCurrentMask;
    }

    if (offset < 4) {
      size_t shift = 3 - offset;
      unsigned char digit = (current & 0x1F << shift) >> shift;
      output[output_len++] = ALPHABET[digit];
      offset = shift;
    }
    // save the offset and the remaining bits
    last = current;
    offset %= 8;
    i++;
  }
  if (offset != 0) {
    unsigned char remaining = (last << (5 - offset)) & 0x1F;
    output[output_len++] = ALPHABET[remaining];
  }

  while (output_len % 8 != 0) {
    output[output_len++] = '=';
  }
  output[output_len] = '\0';
  return output;
}

char* base32_decode(char* string, size_t length, size_t* size_out) {
  static uint8_t inverse_table[256] = {0};
  for (size_t i = 0; i < sizeof(ALPHABET); ++i) {
    inverse_table[(unsigned char)ALPHABET[i]] = i;
  }

  char* out = calloc(length * 5 / 8, 1);
  if (out == NULL) {
    perror("Memory couldn't be allocated");
    return NULL;
  }

  size_t output_len = 0;
  for (size_t i = 0; i < length; i += 8) {
    chunk reader = read_chunk(string + i, length - i);

    unsigned char b32bytes[reader.digits_produced + 1];
    size_t valid_digits = 0;

    for (size_t j = 0; j < reader.digits_produced; j++) {
      if (reader.data[j] == '=') {
        break;  // Stop processing at the first padding.
      }
      b32bytes[valid_digits++] = inverse_table[reader.data[j]];
    }

    // Combine bits based on valid_digits
    if (valid_digits > 1) out[output_len++] = (b32bytes[0] << 3) | (b32bytes[1] >> 2);
    if (valid_digits > 3) out[output_len++] = (b32bytes[1] << 6) | (b32bytes[2] << 1) | (b32bytes[3] >> 4);
    if (valid_digits > 4) out[output_len++] = (b32bytes[3] << 4) | (b32bytes[4] >> 1);
    if (valid_digits > 6) out[output_len++] = (b32bytes[4] << 7) | (b32bytes[5] << 2) | (b32bytes[6] >> 3);
    if (valid_digits > 7) out[output_len++] = (b32bytes[6] << 5) | b32bytes[7];
  }

  *size_out = output_len;
  return out;
}
