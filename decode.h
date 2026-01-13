#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"   // contains Status & OperationType enums

/* Maximum buffer sizes */
#define MAX_SECRET_BUF_SIZE 1                // to decode 1 byte from image at a time
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)  // 1 secret byte = 8 image bytes
#define MAX_FILE_SUFFIX 4                    // max length for extension (e.g., ".txt")

/* ===================== STRUCTURE: DecodeInfo ===================== */
/* This structure stores all data required during decoding procedure */
typedef struct _DecodeInfo
{
    char *out_image_fname;        // encoded BMP image name (input)
    FILE *fptr_out_image;         // file pointer to encoded image
    uint image_capacity;          // total image size (not used always but helpful)

    char *secret_fname;           // output secret file base name (without extension)
    char *secret_file_concat_name;// full name after adding extension
    FILE *fptr_secret;            // file pointer for final decoded secret

    char extn_secret_file[5];     // extension of secret file like ".txt"
    char secret_data[100];        // buffer to temporarily hold decoded characters
    long size_secret_file;        // decoded size of secret file
    int extension_size;           // decoded extension length (ex: 4 for ".txt")

} DecodeInfo;


/* ===================== FUNCTION PROTOTYPES ===================== */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo); // validate user inputs

Status do_decoding(DecodeInfo *decInfo); // full decode procedure flow

Status open_output_image_file(DecodeInfo *decInfo); // open encoded BMP

Status open_decoded_message_file(DecodeInfo *decInfo); // create output secret file

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo); // check presence of magic marker

Status decode_int_from_lsb(int *num, char *image_buffer); // extract 32bit integer

Status decode_bit_from_lsb(char *ch, char *image_buffer); // extract 1 ASCII char

Status decode_secret_file_extn_size(DecodeInfo *decInfo); // extract extension length

Status decode_secret_file_extn(DecodeInfo *decInfo); // extract extension characters

Status decode_secret_file_size(DecodeInfo *decInfo); // extract original file size

Status decode_secret_file_data(DecodeInfo *decInfo); // extract actual secret message data

#endif // DECODE_H
