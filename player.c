/**********************************************************************
  Module: player.c
  Author: Daniel Rea

  Contains all the functions related to the player, including initializing
  a new player, initialize the player thread function, player animation, 


**********************************************************************/

#include "player.h"
#include <stdio.h>
#include <curses.h>
#include <pthread.h>
#include <unistd.h>

//sample player graphic, 3 tile animation.
//feel free to use this or make your own
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
void newPlayer(player *p) 
{
	p->row = p->startRow;
	p->col = p->startCol;
	p->animTile = 0;
	p->state = GAME;
}

void _playerRedrawMoved(player *p, int prevRow, int prevCol, bool lock) 
{
	//TODO
	//Dear students, this function is NOT THREAD SAFE and will require fixing
	//TODO: lock screen (critical shared resource)
	if(lock) {
		wrappedMutexLock(p->mutex);
		consoleClearImage(prevRow, prevCol, PLAYER_HEIGHT, PLAYER_WIDTH);
		consoleDrawImage(p->row, p->col, playerGraphic[p->animTile], PLAYER_HEIGHT);
		wrappedMutexUnlock(p->mutex);
	}
	//TODO: unlock screen
}

void playerRedraw(player *p, bool lock) 
{
	_playerRedrawMoved(p, p->row, p->col, lock);
}

void playerMove(player *f, int dRow, int dCol) {
	_playerRedrawMoved(f, dRow, dCol, true);
}

/********************THREAD functions***************/

player* spawnPlayer(int startRow, int startCol, int lives, pthread_mutex_t *screenLock, pthread_cond_t *cond_cv)
{
    player* p = (player*)(malloc(sizeof(player)));
	p->lives = lives;
	p->score = 0;
	p->startCol = startCol;
	p->startRow = startRow;
	p->running = true;
	p->mutex = screenLock;
	p->cond_cv = cond_cv;

	wrappedMutexInit(p->mutex, NULL);
	wrappedPthreadCreate(&(p->thread), NULL, runPlayerT, (void*)p);
	return p;
}

void *runPlayerT(void *data) 
{
	// Pass the reference to the player p
	player* p = (player*)data;
	/* RESET the player state to start */
	newPlayer(p);
	
	while (p->running && p->lives > 0) 
	{
		//TODO: not threadsafe!!!!
		switch(p->state)
		{
			case DEAD:
				//wrappedMutexLock(p->mutex);
				p->lives--;
				/* Freeze the screen */
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
				wrappedCondSignal(p->cond_cv);
			default:
				;
		}

		// Make the spaceship animation
		wrappedMutexLock(p->mutex);
		p->animTile++;
		p->animTile %= PLAYER_ANIM_TILES;
		wrappedMutexUnlock(p->mutex);

		playerRedraw(p, true);
		sleepTicks(PLAYER_ANIM_TICKS);
		
	}
	pthread_exit(NULL);
}

void killPlayer(player* p) {
	p->state = DEAD;
}