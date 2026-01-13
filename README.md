🖼️ Image Steganography System (C Project)

A secure and efficient **Image Steganography System** developed in **C language**, which hides secret text messages inside BMP image files using **LSB (Least Significant Bit)** technique, ensuring data confidentiality with minimal visual distortion.


🚀 Features

🔐 Hide secret text inside BMP images
🔓 Extract hidden text from stego-images
🖼️ Uses **BMP image format** for lossless data hiding
📄 Supports text file input for secret messages
🧠 Bit-level manipulation using LSB technique
⚙️ Modular and well-structured C code
🖥️ Simple command-line interface
💾 No visible change in original image quality



⚙️ How to Compile & Run

🧰 Using GCC (Linux / WSL / MinGW)

🔹 Compile

```bash
gcc encode.c decode.c test_encode.c -o steganography
```

🔹 Encoding (Hide Message)

```bash
./steganography -e BMW.bmp secret.txt stego.bmp
```

🔹 Decoding (Extract Message)

```bash
./steganography -d stego.bmp decoded.txt


📁 File Structure

| File Name             | Description                                    |
| --------------------- | ---------------------------------------------- |
| `encode.c / encode.h` | Handles embedding secret data into BMP image   |
| `decode.c / decode.h` | Extracts hidden data from stego image          |
| `common.h`            | Common macros and utility functions            |
| `types.h`             | Custom data types and structures               |
| `test_encode.c`       | Main driver file (encoding & decoding control) |
| `BMW.bmp`             | Original cover image                           |
| `stego.bmp`           | Image containing hidden message                |
| `secret.txt`          | Input file containing secret text              |
| `decoded.txt`         | Output file with extracted message             |



🧑‍💻 Key Concepts Used

LSB (Least Significant Bit) Steganography
Bitwise operations
File handling in C
Structures and enums
Modular programming

🧑‍💻 Developer

**Fahed Shaikh**
🎓 BE in Electronics & Telecommunication Engineering
🛠️ Skills: C Programming, Linux Internals, File Handling, Bitwise Operations
