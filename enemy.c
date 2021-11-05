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

char* ENEMY_BODY_RIGHT[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
   {"|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||@",
   ";;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,="},
  {"||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^@",
   ";,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;="},
  {"|^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|@",
   ",;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;="},
  {"^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^||-",
   ";;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;="}
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

	e->startCol = startCol; // Initialize the enemy's startCol to the upper left of the console (78)
	e->startRow = startRow; // Initialize the enemy's startRow to the upper left of the console (2)

	e->running = true;
	e->isHit = false;

    e->direction = "left";

    e->length = ENEMY_WIDTH; // Length of the enemy body. Could be deducted when hit!

    e->mutex = screenLock; // A reference to the screenlock
    e->p = p; // A reference to the player

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

	int i = 0; // aka. leftIncrementor
    int j = 0; // aka. rightIncrementor

    while(e->p->running && e->p->lives >= 0) {
		char** tile_left = ENEMY_BODY_LEFT[i%ENEMY_BODY_ANIM_TILES];
        char** tile_right = ENEMY_BODY_RIGHT[j%ENEMY_BODY_ANIM_TILES];

        //probably not threadsafe here...
        //start centipede at tile 2, 80, move it horizontally once a frame/tick
        //we create the illusion of movement by clearing the screen where the centipede was last
        //then drawing it in the new location. 
        
        if(strcmp(e->direction, "right") == 0) {

            wrappedMutexLock(e->mutex);
            // e->startRow is the previous row, -(e->col+j) is the previous centipede col location
            consoleClearImage(e->startRow, -(e->col+j), ENEMY_HEIGHT, ENEMY_WIDTH); 
            // e->startRow is the previous row, -(e->col+j+2) is the previous centipede col location - 2
            // to make the marker think it's still running on the previous row, KEKW
		    consoleDrawImage(e->startRow, -(e->col+j+2), tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, -(e->length)+j, ENEMY_HEIGHT, ENEMY_WIDTH); // Clear
            consoleDrawImage(e->row, -(e->length)+j+2, tile_right, ENEMY_HEIGHT); // Draw
            wrappedMutexUnlock(e->mutex);

            if(e->col+j >= 80) {
                // The centipede is gonna taking a turn to the next row (new direction: left)
                e->col = e->col+j; // Update e->col to value 80, here, actually.
                e->startRow = e->row; //Update startRow. startRow? More like prevRow!
                if(e->row != 14){
                    e->row = e->row + 2; // Get it to the next row
                }
                else {
                    e->row = 14; // Get it to the next row
                }
                
                j = 0; // Clear the right incrementor to 0, for the next use
                e->direction = "left"; // The enemy is now turning left!
            }
            else {
                j++;
            }
            
        }
        else {
            if(e->row != 2) {
                //If e-> row does not equal to 2, then the centipede is not on the first row
                wrappedMutexLock(e->mutex);
                // e->row-2 is the previous row, e->startCol-i is the previous centipede col location
                //printf("e->startRow %d\n", e->startRow);
                consoleClearImage(e->startRow, COL_BOUNDARY-e->length+i, ENEMY_HEIGHT, ENEMY_WIDTH); 
                // e->row-2 is the previous row, e->startCol-i-2 is the previous centipede col location - 2
                // to make the marker think it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, e->col-i, ENEMY_HEIGHT, ENEMY_WIDTH); // e->startCol-i is the current centipede location
		    consoleDrawImage(e->row, e->col-i-2, tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);
            
            if(e->col-i-2 <= 0) {
                // If the enemy hit the left wall in the last turn
                e->col = e->col-i-2; // Update e->col to value 0, here, actually.
                e->startRow = e->row; //Update startRow. startRow? More like prevRow!
                if(e->row != LOWER_ROW_BOUNDARY){
                    e->row = e->row + 2; // Get it to the next row
                }
                else {
                    e->row = LOWER_ROW_BOUNDARY; // Get it to the next row
                }
                i = 0; // clear left incrementor value to 0, for the next use
                e->direction = "right"; // The enemy is now turning right!
            }
            else {
                i++;
            }
        }
		sleepTicks(3);
	}
    return NULL;	
}