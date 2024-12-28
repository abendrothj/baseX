#include "base64.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char ALPHABET[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static bool in_the_alphabet(char c) {
  for (size_t i = 0; i < sizeof(ALPHABET); ++i) {
    if (ALPHABET[i] == c) {
      return true;
    }
  }
  return false;
}

typedef struct {
  unsigned char data[4];
  size_t bytes_consumed;
  uint8_t digits_produced;
} chunk;

// Try reading 4 digits (as chars) from the array and write the result to
// `*chunk`;
//
// This function will try reading the padding too but it does not do any
// validation.
static chunk read_chunk(char const* string, size_t length) {
  chunk chunk;
  uint8_t iDigit = 0;
  size_t i = 0;
  for (; iDigit < 4 && i < length && string[i]; ++i) {
    if (in_the_alphabet(string[i]) || string[i] == '=') {
      chunk.data[iDigit++] = string[i];
    }
  }

  chunk.digits_produced = iDigit;
  chunk.bytes_consumed = i;

  return chunk;
}

char* base64_encode(char* array, size_t size) {
  const size_t total_output_len = (size + 2) / 3 * 4;
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
      size_t shiftForLast = 4 - offset;
      last &= 0xF >> shiftForLast;
      size_t shiftForCurrent = 8 - (6 - offset);
      unsigned char maskForCurrent = 0xF << shiftForCurrent;
      size_t lengthOfCurrentMask = 6 - offset;
      unsigned char out =
          (last << lengthOfCurrentMask) |
          ((unsigned char)(current & maskForCurrent) >> (shiftForCurrent));
      output[output_len++] = ALPHABET[out];
      offset = lengthOfCurrentMask;
    }
    // Now, we read up to `offset` bits from `buf`.  We may be able to read 1
    // more sextet.
    if (offset < 3) {
      // We can read another base 64 digit
      size_t shift = 2 - offset;
      unsigned char digit = (current & 0x3F << shift) >> shift;
      output[output_len++] = ALPHABET[digit];
      offset = (offset == 2) ? 0 : (offset + 2);
    }
    // save the offset and the remaining bits
    last = current;
    offset %= 8;
    i++;
  }
  if (offset != 0) {
    unsigned char remaining = (last << (6 - offset)) & 0x3F;
    output[output_len++] = ALPHABET[remaining];
  }

  while (output_len % 4 != 0) {
    output[output_len++] = '=';
  }
  output[output_len] = '\0';
  return output;
}

char* base64_decode(char* string, size_t length, size_t* size_out) {
  static uint8_t inverse_table[256] = {0};
  for (size_t i = 0; i < sizeof(ALPHABET); ++i) {
    inverse_table[(unsigned char)ALPHABET[i]] = i;
  }
  size_t out_size = (length * 3) / 4;

  char* out = calloc(out_size + 1, 1);
  if (out == NULL) {
    perror("Memory couldn't be allocated");
    return NULL;
  }
  size_t output_len = 0;
  for (size_t i = 0; i < length; i += 4) {
    chunk reader = read_chunk(string + i, length - i);
    unsigned char b64byte0 = inverse_table[reader.data[0]];
    unsigned char b64byte1 =
        (reader.digits_produced > 1) ? inverse_table[reader.data[1]] : 0;
    unsigned char b64byte2 =
        (reader.digits_produced > 2 && reader.data[2] != '=')
            ? inverse_table[reader.data[2]]
            : 0;
    unsigned char b64byte3 =
        (reader.digits_produced > 3 && reader.data[3] != '=')
            ? inverse_table[reader.data[3]]
            : 0;
    unsigned char byte0 = (b64byte0 << 2) | (b64byte1 >> 4);
    out[output_len++] = byte0;

    if (reader.digits_produced > 2 && reader.data[2] != '=') {
      unsigned char byte1 = (b64byte1 << 4) | (b64byte2 >> 2);
      out[output_len++] = byte1;
    }
    if (reader.digits_produced > 3 && reader.data[3] != '=') {
      unsigned char byte2 = (b64byte2 << 6) | b64byte3;
      out[output_len++] = byte2;
    }
  }
  *size_out = output_len;
  return out;
}
