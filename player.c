/**********************************************************************
  Module: player.c
  Author: Daniel Rea

  Purpose: code to run the player in its own thread.
	For distribution to students. Not all functions implemented.
	This is just from my solution with parts removed. 
    Treat it as a guide. Feel free to implement,
	change, remove, etc, in your own solution.

**********************************************************************/

#include "player.h"

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
	 "||||"},
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
	wrappedMutexLock(&p->mutex);
	consoleClearImage(prevRow, prevCol, PLAYER_HEIGHT, PLAYER_WIDTH);
	consoleDrawImage(p->row, p->col, playerGraphic[p->animTile], PLAYER_HEIGHT);
	wrappedMutexUnlock(&p->mutex);
	//TODO: unlock screen
}

void playerRedraw(player *p, bool lock) 
{
	_playerRedrawMoved(p, p->row, p->col, lock);
}

/********************THREAD functions***************/

player* spawnPlayer(int startRow, int startCol, int lives)
{
    player* p = (player*)(malloc(sizeof(player)));
	p->lives = lives;
	p->startCol = startCol;
	p->startRow = startRow;
	p->running = true;

	//TODO: Init mutex...
	wrappedMutexInit(&p->mutex, NULL);
	wrappedPthreadCreate(&(p->thread), NULL, runPlayerT, (void*)p);
	return p;
}

void *runPlayerT(void *data) 
{
	// Pass the reference to the player p
	player* p = (player*)data;
	/* RESET the player state to start */
	newPlayer(p);
	
	while (p->running && p->lives >= 0) 
	{
		//TODO: not threadsafe!!!!
		switch(p->state)
		{
			case DEAD:
				wrappedMutexLock(&p->mutex);
				p->lives--;
				wrappedMutexUnlock(&p->mutex);
				//...other code here...
			default:
				;
		}

		// Make the spaceship animation
		wrappedMutexLock(&p->mutex);
		p->animTile++;
		p->animTile %= PLAYER_ANIM_TILES;
		wrappedMutexUnlock(&p->mutex);

		playerRedraw(p, true);
		sleepTicks(PLAYER_ANIM_TICKS);
		
	}
	return NULL;
}