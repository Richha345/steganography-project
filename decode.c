#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

//-------------------------------------------------------------------------------//

Status read_and_validate_decode_args(char *argv[], EncodeInfo *encInfo)
{
    // CLA validation
    for(int i = 2; i < 3; i++)
    {
        if(argv[i] == NULL)
        {
            printf("\nInvalid input\n");
            printf("\n-------- SAMPLE INPUTS --------\n");
            printf("\n./a.out -e source_file.bmp secret_file.txt [output_file(.bmp .py .txt)]\n");
            printf("./a.out -d source_file.bmp [output_file(.bmp .py .txt)]\n");
            printf("\n");
            return e_failure;
        }
    }

    // check ".bmp" extension at last
    char *dot = strrchr(argv[2], '.');

    if(dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("\nSource image file extension should be \".bmp\"..\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];

    // check output file is given or not
    if(argv[3] != NULL)
    {
        encInfo->stego_image_fname = argv[3];
    }
    else
    {
        encInfo->stego_image_fname = "decoded_info.txt";
    }

    printf("\nAll validation are passed successfully...\n");

    // open source and output file
    if(open_decode_files(encInfo) == e_failure)
    {
        printf("\nFile doesn't open..\n");
        return e_failure;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status open_decode_files(EncodeInfo *encInfo)
{
    // source file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");

    if(encInfo->fptr_src_image == NULL)
    {
        printf("Source file is not Opened..\n");
        return e_failure;
    }

    // output file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");

    if(encInfo->fptr_stego_image == NULL)
    {
        printf("Secret file is not Opened..\n");
        return e_failure;
    }

    printf("\nFiles opened successfully...\n");

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_byte_to_lsb(char *image_buffer, char *data)
{
    int j = 7;

    *data = 0;

    for(int i = 0; i < 8; i++)
    {
        // get the LSB bit
        if(image_buffer[i] & 1)
        {
            *data = (*data) | (1 << j);
        }

        j--;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char de_magic_buffer[3];
    char image_buffer[8];

    for(int i = 0; i < 2; i++)
    {
        char data = 0;

        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        decode_byte_to_lsb(image_buffer, &data);

        printf("\nmagic str = %c\n", data);

        de_magic_buffer[i] = data;
    }

    de_magic_buffer[2] = '\0';

    if(strcmp(de_magic_buffer, magic_string) != 0)
    {
        printf("\nMagic string doesnt match\n");
        return e_failure;
    }

    printf("\nMagic string decoded successfully...\n");

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_size_to_lsb(char *image_buffer, int *data)
{
    int j = 31;

    *data = 0;

    for(int i = 0; i < 32; i++)
    {
        // get the LSB bit
        if(image_buffer[i] & 1)
        {
            *data = (*data) | (1 << j);
        }

        j--;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char buffer[32];
    int extn_size = 0;

    // read 32 bytes from source image
    fread(buffer, 32, 1, encInfo->fptr_src_image);

    if(decode_size_to_lsb(buffer, &extn_size) == e_failure)
    {
        printf("\nError : file extension size decoding failed..\n");
        return e_failure;
    }

    if(extn_size != 4)
    {
        printf("\nError : file extension size does not match..\n");
        return e_failure;
    }

    printf("\nFile extension size decoded successfully...\n");

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_extn(EncodeInfo *encInfo)
{
    char image_buffer[8];
    char data;

    for(int i = 0; i < 4; i++)
    {
        data = 0;

        fread(image_buffer, 8, 1,
              encInfo->fptr_src_image);

        decode_byte_to_lsb(image_buffer, &data);

        encInfo->extn_secret_file[i] = data;
    }

    encInfo->extn_secret_file[4] = '\0';

    printf("\nSecret file extension = %s\n",
           encInfo->extn_secret_file);

    return e_success;
}
Status decode_secret_file_size(EncodeInfo *encInfo)
{
    char buffer[32];
    int secret_file_size = 0;

    fread(buffer, 32, 1, encInfo->fptr_src_image);

    if(decode_size_to_lsb(buffer, &secret_file_size) == e_failure)
    {
        printf("\nError : secret file size decoding failed..\n");
        return e_failure;
    }

    encInfo->size_secret_file = secret_file_size;

    printf("\nSecret file size = %ld\n", encInfo->size_secret_file);

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_data(EncodeInfo *encInfo)
{
    char image_buffer[8];
    char data;

    for(int i = 0; i < encInfo->size_secret_file; i++)
    {
        data = 0;

        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        decode_byte_to_lsb(image_buffer, &data);

        fwrite(&data, 1, 1, encInfo->fptr_stego_image);
    }

    printf("\nSecret f ile data decoded successfully...\n");

    return e_success;
}
//-------------------------------------------------------------------------------//

Status do_decoding(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_src_image, 54, SEEK_SET);

    printf("\n%lu\n", ftell(encInfo->fptr_src_image));

    // decode magic string
    if(decode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("\nError : unable to decode magic string..\n");
        return e_failure;
    }

    // decode file extension size
    if(decode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("\nError : unable to decode file extension size..\n");
        return e_failure;
    }

    // decode file extension
    if(decode_secret_file_extn(encInfo) == e_failure)
    {
        printf("\nError : unable to decode secret file extension..\n");
        return e_failure;
    }

    // decode secret file size
    if(decode_secret_file_size(encInfo) == e_failure)
    {
        printf("\nError : unable to decode secret file size..\n");
        return e_failure;
    }

    // decode secret file data
    if(decode_secret_file_data(encInfo) == e_failure)
    {
        printf("\nError : unable to decode secret file data..\n");
        return e_failure;
    }

    return e_success;
}