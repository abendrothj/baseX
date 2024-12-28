#ifndef BASE8_H
#define BASE8_H
#include <stddef.h>

// Encode given data in base 8.
//
// The returned buffer is heap-allocated.
//
char* base8_encode(char* array, size_t size);

// Decode given base 8 string.
//
// The returned buffer contains the decoded data, and is heap-allocated.
// `size_out` contains the size of the decoded data.
//
// If decoding fails due to input validation, the return value is NULL.
char* base8_decode(char* string, size_t length, size_t* size_out);
#endif // BASE8_H
