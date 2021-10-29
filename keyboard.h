/**********************************************************************
  Module: 
  Author: 

  Purpose: 

**********************************************************************/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "console.h"
#include "threadwrappers.h"
#include "centipede.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct keyboard_struct 
{
    char input;
	/* remember the start parameters internally */
	pthread_t thread;
	pthread_mutex_t mutex;
} keyboard;

keyboard* initKeyboard();

void *runKeyboard(void *data);

#endif