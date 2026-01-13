#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint;    // short name for unsigned int

typedef enum
{
    e_success,                // function completed correctly
    e_failure                 // function failed to complete
} Status;                     // used as function return type

typedef enum
{
    e_encode,                 // -e user wants to perform encoding
    e_decode,                 // -d user wants to perform decoding
    e_unsupported             // user passed some other wrong option
} OperationType;              // used to select steganography operation

#endif
