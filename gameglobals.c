#include "gameglobals.h"
#include <stdio.h>
#include <unistd.h>

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
        sleepTicks(1);
        enemyNode *enemyList = getEnemyQueue();
        bool gameover = true;
        while(enemyList != NULL){
            if(!enemyList->e->isDead) {
                gameover = false;
                break;
            }
            enemyList = enemyList-> next;
        }

        /* When the player is shot, remove all the bullets on the screen. */
        if(p->state == DEAD) {
            BulletNode *bulletList = getBulletQueue();
            while(bulletList != NULL) {
                if(bulletList->eb != NULL) {
                    wrappedMutexLock(p->mutex);
                    consoleClearImage(bulletList->eb->row, bulletList->eb->col, BULLET_SIZE, BULLET_SIZE);
                    wrappedMutexUnlock(p->mutex);
                    pthread_cancel(bulletList->eb->thread);
                    //pthread_join(bulletList->eb->thread, NULL);
                }
                else if(bulletList->pb != NULL) {
                    wrappedMutexLock(p->mutex);
                    consoleClearImage(bulletList->pb->row, bulletList->pb->col, BULLET_SIZE, BULLET_SIZE);
                    wrappedMutexUnlock(p->mutex);
                    pthread_cancel(bulletList->pb->thread);
                    //pthread_join(bulletList->pb->thread, NULL);
                }
                bulletList = bulletList -> next;
            }
            
        }
    }
    pthread_exit(NULL);
}