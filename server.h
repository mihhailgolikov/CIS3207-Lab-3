#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h> // mutexes, threads, condition variables, etc. for server functionality.
#include <string.h> // spell checking functionality
#include <sys/types.h>
#include <sys/socket.h> // socket descriptors, etc.
#include <netinet/in.h> // sockaddr_in struct, accept(), etc.
#include <unistd.h>

// Defined constants
#define DEFAULT_PORT 7000 // the default port if one is not specified
#define DEFAULT_DICTIONARY "dictionary.txt" // default dictionary name (provided by project description) if not specified
#define NUM_WORKER_THREADS 3 // the amount of worker threads that are concurrently running at server's initialization time
#define DICTIONARY_SIZE 200425 // size of dictionary file to be read in
#define JOB_BUF_LEN 100 // the amount of jobs to be held (size of) job buffer socket descriptors
#define LOG_BUF_LEN 100 // ........................................ log buffer socket descriptors
#define MAX_WORD_SIZE 15 // the maximum size of the word that is able to be processed (in the dictionary)
#define PHRASE_SIZE 25 // the maximum phrase size, after the OK or WRONG status is included within the phrase
////////////////////////// ex. "phrays WRONG" or "phrase OK"

// taken directly from the slides and the book, creating a listener fd, opens a socket descriptor, and sets the socketopt.
int open_listenfd(int);

// the worker and log thread functions, to conduct the operations of the server
// including spell checking the word and writing the word and status to the log file
// concurrently operating with the other connected clients' requests to the server
void *workerThreadFunc(void *);
void *logThreadFunc(void *);

// finding the word in the given dictionary file
int wordSearch(char dictionary[][MAX_WORD_SIZE], char* wordToFind);

#endif
