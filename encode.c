#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    printf("\nImage Capacity = %u\n", width * height * 3);
    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
/*Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}*/

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot;

    /* Check source image extension */
    dot = strrchr(argv[2], '.');

    if(dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("\nSource image file extension should be .bmp\n");
        return e_failure;
    }

    /* Store source image file name */
    encInfo->src_image_fname = argv[2];

    /* Store secret file name */
    encInfo->secret_fname = argv[3];

    /* Check output file */
    if(argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "output.bmp";
    }

    /* Open all files */
    if(open_files(encInfo) == e_failure)
    {
        printf("\nFile opening failed..\n");
        return e_failure;
    }

    char signature[2];
    // read 2 bytes of data from image
    if(fread(signature, 2, 1, encInfo->fptr_src_image) != 1)
    {
        printf("\nERROR : Failed to read image data\n");
        return e_failure;
    }
    signature[2]=0;
    if(strcmp(signature,"BM")!=0)
    {
        printf("Error: Invalid BMP signature\n");
        return e_failure;
    }

    printf("\nAll validations passed successfully...\n");

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    
    /* Open source image */
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");

    if(encInfo->fptr_src_image == NULL)
    {
        printf("Source image file is not opened..\n");
        return e_failure;
    }

    /* Open secret file */
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    if(encInfo->fptr_secret == NULL)
    {
        printf("Secret file is not opened..\n");
        return e_failure;
    }

    /* Open output file */
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");

    if(encInfo->fptr_stego_image == NULL)
    {
        printf("Output file is not opened..\n");
        return e_failure;
    }

    printf("\nFiles opened successfully...\n");

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    
    /* Check image capacity */
    if(check_capacity(encInfo) == e_failure)
    {
        printf("\nError : Insufficient image capacity to store secret file..\n");
        return e_failure;
    }

    /* Copy BMP header */
    if(copy_bmp_header(encInfo->fptr_src_image,
                       encInfo->fptr_stego_image) == e_failure)
    {
        printf("\nError : BMP header not copied..\n");
        return e_failure;
    }

    /* Encode magic string */
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("\nError : Unable to encode magic string..\n");
        return e_failure;
    }

    /* Encode secret file extension size */
    if(encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("\nError : Unable to encode secret file extension size..\n");
        return e_failure;
    }

    /* Encode secret file extension */
    if(encode_secret_file_extn(encInfo->extn_secret_file,
                               encInfo) == e_failure)
    {
        printf("\nError : Unable to encode secret file extension..\n");
        return e_failure;
    }

    /* Encode secret file size */
    if(encode_secret_file_size(encInfo->size_secret_file,
                               encInfo) == e_failure)
    {
        printf("\nError : Unable to encode secret file size..\n");
        return e_failure;
    }

    /* Encode secret file data */
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("\nError : Unable to encode secret file data..\n");
        return e_failure;
    }

    /* Copy remaining image data */
    if(copy_remaining_img_data(encInfo->fptr_src_image,
                               encInfo->fptr_stego_image) == e_failure)
    {
        printf("\nError : Unable to copy remaining image data..\n");
        return e_failure;
    }

    printf("\nEncoded successfully...\n");

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    rewind(encInfo->fptr_secret);

    if(encInfo->image_capacity <
       ((14 + encInfo->size_secret_file) * 8))
    {
        return e_failure;
    }
    
    printf("Sufficient image capacity available to store the secret file...\n");

    return e_success;
}

uint get_file_size(FILE *fptr)
{
    /* Move the file pointer to the end */
    fseek(fptr, 0, SEEK_END);

    /* Return the file size */
    return ftell(fptr);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{   
    char buffer[54];

    /* Move source file pointer to beginning */
    rewind(fptr_src_image);

    /* Read 54 bytes from source image */
    fread(buffer, 54, 1, fptr_src_image);

    /* Write 54 bytes to destination image */
    fwrite(buffer, 54, 1, fptr_dest_image);

    printf("bmp header copied successfully...\n");

    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];

    for(int i = 0; magic_string[i] != '\0'; i++)
    {
        /* Read 8 bytes from source image */
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        /* Encode one character into 8 image bytes */
        encode_byte_to_lsb(magic_string[i], image_buffer);

        /* Write encoded bytes to stego image */
        fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image);
    }

    printf("Magic string encoded successfully...\n");

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int i;

    for(i = 7; i >= 0; i--)
    {
        if((data >> i) & 1)
        {
            /* Set LSB */
            image_buffer[7 - i] =
                image_buffer[7 - i] | 1;
        }
        else
        {
            /* Clear LSB */
            image_buffer[7 - i] =
                image_buffer[7 - i] & ~1;
        }
    }

    return e_success;
}

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char *dot;
    char buffer[32];

    /* Find the extension of secret file */
    dot = strrchr(encInfo->secret_fname, '.');

    if(dot == NULL)
    {
        printf("Secret file extension not found\n");
        return e_failure;
    }

    /* Store secret file extension */
    strcpy(encInfo->extn_secret_file, dot);

    /* Read 32 bytes from source image */
    fread(buffer, 32, 1, encInfo->fptr_src_image);

    /* Encode extension size */
    encode_size_to_lsb(strlen(encInfo->extn_secret_file), buffer);

    /* Write encoded data to stego image */
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);

    printf("Secret file extension size encoded successfully...\n");

    return e_success;
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    
    int i;

    for(i = 31; i >= 0; i--)
    {
        if((size >> i) & 1)
        {
            /* Set LSB */
            image_buffer[31 - i] =
                image_buffer[31 - i] | 1;
        }
        else
        {
            /* Clear LSB */
            image_buffer[31 - i] =
                image_buffer[31 - i] & ~1;
        }
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{

    char buffer[8];

    for(int i = 0; file_extn[i] != '\0'; i++)
    {
        /* Read 8 bytes from source image */
        fread(buffer, 8, 1, encInfo->fptr_src_image);

        /* Encode one character */
        encode_byte_to_lsb(file_extn[i], buffer);

        /* Write encoded bytes */
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }

    printf("Secret file extension encoded successfully...\n");

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{

    char buffer[32];

    /* Read 32 bytes from source image */
    fread(buffer, 32, 1, encInfo->fptr_src_image);

    /* Encode secret file size */
    encode_size_to_lsb(file_size, buffer);

    /* Write encoded bytes to stego image */
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char data;

    while(fread(&data, 1, 1, encInfo->fptr_secret) == 1)
    {
        /* Read 8 bytes from source image */
        fread(buffer, 8, 1, encInfo->fptr_src_image);

        /* Encode one secret character */
        encode_byte_to_lsb(data, buffer);

        /* Write encoded bytes to stego image */
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }

    printf("Secret file data encoded successfully...\n");

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char data;

    while(fread(&data, 1, 1, fptr_src) == 1)
    {
        fwrite(&data, 1, 1, fptr_dest);
    }

    printf("Copied remaining img data successfully...\n");

    return e_success;
}