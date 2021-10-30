/**********************************************************************
  Module: player.h
  Author: Daniel Rea

  Purpose: manages the player's ship for invaders
  cut down from my solution to distribute as sample framework code

  Use this to get an idea of how to add things as part of the game.
  Feel free to use any of this freely in your solution, or change or modify
  this code.

  Some functions unimplemented, just to show how I roughly went about things.

**********************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_ANIM_TILES 3
#define PLAYER_ANIM_TICKS 30
#define PLAYER_WIDTH 3 
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
	int row;
	int col;
	int animTile;
	pthread_t thread;
	pthread_mutex_t mutex;
} player;

void *runPlayerT(void *data);

player* spawnPlayer(int startRow, int startCol, int lives);

/* thread safe for player & ALSO LOCKS SCREEN */
//yours may or may not lock the screen on call.
void playerMove(player *f, int dRow, int dCol); 

/* NOT thread safe */
/* removes a life, ends the game if all lives gone */
//students: I actually left this as not thread safe in my final solution
//and I made sure it was used in a thread safe way
void killPlayer(player* p);

#endif