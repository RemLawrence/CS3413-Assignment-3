/**********************************************************************
  Module: gameglobals.c
  Author: Micah Hanmin Wang

  Purpose: Represents the upkeep threads in the game, to update scores &
  player's lives onto the screen (first row).
  And checks if there are any enemies left, if not, player wins.
  Also manages killing all the bullets when the player's being hit and game froze.

  Includes thread functions for: upkeep thread.

**********************************************************************/

#include "gameglobals.h"
#include <stdio.h>
#include <unistd.h>

/********************************************************** 
    Upkeep Thread function.
    Update scores & player's lives onto the screen (first row).
    And checks if there are any enemies left, if not, player wins.
    Also manages killing all the bullets when the player's being hit and game froze.
 **********************************************************/
void *runUpkeep(void *data) {
    player* p = (player*)data;
    char score[BUF_SIZE]; /* The buffer for storing score string */
    char lives[BUF_SIZE]; /* The buffer for storing player lives string */

    while (p->running && p-> lives > 0) {
        sprintf(score, "%d", p->score); // Convert int score to a string
        sprintf(lives, "%d", p->lives); // Convert int player lives to a string
        wrappedMutexLock(p->mutex);
        /* Put score onto the game board */
        putString(score, UPKEEP_ROW, SCORE_POS, BUF_SIZE);
        wrappedMutexUnlock(p->mutex);

        wrappedMutexLock(p->mutex);
        /* Put player lives onto the game board */
        putString(lives, UPKEEP_ROW, LIVES_POS, BUF_SIZE);
        wrappedMutexUnlock(p->mutex);

        sleepTicks(1); // This sleep tick is for if upkeep thread running faster than the spawn enemy thread, preventing automatic wins at the beginning of the game.
        /* TODO: Regularly check if there is no enemy left */
        enemyNode *enemyList = getEnemyQueue();
        bool gameover = true;
        while(enemyList != NULL){
            if(!enemyList->e->isDead) {
                gameover = false;
                break;
            }
            enemyList = enemyList-> next;
        }

        /* TODO: When the player is shot, remove all the bullets on the screen. */
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