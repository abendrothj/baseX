#include <stddef.h>

// Encode given data in base 64.
//
// The returned buffer is heap-allocated.
//
char* base64_encode(char* array, size_t size);

// Decode given base 64 string.
//
// The returned buffer contains the decoded data, and is heap-allocated.
// `size_out` contains the size of the decoded data.
//
// If decoding fails due to input validation, the return value is NULL.
char* base64_decode(char* string, size_t length, size_t* size_out);
