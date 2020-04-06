#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h> // mutexes, threads, and condition variables
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>

//Search words from the dictionary
int wordSearch(char list_of_words[][MAX_WORD_SIZE], char* wordToFind);

#endif
