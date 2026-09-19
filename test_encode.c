#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    if(argc < 4)
    {
        printf("Usage: ./encode -e source.bmp secret.txt output.bmp\n");
        return 0;
    }
    
    if(check_operation_type(argv[1][1])==e_encode)
    {
       /*-> call read_and_validate_encode_args(argv,&encInfo); //e_success:
            => call do_encoding(&encInfo) == e_success
                print "Encoding is success"
                */
            read_and_validate_encode_args(argv, &encInfo);
            if(do_encoding(&encInfo)==e_failure)
            {
                printf("\nError : unable to encode..\n");
            }
            printf("\nencoding done successfully...\n");
            return 0;
    }

    else
    {
        printf("Validation failed\n");
        return 0;
    }


    return 0;
}

OperationType check_operation_type(char opt)
{
    if(opt=='e')
    {
        return e_encode;
    }
    else if(opt=='d')
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
