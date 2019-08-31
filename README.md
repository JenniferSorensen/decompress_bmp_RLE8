# decompress_bmp_RLE8
A tool that decompresses RLE8 (Run Length Encoding) compressed bitmap files. Bitmap needs to have a color depth of 8 bits per pixel.

Link to the Microsoft documentation for Bitmaps: https://docs.microsoft.com/de-de/windows/win32/gdi/bitmaps

Link to the Microsoft documentation for Bitmap Compression: https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-compression

# How to run this project
This project is supposed to run on a Unix system. Use the makefile to compile the project and run 'decompressbmprle8'.

# main.c
Contains the code that gets the source path of the compressed bitmap file and the target path for the resulting decompressed bitmap file from the user. The code checks if the source file exists and if it is compressed. It also checks if the target path does already exist. If the file is a compressed bitmap the decompressBitmap function in bmp.c gets called.

# bmp.c
Contains the code that reads the bitmap header and calls the decompress assembly function. Once the decompress function returns the resulting decompressed data gets a new header and will be stored at the specified target path.

# decompress.S
Decompresses the bitmap data.
