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

/********************THREAD functions***************/
void *runEnemyBullet(void *data) {
    enemyBullet* eb = (enemyBullet*)data;
    newEnemyBullet(eb);
    while(eb->p->running && eb->p->lives >= 0) {
        // and not being consumed...
        char** enemy_bullet_tile = ENEMY_BULLET[BULLET_ANIM_TILES-1];

        wrappedMutexLock(eb->mutex);
        consoleClearImage(eb->row, eb->col, BULLET_SIZE, BULLET_SIZE); // Clear
        eb->row = eb->row+1;
        consoleDrawImage(eb->row, eb->col, enemy_bullet_tile, BULLET_SIZE); // Draw
        wrappedMutexUnlock(eb->mutex);
        sleepTicks(6); // Speed of the bullet
    }

    pthread_exit(NULL);
}

