/*
   BaseX: Encoder/decoder for base8/base32/base64.

   See the usage() function for the command line interface.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base32.h"
#include "base64.h"
#include "base8.h"

// Print the usage message then exit.
void usage(char* program_name) {
  fprintf(stderr,
          "Usage:\n"
          "%s [options] <format> <file>\n"
          "Read given file and encode/decode it according to chosen format "
          "and write the output to stdout.\n"
          "The format options are:\n"
          "    \"base8\" for octal/base 8."
          "    \"base32\" for base 32."
          "    \"base64\" for base 64."
          "\n"
          "Valid options are:\n"
          "    -d  Decode input.\n"
          "    -h  Print this help message.\n"
          "\n"
          "This program should return with a nonzero error code only if it is "
          "in decoding mode and the input is not valid.\n",
          program_name);
  exit(EXIT_FAILURE);
}

char *read_file(const char *file_name, size_t *length) {
  // Open the file in binary mode
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    return NULL;
  }

  // Make sure file isn't empty
  if (fseek(file, 0, SEEK_END) != 0) {
    return NULL;
  }

  *length = ftell(file);
  if (*length == -1) {
    int err = errno;
    fclose(file);
    errno = err;
    return NULL;
  }
  // Considering null-terminator char, +1
  char* buffer = malloc(*length + 1);
  if (buffer == NULL) {
    int err = errno;
    fclose(file);
    errno = err;
    return NULL;
  }

  // Seek to beginning of file
  if (fseek(file, 0, SEEK_SET) != 0) {
    int err = errno;
    free(buffer);
    fclose(file);
    errno = err;
    return NULL;
  }

  // Read entire file into buffer
  if (fread(buffer, 1, *length, file) != *length) {
    int err = errno;
    free(buffer);
    fclose(file);
    errno = err;
    return NULL;
  }

  // Null-terminate and close stream
  buffer[*length] = '\0';

  if (fclose(file) != 0) {
    int err = errno;
    free(buffer);
    errno = err;
    return NULL;
  }
  return buffer;
}

int main(int argc, char** argv) {
  bool decode_mode = false;
  // CL Options
  int opt;
  while ((opt = getopt(argc, argv, "dh")) != -1) {
    switch (opt) {
      case 'd':
        decode_mode = true;
        break;
      case 'h':
      case '?':
      default:
        usage(argv[0]);
    }
  }
  if (argc - optind != 2) {
    usage(argv[0]);
  }

  // Function pointers for shorthand
  char* (*decoder_func)(char*, size_t, size_t*);
  char* (*encoder_func)(char*, size_t);
  // Setting function pointers
  char const* base_format = argv[optind++];
  if (strcmp(base_format, "base8") == 0) {
    encoder_func = base8_encode;
    decoder_func = base8_decode;
  } else if (strcmp(base_format, "base32") == 0) {
    encoder_func = base32_encode;
    decoder_func = base32_decode;
  } else if (strcmp(base_format, "base64") == 0) {
    encoder_func = base64_encode;
    decoder_func = base64_decode;
  } else {
    fprintf(stderr, "Unknown data format: %s.\n", base_format);
    usage(argv[0]);
    return 1;
  }
  // Read file contents into buffer
  size_t size;
  char* file_contents = read_file(argv[optind++], &size);
  if (file_contents == NULL) {
    int e = errno;
    fprintf(stderr, "Error when reading the file %s: ", argv[2]);
    errno = e;
    perror(NULL);
    return EXIT_FAILURE;
  }

  // Modes (decode/encode)
  if (decode_mode) {
    size_t output_size;
    char* output = decoder_func(file_contents, size, &output_size);
    free(file_contents);
    if (output != NULL) {
      fwrite(output, 1, output_size, stdout);
      free(output);
    } else {
      return EXIT_FAILURE;
    }
  } else {
    char* output = encoder_func(file_contents, size);
    free(file_contents);
    if (output != NULL) {
      puts(output);
      free(output);
    } else {
      return EXIT_FAILURE;
    }
  }

  return 0;
}
