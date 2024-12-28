#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char ALPHABET[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint8_t inverse_table[256];

char* base64_encode(const uint8_t* input, size_t input_length) {
    size_t output_length = ((input_length + 2) / 3) * 4;
    char* output = malloc(output_length + 1);
    if (!output) {
        perror("Failed to allocate memory for Base64 encoding");
        return NULL;
    }

    size_t i, j;
    for (i = 0, j = 0; i < input_length; i += 3) {
        uint32_t octet_a = i < input_length ? input[i] : 0;
        uint32_t octet_b = (i + 1) < input_length ? input[i + 1] : 0;
        uint32_t octet_c = (i + 2) < input_length ? input[i + 2] : 0;

        uint32_t combined = (octet_a << 16) | (octet_b << 8) | octet_c;

        output[j++] = ALPHABET[(combined >> 18) & 0x3F];
        output[j++] = ALPHABET[(combined >> 12) & 0x3F];
        output[j++] = (i + 1) < input_length ? ALPHABET[(combined >> 6) & 0x3F] : '=';
        output[j++] = (i + 2) < input_length ? ALPHABET[combined & 0x3F] : '=';
    }
    output[output_length] = '\0'; // Null-terminate the output string
    return output;
}

uint8_t* base64_decode(const char* input, size_t input_length, size_t* output_length) {
    for (int i = 0; i < 64; ++i) {
        inverse_table[(unsigned char)ALPHABET[i]] = i;
    }
    if (input_length % 4 != 0) {
        fprintf(stderr, "Invalid Base64 input length\n");
        return NULL;
    }

    size_t padding = 0;
    if (input_length >= 2 && input[input_length - 1] == '=') padding++;
    if (input_length >= 3 && input[input_length - 2] == '=') padding++;

    *output_length = (input_length / 4) * 3 - padding;
    uint8_t* output = malloc(*output_length);
    if (!output) {
        perror("Failed to allocate memory for Base64 decoding");
        return NULL;
    }

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = inverse_table[(unsigned char)input[i++]];
        uint32_t sextet_b = inverse_table[(unsigned char)input[i++]];
        uint32_t sextet_c = (input[i] != '=') ? inverse_table[(unsigned char)input[i++]] : 0;
        uint32_t sextet_d = (input[i] != '=') ? inverse_table[(unsigned char)input[i++]] : 0;

        uint32_t combined = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

        if (j < *output_length) output[j++] = (combined >> 16) & 0xFF;
        if (j < *output_length) output[j++] = (combined >> 8) & 0xFF;
        if (j < *output_length) output[j++] = combined & 0xFF;
    }

    return output;
}