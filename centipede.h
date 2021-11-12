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

void *runKeyboard(void *data);

void *runConsoleRefresh(void *data);

void *runSpawnThread(void *data);

void centipedeRun();

void cleanUp(player *p);

#endif
