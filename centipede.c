/**********************************************************************
  Module: centipede.c
  Author: Micah Hanmin Wang

  Purpose: the core source file for the game engine. This manages the
  thread initialization of all parts, provides functions for global lock
  coordination for the screen, initializes all the threads needed for the 
  main game loop, and helps synchronize threads when necessary.

  The main game loop sleeps and waits for when the game ends (either player wins,
  loses, or the game quits)

  Includes thread functions for: keyboard thread, refresh thread, enemy spawn
  thread and upkeep thread.

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

/**** INITIAL GAMEBOARD */
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

/* The one, true, screen lock that will control each thread's drawing to the screen */
pthread_mutex_t screenLock;

/* keyboard thread */
pthread_t keyboard_thread;
/* keyboard thread's mutex */
pthread_mutex_t keyboard_mutex;
/* refresh thread */
pthread_t refresh_thread;
/* refresh thread's mutex */
pthread_mutex_t refresh_mutex;
/* spawn thread */
pthread_t spawn_thread;
/* upkeep thread */
pthread_t upkeep_thread;
/* upkeep thread's mutex */
pthread_mutex_t upkeep_mutex;

/* The condition variable mutex */
pthread_mutex_t cond_mutex;
/* The condition variable which waits for the game to end */
pthread_cond_t  cond_cv;

/********************************************************** 
    Keyboard Thread function.
    Reads keyboard input and move the player
    or shoots the bullets
    and detects if 'Q' is pressed to quit the game.
    Checks if the game ends regularly to stop waiting for
    getchar().
 **********************************************************/
void *runKeyboard(void* data) {
    /* Pass the reference to the player p */
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
            if(p->state == DEAD) {
                /* Disable the keyboard input for 1s if the player being shot */
                sleep(1);
            }
            /* FD_ISSET(0, &rfds) is true so input is available now. */
            char input;
            /* Get input from the keyboard */
            input = getchar(); // TODO: NO, THIS IS NOT PERFECT, GAME STILL WAITING FOR THIS

            int prevRow = p->row;
            int prevCol = p->col;

            switch(input) {
                case KEY_W_PREESSED:
                        /* Player going up 1 row */
                        if(p->row > PLAYER_BOUNDARY_ROW) {
                                /* Update player's pos */
                                p->row = p->row - 1;
                        }
                        break;
                case KEY_A_PREESSED:
                        /* Player going left 1 col */
                        if(p->col > 0) {
                                p->col = p->col - 1;
                        }
                        break;
                case KEY_S_PREESSED:
                        /* Player going down 1 row */
                        if(p->row < GAME_ROWS-PLAYER_HEIGHT) {
                                /* Update player's pos */
                                p->row = p->row + 1;
                        }
                        break;
                case KEY_D_PREESSED:
                        /* Player going up 1 row */
                        if(p->col < GAME_COLS-PLAYER_WIDTH) {
                                /* Update player's pos */
                                p->col = p->col + 1;
                        }
                        break;
                case SPACE_PREESSED:
                        /* Shoot player bullet from its head */
                        spawnPlayerBullet(p->row-1, p->col+2, p, &screenLock);
                        break;
                case KEY_Q_PREESSED:
                        wrappedMutexLock(&screenLock);
                        /* Put the quitter banner onto the screen if 'Q' being pressed */
                        putBanner("quitter....");
                        wrappedMutexUnlock(&screenLock);
                        wrappedCondSignal(&cond_cv);
                        break;
                default:
                        break;
            }
            /* Move player with the updated player's pos */
            playerMove(p, prevRow, prevCol);
        }
    }
    if(p->lives == 0) {
        /* Player loses, put the game over banner onto the screen */
        wrappedMutexLock(&screenLock);
        putBanner("game over...Do, or do not.. there is no try!");
        wrappedMutexUnlock(&screenLock);
        /* Send the signal to kill the game & clean up */
        wrappedCondSignal(&cond_cv);
    }
    pthread_exit(NULL);
}

/********************************************************** 
    Refresh Thread function.
    A really simple refresh thread.
    Performs refresh() every 1 tick, to get the update from
    the screen buffer onto the screen.
 **********************************************************/
void *runConsoleRefresh(void *data) {
        player* p = (player*)data;
        while(p->running && p->lives > 0) {
                wrappedMutexLock(&screenLock);
                consoleRefresh();
                wrappedMutexUnlock(&screenLock);
                sleepTicks(1);
        }
        pthread_exit(NULL);
}

/********************************************************** 
    Enemy Spawn Thread function.
    Calls the spawnEnemy() function to spawn the enemy at a 
    random time (8000 - 10000 ticks)
    This thread function does not have a loop, it only performs 
    one time, because the while loop is in the spawnEnemy() function.
 **********************************************************/
void *runSpawnThread(void *data) {
        player* p = (player*)data;
        /* Spawn 1 enemy for now. It starts from the upper right corner with the direction left */
        spawnEnemy(ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_WIDTH, "left", true, p, &screenLock);
        pthread_exit(NULL);
}

/********************************************************** 
    The main game mechanism function.
    Initialize the game board, needed locks and threads
    Then waits (sleeps) on a condition variable.
    If the signal is caught, it is woken up and do the clean
    up works.
 **********************************************************/
void centipedeRun()
{
        /* Initialize the game board */
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
        {
                /* Initialize screenLock */
                wrappedMutexInit(&screenLock, NULL);

                /* Initialize player on the screen. startRow=20, startColumn=36, lives=4 */
                player *p = spawnPlayer(PLAYER_START_ROW, PLAYER_START_COL, 1, &screenLock, &cond_cv);

                /* Initialize the spawn thread on the screen. startRow=0, startColumn=80 */
                wrappedPthreadCreate(&(spawn_thread), NULL, runSpawnThread, (void*)p);

                wrappedMutexInit(&keyboard_mutex, NULL);
                /* Initialize keyboard thread */
                wrappedPthreadCreate(&(keyboard_thread), NULL, runKeyboard, (void*)p);

                wrappedMutexInit(&refresh_mutex, NULL);
                /* Initialize refresh thread */
                wrappedPthreadCreate(&(refresh_thread), NULL, runConsoleRefresh, (void*)p);

                wrappedMutexInit(&upkeep_mutex, NULL);
                /* Initialize the upkeep thread */
                wrappedPthreadCreate(&(upkeep_thread), NULL, runUpkeep, (void*)p);              

                pthread_cond_init(&cond_cv, NULL);
                wrappedMutexInit(&cond_mutex, NULL);
                wrappedMutexLock(&cond_mutex);
                /* Sleeps on a condition variable */
                wrappedCondWait(&cond_cv, &cond_mutex);
                wrappedMutexUnlock(&cond_mutex);

                /* After woken up, clean up all the threads and memories */
                cleanUp(p);
                /* wait for final key before killing curses and game */
                finalKeypress();

        }
        consoleFinish();
}

/********************************************************** 
    Does the clean up work when the game ends (quit, win, lose).
    Joins:
    1. Player thread
    2. Keyboard thread
    3. Refresh thread
    4. Upkeep thread
    5. Enemy Spawn thread
    6. Enemy threads
    7. Player & Enemy Bullet threads
    And destroys all mutexes, and the condition variable
    Also frees all the malloced memories.
 **********************************************************/
void cleanUp(player *p) {
        p->running = false;

        pthread_cancel(p->thread);
        wrappedPthreadJoin(p->thread, NULL); // Join the only player thread
        pthread_cancel(keyboard_thread);
        wrappedPthreadJoin(keyboard_thread, NULL); // Join the only keyboard thread
        pthread_cancel(refresh_thread);
        wrappedPthreadJoin(refresh_thread, NULL); // Join the only refresh thread
        pthread_cancel(upkeep_thread);
        wrappedPthreadJoin(upkeep_thread, NULL); // Join the only upkeep thread
        pthread_cancel(spawn_thread);
        wrappedPthreadJoin(spawn_thread, NULL); // Join the only spawn thread

        enemyNode *enemyList = getEnemyQueue();
        while(enemyList != NULL) {
                pthread_cancel(enemyList->e->thread);
                wrappedPthreadJoin(enemyList->e->thread, NULL);
                free(enemyList->e);
                enemyList = enemyList -> next;
        }

        BulletNode *bulletList = getBulletQueue();
        while(bulletList != NULL) {
                if(bulletList->eb != NULL) {
                        pthread_cancel(bulletList->eb->thread);
                        wrappedPthreadJoin(bulletList->eb->thread, NULL);
                        free(bulletList->eb);
                }
                else if(bulletList->pb != NULL) {
                        pthread_cancel(bulletList->pb->thread);
                        wrappedPthreadJoin(bulletList->pb->thread, NULL);
                        free(bulletList->pb);
                }
                bulletList = bulletList -> next;
        }

        pthread_mutex_destroy(&cond_mutex);
        pthread_cond_destroy(&cond_cv);

        pthread_mutex_destroy(&keyboard_mutex);
        pthread_mutex_destroy(&refresh_mutex);
        pthread_mutex_destroy(&upkeep_mutex);

        pthread_mutex_destroy(&screenLock);

        free(enemyList);
        free(bulletList);
        free(p);
}

