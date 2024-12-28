#include "base8.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
//
static chunk read_chunk(char const *string, size_t length) {
  chunk chunk;
  uint8_t iDigit = 0;
  size_t i = 0;
  for (; iDigit < 8 && i < length && string[i]; ++i) {
    if (('0' <= string[i] && string[i] <= '7') || string[i] == '=') {
      chunk.data[iDigit++] = string[i];
    }
  }

  chunk.digits_produced = iDigit;
  chunk.bytes_consumed = i;

  return chunk;
}

char const ALPHABET[8] = {'0', '1', '2', '3', '4', '5', '6', '7'};

char *base8_encode(char *array, size_t size) {
  const size_t total_output_len = (size + 2) / 3 * 8;
  char *output = calloc(total_output_len + 1, 1);
  if (output == NULL) {
    return NULL;
  }
  unsigned char last = 0;
  size_t offset = 0;
  size_t output_len = 0;
  unsigned char current = 0;

  for (size_t i = 0; i < size; i++) {
    current = array[i];
    if (offset > 0) {
      // there are leftover bits from the last character, use them.
      size_t shiftForLast = 3 - offset;
      last &= 0x07 >> shiftForLast;
      size_t shiftForCurrent = 8 - (3 - offset);
      unsigned char maskForCurrent = 0x07 << shiftForCurrent;
      size_t lengthOfCurrentMask = 3 - offset;
      unsigned char out =
          (last << lengthOfCurrentMask) |
          ((unsigned char)(current & maskForCurrent) >> (shiftForCurrent));
      output[output_len++] = ALPHABET[out];
      offset = 3 - offset;
    }
    while ((offset + 3) <= 8) {
      unsigned char out = (current >> (5 - offset)) & 0x07;
      output[output_len++] = ALPHABET[out];
      if (8 - offset >= 3) {
        offset += 3;
      }
    }
    if (offset >= 6) {
      offset = 8 - offset;
    }
    // save the offset and the remaining bits
    last = current;
    offset %= 8;
  }
  if (offset > 0) {
    unsigned char remaining = (last << (3 - offset)) & 0x07;
    output[output_len++] = ALPHABET[remaining];
  }

  while (output_len % 8 != 0) {
    output[output_len++] = '=';
  }
  output[output_len] = '\0';
  return output;
}

char *base8_decode(char *string, size_t length, size_t *size_out) {
  static uint8_t inverse_table[256] = {0};
  for (size_t i = 0; i < sizeof(ALPHABET); ++i) {
    inverse_table[(unsigned char)ALPHABET[i]] = i;
  }
  char *out = calloc(length * 3 / 8, 1);
  if (out == NULL) {
    return NULL;
  }
  size_t output_len = 0;
  for (size_t i = 0; i < length; i += 8) {
    chunk reader = read_chunk(string + i, length - i);
    unsigned char base8[reader.digits_produced + 1];
    for (size_t j = 0; j < reader.digits_produced; j++) {
      if (base8[j] != '=') {
        base8[j] = inverse_table[reader.data[j]];
      } else {
        break;
      }
    }

    if (reader.digits_produced > 2) {
      out[output_len++] = (base8[0] << 5 | base8[1] << 2 | base8[2] >> 1);
    }
    if (reader.digits_produced > 5 && reader.data[3] != '=' &&
        reader.data[4] != '=' && reader.data[5] != '=') {
      out[output_len++] = ((base8[2] & 0x1) << 7 | base8[3] << 4 |
                           base8[4] << 1 | base8[5] >> 2);
    }
    if (reader.digits_produced > 7 && reader.data[5] != '=' &&
        reader.data[6] != '=' && reader.data[7] != '=') {
      out[output_len++] = ((base8[5] & 0x3) << 6 | base8[6] << 3 | base8[7]);
    }
  }
  out[output_len] = '\0';
  *size_out = output_len;
  return out;
}