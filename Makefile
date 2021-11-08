CC = gcc

BASEFLAGS = -Wall -pthread -std=c99
NODEBUG_FLAGS = -dNDEBUG 
DEBUG_FLAGS = -g

LDLIBS = -lcurses -pthread

OBJS = main.o console.o centipede.o player.o enemy.o llist.o bullet.o gameglobals.o threadwrappers.o

EXE = centipede

debug: CFLAGS = $(BASEFLAGS) $(DEBUG_FLAGS)
debug: $(EXE)

release: CFLAGS = $(BASEFLAGS) $(NODEBUG_FLAGS) 
release: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

main.o: main.c centipede.h
	$(CC) $(CFLAGS) -c main.c


player.o: player.c player.h centipede.h console.h threadwrappers.h
	$(CC) $(CFLAGS) -c player.c

enemy.o: enemy.c enemy.h console.h threadwrappers.h
	$(CC) $(CFLAGS) -c enemy.c

llist.o: llist.c llist.h enemy.h console.h threadwrappers.h
	$(CC) $(CFLAGS) -c llist.c

bullet.o: bullet.c bullet.h llist.h player.h enemy.h console.h threadwrappers.h
	$(CC) $(CFLAGS) -c bullet.c

upkeep.o: gameglobals.c gameglobals.h bullet.h llist.h player.h enemy.h console.h threadwrappers.h
	$(CC) $(CFLAGS) -c gameglobals.c

console.o: console.c console.h
	$(CC) $(CFLAGS) -c console.c


threadwrappers.o: threadwrappers.c threadwrappers.h
	$(CC) $(CFLAGS) -c threadwrappers.c

centipede.o: centipede.h centipede.c console.h
	$(CC) $(CFLAGS) -c centipede.c

clean:
	rm -f $(OBJS)
	rm -f *~
	rm -f $(EXE)
	rm -f $(EXE)_d

run:
	./$(EXE)

run_debug:
	./$(EXE)_d
