/**********************************************************************
  Module: llist.h
  Author: Micah Hanmin Wang

  Purpose: Manages caterpiller linked list and bullet linked list
  Stores the constants that these two linked lists needed.
  Declares caterpiller's and bullet's linked lists struct

**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "enemy.h"
#include "bullet.h"
#include "threadwrappers.h"

#define ENEMY_MIN_WIDTH 4
#define MIN_SPAWN_TICK 8000
#define MAX_SPAWN_TICK 10000

// A Linked List
typedef struct node
{
    enemy *e;
    struct node *next;
} enemyNode;

// A Linked List
typedef struct nod
{
    enemyBullet *eb;
    playerBullet *pb;
    struct nod *next;
} BulletNode;

void spawnEnemy(int startRow, int startCol, int length, char* direction, bool spawn, player *p, pthread_mutex_t *screenLock);
struct node* createEnemyQueue(enemy *e);
void insertEnemyQueue(enemy *e, enemyNode *enemyQueue);
void deleteEnemy(enemy *e);
enemyNode* getEnemyQueue();

void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);
void spawnPlayerBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);
BulletNode* createBulletQueue(playerBullet *pb, enemyBullet *eb);
void insertBulletQueue(playerBullet *pb, enemyBullet *eb, BulletNode *BulletQueue);
void deleteBullet(playerBullet *pb, enemyBullet *eb);
BulletNode* getBulletQueue();