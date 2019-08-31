#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

extern int decompress(void *ptr, void *res, unsigned int size, unsigned int image_width);

BITMAPHEADER *header;
char *data;
char *result;
int processedImageSize;
int fileSizeBefore;
int fileSizeAfter;

unsigned int readLittleEndian(int bytes, FILE *fp) {
  unsigned char buffer[bytes];
  unsigned int val = 0;

  fread(buffer, 1, bytes, fp);
  for (int i = bytes-1; i >= 0; i--) {
      val = (val << 8) | (unsigned short) buffer[i];
  }
  return val;
}

void readBitmapHeader(FILE *fp) {
  header = (BITMAPHEADER *) malloc(sizeof(BITMAPHEADER));

  /* File type (2 bytes) */
  fread(&header->file_type, 1, 2, fp);
  /* File size (4 bytes) */
  header->file_size = (unsigned int) readLittleEndian(4,fp);
  /* Reserved 1 (2 bytes) */
  header->app_specific1 = (unsigned short) readLittleEndian(2,fp);
  /* Reserved 2 (2 bytes) */
  header->app_specific2 = (unsigned short) readLittleEndian(2,fp);
  /* Offset (4 bytes) */
  header->image_data_offset = (unsigned long) readLittleEndian(4,fp);
  /* DIB_header_Size (4 bytes) */
  header->dib_header_size = (unsigned int) readLittleEndian(4,fp);
  /* Width pixels (4 bytes) */
  header->width_pixels = (int) readLittleEndian(4,fp);
  /* height pixels (4 bytes) */
  header->height_pixels = (int) readLittleEndian(4,fp);
  /* Color planes (2 bytes) */
  header->color_planes = (unsigned short) readLittleEndian(2,fp);
  /* Bits per pixel (2 bytes) */
  header->bits_per_pixel = (unsigned short) readLittleEndian(2,fp);
  /* Compression (4 bytes) */
  header->compression = (unsigned int) readLittleEndian(4,fp);
  /* Image Size Bytes (4 bytes) */
  header->image_size_bytes = (unsigned int) readLittleEndian(4,fp);
  /* X resolution (4 bytes) */
  header->x_resolution = (int) readLittleEndian(4,fp);
  /* Y resolution (4 bytes) */
  header->y_resolution = (int) readLittleEndian(4,fp);
  /* Num colors (4 bytes) */
  header->num_of_colors = (unsigned int) readLittleEndian(4,fp);
  /* Important colors (4 bytes) */
  header->important_colors = (unsigned int) readLittleEndian(4,fp);

  fseek(fp, 0, SEEK_SET);
}

void copyBitmapDataIntoDataArray(FILE* source) {
	data = malloc(header->file_size * sizeof(char));

	for (int i=0; i < header->file_size; i++) {
		data[i] = (char)fgetc(source);
	}
}

void writeResultToTarget(FILE* target, char* newHeader) {
  for(int i=0; i < header->image_data_offset; i++) {
    fwrite(&newHeader[i], 1, sizeof(char), target);
  }

  for(int i=0; i < processedImageSize; i++) {
    fwrite(&result[i], 1, sizeof(char), target);
  }
}

char * UnsignedIntToLittleEndian4Bytes (unsigned int x) {
  char *byteArray = malloc(sizeof(char[4]));
  byteArray[0] = (char)(x >>  0);
  byteArray[1] = (char)(x >>  8);
  byteArray[2] = (char)(x >> 16);
  byteArray[3] = (char)(x >> 24);
  return byteArray;
}

void getNewHeader(int isCompressMethod, char* headerArray) {
  for(int i = 0; i < header->image_data_offset; i++) {
   	headerArray[i] = data[i];
  }
	for(int i = 0; i < 4; i++) {
	  headerArray[2+i] = *(UnsignedIntToLittleEndian4Bytes(processedImageSize + header->image_data_offset)+i);
  }
  for(int i = 0; i < 4; i++) {
	  headerArray[30+i] = *(((isCompressMethod == 1) ? UnsignedIntToLittleEndian4Bytes(1) : UnsignedIntToLittleEndian4Bytes(0))+i);
    headerArray[34+i] = *(UnsignedIntToLittleEndian4Bytes((unsigned int)processedImageSize)+i);
  }
}

int callAsmDecompress(int resultSizeModifier) {
	char* pointerToImageDataOffset = &data[header->image_data_offset];
	result = malloc(header->image_size_bytes * resultSizeModifier);
	return decompress(pointerToImageDataOffset, result, header->image_size_bytes * resultSizeModifier, header->width_pixels);
}

void loadData(FILE* source) {
	readBitmapHeader(source);
	copyBitmapDataIntoDataArray(source);
    fileSizeBefore = header->file_size;
}

void calculateCompressionRateAndPrintToConsole(int method) {
	double compressionRate = (double) fileSizeAfter / (double) fileSizeBefore;
		printf("The file was decompressed from %d Byte to %d Byte. This is %.2f%% of the original size.\n", fileSizeBefore, fileSizeAfter, compressionRate*100);
}

int decompressBitmap(FILE* source, FILE* target) {
	loadData(source);
	processedImageSize = (header->image_size_bytes < 1000) ? callAsmDecompress(100) : callAsmDecompress(20);

  if (processedImageSize == -1) {
     //decompressed image size is bigger than the allocated memory.
     return 1;
  }

	char headerArray[header->image_data_offset];
	getNewHeader(0, headerArray);
	writeResultToTarget(target, headerArray);
	fileSizeAfter = header->image_data_offset + processedImageSize;
	calculateCompressionRateAndPrintToConsole(1);
	return 0;
}
