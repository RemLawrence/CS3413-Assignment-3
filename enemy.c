/**********************************************************************
  Module: enemy.c
  Author: Micah Hanmin Wang

  Contains all the functions related to the enemies (aka. centipede),
  including helper functions like initializing new enemies and reverse
  a string; also includes that big enemy thread function where all the 
  enemy moving logics are at.

  Purpose: Manages every single enemies, make sure they are thread safe.

**********************************************************************/

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

/********************************************************** 
    Reset the enemy state to start,
    Initialize enemy's initial position (row+col)
 **********************************************************/
void newEnemy(enemy *e) 
{
	e->row = e->startRow;
	e->col = e->startCol;
	e->animTile = 0;
}

/********************************************************** 
    reverse a string 
    e.g. hello -> olleh 
**********************************************************/
void strrev(char *str) {
    char *p1, *p2;
    for (p1=str, p2=str+strlen(str)-1; p2>p1; ++p1, --p2){
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
}

/********************THREAD function***************/

/********************************************************** 
    The Enemy (aka caterpillar) Thread function.
    Initialize the new caterpiller. Manages its animation tiles
    and body strings, performs cut on the body when needed.
    Has the logic for each enemy to move, and wraps appropriately, 
    whether it comes from left to right or right to left.
	If the enemy is too small (length < 5), it dies.
 **********************************************************/
void *runEnemy(void *data) {

    // Pass the reference to the player p
	enemy* e = (enemy*)data;
    //wrappedMutexInit(&e->enemyLock, NULL);
	/* RESET the player state to start */
	newEnemy(e);
	int i = 0; // aka. leftIncrementor. Call it i for simplicity, please don't deduct marks on the naming
    int j = 0; // aka. rightIncrementor. Call it j for simplicity, please don't deduct marks on the naming

    while(e->p->running && e->p->lives > 0) {
        /* If the player is being shot, freeze all centipedes for 2s */
        if(e->p->state == DEAD) {
            sleep(2);
        }
        /* Caterpiller's left body */
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
        /* Caterpiller's right body */
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
            /* TODO */
            deleteEnemy(e);
            pthread_exit(NULL);
        }

        char** tile_left = ENEMY_BODY_LEFT[e->animTile]; /* A temporary string for the enemy which goes from right to left */
        char** tile_right = ENEMY_BODY_RIGHT[e->animTile]; /* A temporary string for the enemy which goes from left to right */
        char tempArray[ENEMY_HEIGHT][e->length+1]; /* A temporary 2d string array for the cutted enemy which goes from left to right, if needed */

        if(e->length != ENEMY_WIDTH) {
            /* The enemy is hit and needs to have the anim tiles cut off. */
            /* This one is for right cut off */
            int height_index = 0; 
            int width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_right[ENEMY_HEIGHT][ENEMY_WIDTH+1]={"0","0"}; /* The temp 2d string array for storing right body */
                int z = 0; /* right body's index, for reading the char in */
                for (width_index = 0; width_index < e->length; width_index++) {
                    /* Read the char from left body according to enemy's length, storing it into the temp array */
                    body_right[height_index][z] = tile_left[height_index][width_index];
                    z++;
                }
                body_right[height_index][z+1] = '\0'; // Add NULL terminator to the end of the string
                
                /* Copy the temp 2d string array into another 2d string array, to prevent overwriting on the left body */
                strcpy(tempArray[height_index], body_right[height_index]);
                
                /* Reverse the string to make it turn right */
                strrev(tempArray[height_index]);
                
                /* Pass the reference to the right body, completing cutting the right string body */
                tile_right[height_index] = tempArray[height_index];
            }

            /* This one is for left cut off */
            height_index = 0;
            width_index = 0;
            for(height_index = 0; height_index < ENEMY_HEIGHT; height_index++) {
                char body_left[ENEMY_HEIGHT][ENEMY_WIDTH+1]; /* The temp 2d string array for storing left body */
                for (width_index = 0; width_index < e->length; width_index++) {
                    /* Read the char from left body according to enemy's length, storing it into the temp array */
                    body_left[height_index][width_index] = tile_left[height_index][width_index];
                }
                body_left[height_index][width_index+1] = '\0'; // Add NULL terminator to the end of the string
                tile_left[height_index] = body_left[height_index]; /* Pass the reference to the left body, completing cutting the left string body */
            }
        }

        /* If the caterpiller is going from left to right */
        if(strcmp(e->direction, "right") == 0) {
            
            /********************************************************** 
                This is for cleaning the wrapping body on the prevRow,
                if the enemy is going from left to right.
            **********************************************************/
            if(e->startRow == e->row) {
                /* Remember we use startRow as 'prevRow'
                   If the startRow == current row
                   This means this caterpiller is being CUT while WRAPPING, so startRow is not meant as a prevRow. It's its new row.
                   Hence, we should handle the turning body on the previous row in a different calculation */
                wrappedMutexLock(e->mutex);
                // e->startRow-2 is the previous row, -e->col-1 is the previous caterpiller's col location
                consoleClearImage(e->startRow-2, -e->col-1, ENEMY_HEIGHT, e->length);
                // e->startRow-2 is the previous row, -(e->col+3) is the previous caterpiller's col location - 3
                // to create an illusion that it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow-2, -(e->col+3), tile_left, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }
            else {
                /* Remember we use startRow as 'prevRow'
                   If the startRow != current row
                   This means this caterpiller is NOT being CUT while WRAPPING, so startRow is prevRow.
                   Hence, we just handle the wrapping body on the prevRow normally */
                wrappedMutexLock(e->mutex);
                // e->startRow is the previous row, -j+1 is the previous caterpiller's col location
                consoleClearImage(e->startRow, -j+1, ENEMY_HEIGHT, e->length);
                // e->startRow is the previous row, -(j+2) is the previous caterpiller's col location - 2
                // to create an illusion that it's still running on the previous row, KEKW
                consoleDrawImage(e->startRow, -(j+2), tile_left, ENEMY_HEIGHT);
                wrappedMutexUnlock(e->mutex);
            }
            
            /********************************************************** 
                This is for drawing the body on the currentRow
                if the enemy is going from left to right.
            **********************************************************/
            if(e->startCol != 0) {
                /* This drawing is for if the caterpiller is being cut */
                e->col = e->startCol+j;
                wrappedMutexLock(e->mutex);
                consoleClearImage(e->row, -(e->length)+e->col-1, ENEMY_HEIGHT, e->length); // Clear the prevCol
                consoleDrawImage(e->row, -(e->length)+e->col+1, tile_right, ENEMY_HEIGHT); // Draw on the nextCol
                wrappedMutexUnlock(e->mutex);
            }
            else {
                /* This drawing is for if the caterpiller is not being cut */
                wrappedMutexLock(e->mutex);
                consoleClearImage(e->row, -(e->length)+j-1, ENEMY_HEIGHT, e->length); // Clear the prevCol
                consoleDrawImage(e->row, -(e->length)+j+1, tile_right, ENEMY_HEIGHT); // Draw on the nextCol
                wrappedMutexUnlock(e->mutex);
                e->col = j; // Update e->col's position
            }
            

            srand(time(NULL));   // Initialization, should only be called once.
            if(rand()%10 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+1, e->col, e->p, e->mutex);
            }

            /* If the caterpiller going from left to right is hitting the right boundary (col=80) */
            if(j >= COL_BOUNDARY) {
                // The caterpiller is gonna taking a turn to the next row (new direction: right to left)
                e->col = j; // Update e->col to value 80 (j's current value), here, actually.
                e->startRow = e->row; //Update startRow. startRow is used as prevRow. <- REMEMBER THIS
                e->startCol = COL_BOUNDARY; // The caterpiller is gonna start at col=80 at next row.
                if(e->row != LAST_ROW) {
                    e->row = e->row + 2; // Get it to the next row
                }
                else {
                    e->row = LAST_ROW; // If caterpiller is already on the last row allowed, it's gonna repeat on this row
                }
                
                j = 0; // Clear the right incrementor to 0, for the next use
                e->direction = "left"; // The enemy is now turning left!
            }
            else {
                j++;
            }
            
        }
        /********************************************************** 
            This is for cleaning the wrapping body on the prevRow,
            if the enemy is going from right to left.
        **********************************************************/
        else {
            /* If the caterpiller going from right to left is not on ROW 2, then it must have a wrapping body on the prevRow. */
            if(e->row != ENEMY_FIRST_ROW) {
                if(e->startRow == e->row) {
                    /* This means this centipede is being cut, so startRow is not meant as a prevRow. It's its new row. */
                    /* Hence, we should handle the turning body on the previous row in a different calculation */
                    wrappedMutexLock(e->mutex);
                    // e->startRow-2 is the previous row, COL_BOUNDARY-e->length+i-1 is the previous centipede col location - 1
                    consoleClearImage(e->startRow-2, COL_BOUNDARY-e->length+i-1, ENEMY_HEIGHT, e->length); 
                    // e->startRow-2 is the previous row, COL_BOUNDARY-e->length+i+2 is the previous centipede col location - 2
                    // to create an illusion that it's still running on the previous row, KEKW
                    consoleDrawImage(e->startRow-2, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                    wrappedMutexUnlock(e->mutex);
                }
                else {
                    /* Remember we use startRow as 'prevRow'
                        If the startRow != current row
                        This means this caterpiller is NOT being CUT while WRAPPING, so startRow is prevRow.
                        Hence, we just handle the wrapping body on the prevRow normally */
                    wrappedMutexLock(e->mutex);
                    // e->startRow is the previous row, COL_BOUNDARY-e->length+i-1 is the previous centipede col location
                    consoleClearImage(e->startRow, COL_BOUNDARY-e->length+i-1, ENEMY_HEIGHT, e->length); 
                    // e->startRow is the previous row, COL_BOUNDARY-e->length+i+2 is the previous centipede col location - 2
                    // to create an illusion that it's still running on the previous row, KEKW
                    consoleDrawImage(e->startRow, COL_BOUNDARY-e->length+i+2, tile_right, ENEMY_HEIGHT);
                    wrappedMutexUnlock(e->mutex);
                }
            }

            /********************************************************** 
                This is for drawing the body on the currentRow
                if the enemy is going from right to left.
            **********************************************************/
            wrappedMutexLock(e->mutex);
            consoleClearImage(e->row, e->col+1, ENEMY_HEIGHT, e->length); // e->col+1 is the current centipede location
            e->col = e->startCol-i-2; // Update e->col's position, e->startCol is the new location
		    consoleDrawImage(e->row, e->col, tile_left, ENEMY_HEIGHT);
            wrappedMutexUnlock(e->mutex);

            srand(time(NULL)); // Initialize randomizer
            if(rand()%10 == 0) {
                // Returns a pseudo-random integer between 0 and RAND_MAX.
                spawnEnemyBullet(e->row+2, e->col, e->p, e->mutex);
            }  
            
            /* If the caterpiller going from right to left is hitting the left boundary (col=0) */
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
                
            }
            else {
                /* Maximum speed: 40/2 */
                e->speed = e->speed/2;
            }
            e->isHit = false; // Turn off the hit flag
        }
        /* Control the caterpiller's speed */
		sleepTicks(e->speed);
	}
    return NULL;
}