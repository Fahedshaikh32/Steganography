/************************************************************
 * Steganography Tool - Main Program
 ************************************************************/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

/************************************************************
 * Function: check_operation_type
 ************************************************************/
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;   // User selected encoding mode
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;   // User selected decoding mode
    }
    else
    {
        return e_unsupported; // Invalid operation input
    }
}

/************************************************************
 * Function: main
 * Program Entry Point
 ************************************************************/
int main(int argc, char *argv[])
{
    if (argc < 3)   // At least 3 arguments required
    {
        printf("\n[USER ERROR] Missing or invalid arguments.\n");
        printf("Usage for Encoding: ./stego -e <source.bmp> <secret.txt> <output.bmp>\n");
        printf("Usage for Decoding: ./stego -d <encoded.bmp> <output_basename>\n\n");
        return 1;   // return error status
    }

    OperationType opt = check_operation_type(argv); // Identify user operation

    /* ======================== ENCODING MODE ======================== */
    if (opt == e_encode)
    {
        //printf("\n[INFO] Encoding mode selected.\n");

        EncodeInfo encInfo; // Object storing all encode-related data

       // printf("OPERATION: Validating inputs...\n");
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
           // printf("[ERROR] Invalid inputs! Please check file names & extensions.\n");
            return 1;
        }

       // printf("[DONE] Input validation successful.\n");
        //printf("[INFO] Encoding started...\n");

        if (do_encoding(&encInfo) == e_success)
        {
            //printf("\n[SUCCESS] Encoding completed & output saved successfully.\n");
        }
        else
        {
            //printf("\n[ERROR] Encoding failed! Check source BMP or available space.\n");
        }
    }

    /* ======================== DECODING MODE ======================== */
    else if (opt == e_decode)
    {
        //printf("\n[INFO] Decoding mode selected.\n");

        DecodeInfo decInfo; // Stores decode configuration

        //printf("OPERATION: Validating inputs...\n");
        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            //printf("[ERROR] Wrong decode arguments!\n");
            //printf("Correct Usage: ./stego -d <encoded.bmp> <output_basename>\n");
            return 1;
        }

       // printf("[DONE] Input validation successful.\n");
        //printf("[INFO] Decoding started...\n");

        if (do_decoding(&decInfo) == e_success)
        {
            //printf("\n[SUCCESS] Decoding completed & secret file extracted.\n");
        }
        else
        {
            //printf("\n[ERROR] Decoding failed! Check if BMP contains hidden data.\n");
        }
    }

    /* ===================== INVALID INPUT OPERATION ==================== */
    else
    {
        printf("\n[ERROR] Unsupported operation selected!\n");
        printf("Use -e for encoding or -d for decoding\n");
        return 1; // exit with failure
    }

    return 0; // Program executed successfully
}
