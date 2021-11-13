#include "enemy.h"
#include "llist.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>



/********************support functions****************/
/* reset the enemy state to start */
void newEnemy(enemy *e) 
{
	e->row = e->startRow;
	e->col = e->startCol;
	e->animTile = 0;
}

/* reverse a string 
    e.g. hello -> olleh */
void strrev(char *str) {
    char *p1, *p2;
    for (p1=str, p2=str+strlen(str)-1; p2>p1; ++p1, --p2){
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
}

/********************THREAD function***************/
void *runEnemy(void *data) {

    // Pass the reference to the player p
	enemy* e = (enemy*)data;
    //wrappedMutexInit(&e->enemyLock, NULL);
	/* RESET the player state to start */
	newEnemy(e);
	int i = 0; // aka. leftIncrementor
    int j = 0; // aka. rightIncrementor

    while(e->p->running && e->p->lives > 0) {
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
        e->animTile++;
		e->animTile %= ENEMY_BODY_ANIM_TILES;
        //wrappedMutexLock(&e->enemyLock);
        /* If the enemy's length < 5, then it will die and thread will exit */
        if(e->length <= 4) {
            e->isDead = true;
            pthread_exit(NULL);
        }

        char** tile_left = ENEMY_BODY_LEFT[e->animTile];
        char** tile_right = ENEMY_BODY_RIGHT[e->animTile];

        if(e->length != ENEMY_WIDTH) {
            // The enemy is hit and needs to have the anim tiles cut off.
            int height_index = 0;
            int width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_right[2][81];
                //int z = 0;
                //for (width_index = 0; width_index < e->length; width_index++) {
                    char subBuff[e->length+1];
                    memcpy(subBuff, &tile_right[height_index][80-e->length], e->length);
                    subBuff[e->length] = '\0';
                    //body_right[height_index][z] = tile_left[height_index][width_index];
                    //z++;
                //}
                // Reverse the string to make it turn right
                //strrev(body_right[height_index]);
                

                //body_right[height_index][z+1] = '\0'; // Add NULL terminator to the end of the string
                tile_right[height_index] = subBuff; // Assign tile_right the body value
                //printf("%s\n", tile_right[height_index]);
            }

            height_index = 0;
            width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_left[2][81];
                for (width_index = 0; width_index < e->length; width_index++) {
                    body_left[height_index][width_index] = tile_left[height_index][width_index];
                }
                body_left[height_index][width_index+1] = '\0'; // Add NULL terminator to the end of the string
                tile_left[height_index] = body_left[height_index]; // Assign tile_left the body value
            }
                
        }
        
        if(strcmp(e->direction, "right") == 0) {
            wrappedMutexLock(e->mutex);
            // e->startRow is the previous row, -(e->col+j) is the previous centipede col location
            consoleClearImage(e->startRow, -j, ENEMY_HEIGHT, e->length);
            // e->startRow is the previous row, -(e->col+j+2) is the previous centipede col location - 2
            // to make the marker think it's still running on the previous row, KEKW
		    consoleDrawImage(e->startRow, -(j+2), tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, -(e->length)+j, ENEMY_HEIGHT, e->length); // Clear
            consoleDrawImage(e->row, -(e->length)+j+1, tile_right, ENEMY_HEIGHT); // Draw
            wrappedMutexUnlock(e->mutex);
            e->col = j; // Update e->col's position

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%8 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                //spawnEnemyBullet(e->row+1, e->col, e->p, e->mutex);
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
            /* The enemy is hit and needs to have the anim tiles cut off. */

            if(e->row != ENEMY_FIRST_ROW) {
                //If e-> row does not equal to 2, then the centipede is not on the first row
                wrappedMutexLock(e->mutex);
                // e->row-2 is the previous row, e->startCol-i is the previous centipede col location
                //printf("e->startRow %d\n", e->startRow);
                consoleClearImage(e->startRow, COL_BOUNDARY-e->length+i, ENEMY_HEIGHT, e->length); 
                // e->row-2 is the previous row, e->startCol-i-2 is the previous centipede col location - 2
                // to make the marker think it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, e->col, ENEMY_HEIGHT, e->length); // e->startCol-i is the current centipede location
            e->col = e->startCol-i-2; // Update e->col's position
		    consoleDrawImage(e->row, e->col, tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%8 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                //spawnEnemyBullet(e->row+2, e->col, e->p, e->mutex);
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
            /* Catch the isHit signal from player bullet thread, consume it by increasing the speed of this enemy. */
            if(e->speed == ENEMY_SPEED/2) {
                /* Maximum speed: 40/2 */
            }
            else {
                e->speed = e->speed/2;
            }
            e->isHit = false; // Turn off the hit flag
        }
        //wrappedMutexUnlock(&e->enemyLock);
		sleepTicks(e->speed);
	}

    pthread_exit(NULL);	
}