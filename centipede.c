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
#include "player.h"
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "stdio.h"
#include <sys/select.h>


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

#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'
#define KEY_Q_PREESSED 'q'

pthread_mutex_t keyboard_mutex;
pthread_mutex_t refresh_mutex;

//the rest will hold the main game engine
//it's likely you'll add quite a bit to it (input, cleanup, synchronization, etc)

//you'll probably want a sort of globally accessible function to the player, enemy, etc
//to control screen locking here.

void *runKeyboard(void* data) {

    // Pass the reference to the player p
    player* p = (player*)data;

    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0 to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /* Wait up to 1 seconds. */
    wrappedMutexLock(&keyboard_mutex);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    wrappedMutexUnlock(&keyboard_mutex);

    while(1) {
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        else {
            /* FD_ISSET(0, &rfds) is true so input is available now. */
            char input;
            input = getchar();
                
            int prevRow = p->row;
            int prevCol = p->col;

            switch(input) {
                case KEY_W_PREESSED:
                        p->row = p->row - 1;
                        playerMove(p, prevRow, prevCol);
                        break;
                case KEY_A_PREESSED:
                        p->col = p->col - 1;
                        playerMove(p, prevRow, prevCol);
                        break;
                case KEY_S_PREESSED:
                        p->row = p->row + 1;
                        playerMove(p, prevRow, prevCol);
                        break;
                case KEY_D_PREESSED:
                        p->col = p->col + 1;
                        playerMove(p, prevRow, prevCol);
                        break;
                case KEY_Q_PREESSED:
                        putBanner("quitter....");
                        break;
                default:
                        break;
            }
            
            //TODO: CHECK IF GAME IS OVER? If over, before blocking 
            //again, quit the thread
        }
    }
    putBanner("game over...Do, or do not.. there is no try!");

    return NULL;
}

void *runConsoleRefresh(void *data) {
        while(1) {
                wrappedMutexLock(&refresh_mutex);
    
                consoleRefresh();

                wrappedMutexUnlock(&refresh_mutex);
        }
}

// THE MAIN, ULTIMATE GAME ENGINE
void centipedeRun()
{
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
        {
                //initialize player on the screen. startRow=20, startColumn=36, lives=4
                player *p = spawnPlayer(20, 36, 4);

                //initialize keyboard thread
                pthread_t keyboard_thread;
                wrappedMutexInit(&keyboard_mutex, NULL);
                wrappedPthreadCreate(&(keyboard_thread), NULL, runKeyboard, (void*)p);

                //initialize redraw/refresh thread
                pthread_t refresh_thread;
                wrappedMutexInit(&refresh_mutex, NULL);
                wrappedPthreadCreate(&(refresh_thread), NULL, runConsoleRefresh, (void*)p);
                
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
			//consoleRefresh(); //draw everything to screen.
			sleepTicks(60);
                        i++;
		}		

                //note after this the player thread keeps running and isn't cleaned
                //up properly. Why don't we see it update on screen?
                
                finalKeypress(); /* wait for final key before killing curses and game */
                p->running = false;
                pthread_join(p->thread, NULL);
                pthread_join(keyboard_thread, NULL);
        }       
        
        consoleFinish();
        
}

