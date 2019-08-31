# decompress_bmp_RLE8
A tool that decompresses RLE8 (Run Length Encoding) compressed bitmap files with a color depth of 8 bits per pixel.

Microsoft documentation for bitmaps: https://docs.microsoft.com/de-de/windows/win32/gdi/bitmaps

Microsoft documentation for bitmap compression: https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-compression

# How to run this tool
Use the makefile to compile the project and run 'decompressbmprle8'. You can use the test image 'lena_compressed.bmp' to test the tool.

# main.c
- Gets user input/output, i.e. the source path of the compressed bitmap and the target path for the resulting decompressed bitmap

- Checks if the source exists and whether it is compressed

- Confirms that the target does not already exist and calls the decompressBitmap function of bmp.c

# bmp.c
- Reads the bitmap's header and calls the decompress assembly function

- Calculates a new bitmap header and stores it with the returned decompressed data at the specified target path

# decompress.S
- Decompresses the bitmap data
