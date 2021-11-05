#ifndef ENEMY_H
#define ENEMY_H

#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 80
#define ENEMY_BODY_ANIM_TILES 4 
#define ENEMY_SPEED
#define NUM_THREADS 1

#define LOWER_ROW_BOUNDARY 14
#define COL_BOUNDARY 80

#include "player.h"
#include "console.h"
#include "threadwrappers.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

// typedef enum playerState_enum 
// {
// 	GAME, // In game
// 	DEAD, // Player dead
// 	GAMEOVER // player win
// } playerState; 

typedef struct enemy_struct 
{
	/* remember the start parameters internally */
	int startCol;
	int startRow;

	bool running;
    bool isHit;

    int length;
    char* direction;

    player *p;
    
	int row;
	int col;
	int animTile;
    
	pthread_t thread;
	pthread_mutex_t *mutex; //pointer to screenLock
} enemy;

void newEnemy(enemy *e);
void *runEnemy(void *data);

#endif