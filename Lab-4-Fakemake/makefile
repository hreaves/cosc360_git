CC = gcc
INCLUDES = -I/home/mrjantz/cs360/include
CFLAGS = $(INCLUDES)
LIBDIR = /home/mrjantz/cs360/lib
LIBS = $(LIBDIR)/libfdr.a

EXECUTABLES = fakemake 

all: $(EXECUTABLES)

fakemake: fakemake.c
	$(CC) $(CFLAGS) -o fakemake fakemake.c $(LIBS)

clean:
	rm core $(EXECUTABLES) fakemake
