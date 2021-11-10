#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool bulletQInitialized = false;
BulletNode *bulletQueue;

void spawnEnemy(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    bool first = true;
    enemyNode* enemyQueue;
    while(p->running && p->lives > 0) {
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

        if(first) {
            enemyQueue = createEnemyQueue(e);
            first = false;
        }
        else {
            insertEnemyQueue(e, enemyQueue);
        }

        sleepTicks(10000); // TODO: should be a random interval
    }
}

enemyNode* createEnemyQueue(enemy *e) {
    enemyNode *newEnemyQueue = (enemyNode*)malloc(sizeof(enemyNode));
    newEnemyQueue->e = e;
    newEnemyQueue->next = NULL;
    return newEnemyQueue; // node created and return it
}

void insertEnemyQueue(enemy *e, enemyNode *enemyQueue) {
    enemyNode *newEnemyQueue = createEnemyQueue(e);
    while(enemyQueue->next != NULL) {
        enemyQueue = enemyQueue -> next;
    }
    enemyQueue->next = newEnemyQueue;
}


void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    enemyBullet* eb = (enemyBullet*)(malloc(sizeof(enemyBullet)));
	eb->startCol = startCol;
	eb->startRow = startRow;
	eb->mutex = screenLock;
    eb->p = p;

    if (!bulletQInitialized) {
        bulletQueue = createBulletQueue(NULL, eb);
        bulletQInitialized = true;
    }
    else {
        insertBulletQueue(NULL, eb, bulletQueue);
    }

	//TODO: Init mutex...
	wrappedMutexInit(eb->mutex, NULL);
	wrappedPthreadCreate(&(eb->thread), NULL, runEnemyBullet, (void*)eb);
}

void spawnPlayerBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    playerBullet* pb = (playerBullet*)(malloc(sizeof(playerBullet)));
	pb->startCol = startCol;
	pb->startRow = startRow;
	pb->mutex = screenLock;
    pb->p = p;

    if (!bulletQInitialized) {
        bulletQueue = createBulletQueue(pb, NULL);
        bulletQInitialized = true;
    }
    else {
        insertBulletQueue(pb, NULL, bulletQueue);
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
        printf("ok");
        BulletQueue = BulletQueue -> next;
    }
    BulletQueue->next = newBulletQueue;
}

//void deleteBullet()