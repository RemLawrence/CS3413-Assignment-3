/**********************************************************************
  Module: 
  Author: 

  Purpose: 

**********************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_W_PREESSED 'w'
#define KEY_A_PREESSED 'a'
#define KEY_S_PREESSED 's'
#define KEY_D_PREESSED 'd'

#include "player.h"
#include "console.h"
#include "threadwrappers.h"
#include "centipede.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct keyboard_struct 
{
    char input;
    player *player;
	/* remember the start parameters internally */
	pthread_t thread;
	pthread_mutex_t mutex;
} keyboard;

keyboard* initKeyboard(player *player);

void *runKeyboard(void *data);

#endif