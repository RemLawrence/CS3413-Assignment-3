#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

enemyNode* enemyQueue;

BulletNode *bulletQueue;
pthread_mutex_t bulletListLock;
bool first = true;
pthread_mutex_t enemyListLock;

void spawnEnemy(int startRow, int startCol, int length, char* direction, bool spawn, player *p, pthread_mutex_t *screenLock)
{   
    while(p->running && p->lives > 0) {
        enemy* e = (enemy*)(malloc(sizeof(enemy)));
        e->startCol = startCol; // Initialize the enemy's startCol to the upper left of the console (78)
        e->startRow = startRow; // Initialize the enemy's startRow to the upper left of the console (2)

        e->isDead = false;
        e->isHit = false;

        e->direction = direction;

        e->length = length; // Length of the enemy body. Could be deducted when hit!
        e->speed = ENEMY_SPEED; // Initialize the enemy speed to 40

        e->mutex = screenLock; // A reference to the screenlock
        e->p = p; // A reference to the player

        //TODO: Init mutex...
        wrappedMutexInit(&enemyListLock, NULL);
        wrappedMutexInit(e->mutex, NULL);
        wrappedPthreadCreate(&(e->thread), NULL, runEnemy, (void*)e);

        if(first) {
            enemyQueue = createEnemyQueue(e);
            first = false;
        }
        else {
            insertEnemyQueue(e, enemyQueue);
        }

        /* If this method is called by the spawn thread, it has the responsibility to spawn new enemy with length=80 */
        if(spawn) {
            sleepTicks(rand() % (10000 + 1 - 8000) + 8000); // Generate a new enemy randomly between ticks 8000-10000
        }
        else {
            /* If this method is not called by the spawn thread, it just spawns one enemy */
            break;
        }
    }
}

enemyNode* createEnemyQueue(enemy *e) {
    enemyNode *newEnemyQueue = (enemyNode*)malloc(sizeof(enemyNode));
    wrappedMutexLock(&enemyListLock);
    newEnemyQueue->e = e;
    newEnemyQueue->next = NULL;
    wrappedMutexUnlock(&enemyListLock);
    return newEnemyQueue; // node created and return it
}

void insertEnemyQueue(enemy *e, enemyNode *enemyQueue) {
    enemyNode *newEnemyQueue = createEnemyQueue(e);
    while(enemyQueue->next != NULL) {
        wrappedMutexLock(&enemyListLock);
        enemyQueue = enemyQueue -> next;
        wrappedMutexUnlock(&enemyListLock);
    }
    wrappedMutexLock(&enemyListLock);
    enemyQueue->next = newEnemyQueue;
    wrappedMutexUnlock(&enemyListLock);
}

enemyNode* getEnemyQueue() {
    return enemyQueue;
}




void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    //wrappedMutexInit(&bulletListLock, NULL); // Initialize bulletListLock
    enemyBullet* eb = (enemyBullet*)(malloc(sizeof(enemyBullet)));
	eb->startCol = startCol;
	eb->startRow = startRow;
	eb->mutex = screenLock;
    eb->llist = &bulletListLock;
    eb->p = p;

    if (bulletQueue == NULL) {
        //wrappedMutexLock(&bulletListLock);
        bulletQueue = createBulletQueue(NULL, eb);
        //wrappedMutexUnlock(&bulletListLock);
    }
    else {
        //wrappedMutexLock(&bulletListLock);
        insertBulletQueue(NULL, eb, bulletQueue);
        //wrappedMutexUnlock(&bulletListLock);
    }

	//TODO: Init mutex...
	wrappedMutexInit(eb->mutex, NULL);
	wrappedPthreadCreate(&(eb->thread), NULL, runEnemyBullet, (void*)eb);
}

void spawnPlayerBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    //wrappedMutexInit(&bulletListLock, NULL); // Initialize bulletListLock
    playerBullet* pb = (playerBullet*)(malloc(sizeof(playerBullet)));
	pb->startCol = startCol;
	pb->startRow = startRow;
	pb->mutex = screenLock;
    pb->llist = &bulletListLock;
    pb->p = p;

    if (bulletQueue == NULL) {
        //wrappedMutexLock(&bulletListLock);
        bulletQueue = createBulletQueue(pb, NULL);
        //wrappedMutexUnlock(&bulletListLock);
    }
    else {
        //wrappedMutexLock(&bulletListLock);
        insertBulletQueue(pb, NULL, bulletQueue);
        //wrappedMutexUnlock(&bulletListLock);
    }

	//TODO: Init mutex...
	wrappedMutexInit(pb->mutex, NULL);
	wrappedPthreadCreate(&(pb->thread), NULL, runPlayerBullet, (void*)pb);
}

BulletNode* createBulletQueue(playerBullet *pb, enemyBullet *eb) {
    BulletNode *newBulletQueue = (BulletNode*)malloc(sizeof(BulletNode));
    newBulletQueue->pb = pb;
    newBulletQueue->eb = eb;
    newBulletQueue->next = NULL;
    return newBulletQueue; // node created and return it
}

void insertBulletQueue(playerBullet *pb, enemyBullet *eb, BulletNode *BulletQueue) {
    BulletNode *newBulletQueue = createBulletQueue(pb, eb);
    while(BulletQueue->next != NULL) {
        BulletQueue = BulletQueue -> next;
    }
    BulletQueue->next = newBulletQueue;
}

void deleteBullet(playerBullet *pb, enemyBullet *eb) {
    if(pb != NULL) {
        // Delete a player bullet
        if(bulletQueue->pb->thread == pb->thread) {
            // head is the player bullet we wanna delete
            if(bulletQueue->next == NULL) {
                bulletQueue = NULL;
            }
            else {
                bulletQueue = bulletQueue -> next;
            }
            pthread_cancel(pb->thread);
            pthread_join(pb->thread, NULL);
            free(pb);
        }
        else {
            // the bullet we wanna delete it not the head
            while(bulletQueue->next != NULL) {
                if(bulletQueue->next->pb->thread == pb->thread) {
                    if(bulletQueue -> next -> next == NULL) {
                        bulletQueue->next = NULL;
                    }
                    else {
                        bulletQueue->next = bulletQueue -> next -> next;
                    }
                    pthread_cancel(pb->thread);
                    pthread_join(pb->thread, NULL);
                    free(pb);
                    break;
                }
            }
        }
    }
    else if(eb != NULL) {
        // Delete an enemy bullet
        if(bulletQueue->eb->thread == eb->thread) {
            // head is the player bullet we wanna delete
            if(bulletQueue->next == NULL) {
                bulletQueue = NULL;
            }
            else {
                bulletQueue = bulletQueue -> next;
            }
            pthread_cancel(eb->thread);
            pthread_join(eb->thread, NULL);
            free(eb);
        }
        else {
            // the bullet we wanna delete it not the head
            while(bulletQueue->next != NULL) {
                if(bulletQueue->next->eb->thread == eb->thread) {
                    if(bulletQueue -> next -> next == NULL) {
                        bulletQueue->next = NULL;
                    }
                    else {
                        bulletQueue->next = bulletQueue -> next -> next;
                    }
                    pthread_cancel(eb->thread);
                    pthread_join(eb->thread, NULL);
                    free(eb);
                    break;
                }
            }
        }
    }
    
}

BulletNode* getBulletQueue() {
    return bulletQueue;
}