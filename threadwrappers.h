/**********************************************************************
  Module: threadwrappers.h
  Author: Daniel Rea

  Purpose: error-checking wrappers for pthread functions
	For distribution to students. Not all functions implemented.
	This is just from my solution with parts removed. 
  Treat it as a guide. Feel free to implement,
	change, remove, etc, in your own solution.

**********************************************************************/

#ifndef THREADWRAPPERS_H
#define THREADWRAPPERS_H

#include <pthread.h>

int wrappedMutexLock(pthread_mutex_t *mutex);
int wrappedMutexUnlock(pthread_mutex_t *mutex);
int wrappedMutexInit(pthread_mutex_t  *mutex,  const  pthread_mutexattr_t *mutexattr);

int wrappedPthreadCreate(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
int wrappedPthreadJoin(pthread_t thread, void **retval);

int wrappedCondSignal(pthread_cond_t *cond);

int wrappedCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex);


#endif /* THREADWRAPPERS_H*/
