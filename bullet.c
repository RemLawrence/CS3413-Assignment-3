#include "bullet.h"
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

/********************support functions***************
/* reset the player state to start */
void newEnemyBullet(enemyBullet *eb) 
{
	eb->row = eb->startRow;
	eb->col = eb->startCol;
	eb->animTile = 0;
	//p->state = GAME;
}

void newPlayerBullet(playerBullet *pb) 
{
	pb->row = pb->startRow;
	pb->col = pb->startCol;
	pb->animTile = 0;
	//p->state = GAME;
}

/********************THREAD functions***************/
void *runEnemyBullet(void *data) {
    enemyBullet* eb = (enemyBullet*)data;
    newEnemyBullet(eb);
    while(eb->p->running && eb->p->lives >= 0) {
        // and not being consumed...
        char** enemy_bullet_tile = ENEMY_BULLET[BULLET_ANIM_TILES-1];

        wrappedMutexLock(eb->mutex);
        consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear
        if(eb->row >= eb->p->row) {
            // Enemy bullet didn't hit player, it's about to go out of the lower boundary
            wrappedMutexUnlock(eb->mutex);
            break;
        }
        else {
            eb->row = eb->row+1;
        }
        consoleDrawImage(eb->row, eb->col, enemy_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(eb->mutex);

        if(eb->row == eb->p->row && (eb->col == eb->p->col || eb->col == eb->p->col+1 || eb->col == eb->p->col+2 || eb->col == eb->p->col+3 || eb->col == eb->p->col+4)){
            killPlayer(eb->p);
        }

        sleepTicks(6); // Speed of the bullet
    }

    pthread_exit(NULL);
}

void *runPlayerBullet(void *data) {
    playerBullet* pb = (playerBullet*)data;
    newPlayerBullet(pb);
    while(pb->p->running && pb->p->lives >= 0) {
        // and not being consumed...
        char** player_bullet_tile = PLAYER_BULLET[BULLET_ANIM_TILES-1];

        wrappedMutexLock(pb->mutex);
        consoleClearImage(pb->row, pb->col, BULLET_SIZE, BULLET_SIZE); // Clear
       
        if(pb->row <= UPPER_BOUNDARY) {
            // If the bullet get passed enemy and is out of the boundary
            wrappedMutexUnlock(pb->mutex);
            break;
        }
        else {
            pb->row = pb->row-1;
        }

        consoleDrawImage(pb->row, pb->col, player_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(pb->mutex);
        sleepTicks(6); // Speed of the bullet
    }

    pthread_exit(NULL);
}

