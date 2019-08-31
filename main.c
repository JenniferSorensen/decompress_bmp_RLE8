#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bmp.h"

char sourcePath[100];
char targetPath[100];
int method = 3; //"compress" == 0; "decompress" == 1; "RLE4 (not supported)" == 2

int checkIfFileExists(const char *filename) {
	FILE *file;
	if ((file = fopen(filename, "r"))) {
		fclose(file);
		return 1;
	}
	return 0;
}

void getSourcepathFromUser() {
	printf("Please specify the path of the .bmp file you want to decompress.\n");
	fgets(sourcePath, 100, stdin);
	strtok(sourcePath, "\n");

	if (checkIfFileExists(sourcePath) == 0) {
		printf("This file does not exist. Please try again.\n");
		getSourcepathFromUser();
	}
}

void extractMethod() {
    FILE *file;
    if ((file = fopen(sourcePath, "r"))) {
        for (int i=0; i <= 30; i++) {
            method = (char)fgetc(file);
        }
        fclose(file);
    }
}

void checkMethod() {
    if(method == 2) {
        printf("This file is RLE4 compressed. This tool can't decompress RLE4 compressed files.\n");
        abort();
    } else if (method == 0) {
        printf("This is not a compressed file.");
        abort();
    } else if (method != 1) {
        printf("There was a problem with your file. Maybe the file is corrupted.\n");
        abort();
    }
}

void getTargetpathFromUser() {
    if (method == 1) {
		printf("This file is comrpressed. Please specify the path for the decompressed image.\n");
	}
    fgets(targetPath, 100, stdin);
	strtok(targetPath, "\n");

	if (checkIfFileExists(targetPath) == 1) {
		printf("This file already exists.\n");
		getTargetpathFromUser();
	}
}

void decompressFile(){
	FILE * sourceFile;
	FILE * targetFile;
	sourceFile = fopen(sourcePath, "rb");
	targetFile = fopen(targetPath, "wb");
    
    if (method == 1) {
		switch(decompressBitmap(sourceFile, targetFile)) {
			case 0: printf("The file was successfully decompressed.\n"); break;
			case 1: printf("The file couldn't be decompressed.\n"); abort();
			default: printf("Something went wrong."); abort();
		}
	}

  fclose(sourceFile);
  fclose(targetFile);
}

int main(int argc, char *argv[]) {
	getSourcepathFromUser();
	extractMethod();
	checkMethod();
	getTargetpathFromUser();
	decompressFile();

	return 0;
}
