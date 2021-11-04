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
#include "enemy.h"
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

#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'
#define KEY_Q_PREESSED 'q'
#define PLAYER_BOUNDARY_ROW 17

pthread_mutex_t screenLock; // Screen Lock (p->mutex now)
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
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while(p->running && p->lives >= 0) {
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
                        // Upper boundary
                        if(p->row > PLAYER_BOUNDARY_ROW) {
                                p->row = p->row - 1;
                        }
                        break;
                case KEY_A_PREESSED:
                        // Left boundary
                        if(p->col > 0) {
                                p->col = p->col - 1;
                        }
                        break;
                case KEY_S_PREESSED:
                        // Lower boundary
                        if(p->row < GAME_ROWS-PLAYER_HEIGHT) {
                                p->row = p->row + 1;
                        }
                        break;
                case KEY_D_PREESSED:
                        // Right boundary
                        if(p->col < GAME_COLS-PLAYER_WIDTH) {
                                p->col = p->col + 1;
                        }
                        break;
                case KEY_Q_PREESSED:
                        wrappedMutexLock(&screenLock);
                        putBanner("quitter....");
                        wrappedMutexUnlock(&screenLock);
                        break;
                default:
                        break;
            }
                playerMove(p, prevRow, prevCol);
        }
    }
    wrappedMutexLock(&screenLock);
    putBanner("game over...Do, or do not.. there is no try!");
    wrappedMutexUnlock(&screenLock);

    return NULL;
}

void *runConsoleRefresh(void *data) {
        player* p = (player*)data;
        while(p->lives >= 0) {
                wrappedMutexLock(&screenLock);
                consoleRefresh();
                wrappedMutexUnlock(&screenLock);
        }
        return NULL;
}

// THE MAIN, ULTIMATE GAME ENGINE
void centipedeRun()
{
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
        {
                wrappedMutexInit(&screenLock, NULL); // Initialize screenLock
                //initialize player on the screen. startRow=20, startColumn=36, lives=4
                player *p = spawnPlayer(20, 36, 4, &screenLock);

                //initialize keyboard thread
                pthread_t keyboard_thread;
                wrappedMutexInit(&keyboard_mutex, NULL);
                wrappedPthreadCreate(&(keyboard_thread), NULL, runKeyboard, (void*)p);

                //initialize redraw/refresh thread
                pthread_t refresh_thread;
                wrappedMutexInit(&refresh_mutex, NULL);
                wrappedPthreadCreate(&(refresh_thread), NULL, runConsoleRefresh, (void*)p);

                //initialize enemy on the screen. startRow=0, startColumn=80
                enemy *e = spawnEnemy(2, 80, p, &screenLock);
                
                //above, initialize all the threads you need
                //below, you should make a "gameplay loop" that manages screen drawing
                //that  waits on a condition variable until the game is over
                //and coordinates all threads to end

                	

                //note after this the player thread keeps running and isn't cleaned
                //up properly. Why don't we see it update on screen?
                while(p->running && p->lives >= 0) {

                }
                //finalKeypress(); /* wait for final key before killing curses and game */
                p->running = false;
                pthread_join(p->thread, NULL);
                pthread_join(keyboard_thread, NULL);
                pthread_join(refresh_thread, NULL);
                pthread_join(e->thread, NULL);

                free(p);
                free(e);
        }       
        
        consoleFinish();
        
}

