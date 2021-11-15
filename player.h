/**********************************************************************
  Module: player.h
  Author: Micah Hanmin Wang

  Purpose: manages the player's ship for invaders
  Stores the constants that the Upkeep threads need.

**********************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_ANIM_TILES 3
#define PLAYER_ANIM_TICKS 30
#define PLAYER_WIDTH 5 
#define PLAYER_HEIGHT 4 

#include "console.h"
#include "threadwrappers.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>


typedef enum playerState_enum 
{
	GAME, // In game
	DEAD, // Player dead
	GAMEOVER // player win
} playerState; 

typedef struct player_struct 
{
	/* remember the start parameters internally */
	int startCol;
	int startRow;

	playerState state;

	bool running;
	int lives;
	int score;
	int row;
	int col;
	int animTile;
	pthread_t thread;
	pthread_mutex_t *mutex; //pointer to screenLock
	pthread_cond_t  *cond_cv;
} player;

void *runPlayerT(void *data);

player* spawnPlayer(int startRow, int startCol, int lives, pthread_mutex_t *screenLock, pthread_cond_t *cond_cv);

/* thread safe for player & ALSO LOCKS SCREEN */
//yours may or may not lock the screen on call.
void playerMove(player *f, int dRow, int dCol); 

/* NOT thread safe */
/* removes a life, ends the game if all lives gone */
//students: I actually left this as not thread safe in my final solution
//and I made sure it was used in a thread safe way
void killPlayer(player* p);

#endif