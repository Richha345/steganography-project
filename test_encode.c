#include <stdio.h>
#include "encode.h"
#include "types.h"

//-------------------------------------------------------------------------------//

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    if(argc < 3)
    {
        printf("\nInvalid input..\n");
        printf("\n-------- SAMPLE INPUTS --------\n");
        printf("\n./a.out -e source_file.bmp secret_file.txt [output_file.bmp]\n");
        printf("./a.out -d source_file.bmp [output_file.txt]\n");
        printf("\n");
        return 0;
    }

    // check e or d from argv[1][1]
    if(check_operation_type(argv[1][1]) == e_encode)
    {
        if(read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("\nInvalid input..\n");
            return 0;
        }

        if(do_encoding(&encInfo) == e_failure)
        {
            printf("\nError : unable to encode..\n");
            return 0;
        }

        printf("\nEncoding done successfully...\n");
        printf("\n");
    }
    else if(check_operation_type(argv[1][1]) == e_decode)
    {
        if(read_and_validate_decode_args(argv, &encInfo) == e_failure)
        {
            printf("\nInvalid input..\n");
            return 0;
        }

        if(do_decoding(&encInfo) == e_failure)
        {
            printf("\nError : unable to decode..\n");
            return 0;
        }

        printf("\nDecoding done successfully...\n");
        printf("\n");
    }
    else
    {
        printf("\nValidation failed\n");
        printf("\n-------- SAMPLE INPUTS --------\n");
        printf("\n./a.out -e source_file.bmp secret_file.txt [output_file.bmp]\n");
        printf("./a.out -d source_file.bmp [output_file.txt]\n");
        printf("\n");
    }

    return 0;
}

//-------------------------------------------------------------------------------//

OperationType check_operation_type(char opt)
{
    if(opt == 'e')
    {
        return e_encode;
    }
    else if(opt == 'd')
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

//-------------------------------------------------------------------------------//