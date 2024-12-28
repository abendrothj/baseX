#ifndef BASE32_H
#define BASE32_H
#include <stddef.h>

// Encode given data in base 32.
//
// The returned buffer is heap-allocated.
//
char* base32_encode(char* array, size_t size);

// Decode given base 32 string.
//
// The returned buffer contains the decoded data, and is heap-allocated.
// `size_out` contains the size of the decoded data.
//
// If decoding fails due to input validation, the return value will be NULL.
char* base32_decode(char* string, size_t length, size_t* size_out);
#endif // BASE32_H
