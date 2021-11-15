#include "enemy.h"
#include "llist.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

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
        /* If the player is being shot, freeze all centipede for 2s */
        if(e->p->state == DEAD) {
            sleep(2);
        }
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
        /* If the enemy's length < 5, then it will die and thread will exit */
        if(e->length <= ENEMY_MIN_WIDTH) {
            e->isDead = true;
            //deleteEnemy(e);
            pthread_exit(NULL);
        }

        char** tile_left = ENEMY_BODY_LEFT[e->animTile];
        char** tile_right = ENEMY_BODY_RIGHT[e->animTile];
        char tempArray[ENEMY_HEIGHT][e->length+1];

        if(e->length != ENEMY_WIDTH) {
            /* The enemy is hit and needs to have the anim tiles cut off. */
            /* This one is for right cut off */
            int height_index = 0;
            int width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_right[ENEMY_HEIGHT][ENEMY_WIDTH+1]={"0","0"};
                int z = 0;
                for (width_index = 0; width_index < e->length; width_index++) {
                    body_right[height_index][z] = tile_left[height_index][width_index];
                    z++;
                }
                body_right[height_index][z+1] = '\0'; // Add NULL terminator to the end of the string
                
                strcpy(tempArray[height_index], body_right[height_index]);
                
                // Reverse the string to make it turn right
                strrev(tempArray[height_index]);
                
                tile_right[height_index] = tempArray[height_index]; // Assign tile_right the body value
            }

            /* This one is for left cut off */
            height_index = 0;
            width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_left[ENEMY_HEIGHT][ENEMY_WIDTH+1];
                for (width_index = 0; width_index < e->length; width_index++) {
                    body_left[height_index][width_index] = tile_left[height_index][width_index];
                }
                body_left[height_index][width_index+1] = '\0'; // Add NULL terminator to the end of the string
                tile_left[height_index] = body_left[height_index]; // Assign tile_left the body value
            }
        }

        if(strcmp(e->direction, "right") == 0) {
            
            if(e->startRow == e->row) {
                /* This means this centipede is being cut, so startRow is not meant as a prevRow. It's its new row. */
                /* Hence, we should handle the turning body on the previous row in a different calculation */
                wrappedMutexLock(e->mutex);
                // e->startRow is the previous row, -(e->col+j) is the previous centipede col location
                consoleClearImage(e->startRow-2, -e->col-1, ENEMY_HEIGHT, e->length);
                // e->startRow is the previous row, -(e->col+j+2) is the previous centipede col location - 2
                // to make the marker think it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow-2, -(e->col+3), tile_left, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }
            else {
                wrappedMutexLock(e->mutex);
                // e->startRow is the previous row, -(e->col+j) is the previous centipede col location
                consoleClearImage(e->startRow, -j+1, ENEMY_HEIGHT, e->length);
                // e->startRow is the previous row, -(e->col+j+2) is the previous centipede col location - 2
                // to make the marker think it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow, -(j+2), tile_left, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }
            
            if(e->startCol != 0) {
                /* being cut */
                e->col = e->startCol+j;
                wrappedMutexLock(e->mutex);
                consoleClearImage(e->row, -(e->length)+e->col-1, ENEMY_HEIGHT, e->length); // Clear
                consoleDrawImage(e->row, -(e->length)+e->col+1, tile_right, ENEMY_HEIGHT); // Draw
                wrappedMutexUnlock(e->mutex);
            }
            else {
                wrappedMutexLock(e->mutex);
                consoleClearImage(e->row, -(e->length)+j-1, ENEMY_HEIGHT, e->length); // Clear
                consoleDrawImage(e->row, -(e->length)+j+1, tile_right, ENEMY_HEIGHT); // Draw
                wrappedMutexUnlock(e->mutex);
                e->col = j; // Update e->col's position
            }
            

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%10 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+1, e->col, e->p, e->mutex);
            }

            if(j >= COL_BOUNDARY) {
                // The centipede is gonna taking a turn to the next row (new direction: left)
                e->col = j; // Update e->col to value 80, here, actually.
                e->startRow = e->row; //Update startRow. startRow? More like prevRow!
                e->startCol = 80;
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
            if(e->row != ENEMY_FIRST_ROW) {
                if(e->startRow == e->row) {
                    /* This means this centipede is being cut, so startRow is not meant as a prevRow. It's its new row. */
                    /* Hence, we should handle the turning body on the previous row in a different calculation */
                    //If e-> row does not equal to 2, then the centipede is not on the first row
                    wrappedMutexLock(e->mutex);
                    // e->row-2 is the previous row, e->startCol-i is the previous centipede col location
                    consoleClearImage(e->startRow-2, COL_BOUNDARY-e->length+i-1, ENEMY_HEIGHT, e->length); 
                    // e->row-2 is the previous row, e->startCol-i-2 is the previous centipede col location - 2
                    // to make the marker think it's still running on the previous row, KEKW
                    consoleDrawImage(e->startRow-2, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                    wrappedMutexUnlock(e->mutex);
                }
                else {
                    //If e-> row does not equal to 2, then the centipede is not on the first row
                    wrappedMutexLock(e->mutex);
                    // e->row-2 is the previous row, e->startCol-i is the previous centipede col location
                    consoleClearImage(e->startRow, COL_BOUNDARY-e->length+i-1, ENEMY_HEIGHT, e->length); 
                    // e->row-2 is the previous row, e->startCol-i-2 is the previous centipede col location - 2
                    // to make the marker think it's still running on the previous row, KEKW
                    consoleDrawImage(e->startRow, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                    wrappedMutexUnlock(e->mutex);
                }
            }

            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, e->col+1, ENEMY_HEIGHT, e->length); // e->startCol-i is the current centipede location
            e->col = e->startCol-i-2; // Update e->col's position
		    consoleDrawImage(e->row, e->col, tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%10 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+2, e->col, e->p, e->mutex);
            }  
            
            if(e->col <= 0) {
                // If the enemy hit the left wall in the last turn
                e->col = e->col-i-2; // Update e->col to value 0, here, actually.
                e->startRow = e->row; //Update startRow. startRow? More like prevRow!
                e->startCol = 0;
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

    return NULL;
}