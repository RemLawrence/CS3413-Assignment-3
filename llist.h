#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "enemy.h"
#include "bullet.h"

// A Linked List
struct node
{
    enemy *e;
    struct node *next;
};

void spawnEnemy(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);

struct node* createEnemyQueue(enemy *e);
void insertEnemyQueue(enemy *e, struct node *enemyQueue);
void spawnEnemyBullet(int startRow, int startCol, player *p, pthread_mutex_t *screenLock);