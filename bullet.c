//1. Cut 2d string array
//2. Catch isHit in enemy thread, cut off its body and accelerate, turn the isHit flag off
//3. spawnEnemy() in bullet thread?, initialize start row & col, length, form a new enemy.

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
/* reset the player state to start */
void newEnemyBullet(enemyBullet *eb) 
{
	eb->row = eb->startRow;
	eb->col = eb->startCol;
	eb->animTile = 0;
}

void newPlayerBullet(playerBullet *pb) 
{
	pb->row = pb->startRow;
	pb->col = pb->startCol;
	pb->animTile = 0;
}

/********************THREAD functions***************/
void *runEnemyBullet(void *data) {
    enemyBullet* eb = (enemyBullet*)data;
    newEnemyBullet(eb);
    while(eb->p->running && eb->p->lives > 0) {

        char** enemy_bullet_tile = ENEMY_BULLET[BULLET_ANIM_TILES-1];

        wrappedMutexLock(eb->mutex);
        consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear
        if(eb->row >= eb->p->row) {
            // Enemy bullet didn't hit player, it's about to go out of the lower boundary
            eb->isDead = true;
            wrappedMutexUnlock(eb->mutex);
            //wrappedMutexLock(eb->llist);
            deleteBullet(NULL, eb);
            //wrappedMutexUnlock(eb->llist);
            break;
        }
        else {
            eb->row = eb->row+1;
        }
        consoleDrawImage(eb->row, eb->col, enemy_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(eb->mutex);

        if(eb->row == eb->p->row-1 && (eb->col == eb->p->col || eb->col == eb->p->col+1 || eb->col == eb->p->col+2 || eb->col == eb->p->col+3 || eb->col == eb->p->col+4)) {
            // The player got hit by the bullet
            killPlayer(eb->p);
            wrappedMutexLock(eb->mutex);
            consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear bullet when it hits the player
            wrappedMutexUnlock(eb->mutex);
            eb->isDead = true;
            //wrappedMutexLock(eb->llist);
            deleteBullet(NULL, eb);
            //wrappedMutexUnlock(eb->llist);
            break;
        }

        sleepTicks(6); // Speed of the bullet
    }
    //TODO: delete this bullet from linked list
    pthread_exit(NULL);
}

void *runPlayerBullet(void *data) {
    playerBullet* pb = (playerBullet*)data;
    newPlayerBullet(pb);
    pb->p->score = pb->p->score + 1;
    while(pb->p->running && pb->p->lives > 0) {

        char** player_bullet_tile = PLAYER_BULLET[BULLET_ANIM_TILES-1];

        enemyNode *enemyList = getEnemyQueue();
        while(enemyList != NULL) {
            if(strcmp(enemyList->e->direction, "left") == 0) {
                if(pb->row == enemyList->e->row+2 && (pb->col >= enemyList->e->col && pb->col <= enemyList->e->col + enemyList->e->length)){
                    pb->p->score = pb->p->score + 20; // Centipede's been hit, update player's score.
                    int length = enemyList->e->length;
                    enemyList->e->length = pb->col - enemyList->e->col; // Update previous enemy's length
                    enemyList->e->isHit = true;
                    spawnEnemy(enemyList->e->row, pb->col+3, length-(pb->col - enemyList->e->col), enemyList->e->direction, false, pb->p, pb->mutex); // Spawn a new enemy on the pb cut
                    pb->isDead = true;

                    //wrappedMutexLock(pb->llist);
                    deleteBullet(pb, NULL);
                    //wrappedMutexUnlock(pb->llist);
                    pthread_exit(NULL);
                }
            }
            else { //right
                if(pb->row == enemyList->e->row+2 && (pb->col <= enemyList->e->col && pb->col >= enemyList->e->col - enemyList->e->length)){
                    pb->p->score = pb->p->score + 20; // Centipede's been hit, update player's score.
                    enemyList->e->isHit = true;
                    spawnEnemy(enemyList->e->row, pb->col-2, enemyList->e->length-(enemyList->e->col - pb->col), enemyList->e->direction, false, pb->p, pb->mutex);
                    enemyList->e->length = enemyList->e->col - pb->col; // Update previous enemy's length
                    pb->isDead = true;
                    //wrappedMutexLock(pb->llist);
                    deleteBullet(pb, NULL);
                    //wrappedMutexUnlock(pb->llist);
                    
                    pthread_exit(NULL);
                }
            }
            enemyList = enemyList->next;
        }

        wrappedMutexLock(pb->mutex);
        consoleClearImage(pb->row, pb->col, BULLET_SIZE, BULLET_SIZE); // Clear
        if(pb->row <= UPPER_BOUNDARY) {
            // If the bullet get passed enemy and is out of the boundary
            pb->isDead = true;
            wrappedMutexUnlock(pb->mutex);
            //wrappedMutexLock(pb->llist);
            deleteBullet(pb, NULL);
            //wrappedMutexUnlock(pb->llist);
            break;
        }
        else {
            // Player bullet continues to go upwards.
            pb->row = pb->row-1;
        }
        consoleDrawImage(pb->row, pb->col, player_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(pb->mutex);

        sleepTicks(6); // Speed of the bullet
    }
    //TODO: delete this bullet from linked list
    pthread_exit(NULL);
}

