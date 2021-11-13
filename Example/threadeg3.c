/*  
  threadeg3.c

	A program illustrating the use of condition variables to
	synchronize the activities of threads.
	
	Three threads are created, and they will access the shared variable
	count. Access to count is controlled by the mutex count_mutex.
	
	Two threads run the code in inc_count; they will increment count.
	They will also signal the third thread (using the condition variable
	count_cv) when the COUNT_LIMIT is reached.
	
	One thread runs the code in watch_count; it is waiting for the
	signal from one of the other threads.
	
	To compile: gcc -lpthread threadeg3.c
*/

#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS  3
#define TCOUNT 5
#define COUNT_LIMIT 7

int     		count = 0;
int     		tnums[3] = {0,1,2};
pthread_mutex_t count_mutex;
pthread_cond_t  count_cv;

void *inc_count(void *idp) 
{
  int j,i;
  double result=0.0;
  int *my_id = idp;

  for (i=0; i < TCOUNT; i++) {
    pthread_mutex_lock(&count_mutex);
    count++;

    /* Check value of count and signal when condition is reached. */
    if (count == COUNT_LIMIT) {
      pthread_cond_signal(&count_cv);
      printf("inc_count(): thread %d, count = %d  Threshold reached.\n", *my_id, count);
    }
    printf("inc_count(): thread %d, count = %d, unlocking mutex\n", *my_id, count);
    pthread_mutex_unlock(&count_mutex);

    /* Do some work so threads can alternate on mutex lock */
    for (j=0; j < 1000; j++)
      result = result + (double)random();
    }
  pthread_exit(NULL);
}

void *watch_count(void *idp) 
{
  int *my_id = idp;

  printf("Starting watch_count(): thread %d\n", *my_id);

  /* Lock mutex and wait for signal.  */
  pthread_mutex_lock(&count_mutex);
  while (count < COUNT_LIMIT) {
    pthread_cond_wait(&count_cv, &count_mutex);
    printf("watch_count(): thread %d Condition signal received.\n", *my_id);
    }
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
  int 		i;
  pthread_t tids[3];

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_cv, NULL);

  pthread_create(&tids[2], NULL, watch_count, (void *)&tnums[2]);
  pthread_create(&tids[0], NULL, inc_count, (void *)&tnums[0]);
  pthread_create(&tids[1], NULL, inc_count, (void *)&tnums[1]);

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(tids[i], NULL);
  }

  /* Clean up and exit */
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_cv);
  pthread_exit(NULL);
} // end main
