#Add additional compiler flags here
CFLAGS=-O0

all: decompressbmprle8
decompressbmprle8: main.c bmp.c decompress.S
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f main

