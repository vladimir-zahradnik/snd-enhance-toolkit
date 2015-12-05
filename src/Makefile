INCLUDES = -I. -I/usr/include 
LIBDIR  = -L. -L/usr/lib
OPTIONS=-g -pg
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig


COMPILERFLAGS = -Wall  -lm
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES =

OBJ = toolkit.o common.o
PROGRAM = setk

main:$(OBJ)
	$(CC) `pkg-config --libs sndfile fftw3` -o $(PROGRAM) $(CFLAGS) $(OBJ) $(LIBDIR) $(LIBRARIES)

toolkit.o : toolkit.c common.c common.h
	$(CC) -c toolkit.c  $(INCLUDES) $(OPTIONS)

common.o : common.c common.h
	$(CC) -c common.c  $(INCLUDES) $(OPTIONS)

.PHONY : clean

clean :
	@rm -rf *.o *~ sample