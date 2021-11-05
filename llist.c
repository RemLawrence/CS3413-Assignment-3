#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void spawnEnemy(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    while(p->running && p->lives >= 0) {
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

        sleepTicks(2000);
    }
}