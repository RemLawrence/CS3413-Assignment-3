#include "gameglobals.h"
#include <stdio.h>

void *runUpkeep(void *data) {
    player* p = (player*)data;
    char score[BUF_SIZE];
    char lives[BUF_SIZE];

    while (p->running && p-> lives > 0) {
        sprintf(score, "%d", p->score); // Convert int score to a string
        sprintf(lives, "%d", p->lives); // Convert int lives to a string
        wrappedMutexLock(p->mutex);
        putString(score, UPKEEP_ROW, SCORE_POS, BUF_SIZE);
        wrappedMutexUnlock(p->mutex);

        wrappedMutexLock(p->mutex);
        putString(lives, UPKEEP_ROW, LIVES_POS, BUF_SIZE);
        wrappedMutexUnlock(p->mutex);

        /* Regularly check if there is no enemy left */
        sleepTicks(3);
        enemyNode *enemyList = getEnemyQueue();
        if(enemyList == NULL) {
            p->state = GAMEOVER; // Let the player win
        }
    }
    pthread_exit(NULL);
}