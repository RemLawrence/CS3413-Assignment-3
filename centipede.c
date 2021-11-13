/**********************************************************************
  Module: centipede.c
  Author: Daniel Rea

  Purpose: the core source file for the game engine. This manages the
  thread initialization of all parts, provides functions for global lock
  coordination for the screen, initializes all the threads needed for the 
  main game loop, and helps synchronize threads when necessary.

  The main game loop sleeps and waits for when the game ends (either player wins,
  loses, or the game quits)

  Includes thread functions for: keyboard thread, refresh thread, and enemy spawn
  thread.

**********************************************************************/
#include "centipede.h"
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>


//all these constants and gameboard should probably go in a constants file...hint hint
#define GAME_ROWS 24
#define GAME_COLS 80

/**** DIMENSIONS MUST MATCH the ROWS/COLS */
char *GAME_BOARD[] = {
"                   Score:    0     Lives:    4",
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

pthread_mutex_t screenLock; // Screen Lock

//initialize keyboard thread
pthread_t keyboard_thread;
pthread_mutex_t keyboard_mutex;
//initialize redraw/refresh thread
pthread_t refresh_thread;
pthread_mutex_t refresh_mutex;
//initialize spawn enemy thread
pthread_t spawn_thread;
//initialize upkeep thread
pthread_t upkeep_thread;
pthread_mutex_t upkeep_mutex;

pthread_mutex_t cond_mutex;
pthread_cond_t  cond_cv;

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

    while(p->running && p->lives > 0) {
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        else {
            /* FD_ISSET(0, &rfds) is true so input is available now. */
            char input;
            input = getchar(); // TODO: NO, THIS IS NOT PERFECT, GAME STILL WAITING FOR THIS 

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
                case SPACE_PREESSED:
                        // Shoot bullet
                        spawnPlayerBullet(p->row-1, p->col+2, p, &screenLock);
                        break;
                case KEY_Q_PREESSED:
                        wrappedMutexLock(&screenLock);
                        putBanner("quitter....");
                        wrappedMutexUnlock(&screenLock);
                        wrappedCondSignal(&cond_cv);
                        break;
                default:
                        break;
            }
                playerMove(p, prevRow, prevCol);
        }
    }
    if(p->lives == 0) {
        wrappedMutexLock(&screenLock);
        putBanner("game over...Do, or do not.. there is no try!");
        wrappedMutexUnlock(&screenLock);
        wrappedCondSignal(&cond_cv);
    }

    pthread_exit(NULL);
}

void *runConsoleRefresh(void *data) {
        player* p = (player*)data;
        while(p->running && p->lives > 0) {
                wrappedMutexLock(&screenLock);
                consoleRefresh();
                wrappedMutexUnlock(&screenLock);
        }
        pthread_exit(NULL);
}

void *runSpawnThread(void *data) {
        player* p = (player*)data;
        spawnEnemy(2, 80, ENEMY_WIDTH, "left", true, p, &screenLock);
        pthread_exit(NULL);
}

// THE MAIN, ULTIMATE GAME ENGINE
void centipedeRun()
{
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
        {
                wrappedMutexInit(&screenLock, NULL); // Initialize screenLock

                //initialize player on the screen. startRow=20, startColumn=36, lives=4
                player *p = spawnPlayer(20, 36, 4, &screenLock, &cond_cv);

                //initialize the spawn thread on the screen. startRow=0, startColumn=80
                wrappedPthreadCreate(&(spawn_thread), NULL, runSpawnThread, (void*)p);

                wrappedMutexInit(&keyboard_mutex, NULL);
                wrappedPthreadCreate(&(keyboard_thread), NULL, runKeyboard, (void*)p);

                wrappedMutexInit(&refresh_mutex, NULL);
                wrappedPthreadCreate(&(refresh_thread), NULL, runConsoleRefresh, (void*)p);

                wrappedMutexInit(&upkeep_mutex, NULL);
                wrappedPthreadCreate(&(upkeep_thread), NULL, runUpkeep, (void*)p);              
                
                //above, initialize all the threads you need
                //below, you should make a "gameplay loop" that manages screen drawing
                //that  waits on a condition variable until the game is over
                //and coordinates all threads to end

                //note after this the player thread keeps running and isn't cleaned
                //up properly. Why don't we see it update on screen?
                pthread_cond_init(&cond_cv, NULL);
                wrappedMutexInit(&cond_mutex, NULL); // Initialize screenLock
                wrappedMutexLock(&cond_mutex);
                wrappedCondWait(&cond_cv, &cond_mutex);
                wrappedMutexUnlock(&cond_mutex);

                cleanUp(p);
                finalKeypress(); /* wait for final key before killing curses and game */

        }
        consoleFinish();
}

void cleanUp(player *p) {
        p->running = false;

        pthread_cancel(p->thread);
        pthread_join(p->thread, NULL); // Join the only player thread
        pthread_cancel(keyboard_thread);
        pthread_join(keyboard_thread, NULL); // Join the only keyboard thread
        pthread_cancel(refresh_thread);
        pthread_join(refresh_thread, NULL); // Join the only refresh thread
        pthread_cancel(upkeep_thread);
        pthread_join(upkeep_thread, NULL); // Join the only upkeep thread
        pthread_cancel(spawn_thread);
        pthread_join(spawn_thread, NULL); // Join the only spawn thread

        enemyNode *enemyList = getEnemyQueue();
        while(enemyList != NULL) {
                pthread_cancel(enemyList->e->thread);
                pthread_join(enemyList->e->thread, NULL);
                free(enemyList->e);
                enemyList = enemyList -> next;
        }

        BulletNode *bulletList = getBulletQueue();
        while(bulletList != NULL) {
                if(bulletList->eb != NULL) {
                        pthread_cancel(bulletList->eb->thread);
                        pthread_join(bulletList->eb->thread, NULL);
                        free(bulletList->eb);
                }
                else if(bulletList->pb != NULL) {
                        pthread_cancel(bulletList->pb->thread);
                        pthread_join(bulletList->pb->thread, NULL);
                        free(bulletList->pb);
                }
                bulletList = bulletList -> next;
        }

        pthread_mutex_destroy(&cond_mutex);
        pthread_cond_destroy(&cond_cv);

        pthread_mutex_destroy(&screenLock);

        free(enemyList);
        free(bulletList);
        free(p);
}

