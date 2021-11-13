/*  
  threadeg3b.c

	A program illustrating the use of condition variables to
	synchronize the activities of threads.
	
	Three threads are created, and they will access the shared variable
	count. Access to count is controlled by the mutex count_mutex.
	
	Two threads run the code in change_count; they will access count
	(they may increment it, decrement it, or leave it unchanged).
	They will also signal the third thread (using the condition variable
	count_cv) when the THRESHOLD is reached.
	
	One thread runs the code in watch_count; it is waiting for the
	signal from one of the other threads. Because the value of count
	may be changed between the time this thread is put back on the
	ready queue and the time it actually accesses count, the value
	may not be equal to THRESHOLD. (How can we fix this?)

  To compile: gcc -lpthread threadeg3b.c
*/

#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS  3
#define TCOUNT 60
#define THRESHOLD 27
#define MIN 50
#define MAX 200

// a macro to generate a random int between L and H
#define randInt(L,H) (L)+random()%((H)-(L)+1)

int     		count = 0;
int     		tnums[3] = {0,1,2};
pthread_mutex_t count_mutex;
pthread_cond_t  count_cv = PTHREAD_COND_INITIALIZER;

/*
   change_count
   This thread changes the counter value and signals when
   the threshold value has been reached.
*/
void *change_count(void *idp){
  int i,j;
  int my_id = *(int *)idp;
  int start = my_id;
  int stop = TCOUNT + start;

  for (i=start; i < stop; i++) {
    pthread_mutex_lock(&count_mutex);
    if(i % 4 < 2)
      count++;
    else if(i % 4 == 2)
      count--;

    /* Check value of count and signal when condition is reached. */
    if (count == THRESHOLD) {
      pthread_cond_signal(&count_cv);
      printf("inc_count: thread %d, count=%d Threshold reached.\n",
      my_id, count);
    }
    printf("inc_count: thread %d, count = %d, unlocking mutex\n",
    my_id, count);
    pthread_mutex_unlock(&count_mutex);

    // Sleep so threads can alternate on mutex lock
    usleep(randInt(MIN, MAX));
  }// for
  pthread_exit(NULL);
}// change_count

/*
   watch_count
   This thread watches for the signal that the threshold value
   has been reached.
*/
void *watch_count(void *idp){
  int my_id = *(int *)idp;

  printf("Starting watch_count(): thread %d\n", my_id);

  /* Lock mutex and wait for signal.  */
  pthread_mutex_lock(&count_mutex);
  pthread_cond_wait(&count_cv, &count_mutex);

  printf("watch_count: thread %d Condition signal received.\n",
  my_id);
  printf("watch_count: thread %d, count = %d.\n", my_id, count);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}// watch_count


int main (int argc, char *argv[]){
  int 		i;
  pthread_t tids[3];

  // initialize the mutex
  pthread_mutex_init(&count_mutex, NULL);

  // start the threads
  pthread_create(&tids[2], NULL, watch_count, (void *)&tnums[2]);
  pthread_create(&tids[0], NULL, change_count, (void *)&tnums[0]);
  pthread_create(&tids[1], NULL, change_count, (void *)&tnums[1]);

  // wait for the threads to exit
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(tids[i], NULL);
  }

  // Clean up and exit
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_cv);
  printf("\nEnd of processing.\n");
  pthread_exit(NULL);
} // end main
