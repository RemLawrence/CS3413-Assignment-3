/**********************************************************************
  Module: llist.c
  Author: Micah Hanmin Wang

  Contains the function for spawning enemy, and adds it to linked list 
  once a caterpiller thread is created.
  Also contains functions for spawning player bullet and enemy bullet, 
  same for the caterpiller ones, store them to the bullet linked list
  once created.

  Purpose: Manages spawning new enemy and adds enemy & both bullets to the
  linked list. Manages linked list (create, insert and delete from it)

**********************************************************************/

#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

enemyNode* enemyQueue; /* Caterpiller's Linked List */

BulletNode *bulletQueue; /* Linked List for both type of bullets */
pthread_mutex_t bulletListLock;
bool first = true; /* Flag for controlling if the caterpiller linked list is created or not */
pthread_mutex_t enemyListLock;

/********************************************************** 
    Initializing and creating new caterpiller and new caterpiller
    thread. Scheduling spawning new enemies and also create small
    ones after the original big caterpiller's being hit. 
 **********************************************************/
void spawnEnemy(int startRow, int startCol, int length, int speed, char* direction, bool spawn, player *p, pthread_mutex_t *screenLock)
{   
    while(p->running && p->lives > 0) {
        enemy* e = (enemy*)(malloc(sizeof(enemy)));
        /* error checking */
        if (e == NULL) {
            fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(enemy));
            abort();
        }
        e->startCol = startCol; // Initialize the enemy's startCol
        e->startRow = startRow; // Initialize the enemy's startRow
        e->isDead = false;
        e->isHit = false;
        e->direction = direction;
        e->length = length; // Length of the enemy body. Could be deducted when hit!
        e->speed = speed; // Initialize the enemy speed to 40
        e->mutex = screenLock; // A reference to the screenlock
        e->p = p; // A reference to the player

        wrappedMutexInit(&enemyListLock, NULL);
        wrappedMutexInit(e->mutex, NULL);
        wrappedPthreadCreate(&(e->thread), NULL, runEnemy, (void*)e);

        /* If the enemy's length < 5, then it will die and thread will exit */
        if(e->length <= ENEMY_MIN_WIDTH) {
            /* CAUTION: This check is to prevent scheduling spawning new caterpillers when the original ones being cut */
            break;
        }
        else {
            if(first) {
                /* If the `first` flag is still on, then the enemy linked list is not created yet */
                enemyQueue = createEnemyQueue(e);
                first = false; // Turn off the flag
            }
            else {
                /* If the caterpiller Linked list is created, then ust insert the new one */
                insertEnemyQueue(e, enemyQueue);
            }
        }

        /* If this method is called by the spawn thread, it has the responsibility to spawn new enemy with length=80 */
        if(spawn) {
            sleepTicks(rand() % (MAX_SPAWN_TICK + 1 - MIN_SPAWN_TICK) + MIN_SPAWN_TICK); // Generate a new enemy randomly between ticks 8000-10000
        }
        else {
            /* If this method is not called by the spawn thread, it just spawns one enemy */
            break;
        }
    }
}

/********************************************************** 
    Just creates a new Caterpiller Linked List.
 **********************************************************/
enemyNode* createEnemyQueue(enemy *e) {
    enemyNode *newEnemyQueue = (enemyNode*)malloc(sizeof(enemyNode));
    /* error checking */
    if (newEnemyQueue == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(enemyNode));
        abort();
    }
    //wrappedMutexLock(&enemyListLock);
    newEnemyQueue->e = e;
    newEnemyQueue->next = NULL;
    //wrappedMutexUnlock(&enemyListLock);
    return newEnemyQueue; // node created and return it
}

/********************************************************** 
    Insert a new created caterpiller into the caterpiller Linked
    list.
 **********************************************************/
void insertEnemyQueue(enemy *e, enemyNode *enemyQueue) {
    enemyNode *newEnemyQueue = createEnemyQueue(e);
    while(enemyQueue->next != NULL) {
        //wrappedMutexLock(&enemyListLock);
        enemyQueue = enemyQueue -> next;
        //wrappedMutexUnlock(&enemyListLock);
    }
    //wrappedMutexLock(&enemyListLock);
    enemyQueue->next = newEnemyQueue;
    //wrappedMutexUnlock(&enemyListLock);
}

/********************************************************** 
    Delete a caterpiller based on its thread # from the caterpiller
    Linked List.
 **********************************************************/
void deleteEnemy(enemy *e) {
    if(enemyQueue->e->thread == e->thread) {
        if(enemyQueue->next == NULL) {
            wrappedMutexLock(&enemyListLock);
            enemyQueue = NULL;
            wrappedMutexUnlock(&enemyListLock);
        }
        else {
            wrappedMutexLock(&enemyListLock);
            enemyQueue = enemyQueue->next;
            wrappedMutexUnlock(&enemyListLock);
        }
        wrappedMutexLock(&enemyListLock);
        pthread_cancel(e->thread);
        wrappedPthreadJoin(e->thread, NULL);
        free(e);
        wrappedMutexUnlock(&enemyListLock);
    }
    else {
        while(enemyQueue->next != NULL) {
            if(enemyQueue->next->e->thread == e->thread) {
                wrappedMutexLock(&enemyListLock);
                enemyQueue->next = enemyQueue->next->next;

                pthread_cancel(e->thread);
                wrappedPthreadJoin(e->thread, NULL);
                free(e);
                wrappedMutexUnlock(&enemyListLock);
                break;
            }
        }
    }
}

/********************************************************** 
    Get the head pointer to the enemy Linked list
 **********************************************************/
enemyNode* getEnemyQueue() {
    return enemyQueue;
}



/********************************************************** 
    Initializing and creating new enemy bullets and enemy bullet
    threads. 
    Also manages creating Bullet Linked List (if enemy bullet appears first)
    and inserting new enemy bullets into the Bullet Linked List.
 **********************************************************/
void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    enemyBullet* eb = (enemyBullet*)(malloc(sizeof(enemyBullet)));
    /* error checking */
    if (eb == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(enemyBullet));
        abort();
    }
	eb->startCol = startCol;
	eb->startRow = startRow;
	eb->mutex = screenLock;
    eb->llist = &bulletListLock;
    eb->p = p;

    wrappedMutexInit(&bulletListLock, NULL); // Initialize bulletListLock
	wrappedMutexInit(eb->mutex, NULL);
	wrappedPthreadCreate(&(eb->thread), NULL, runEnemyBullet, (void*)eb);

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
}

/********************************************************** 
    Initializing and creating new player bullets and player bullet
    threads. 
    Also manages creating Bullet Linked List (if player bullet appears first)
    and inserting new player bullets into the Bullet Linked List.
 **********************************************************/
void spawnPlayerBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock)
{
    //wrappedMutexInit(&bulletListLock, NULL); // Initialize bulletListLock
    playerBullet* pb = (playerBullet*)(malloc(sizeof(playerBullet)));
    /* error checking */
    if (pb == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(playerBullet));
        abort();
    }
	pb->startCol = startCol;
	pb->startRow = startRow;
	pb->mutex = screenLock;
    pb->llist = &bulletListLock;
    pb->p = p;

	//TODO: Init mutex...
    wrappedMutexInit(&bulletListLock, NULL); // Initialize bulletListLock
	wrappedMutexInit(pb->mutex, NULL);
	wrappedPthreadCreate(&(pb->thread), NULL, runPlayerBullet, (void*)pb);

    if (bulletQueue == NULL) {
        bulletQueue = createBulletQueue(pb, NULL);
    }
    else {
        insertBulletQueue(pb, NULL, bulletQueue);
    }
}

/********************************************************** 
    Just creates a new Bullet Linked List
 **********************************************************/
BulletNode* createBulletQueue(playerBullet *pb, enemyBullet *eb) {
    BulletNode *newBulletQueue = (BulletNode*)malloc(sizeof(BulletNode));
    /* error checking */
    if (newBulletQueue == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(BulletNode));
        abort();
    }
    //wrappedMutexLock(&bulletListLock);
    newBulletQueue->pb = pb;
    newBulletQueue->eb = eb;
    newBulletQueue->next = NULL;
    //wrappedMutexUnlock(&bulletListLock);
    return newBulletQueue; // node created and return it
}

/********************************************************** 
    Insert a new created bullet into the Bullet Linked
    list.
 **********************************************************/
void insertBulletQueue(playerBullet *pb, enemyBullet *eb, BulletNode *BulletQueue) {
    BulletNode *newBulletQueue = createBulletQueue(pb, eb);
    while(BulletQueue->next != NULL) {
        //wrappedMutexLock(&bulletListLock);
        BulletQueue = BulletQueue -> next;
        //wrappedMutexUnlock(&bulletListLock);
    }
    //wrappedMutexLock(&bulletListLock);
    BulletQueue->next = newBulletQueue;
    //wrappedMutexUnlock(&bulletListLock);
}

/********************************************************** 
    Delete a bullet based on its thread # from the bullet
    Linked List.
 **********************************************************/
void deleteBullet(playerBullet *pb, enemyBullet *eb) {
    //BulletNode *jobQueueInProcess = bulletQueue;
    if(pb != NULL) {
        /* Delete a player bullet */
        while(bulletQueue != NULL) {
            if(bulletQueue->pb != NULL) {
                if(bulletQueue->pb->isDead) {
                    pthread_cancel(pb->thread);
                    wrappedPthreadJoin(pb->thread, NULL);
                    free(pb);
                }
            }
            bulletQueue = bulletQueue -> next;
        }
    }
    else if(eb != NULL) {
        /* Delete an enemy bullet */
        while(bulletQueue != NULL) {
            if(bulletQueue->eb != NULL) {
                if(bulletQueue->eb->isDead) {
                    pthread_cancel(eb->thread);
                    wrappedPthreadJoin(eb->thread, NULL);
                    free(eb);
                }
            }
            bulletQueue = bulletQueue -> next;
        }
    }
}

/********************************************************** 
    Get the head pointer to the Bullet Linked list
 **********************************************************/
BulletNode* getBulletQueue() {
    return bulletQueue;
}