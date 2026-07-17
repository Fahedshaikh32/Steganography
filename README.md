<div align="center">

# 🖼️ Image Steganography (LSB) — C

**A command-line tool that hides and extracts secret files inside BMP images using Least Significant Bit (LSB) encoding.**

Written in portable C with a modular encode/decode architecture and zero visible distortion to the cover image.

[![Language](https://img.shields.io/badge/language-C-00599C?logo=c&logoColor=white)](#)
[![Build](https://img.shields.io/badge/build-gcc-blue)](https://gcc.gnu.org/)
[![License](https://img.shields.io/badge/license-MIT-green)](#license)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20WSL%20%7C%20MinGW-lightgrey)](#)

[Overview](#overview) •
[How It Works](#how-it-works) •
[Build](#build) •
[Usage](#usage) •
[Data Layout](#data-layout) •
[Project Structure](#project-structure) •
[Limitations](#limitations)

</div>

---

## Overview

This tool embeds an arbitrary secret file inside a `.bmp` image by rewriting the least significant bit of each pixel byte to carry one bit of secret data. Because only the lowest bit of each byte changes, the modification is imperceptible to the human eye while remaining fully recoverable byte-for-byte.

The same binary handles both directions:
- **Encode** — hide a secret file inside a BMP, producing a new "stego" image
- **Decode** — extract the hidden file back out of a stego image

A magic signature is embedded first, so decoding can verify the image actually contains hidden data before attempting extraction.

## How It Works

### Encoding pipeline

1. **Capacity check** — the cover image must have at least `54 + 8 × (payload size)` bytes available (the LSB scheme spends one full image byte per secret *bit*)
2. **Header transfer** — the 54-byte BMP header is copied unchanged from source to output
3. **Magic string** — `#*` is encoded first, byte-by-byte, one bit per image byte
4. **Extension metadata** — the secret file's extension length, then the extension itself (e.g. `.txt`)
5. **Payload size** — the secret file's size, encoded as a 32-bit integer across 32 image bytes
6. **Payload data** — the actual secret file contents, one bit per image byte
7. **Passthrough** — any remaining image bytes are copied unchanged

### Decoding pipeline

Decoding walks the same structure in reverse: it seeks past the BMP header, verifies the magic string, reads the extension length and name, reads the payload size, then extracts that many bytes — each reconstructed by reading the LSB of 8 consecutive image bytes.

```
[54-byte BMP header]
[8 bytes/bit]  →  "#*"                  (magic string)
[32 bytes]     →  extension length
[N×8 bytes]    →  extension (e.g. ".txt")
[32 bytes]     →  secret file size
[M×8 bytes]    →  secret file data
[remaining image bytes, unmodified]
```

## Build

Requires a C compiler (`gcc`, `clang`, or MinGW on Windows).

```bash
gcc encode.c decode.c test_encode.c -o steganography
```

## Usage

### Encode — hide a secret file inside an image

```bash
./steganography -e <source.bmp> <secret_file> <output.bmp>
```

Example:
```bash
./steganography -e BMW.bmp secret.txt stego.bmp
```

### Decode — extract the hidden file from a stego image

```bash
./steganography -d <stego.bmp> [output_basename]
```

Example:
```bash
./steganography -d stego.bmp decoded
# → writes decoded.txt (extension is recovered automatically)
```

If no output name is given, the extracted file defaults to `decoded.<original-extension>`.

### Sample run

```
───────────────────────────────────────────────
🕵️  STEGANOGRAPHY TOOL - DECODING STARTED
───────────────────────────────────────────────
📁 Input Image : stego.bmp

🔍 Steps:

   1️⃣  Opening encoded image ............ ✔️
   2️⃣  Checking magic signature (#*) .... ✔️  Valid
   3️⃣  Reading extension size .......... ✔️  (4)
   4️⃣  Reading extension ............... ✔️  (.txt)
   5️⃣  Creating output file ............ ✔️  (decoded.txt)
   6️⃣  Reading file size ............... ✔️  (26 bytes)
   7️⃣  Extracting secret data .......... ⏳

🎯 STATUS: SUCCESS — Secret restored!
📌 Extracted File: decoded.txt
───────────────────────────────────────────────
```

## Data Layout

| Stage | Size | Content |
|---|---|---|
| BMP header | 54 bytes (untouched) | Standard bitmap file header |
| Magic string | `len("#*") × 8` bytes | Validates the file as a stego image |
| Extension size | 32 bytes | Length of the secret file's extension |
| Extension | `size × 8` bytes | e.g. `.txt` |
| Secret file size | 32 bytes | Total payload length in bytes |
| Secret file data | `size × 8` bytes | The actual hidden content, one bit per byte |

## Project Structure

```
.
├── test_encode.c    # Entry point: argument parsing, mode dispatch
├── encode.c/.h      # Encoding pipeline: header copy, magic string, payload
├── decode.c/.h      # Decoding pipeline: magic check, metadata & payload extraction
├── common.h         # Shared MAGIC_STRING definition
├── types.h          # Status / OperationType enums, uint typedef
├── BMW.bmp           # Sample cover image
├── secret.txt        # Sample secret payload
├── LICENSE
└── README.md
```

## Key Concepts Used

- LSB (Least Significant Bit) steganography
- Bitwise operations for sub-byte data packing
- Binary file I/O in C
- Structures (`EncodeInfo`, `DecodeInfo`) and enums for clean state passing
- Modular separation of encode/decode logic

## Limitations

- Only uncompressed `.bmp` cover images are supported
- No encryption — the hidden data is only concealed, not cryptographically protected; anyone who runs the decoder can extract it
- Capacity is bounded by cover image size (1 image byte hides 1 secret bit)

## License

This project is licensed under the MIT License — see [`LICENSE`](./LICENSE) for details.

## Author

**Fahed Shaikh**
BE, Electronics & Telecommunication Engineering
C Programming · Linux Internals · File Handling · Bitwise Operations

<div align="center">

If you find this project useful, consider giving it a ⭐

</div>
