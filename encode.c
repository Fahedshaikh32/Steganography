#define _XOPEN_SOURCE 700   // MUST be first line
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>           // for nanosleep in delay()
#include "encode.h"
#include "types.h"
#include "common.h"

/* ===================== small portable delay ===================== */
static void delay_ms(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* ===================== COLOR CODES FOR PROGRESS BAR ===================== */
#define GREEN  "\033[0;32m"
#define GRAY   "\033[0;90m"
#define RESET  "\033[0m"

/* ===================== IMAGE SIZE FETCHING ===================== */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width = 0, height = 0;          // store width and height of BMP image

    if (!fptr_image) return 0;

    fseek(fptr_image, 18, SEEK_SET);    // jump to header offset where width is stored
    fread(&width, sizeof(int), 1, fptr_image);   // read 4 bytes width value
    fread(&height, sizeof(int), 1, fptr_image);  // read 4 bytes height value

    return width * height * 3;          // total pixel data bytes (RGB → 3 bytes per pixel)
}

/* ===================== SECRET FILE SIZE FETCHING - return 0 on failure ===================== */
uint get_file_size(FILE *fptr)
{
    if (!fptr) return 0;

    long size;
    if (fseek(fptr, 0, SEEK_END) != 0) return 0;
    size = ftell(fptr);
    if (size <= 0)
    {
        rewind(fptr);
        return 0;
    }
    rewind(fptr);
    return (uint)size;
}

/* ===================== OPENING NECESSARY FILES (binary mode) ===================== */
Status open_files(EncodeInfo *encInfo)
{
    /* Open files in binary mode to preserve exact bytes (important for BMP) */
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image)
    {
        printf("[ERROR] Cannot open image: %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (!encInfo->fptr_secret)
    {
        printf("[ERROR] Cannot open secret file: %s\n", encInfo->secret_fname);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (!encInfo->fptr_stego_image)
    {
        printf("[ERROR] Cannot create output image: %s\n", encInfo->stego_image_fname);
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }

    return e_success;
}

/* ===================== ARGUMENT VALIDATION FOR ENCODING ===================== */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] == NULL || argv[3] == NULL)   // minimum args check
    {
        printf("[ERROR] Missing arguments.\n");
        printf("Usage: ./stego -e <source.bmp> <secret.txt> <output.bmp>\n");
        return e_failure;
    }

    /* validate BMP extension - check last occurrence of ".bmp" to be safe */
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("[ERROR] Source must be .bmp file\n");
        return e_failure;
    }

    char *ext = strrchr(argv[3], '.');        // find last dot = extension start
    if (!ext)                                 // no extension
    {
        printf("[ERROR] Secret file needs extension (.txt, .c, .h, .sh)\n");
        return e_failure;
    }

    if (!strcmp(ext, ".txt") || !strcmp(ext, ".c") ||
        !strcmp(ext, ".h") || !strcmp(ext, ".sh"))      // allowed types
    {
        encInfo->secret_fname = argv[3];
        encInfo->extn_secret_file = ext;
    }
    else
    {
        printf("[ERROR] Secret file type not supported\n");
        return e_failure;
    }

    if (argv[4] && strstr(argv[4], ".bmp"))    // if output name given & valid
    {
        encInfo->stego_image_fname = argv[4];
    }
    else if (!argv[4])                          // if not given → default name
    {
        printf("[INFO] Output name missing → using default output.bmp\n");
        encInfo->stego_image_fname = "output.bmp";
    }
    else
    {
        printf("[ERROR] Output must be .bmp\n");
        return e_failure;
    }

    return e_success;
}

/* ===================== IMAGE CAPACITY CHECK ===================== */
Status verify_capacity(EncodeInfo *encInfo)
{
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret); // check secret size
    if (encInfo->size_secret_file == 0)
    {
        printf("[ERROR] Secret file is empty or unreadable.\n");
        return e_failure;
    }

    encInfo->extension_size = strlen(encInfo->extn_secret_file); // extension length (.txt etc.)

    uint img_size = get_image_size_for_bmp(encInfo->fptr_src_image); // capacity of image
    uint required = 54 + 16 + 32 + (encInfo->extension_size * 8)
                    + 32 + ((uint)encInfo->size_secret_file * 8); // required bytes for hiding data

    if (img_size < required)              // compare image capacity vs needed
    {
        printf("[ERROR] Image too small to hide secret data.\n");
        return e_failure;
    }

    return e_success;
}

/* ===================== HEADER COPYING ===================== */
Status transfer_header(FILE *src, FILE *dest)
{
    char header[54];                      // store 54 bytes BMP header
    rewind(src);                          // jump to beginning of source file
    if (fread(header, 54, 1, src) != 1) return e_failure;
    if (fwrite(header, 54, 1, dest) != 1) return e_failure;
    return e_success;
}

/* ===================== ENCODE SECRET FILE EXTENSION ===================== */
Status encode_secret_extn(const char *extn, EncodeInfo *encInfo)
{
    char buffer[8];   // 1 character = 8 bits = 8 pixels needed

    for (int i = 0; i < (int)strlen(extn); i++)   // loop through extension characters
    {
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1) return e_failure;
        for (int j = 0; j < 8; j++)   // 8 bits of extension char
        {
            buffer[j] = (buffer[j] & ~1) | ((extn[i] >> j) & 1);   // push 1 bit into LSB
        }
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1) return e_failure;
    }

    return e_success;
}

/* ===================== ENCODE SECRET FILE SIZE ===================== */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];   // 32 bytes → store file size

    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1) return e_failure;
    encode_int_lsb((int)file_size, buffer);               // cast to int for storing 32 bits
    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1) return e_failure;

    return e_success;
}

/* ===================== ENCODE INTEGER USING LSB ===================== */
Status encode_int_lsb(int data, char *buffer)
{
    for (int i = 0; i < 32; i++)            // 32 bits required for int
    {
        buffer[i] = (buffer[i] & ~1) | ((data >> i) & 1);  // Set LSB bit by bit
    }
    return e_success;
}

/* ===================== ENCODE MAGIC STRING ===================== */
Status store_magic_data(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1) return e_failure;
        for (int j = 0; j < 8; j++)
        {
            buffer[j] = (buffer[j] & ~1) | ((magic_string[i] >> j) & 1);  // Store each bit
        }
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1) return e_failure;
    }

    return e_success;
}

/* ===================== ENCODE EXTENSION SIZE ===================== */
Status encode_secret_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    if (fread(buffer, 32, 1, encInfo->fptr_src_image) != 1) return e_failure;
    encode_int_lsb(size, buffer);                         // Store size into LSB
    if (fwrite(buffer, 32, 1, encInfo->fptr_stego_image) != 1) return e_failure;

    return e_success;
}

/* ===================== COPY REMAINING IMAGE BYTES ===================== */
Status copy_remaining_data(FILE *src, FILE *dest)
{
    char buffer[4096];   // larger buffer for efficiency
    size_t bytes;        // number of bytes read per loop

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0)   // copy until no bytes left
    {
        if (fwrite(buffer, 1, bytes, dest) != bytes) return e_failure;  // ensure exact write
    }

    return e_success;
}

/* ===================== PROGRESS BAR FOR ENCODING SECRET DATA ===================== */
void show_progress_encode(long current, long total)
{
    int barWidth = 50;
    float progress = (total > 0) ? ((float)current / total) : 0.0f;
    int fill = (int)(progress * barWidth);

    printf("\r[");  // Update same line
    for (int i = 0; i < barWidth; i++)
    {
        if (i < fill)
        {
            printf(GREEN "■" RESET); // Filled (green)
        }
        else
        {
            printf(GRAY "□" RESET);  // Empty (gray)
        }
    }
    printf("] %3d%%", (int)(progress * 100));
    fflush(stdout);
}

/* ===================== ENCODE SECRET FILE DATA (use int for fgetc) ===================== */
Status encode_secret_data(EncodeInfo *encInfo)
{
    char buffer[8];
    int c;               /* must be int to compare with EOF */
    unsigned char ch;
    long done = 0;   /* Track encoded bytes */

    rewind(encInfo->fptr_secret);   /* Start from beginning of secret file */

    while ((c = fgetc(encInfo->fptr_secret)) != EOF)
    {
        ch = (unsigned char)c;                    /* safe cast */

        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            printf("\n[ERROR] Unexpected EOF while reading source image data.\n");
            return e_failure;
        }

        for (int i = 0; i < 8; i++)
        {
            buffer[i] = (buffer[i] & ~1) | ((ch >> i) & 1);
        }

        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            printf("\n[ERROR] Failed writing stego image.\n");
            return e_failure;
        }

        done++;
        show_progress_encode(done, encInfo->size_secret_file);
        delay_ms(200);   /* 200 ms delay for animation */
    }

    printf("\n" GREEN "✔ Encoding Completed Successfully!" RESET "\n");
    return e_success;
}

/* ===================== COMPLETE ENCODING PROCESS ===================== */
Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n───────────────────────────────────────────────\n");
    printf("🔐 STEGANOGRAPHY TOOL - ENCODING STARTED\n");
    printf("───────────────────────────────────────────────\n\n");

    printf("📁 Files:\n");
    printf("   Source Image    : %s\n", encInfo->src_image_fname);
    printf("   Secret File     : %s\n", encInfo->secret_fname);
    printf("   Output Image    : %s\n\n", encInfo->stego_image_fname);

    printf("⚙️  Steps:\n");
    printf("   1️⃣  Validating arguments .............. ✔️\n");

    /* Step 2: Open all files */
    printf("   2️⃣  Opening files ..................... ");
    if (open_files(encInfo) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Could not open required files.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Step 3: Check image capacity */
    printf("   3️⃣  Checking image capacity ........... ");
    if (verify_capacity(encInfo) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Image too small to hide secret.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️  (Enough space)\n");

    /* Step 4: Copy BMP header */
    printf("   4️⃣  Copying BMP header ................ ");
    if (transfer_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Unable to copy BMP header.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Step 5: Embed magic string */
    printf("   5️⃣  Embedding magic signature (#*) .... ");
    if (store_magic_data(MAGIC_STRING, encInfo) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Could not store magic string.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Step 6: Hide extension size and extension */
    printf("   6️⃣  Hiding extension (%s) ........... ", encInfo->extn_secret_file);
    if (encode_secret_extn_size(encInfo->extension_size, encInfo) != e_success ||
        encode_secret_extn(encInfo->extn_secret_file, encInfo) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Could not encode extension.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Step 7: Hide file size */
    printf("   7️⃣  Hiding file size (%ld bytes) ..... ", encInfo->size_secret_file);
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Could not encode file size.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Step 8: Encode secret data (with progress bar) */
    printf("   8️⃣  Encoding secret data .............. ⏳\n\n");
    printf("⚙️  Encoding Secret Data...\n");
    if (encode_secret_data(encInfo) != e_success)
    {
        printf("\n🎯 STATUS: FAILED — Error while encoding secret data.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }

    /* Step 9: Copy remaining image bytes */
    printf("\n   9️⃣  Writing padding bytes ............ ");
    if (copy_remaining_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("✖️\n");
        printf("\n🎯 STATUS: FAILED — Could not copy remaining image data.\n");
        printf("───────────────────────────────────────────────\n");
        return e_failure;
    }
    printf("✔️\n");

    /* Final status */
    printf("\n🎯 STATUS: SUCCESS — Secret hidden safely!\n");
    printf("📌 Output Saved: %s\n", encInfo->stego_image_fname);
    printf("───────────────────────────────────────────────\n");

    return e_success;
}
