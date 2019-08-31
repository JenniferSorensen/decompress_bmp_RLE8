#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char file_type[3];
    unsigned int file_size;
    unsigned short app_specific1;
    unsigned short app_specific2;
    unsigned long image_data_offset;
    unsigned int dib_header_size;
    int width_pixels;
    int height_pixels;
    unsigned short color_planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int image_size_bytes;
    int x_resolution;
    int y_resolution;
    unsigned int num_of_colors;
    unsigned int important_colors;
}BITMAPHEADER;

int compressBitmap(FILE* source, FILE* target);
int decompressBitmap(FILE* source, FILE* target);
