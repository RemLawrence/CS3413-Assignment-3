/**********************************************************************
  Module: bullet.c
  Author: Micah Hanmin Wang

  Contains two initialize functions. One for enemy bullets, one for player
  bullets.
  Also contains two threading functions, one for enemy bullets, one for 
  player bullets.

  Purpose: Manages every single bullets in their threads, 
  make sure they are thread safe.

**********************************************************************/

#include "bullet.h"
#include "llist.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>

char* PLAYER_BULLET[BULLET_SIZE][BULLET_ANIM_TILES] = 
{
   {"|"}
};

char* ENEMY_BULLET[BULLET_SIZE][BULLET_ANIM_TILES] = 
{
   {"."}
};

/********************support functions***************/
/* Initialize the enemy bullet. Including its positions */
void newEnemyBullet(enemyBullet *eb) 
{
	eb->row = eb->startRow;
	eb->col = eb->startCol;
	eb->animTile = 0;
}

/* Initialize the player bullet. Including its positions */
void newPlayerBullet(playerBullet *pb) 
{
	pb->row = pb->startRow;
	pb->col = pb->startCol;
	pb->animTile = 0;
}

/********************THREAD functions***************/

/********************************************************** 
    The Enemy (aka caterpillar) Bullet Thread function.
    Initialize the new enemy bullet and body strings.
    Manages it while it's living and decide when it's about to die.
    Has the logic for enemy bullet to move, and when to die.
    When it hits the bottom boundary or hits the player, it dies.
 **********************************************************/
void *runEnemyBullet(void *data) {
    enemyBullet* eb = (enemyBullet*)data;
    newEnemyBullet(eb);
    while(eb->p->running && eb->p->lives > 0) {
        char** enemy_bullet_tile = ENEMY_BULLET[BULLET_ANIM_TILES-1]; /* Enemy bullet's body */

        wrappedMutexLock(eb->mutex);
        consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear enemy bullet's prev position
        if(eb->row >= eb->p->row) {
            /* Enemy bullet didn't hit player, it's about to go out of the lower boundary */
            eb->isDead = true;
            wrappedMutexUnlock(eb->mutex);
            //wrappedMutexLock(eb->llist);
            deleteBullet(NULL, eb); //TODO: delete this bullet from linked list
            //wrappedMutexUnlock(eb->llist);
            break;
        }
        else {
            /* If it didn't hit player, continue going down then */
            eb->row = eb->row+1;
        }
        consoleDrawImage(eb->row, eb->col, enemy_bullet_tile, BULLET_SIZE); // Draw the bullet in the new position
        wrappedMutexUnlock(eb->mutex);

        if(eb->row == eb->p->row-1 && (eb->col == eb->p->col || eb->col == eb->p->col+1 || eb->col == eb->p->col+2 || eb->col == eb->p->col+3 || eb->col == eb->p->col+4)) {
            /* The player got hit by the bullet */
            killPlayer(eb->p);
            wrappedMutexLock(eb->mutex);
            consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear bullet when it hits the player
            wrappedMutexUnlock(eb->mutex);
            eb->isDead = true;
            //wrappedMutexLock(eb->llist);
            deleteBullet(NULL, eb); //TODO: delete this bullet from linked list
            //wrappedMutexUnlock(eb->llist);
            break;
        }

        sleepTicks(6); // Speed of the bullet
    }
    pthread_exit(NULL);
}

/********************************************************** 
    The Player Bullet Thread function.
    Initialize the new player bullet and body strings.
    Manages it while it's living and decide when it's about to die.
    Has the logic for player bullet to move, and when to die.
    When it hits the upper boundary or hits any caterpillers, it dies.
 **********************************************************/
void *runPlayerBullet(void *data) {
    playerBullet* pb = (playerBullet*)data;
    newPlayerBullet(pb);
    pb->p->score = pb->p->score + 1;
    while(pb->p->running && pb->p->lives > 0) {
        char** player_bullet_tile = PLAYER_BULLET[BULLET_ANIM_TILES-1];

        /********************************************************** 
            This is detecting if the player bullet hits any caterpillers
            by going through each caterpiller when it moves
        **********************************************************/
        enemyNode *enemyList = getEnemyQueue();
        while(enemyList != NULL) {
            if(strcmp(enemyList->e->direction, "left") == 0) {
                /* If the player bullet hits the enemy going right to left */
                if(pb->row == enemyList->e->row+2 && (pb->col >= enemyList->e->col && pb->col <= enemyList->e->col + enemyList->e->length)){
                    pb->p->score = pb->p->score + 20; // Centipede's been hit, update player's score.
                    int length = enemyList->e->length;
                    enemyList->e->length = pb->col - enemyList->e->col; // Update previous enemy's length
                    enemyList->e->isHit = true;
                    /* Spawn new caterpiller based on its new position and new length, inherits the direction from another caterpiller, though. */
                    spawnEnemy(enemyList->e->row, pb->col+3, length-(pb->col - enemyList->e->col), enemyList->e->speed, enemyList->e->direction, false, pb->p, pb->mutex); // Spawn a new enemy on the pb cut
                    pb->isDead = true; // Turn the isDead flag on for the previous enemy to speed up

                    //wrappedMutexLock(pb->llist);
                    deleteBullet(pb, NULL); //TODO: delete this bullet from linked list
                    //wrappedMutexUnlock(pb->llist);
                    pthread_exit(NULL);
                }
            }
            else { //right
                /* If the player bullet hits the enemy going left to right */
                if(pb->row == enemyList->e->row+2 && (pb->col <= enemyList->e->col && pb->col >= enemyList->e->col - enemyList->e->length)){
                    pb->p->score = pb->p->score + 20; // Centipede's been hit, update player's score.
                    enemyList->e->isHit = true;
                    /* Spawn new caterpiller based on its new position and new length, inherits the direction from another caterpiller, though. */
                    spawnEnemy(enemyList->e->row, pb->col-2, enemyList->e->length-(enemyList->e->col - pb->col), enemyList->e->speed, enemyList->e->direction, false, pb->p, pb->mutex);
                    enemyList->e->length = enemyList->e->col - pb->col; // Update previous enemy's length
                    pb->isDead = true; // Turn the isDead flag on for the previous enemy to speed up
                    //wrappedMutexLock(pb->llist);
                    deleteBullet(pb, NULL); //TODO: delete this bullet from linked list
                    //wrappedMutexUnlock(pb->llist);
                    pthread_exit(NULL);
                }
            }
            enemyList = enemyList->next; // Iterate through enemy list
        }

        wrappedMutexLock(pb->mutex);
        consoleClearImage(pb->row, pb->col, BULLET_SIZE, BULLET_SIZE); // Clear
        if(pb->row <= UPPER_BOUNDARY) {
            // If the bullet get passed enemy and is out of the boundary
            pb->isDead = true;
            wrappedMutexUnlock(pb->mutex);
            //wrappedMutexLock(pb->llist);
            deleteBullet(pb, NULL); //TODO: delete this bullet from linked list
            //wrappedMutexUnlock(pb->llist);
            break;
        }
        else {
            // Otherwise, Player bullet continues to go upwards.
            pb->row = pb->row-1;
        }
        consoleDrawImage(pb->row, pb->col, player_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(pb->mutex);

        sleepTicks(6); // Speed of the bullet
    }
    pthread_exit(NULL);
}

