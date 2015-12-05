INCLUDES = -I. -I/usr/include 
LIBDIR  = -L. -L/usr/lib
OPTIONS=-g -pg
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig


COMPILERFLAGS = -Wall  -lm
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES =

OBJ = toolkit.o common.o window.o snd_enhance.o noise_est.o tbessi.o lpc.o
PROGRAM = snd-enhance

main:$(OBJ)
	$(CC) `pkg-config --libs sndfile fftw3` -o $(PROGRAM) $(CFLAGS) $(OBJ) $(LIBDIR) $(LIBRARIES)

toolkit.o : toolkit.c toolkit.h common.h snd_enhance.h noise_est.h window.h
	$(CC) -c toolkit.c  $(INCLUDES) $(OPTIONS)

common.o : common.c common.h
	$(CC) -c common.c  $(INCLUDES) $(OPTIONS)
window.o : window.c window.h
	$(CC) -c window.c  $(INCLUDES) $(OPTIONS)
snd_enhance.o : snd_enhance.c snd_enhance.h noise_est.h tbessi.h lpc.h
	$(CC) -c snd_enhance.c  $(INCLUDES) $(OPTIONS)

noise_est.o : noise_est.c noise_est.h
	$(CC) -c noise_est.c  $(INCLUDES) $(OPTIONS)
tbessi.o : tbessi.c tbessi.h
	$(CC) -c tbessi.c  $(INCLUDES) $(OPTIONS)
lpc.o : lpc.c lpc.h
	$(CC) -c lpc.c  $(INCLUDES) $(OPTIONS)

.PHONY : clean

clean :
	@rm -rf *.o *~ sample
