/**********************************************************************
  Module: centipede.c
  Author: Daniel Rea

  Purpose: the core source file for the game engine. This manages the
  thread initialization of all parts, provides functions for global lock
  coordination for the screen, and helps synchronize threads when necessary.

  Use this to get an idea of how to add things as part of the game.
  Feel free to use any of this freely in your solution, or change or modify
  this code.

  Some functions unimplemented, just to show how I roughly went about things.

**********************************************************************/
#include "centipede.h"
#include <string.h>
#include <unistd.h>
#include <pthread.h>


//all these constants and gameboard should probably go in a constants file...hint hint
#define GAME_ROWS 24
#define GAME_COLS 80

/**** DIMENSIONS MUST MATCH the ROWS/COLS */
char *GAME_BOARD[] = {
"                   Score:          Lives:",
"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-centipiede!=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"",
"",
"",
"",
"",
"",
"", 
"" };


#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 1
#define ENEMY_BODY_ANIM_TILES 4 
#define NUM_THREADS 1
char* ENEMY_BODY[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
  {"1",
   "1"},
  {"2",
   "2"},
  {"3",
   "3"},
  {"4",
   "4"}
};



//the rest will hold the main game engine
//it's likely you'll add quite a bit to it (input, cleanup, synchronization, etc)

//you'll probably want a sort of globally accessible function to the player, enemy, etc
//to control screen locking here.

// THE MAIN, ULTIMATE GAME ENGINE
void centipedeRun()
{
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
        {
                //initialize player on the screen. startRow=20, startColumn=36, lives=4
                player *p = spawnPlayer(20, 36, 4);
                
                //above, initialize all the threads you need
                //below, you should make a "gameplay loop" that manages screen drawing
                //that  waits on a condition variable until the game is over
                //and coordinates all threads to end

                //animate an "enemy" made of numbers on the screen every second for 10s
                //this isn't part of my solution, but is for illustration purposes
		int i = 0;
                while(p->lives >= 0)
		{
		        char** tile = ENEMY_BODY[i%ENEMY_BODY_ANIM_TILES];

                        //probably not threadsafe here...
                        //start centipede at tile 10, 10, move it horizontally once a frame/tick
                        //we create the illusion of movement by clearing the screen where the centipede was last
                        //then drawing it in the new location. 
                        consoleClearImage(10, 10+i-1, ENEMY_HEIGHT, ENEMY_WIDTH);
			consoleDrawImage(10, 10+i, tile, ENEMY_HEIGHT);
			consoleRefresh(); //draw everything to screen.
			sleepTicks(60);
                        i++;
		}		

                //note after this the player thread keeps running and isn't cleaned
                //up properly. Why don't we see it update on screen?
                
                finalKeypress(); /* wait for final key before killing curses and game */
                p->running = false;
                pthread_join(p->thread, NULL);
        }       
        
        consoleFinish();        	
}

