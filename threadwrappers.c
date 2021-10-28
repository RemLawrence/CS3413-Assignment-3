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

int wrappedPthreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	return statusCheck(pthread_create(thread, attr, start_routine, arg));
}

int wrappedPthreadJoin(pthread_t thread, void **retval) {
	return statusCheck(pthread_join(thread, retval));
}



int wrappedMutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return statusCheck(pthread_mutex_init(mutex, mutexattr));
}

int wrappedMutexLock(pthread_mutex_t *mutex) {
	return statusCheck(pthread_mutex_lock(mutex));
}

int wrappedMutexUnlock(pthread_mutex_t *mutex) {
	return statusCheck(pthread_mutex_unlock(mutex));
}



int wrappedCondSignal(pthread_cond_t *cond) {
	return statusCheck(pthread_cond_signal(cond));
}

int wrappedCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	return statusCheck(pthread_cond_wait(cond, mutex));
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