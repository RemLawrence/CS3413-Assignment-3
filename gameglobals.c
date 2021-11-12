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

        // Clean up for bullets
        BulletNode *bulletList = getBulletQueue();
        while(bulletList != NULL) {
                if(bulletList->eb != NULL) {
                    if(bulletList->eb->isDead) {
                        deleteBullet(NULL, bulletList->eb);
                    }
                }
                else if(bulletList->pb != NULL) {
                    if(bulletList->pb->isDead) {
                        deleteBullet(bulletList->pb, NULL);
                    }
                }
                bulletList = bulletList -> next;
        }
    }
    pthread_exit(NULL);
}