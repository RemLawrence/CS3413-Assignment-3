/**********************************************************************
  Module: centipede.h
  Author: Micah Hanmin Wang

  Stores the constants that the main program (centipede.c) needs, such 
  as the keyboard strokes.

**********************************************************************/

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "console.h"
#include "centipede.h"
#include "player.h"
#include "enemy.h"
#include "llist.h"
#include "bullet.h"
#include "gameglobals.h"

#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'
#define SPACE_PREESSED ' '
#define KEY_Q_PREESSED 'q'
#define PLAYER_BOUNDARY_ROW 17

#define ENEMY_HEIGHT 2
#define ENEMY_WIDTH 80
#define ENEMY_SPEED 40

#define PLAYER_START_ROW 20
#define PLAYER_START_COL 36
#define PLAYER_INIT_LIVES 4

void *runKeyboard(void *data);

void *runConsoleRefresh(void *data);

void *runSpawnThread(void *data);

void centipedeRun();

void cleanUp(player *p);

#endif
