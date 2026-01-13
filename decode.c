#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>           // for nanosleep
#include "decode.h"
#include "types.h"
#include "common.h"

/* small portable delay */
static void delay_ms(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* Color codes */
#define GREEN  "\033[0;32m"
#define GRAY   "\033[0;90m"
#define RESET  "\033[0m"

/* ========================= INPUT VALIDATION ========================= */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (strstr(argv[2], ".bmp") != NULL)
        decInfo->out_image_fname = argv[2];
    else
    {
        printf("✖️ Invalid input image. Must be .bmp\n");
        return e_failure;
    }

    if (argv[3] == NULL)
        decInfo->secret_fname = "decoded";
    else
    {
        char *dot = strchr(argv[3], '.');
        if (dot != NULL) *dot = '\0';
        decInfo->secret_fname = argv[3];
    }

    return e_success;
}

/* ========================= OPEN ENCODED IMAGE ========================= */
Status open_output_image_file(DecodeInfo *decInfo)
{
    decInfo->fptr_out_image = fopen(decInfo->out_image_fname, "rb");
    if (decInfo->fptr_out_image == NULL)
    {
        printf("✖️ Cannot open encoded image: %s\n", decInfo->out_image_fname);
        return e_failure;
    }

    fseek(decInfo->fptr_out_image, 54, SEEK_SET);
    return e_success;
}

/* ========================= CREATE SECRET OUTPUT FILE ========================= */
Status open_decoded_message_file(DecodeInfo *decInfo)
{
    int len = strlen(decInfo->secret_fname) + strlen(decInfo->extn_secret_file) + 1;
    decInfo->secret_file_concat_name = malloc(len);

    strcpy(decInfo->secret_file_concat_name, decInfo->secret_fname);
    strcat(decInfo->secret_file_concat_name, decInfo->extn_secret_file);

    decInfo->fptr_secret = fopen(decInfo->secret_file_concat_name, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        printf("✖️ Cannot create output secret file\n");
        return e_failure;
    }

    return e_success;
}

/* ========================= BASIC DECODERS ========================= */
Status decode_bit_from_lsb(char *ch, char *buffer)
{
    *ch = 0;
    for (int i = 0; i < 8; i++)
        *ch |= ((buffer[i] & 1) << i);
    return e_success;
}

Status decode_int_from_lsb(int *num, char *buffer)
{
    *num = 0;
    for (int i = 0; i < 32; i++)
        *num |= ((buffer[i] & 1) << i);
    return e_success;
}

/* ========================= MAGIC CHECK ========================= */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8], ch;
    char data[strlen(magic_string) + 1];

    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_out_image) != 1) return e_failure;
        decode_bit_from_lsb(&ch, buffer);
        data[i] = ch;
    }
    data[strlen(magic_string)] = '\0';

    if (!strcmp(magic_string, data)) return e_success;
    return e_failure;
}

/* ========================= EXTENSION SIZE ========================= */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    if (fread(buffer, 32, 1, decInfo->fptr_out_image) != 1) return e_failure;
    decode_int_from_lsb(&decInfo->extension_size, buffer);
    return e_success;
}

/* ========================= EXTENSION NAME ========================= */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8], ch;

    for (int i = 0; i < decInfo->extension_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_out_image) != 1) return e_failure;
        decode_bit_from_lsb(&ch, buffer);
        decInfo->extn_secret_file[i] = ch;
    }

    decInfo->extn_secret_file[decInfo->extension_size] = '\0';
    return e_success;
}

/* ========================= FILE SIZE ========================= */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int temp_size = 0;
    if (fread(buffer, 32, 1, decInfo->fptr_out_image) != 1) return e_failure;
    decode_int_from_lsb(&temp_size, buffer);
    decInfo->size_secret_file = (long)temp_size;
    return e_success;
}

/* ===================== PROGRESS BAR (Decoding) ===================== */
void show_progress_decode(long done, long total)
{
    int barWidth = 50;
    float progress = (total > 0) ? ((float)done / total) : 0.0f;
    int fill = (int)(progress * barWidth);

    printf("\r[");
    for(int i = 0; i < barWidth; i++)
        printf(i < fill ? GREEN "■" RESET : GRAY "□" RESET);

    printf("] %3d%%", (int)(progress * 100));
    fflush(stdout);
}

/* ========================= SECRET DATA DECODE ========================= */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8], ch;
    long done = 0;

    printf("\n⚙️  Extracting Secret Data...\n");

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_out_image) != 1)
        {
            printf("\n[ERROR] Unexpected EOF while reading encoded data.\n");
            return e_failure;
        }
        decode_bit_from_lsb(&ch, buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_secret);

        done++;
        show_progress_decode(done, decInfo->size_secret_file);
        delay_ms(200);
    }

    printf("\n" GREEN "✔ Secret Data Extracted Successfully!" RESET "\n");
    return e_success;
}

/* ========================= MAIN DECODING PROCESS ========================= */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n───────────────────────────────────────────────\n");
    printf("🕵️  STEGANOGRAPHY TOOL - DECODING STARTED\n");
    printf("───────────────────────────────────────────────\n");
    printf("📁 Input Image : %s\n\n", decInfo->out_image_fname);

    printf("🔍 Steps:\n");

    printf("\n   1️⃣  Opening encoded image ............ ");
    if (open_output_image_file(decInfo) != e_success) { printf("✖️\n"); return e_failure; }
    printf("✔️\n");

    printf("   2️⃣  Checking magic signature (#*) .... ");
    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success) { printf("✖️ Invalid!\n"); return e_failure; }
    printf("✔️  Valid\n");

    printf("   3️⃣  Reading extension size .......... ");
    decode_secret_file_extn_size(decInfo);
    printf("✔️  (%d)\n", decInfo->extension_size);

    printf("   4️⃣  Reading extension ............... ");
    decode_secret_file_extn(decInfo);
    printf("✔️  (%s)\n", decInfo->extn_secret_file);

    printf("   5️⃣  Creating output file ............ ");
    open_decoded_message_file(decInfo);
    printf("✔️  (%s)\n", decInfo->secret_file_concat_name);

    printf("   6️⃣  Reading file size ............... ");
    decode_secret_file_size(decInfo);
    printf("✔️  (%ld bytes)\n", decInfo->size_secret_file);

    printf("   7️⃣  Extracting secret data .......... ⏳\n");
    decode_secret_file_data(decInfo);

    printf("\n🎯 STATUS: SUCCESS — Secret restored!\n");
    printf("📌 Extracted File: %s\n", decInfo->secret_file_concat_name);
    printf("───────────────────────────────────────────────\n\n");

    return e_success;
}
