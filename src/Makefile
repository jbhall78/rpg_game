NAME=../game
OBJECTS=main.o map.o npc.o player.o screen.o util.o window.o
COMPILER=gcc -pg
LINKER=gcc -pg
OPTIONS=-Wall
CFLAGS = $(shell sdl-config --cflags) $(OPTIONS)
LIBS = $(shell sdl-config --libs) #-lSDL_mixer

all: $(OBJECTS)
	$(LINKER) -o $(NAME) $(OBJECTS) $(LIBS)
.c.o :
	$(COMPILER) $(CFLAGS) -c -o $*.o $*.c 
clean :
	rm -f *.o
