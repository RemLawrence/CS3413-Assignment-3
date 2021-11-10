#include "gameglobals.h"
#include <stdio.h>

void *runUpkeep(void *data) {
    player* p = (player*)data;
    char score[BUF_SIZE];
    char lives[BUF_SIZE];

    while (p->running && p-> lives > 0) {
        sprintf(score, "%d", p->score); // Convert int score to a string
        sprintf(lives, "%d", p->lives); // Convert int score to a string
        wrappedMutexLock(p->mutex);
        putString(score, 0, 29, 4);
        wrappedMutexUnlock(p->mutex);

        wrappedMutexLock(p->mutex);
        putString(lives, 0, 45, 1);
        wrappedMutexUnlock(p->mutex);
    }
    pthread_exit(NULL);
}