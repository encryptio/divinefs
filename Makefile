CFLAGS += -O2 -Wall -g -std=c99 -D_FILE_OFFSET_BITS=64 -I.

FSOBJECTS = fs/ext.o fs/linuxswap.o fs/reiserfs.o fs/ffs.o fs/btrfs.o fs/ntfs.o fs/fat.o fs/hfsplus.o
OBJECTS = main.o scan.o format.o util.o cachedread.o $(FSOBJECTS)

.SUFFIXES: .c .o

all: divinefs

divinefs: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o divinefs

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f divinefs