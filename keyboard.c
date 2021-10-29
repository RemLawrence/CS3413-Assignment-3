#include "keyboard.h"
#include "stdio.h"
#include <sys/select.h>

keyboard* initKeyboard()
{
    keyboard* k = (keyboard*)(malloc(sizeof(keyboard)));
    k->input = NULL;

	//TODO: Init mutex...
    wrappedMutexInit(&k->mutex, NULL);
	wrappedPthreadCreate(&(k->thread), NULL, runKeyboard, (void*)k);
	return k;
}

void *runKeyboard(void* data) {
    // Pass the reference to the keyboard k
	keyboard* k = (keyboard*)data;

    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /* Wait up to 1 seconds. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while(1){
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        else {
            /* FD_ISSET(0, &rfds) is true so input is available now. */
            char input;
            input = getchar();
            putchar(input);
            //TODO: CHECK IF GAME IS OVER? If over, before blocking 
            // again, quit the thread
        }
    }

    return NULL;
}