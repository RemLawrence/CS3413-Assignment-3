#ifndef BULLET_H
#define BULLET_H

#include "enemy.h"
#include "player.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>

#define BULLET_SIZE 1
#define BULLET_ANIM_TILES 1

#define UPPER_BOUNDARY 2

typedef struct playerBullet_struct 
{
	/* remember the start parameters internally */
	int startCol;
	int startRow;
    int animTile;

    bool hit;
    bool outOfUpperBounds; //TODO: delete it in upkeep thread
    
	int row;
	int col;

    player *p;
    
	pthread_t thread;
	pthread_mutex_t *mutex; //pointer to screenLock
} playerBullet;

typedef struct enemyBullet_struct 
{
	/* remember the start parameters internally */
	int startCol;
	int startRow;
    int animTile;

    bool hit;
    bool outOfLowerBounds; //TODO: delete it in upkeep thread

    player *p;
    
	int row;
	int col;
    
	pthread_t thread;
	pthread_mutex_t *mutex; //pointer to screenLock
} enemyBullet;

void newEnemyBullet(enemyBullet *eb);
void newPlayerBullet(playerBullet *pb);
void *runEnemyBullet(void *data);
void *runPlayerBullet(void *data);

#endif