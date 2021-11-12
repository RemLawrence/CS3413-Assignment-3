#include "enemy.h"
#include "llist.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

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
   {"^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^||-",
   ";;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;="},
   {"|^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|@",
   ",;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;="},
   {"||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^|||^@",
   ";,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;;;,;="}
};

/********************support functions***************
/* reset the enemy state to start */
void newEnemy(enemy *e) 
{
	e->row = e->startRow;
	e->col = e->startCol;
	e->animTile = 0;
}

char** cutEnemyBody(char** enemyBody, int length, char* direction) {
    //char newEnemyBody[ENEMY_HEIGHT][length];
    char **newEnemyBody = (char**)(malloc(sizeof(char**)));

    if(strcmp(direction, "left") == 0) {
        int i = 0;
        int j = 0;
        for (i = 0; i < ENEMY_HEIGHT; i++) {
            for (j = 0; j < length; j++) {
                printf("%c\n", enemyBody[i][j]); // char level assign
            }
        }
    }
    else {
        int i = ENEMY_HEIGHT-1;
        int j = length-1;
        for (i = ENEMY_HEIGHT-1; i >= 0; i--) {
            int z = ENEMY_WIDTH-1;
            for (j = length-1; j >= 0; j--) {
                newEnemyBody[i][j] = enemyBody[i][z]; // char level assign
                z--;
            }
        }
    }
    return newEnemyBody;
}

/********************THREAD functions***************/
void *runEnemy(void *data) {
    // Pass the reference to the player p
	enemy* e = (enemy*)data;
	/* RESET the player state to start */
	newEnemy(e);

	int i = 0; // aka. leftIncrementor
    int j = 0; // aka. rightIncrementor

    while(e->p->running && e->p->lives > 0) {
        char** tile_left;
        char** tile_right;
        if(e->length == ENEMY_WIDTH) {
            // If the enemy has a width of exactly 80, then it hasen't been hit just yet.
            tile_left = ENEMY_BODY_LEFT[i%ENEMY_BODY_ANIM_TILES];
            tile_right = ENEMY_BODY_RIGHT[j%ENEMY_BODY_ANIM_TILES];
        }
        else {
            // The enemy is hit and needs to have the anim tiles cut off.
            tile_left = cutEnemyBody(ENEMY_BODY_LEFT[i%ENEMY_BODY_ANIM_TILES], e->length, "left");
            tile_right = cutEnemyBody(ENEMY_BODY_RIGHT[j%ENEMY_BODY_ANIM_TILES], e->length, "right");
        }

        //probably not threadsafe here...
        //start centipede at tile 2, 80, move it horizontally once a frame/tick
        //we create the illusion of movement by clearing the screen where the centipede was last
        //then drawing it in the new location. 
        
        if(strcmp(e->direction, "right") == 0) {

            wrappedMutexLock(e->mutex);
            // e->startRow is the previous row, -(e->col+j) is the previous centipede col location
            consoleClearImage(e->startRow, -j, ENEMY_HEIGHT, ENEMY_WIDTH); 
            // e->startRow is the previous row, -(e->col+j+2) is the previous centipede col location - 2
            // to make the marker think it's still running on the previous row, KEKW
		    consoleDrawImage(e->startRow, -(j+2), tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, -(e->length)+j, ENEMY_HEIGHT, ENEMY_WIDTH); // Clear
            consoleDrawImage(e->row, -(e->length)+j+1, tile_right, ENEMY_HEIGHT); // Draw
            wrappedMutexUnlock(e->mutex);
            e->col = j;

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%8 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+1, e->col, e->p, e->mutex);
            }

            if(j >= COL_BOUNDARY) {
                // The centipede is gonna taking a turn to the next row (new direction: left)
                e->col = j; // Update e->col to value 80, here, actually.
                e->startRow = e->row; //Update startRow. startRow? More like prevRow!
                if(e->row != LAST_ROW) {
                    e->row = e->row + 2; // Get it to the next row
                }
                else {
                    e->row = LAST_ROW; // Get it to the next row
                }
                
                j = 0; // Clear the right incrementor to 0, for the next use
                e->direction = "left"; // The enemy is now turning left!
            }
            else {
                j++;
            }
            
        }
        else {
            //printf("%c\n", tile_left[0][0]);
            if(e->row != ENEMY_FIRST_ROW) {
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
            consoleClearImage(e->row, e->col, ENEMY_HEIGHT, ENEMY_WIDTH); // e->startCol-i is the current centipede location
            e->col = COL_BOUNDARY-i-2;
		    consoleDrawImage(e->row, e->col, tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%8 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+2, e->col, e->p, e->mutex);
            }  
            
            if(e->col <= 0) {
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

        if(e->isHit) {
            // Catch the isHit signal from player bullet thread, consume it by increasing the speed of this enemy.
            e->speed = e->speed/2;
            e->isHit = false;
        }

		sleepTicks(e->speed);
	}
    // free(tile_left);
    // free(tile_right);
    pthread_exit(NULL);	
}