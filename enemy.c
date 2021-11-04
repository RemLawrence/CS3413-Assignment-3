#include "enemy.h"
#include <stdio.h>
#include <curses.h>

char* ENEMY_BODY[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
  {"1",
   "1"},
  {"2",
   "2"},
  {"3",
   "3"},
  {"4",
   "4"}
};

/********************support functions***************/
/* reset the player state to start */
void newEnemy(enemy *e) 
{
	e->row = e->startRow;
	e->col = e->startCol;
	e->animTile = 0;
	//p->state = GAME;
}

/********************THREAD functions***************/

enemy* spawnEnemy(int startRow, int startCol, pthread_mutex_t *screenLock)
{
    enemy* e = (enemy*)(malloc(sizeof(enemy)));

	e->startCol = startCol;
	e->startRow = startRow;
	e->running = true;
	e->isHit = false;
    e->mutex = screenLock;

	//TODO: Init mutex...
	wrappedMutexInit(e->mutex, NULL);
	wrappedPthreadCreate(&(e->thread), NULL, runEnemy, (void*)e);
	return e;
}

void *runEnemy(void *data) {
    // Pass the reference to the player p
	enemy* e = (enemy*)data;
	/* RESET the player state to start */
	newEnemy(e);

    //animate an "enemy" made of numbers on the screen every second for 10s
    //this isn't part of my solution, but is for illustration purposes
	int i = 0;
    while(1) {
		char** tile = ENEMY_BODY[i%ENEMY_BODY_ANIM_TILES];

        //probably not threadsafe here...
        //start centipede at tile 0, 80, move it horizontally once a frame/tick
        //we create the illusion of movement by clearing the screen where the centipede was last
        //then drawing it in the new location. 

        wrappedMutexLock(e->mutex);
        consoleClearImage(2, 80-i, ENEMY_HEIGHT, ENEMY_WIDTH);
		consoleDrawImage(2, 80-i-1, tile, ENEMY_HEIGHT);
        wrappedMutexUnlock(e->mutex);
		//consoleRefresh(); //draw everything to screen.
		sleepTicks(60);
        i++;
	}	
}