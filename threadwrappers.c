/**********************************************************************
  Module: threadwrappers.h
  Author: Daniel Rea 

  Purpose: error-checking wrappers for pthread functions
	For distribution to students. Not all functions implemented.
	This is just from my solution with parts removed. 
  	Treat it as a guide. Feel free to implement,
	change, remove, etc, in your own solution.

**********************************************************************/
#include <errno.h>
#include <stdio.h>
#include "threadwrappers.h"

int statusCheck(int s)
{
	if ((errno = s) != 0)
		perror(NULL);
	return s;
}

int wrappedPthreadCreate(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
	return statusCheck(pthread_create(thread, attr, start_routine, arg));
}

/*
add other helper functions here like 

mutex init
mutex lock
mutex unlock 
thread join,
signal,
wait,
etc
etc
*/