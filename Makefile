CC=gcc
# Obtenu avec 'pkg-config --cflags --libs libavformat libavcodec libavutil libswscale sdl'
CFLAGS= -g -Wall -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/local/include -I/usr/include/SDL
LDFLAGS= -pthread -L/usr/local/lib -lavformat -lavcodec -lXv -lXext -lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lX11 -lasound -lz -lswscale -lavutil -lm -lSDL

%OBJ= xtract.o

all: clean xtract

xtract: xtract.o
	$(CC) $(CFLAGS) xtract.o $(LDFLAGS) -o xtract

xtract.o: xtract.c

clean:
	rm -f *~ $(OBJ) xtract

#gcc --coverage -g extract.o -lswscale -lavutil -lavformat -lavcodec -lm -o extract

#gcc --coverage -g -o extract extract.c -lswscale -lavutil -lavformat -lavcodec -lavutil -lm
