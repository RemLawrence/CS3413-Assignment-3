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

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "console.h"
#include "centipede.h"
#include "player.h"
#include "enemy.h"
#include "llist.h"
#include "bullet.h"

#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'
#define SPACE_PREESSED ' '
#define KEY_Q_PREESSED 'q'
#define PLAYER_BOUNDARY_ROW 17

void *runKeyboard(void *data);

void *runConsoleRefresh(void *data);

void *runSpawnThread(void *data);

void centipedeRun();


#endif
