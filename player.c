/**********************************************************************
  Module: player.c
  Author: Micah Hanmin Wang

  Contains all the functions related to the player, including initializing
  a new player, initialize the player thread function, player animation, 
  player moves, and killing player (lives--).

  Also contains the thread function for the player thread.

**********************************************************************/

#include "player.h"
#include <stdio.h>
#include <curses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

//sample player graphic, 3 tile animation.
char* playerGraphic[PLAYER_ANIM_TILES][PLAYER_HEIGHT] = 
{
    {"/T_T\\",
     "|-o-|",
     "/*^*\\",
	 "|||||"},
    {"/UwU\\",
     "|o--|",
     "/***\\",
	 "|||||"},
    {"/OvO\\",
     "|--o|",
     "/***\\",
	 "|||||"}
};

/********************support functions***************/
/* reset the player state to start */

/********************************************************** 
    Initialize player's initial position (row+col)
 **********************************************************/
void newPlayer(player *p) 
{
	p->row = p->startRow;
	p->col = p->startCol;
	p->animTile = 0;
	p->state = GAME;
}

/********************************************************** 
    Refraw the player based on updated positions.
	Screen lock required.
 **********************************************************/
void _playerRedrawMoved(player *p, int prevRow, int prevCol, bool lock) 
{
	if(lock) {
		/* Lock the screen when redraw (clear_draw) the player */
		wrappedMutexLock(p->mutex);
		consoleClearImage(prevRow, prevCol, PLAYER_HEIGHT, PLAYER_WIDTH);
		consoleDrawImage(p->row, p->col, playerGraphic[p->animTile], PLAYER_HEIGHT);
		wrappedMutexUnlock(p->mutex);
	}
}

/********************************************************** 
    Calls _playerRedrawMoved function to redraw the player 
	based on the updated player's poistions
 **********************************************************/
void playerRedraw(player *p, bool lock) 
{
	_playerRedrawMoved(p, p->row, p->col, lock);
}

/********************************************************** 
    Move (redraw) the player based on the updated player's 
	positions (dRow and dCol)
 **********************************************************/
void playerMove(player *f, int dRow, int dCol) {
	_playerRedrawMoved(f, dRow, dCol, true);
}

/********************THREAD functions***************/

/********************************************************** 
    Initialize and spawn the player, calls the player thread
	function with the created player object passed in.
 **********************************************************/
player* spawnPlayer(int startRow, int startCol, int lives, pthread_mutex_t *screenLock, pthread_cond_t *cond_cv)
{
    player* p = (player*)(malloc(sizeof(player)));
	/* error checking */
    if (p == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", sizeof(player));
        abort();
    }

	p->lives = lives; // Lives Initialized to 4
	p->score = 0; // Score initialized to 0
	p->startCol = startCol;
	p->startRow = startRow;
	p->running = true;
	p->mutex = screenLock;
	p->cond_cv = cond_cv;

	wrappedMutexInit(p->mutex, NULL);
	wrappedPthreadCreate(&(p->thread), NULL, runPlayerT, (void*)p);
	return p;
}

/********************************************************** 
    Player Thread function.
    Initialize the new player. Manages the player's state switch.
	If the player is being hit, respawn the player to the original position.
	If player wins, send the signal.
	Manages the change of animTiles, to let the player animate when redraw.
 **********************************************************/
void *runPlayerT(void *data) 
{
	// Pass the reference to the player p
	player* p = (player*)data;
	/* RESET the player state to start */
	newPlayer(p);
	
	while (p->running && p->lives > 0) 
	{
		switch(p->state)
		{
			case DEAD:
				p->lives--;
				/* Freeze the player when it's being hit */
				sleep(2);
				wrappedMutexLock(p->mutex);
				consoleClearImage(p->row, p->col, PLAYER_HEIGHT, PLAYER_WIDTH); /* Clears the hit position */
				wrappedMutexUnlock(p->mutex);
				p->row = p->startRow;
				p->col = p->startCol;
				wrappedMutexLock(p->mutex);
				consoleDrawImage(p->row, p->col, playerGraphic[p->animTile], PLAYER_HEIGHT); /* Respawns to the original pos */
				wrappedMutexUnlock(p->mutex);
				
				if(p->lives > 0) {
					/* Respawns to the original pos, if player's lives > 0 */
					p->state = GAME; 
				}
				else {
					/* player loses. The signal is sent via keyboard thread. No need to do anything here. */
				}
				break;
			case GAMEOVER:
				/* No enemy left. player wins */
				wrappedMutexLock(p->mutex);
				putBanner("You win!!!");
				wrappedMutexUnlock(p->mutex);
				/* Send the signal to let the main program wake up & do the clean up */
				wrappedCondSignal(p->cond_cv);
			default:
				;
		}

		/* Make the spaceship animation */
		wrappedMutexLock(p->mutex);
		p->animTile++;
		p->animTile %= PLAYER_ANIM_TILES;
		wrappedMutexUnlock(p->mutex);

		playerRedraw(p, true);
		sleepTicks(PLAYER_ANIM_TICKS);
	}
	pthread_exit(NULL);
}

/********************************************************** 
    Kill the player when its being hit by an enemy bullet.
	Switch the player's STATE to DEAD
 **********************************************************/
void killPlayer(player* p) {
	p->state = DEAD;
}