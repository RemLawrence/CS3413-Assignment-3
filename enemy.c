#include "enemy.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>

char* ENEMY_BODY_LEFT[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
   {"@|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||",
   "=;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,"},
  {"@||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^",
   "=;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;"},
  {"@|^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|",
   "=,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;"},
  {"-^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^||",
   "=;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;"}
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

enemy* spawnEnemy(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    enemy* e = (enemy*)(malloc(sizeof(enemy)));

	e->startCol = startCol;
	e->startRow = startRow;
	e->running = true;
	e->isHit = false;
    e->turningRight = false;
    e->turningLeft = false;
    e->length = 80;
    e->mutex = screenLock;
    e->p = p;

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

	int i = 0;
    while(e->p->running && e->p->lives >= 0) {
		char** tile = ENEMY_BODY_LEFT[i%ENEMY_BODY_ANIM_TILES];

        //probably not threadsafe here...
        //start centipede at tile 2, 80, move it horizontally once a frame/tick
        //we create the illusion of movement by clearing the screen where the centipede was last
        //then drawing it in the new location. 

        if(e->col == 0) {
            // If the enemy hit the left wall in the last turn
            e->row = e->row + 2; // Get it to the second row
            e->startRow = e->row; //Update startRow
            i = 0; // clear i's value to 0
        }
        
        if(e->turningRight) {

            // wrappedMutexLock(e->mutex);
            // consoleClearImage(e->row, e->col, ENEMY_HEIGHT, ENEMY_WIDTH); // Clear
            // e->col = e->col+i; // col got incremented
		    // consoleDrawImage(e->row, e->col, tile, ENEMY_HEIGHT);
            // wrappedMutexUnlock(e->mutex);
        }
        else {
            e->row = e->startRow;
            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, e->startCol-i, ENEMY_HEIGHT, ENEMY_WIDTH);
            e->col = e->startCol-i-2;
		    consoleDrawImage(e->row, e->col, tile, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);
        }
		//consoleRefresh(); //draw everything to screen.
		sleepTicks(3);
        i++;
	}
    return NULL;	
}