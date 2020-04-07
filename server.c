#include "server.h"
char ERR_JOB_MUTEX_INIT[] = "***ERROR: Cannot start job mutex.***";
char ERR_LOG_MUTEX_INIT[] = "***ERROR: Cannot start log mutex. ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";
char SHELL_GREET[] = "***ERROR: ***";

int main(int argc, char *argv) {
    //char* word = "sdfsdf";
	if(pthread_mutex_init(&job_mutex, NULL) != 0) { 
		write(1,ERR_JOB_MUTEX_INIT,strlen(ERR_JOB_MUTEX_INIT)); // check that the job mutex was initialized
		return -1;
	}
	if(pthread_mutex_init(&log_mutex, NULL) != 0) {
		write(1,ERR_LOG_MUTEX_INIT,strlen(ERR_LOG_MUTEX_INIT)); // check that the log mutex was initialized
		return -1;
	}

}

int wordSearch(char dictionary[][MAX_WORD_SIZE], char* wordToFind) {
	int i = 0; // i is the index to keep track of words within the dictionary as we go through it,
  
	// while i <  wordsInDictionary - 1 (because start i = 0)
	while(i < dictionaryWordCount - 1) { // iteration through the dictionary
		if (strcmp(dictionary[i], wordToFind) == 0) { // if word found
			return 1; // return word found
		}
		i++; // i++ to check next word
	} 
	return 0; // return word not found
}
