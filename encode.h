#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"    // using Status, OperationType, uint etc.

/* Buffer sizes for processing */
#define MAX_SECRET_BUF_SIZE 1        // We process 1 byte of secret file at a time
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)  // 1 byte → 8 image bytes
#define MAX_FILE_SUFFIX 4            // e.g. ".txt" = 4 characters

/* Structure to store all encoding-related information */
typedef struct _EncodeInfo
{
    /* Source Image Details */
    char *src_image_fname;           // Input BMP file name
    FILE *fptr_src_image;            // File pointer of source image
    uint image_capacity;             // Maximum data image can hide

    /* Secret File Details */
    char *secret_fname;              // Name of the secret file (to hide)
    FILE *fptr_secret;               // Secret file pointer
    char *extn_secret_file;          // Secret file extension (e.g. .txt)
    char secret_data[100];           // Buffer for secret file data
    long size_secret_file;           // Size of secret file in bytes
    int extension_size;              // Length of extension string (e.g. 4)

    /* Output Stego Image Details */
    char *stego_image_fname;         // Output BMP after storing secret
    FILE *fptr_stego_image;          // File pointer for stego image

} EncodeInfo;

/*----------------------------------------------------------
    Function Prototypes
----------------------------------------------------------*/

/* Validate input arguments given by user (file names etc.) */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Perform entire encoding process */
Status do_encoding(EncodeInfo *encInfo);

/* Open required files for reading/writing */
Status open_files(EncodeInfo *encInfo);

/* Check if source image can store secret file */
Status verify_capacity(EncodeInfo *encInfo);

/* Find total usable pixel data size of BMP (WxHx3) */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size (used for secret file) */
uint get_file_size(FILE *fptr);

/* Copy BMP header (first 54 bytes) to output */
Status transfer_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Encode magic identifier (#*) to verify valid stego image during decode */
Status store_magic_data(const char *magic_string, EncodeInfo *encInfo);

/* Store integer information into LSBs of image bytes */
Status encode_int_lsb(int data, char *image_buffer);

/* Store the size of secret file extension */
Status encode_secret_extn_size(int size, EncodeInfo *encInfo);

/* Store extension characters (e.g. ".txt") */
Status encode_secret_extn(const char *extn, EncodeInfo *encInfo);

/* Store secret file size for later extraction */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Store secret file data (actual hidden content) */
Status encode_secret_data(EncodeInfo *encInfo);

/* Copy remaining image bytes unchanged */
Status copy_remaining_data(FILE *src, FILE *dest);

#endif
