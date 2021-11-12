#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "enemy.h"
#include "bullet.h"

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

void spawnEnemy(int startRow, int startCol, int length, char* direction, player *p, pthread_mutex_t *screenLock);
struct node* createEnemyQueue(enemy *e);
void insertEnemyQueue(enemy *e, enemyNode *enemyQueue);
enemyNode* getEnemyQueue();

void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);
void spawnPlayerBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);
BulletNode* createBulletQueue(playerBullet *pb, enemyBullet *eb);
void insertBulletQueue(playerBullet *pb, enemyBullet *eb, BulletNode *BulletQueue);
BulletNode* getBulletQueue();