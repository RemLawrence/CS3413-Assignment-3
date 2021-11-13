/*  
  threadeg2.c

	A program illustrating the use of a mutex lock to protect access
	to shared data. Create 100 threads that will all access the shared
	variable sum. Access to sum is controlled by the mutex sum_mutex
    
	To compile: gcc -lpthread threadeg2.c
*/
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS     100


pthread_mutex_t	sum_mutex;	// a mutex variable
long			sum=0;	// shared variable


void *Thread(void *dummy){
   printf("\nThread <%d>: \n", (int)pthread_self());
   pthread_mutex_lock(&sum_mutex);
   sum += (int) pthread_self()/10000;
   pthread_mutex_unlock(&sum_mutex);
   pthread_exit(NULL);
} // end firstThread


int main (int argc, char * argv[]){

   pthread_t tids[NUM_THREADS];
   int rc, t;

   for(t=0;t<NUM_THREADS;t++){
		pthread_create(&tids[t],NULL,Thread,NULL);
	 }
   for(t=0;t<NUM_THREADS;t++){
		pthread_join(tids[t],NULL);
	 }
	 printf("Sum is %ld\n",sum);
   pthread_exit(NULL);
} // end main

