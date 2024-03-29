/**********************************************************************
  Module: enemy.h
  Author: Micah Hanmin Wang

  Purpose: Manages the invaders (aka enemies, centipedes, caterpillers)
  Stores the constants that the enemy threads needed.
  Declares the enemy struct

**********************************************************************/

#ifndef ENEMY_H
#define ENEMY_H

#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 80
#define ENEMY_BODY_ANIM_TILES 4 
#define NUM_THREADS 1
#define ENEMY_MIN_WIDTH 4

#define LOWER_ROW_BOUNDARY 14
#define COL_BOUNDARY 80
#define LAST_ROW 14
#define ENEMY_FIRST_ROW 2

#define ENEMY_SPEED 40

#include "player.h"
#include "console.h"
#include "threadwrappers.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct enemy_struct 
{
	/* remember the start parameters internally */
	int startCol;
	int startRow;

	bool isDead;
    bool isHit;

    int length;
    int speed;
    char* direction;

    player *p;
    
	int row;
	int col;
	int animTile;
    
	pthread_t thread;
	pthread_mutex_t *mutex; //pointer to screenLock
    pthread_mutex_t *enemyLock;
} enemy;

void newEnemy(enemy *e);
void *runEnemy(void *data);

#endif