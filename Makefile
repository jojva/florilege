CC=gcc
# Following flags obtained through 'pkg-config --cflags --libs libavformat libavcodec libavutil libswscale sdl'
CFLAGS= -g -Wall -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/local/include -I/usr/include/SDL
LDFLAGS= -pthread -L/usr/local/lib -lavformat -lavcodec -lXv -lXext -lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lX11 -lasound -lz -lswscale -lavutil -lm -lSDL

%OBJ= florilege.o

all: clean florilege

florilege: florilege.o
	$(CC) $(CFLAGS) florilege.o $(LDFLAGS) -o florilege

florilege.o: florilege.c

clean:
	rm -f *~ $(OBJ) florilege
