# BaseX: A Base8/Base32/Base64 Encoder/Decoder in C

BaseX is a command-line tool for encoding and decoding files using the Base8, Base32, and Base64 formats. It provides a straightforward interface for transforming file content into these formats or decoding them back to their original state.

## Features

- Encode files into Base8, Base32, or Base64 formats.
- Decode files from Base8, Base32, or Base64 formats.
- Supports binary file input and produces output via `stdout`.
- Returns nonzero error codes for invalid input during decoding.

## Installation

To use BaseX, compile the source code with a C compiler using std C23:

```sh
mkdir build | clang -Iinclude -std=c23 src/*.c -o build/basex
```

Make sure that the `base8.h`, `base32.h`, and `base64.h` header files are in root/include, and source files are in root/src, or this command will not work.

## Usage

```sh
basex [options] <format> <file>
```

### Arguments

- `<format>`: The encoding/decoding format. Valid options are:
  - `base8`: Base8 (octal) encoding/decoding.
  - `base32`: Base32 encoding/decoding.
  - `base64`: Base64 encoding/decoding.
- `<file>`: The path to the file to encode or decode.

### Options

- `-d`: Decode the input file.
- `-h`: Display the help message and usage instructions.

### Examples

#### Encoding a File

To encode a file named `input.txt` in Base64:

```sh
basex base64 input.txt
```

#### Decoding a File

To decode a file named `encoded.txt` from Base32:

```sh
basex -d base32 encoded.txt
```

#### Displaying Help

```sh
basex -h
```

## Error Handling

- If the input file is invalid or cannot be opened, an error message is printed to `stderr`.
- If decoding fails due to invalid input, the program exits with a nonzero error code.


