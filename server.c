#include "server.h"
pthread_t threadPool[NUM_WORKER_THREADS], logThread; // thread pool declaration
pthread_mutex_t job_mutex, log_mutex; // mutex declaration for job and log
pthread_cond_t job_cv_cs, job_cv_pd; // job buffer condition var (consume/produce)
pthread_cond_t log_cv_cs, log_cv_pd; // log buffer condition var (consume/produce)
char ERR_JOB_MUTEX_INIT[] = "***ERROR: Cannot start job mutex.***\n";
char ERR_LOG_MUTEX_INIT[] = "***ERROR: Cannot start log mutex. ***\n";
char ERR_CV_JOB_CONSUME[] = "***ERROR: Cannot initialize condition variable for consume job buffer.***\n";
char ERR_CV_JOB_PRODUCE[] = "***ERROR: Cannot initialize condition variable for produce job buffer.***\n";
char ERR_CV_LOG_CONSUME[] = "***ERROR: Cannot initialize condition variable for consume log buffer.***\n";
char ERR_CV_LOG_PRODUCE[] = "***ERROR: Cannot initialize condition variable for produce log buffer.***\n";

int main(int argc, char** argv) {
	if(pthread_mutex_init(&job_mutex, NULL) != 0) { 
		write(1,ERR_JOB_MUTEX_INIT,strlen(ERR_JOB_MUTEX_INIT)); // check that the job mutex was initialized
		return -1;
	}
	if(pthread_mutex_init(&log_mutex, NULL) != 0) {
		write(1,ERR_LOG_MUTEX_INIT,strlen(ERR_LOG_MUTEX_INIT)); // check that the log mutex was initialized
		return -1;
	}
// initialize condition vars
	if (pthread_cond_init(&job_cv_cs, NULL) != 0) { 
		write(1,ERR_CV_JOB_CONSUME,strlen(ERR_CV_JOB_CONSUME));  // Condition var for job consume buffer initialization
		return -1;
	}
	if (pthread_cond_init(&job_cv_pd, NULL) != 0) { 
		write(1,ERR_CV_JOB_PRODUCE,strlen(ERR_CV_JOB_PRODUCE));   // Condition var for job produce buffer initialization
		return -1;
	}
	if (pthread_cond_init(&log_cv_cs, NULL) != 0) {
		write(1,ERR_CV_LOG_CONSUME,strlen(ERR_CV_LOG_CONSUME));  // Condition var for log consume buffer initialization
		return -1;
	}

	if (pthread_cond_init(&log_cv_pd, NULL) != 0) { 
		write(1,ERR_CV_LOG_PRODUCE,strlen(ERR_CV_LOG_PRODUCE));  // Condition var for log produce buffer initialization
		return -1;
	}

}

void* logThreadFunc(void* arg) {
	while(1) {

	  // take the given phrase out of the log buffer (by a log mutex lock), 
	  // and while log buffer is empty, wait to consume from log buffer
		pthread_mutex_lock(&log_mutex); 
		while(amountofLogs == 0) { 
			pthread_cond_wait(&log_cv_cs, &log_mutex); 
		}

	
	  // remove the words from the log buff and write it to the log file, and if the log buff is empty we print out a message
		char* wordsToLog;
		if (amountofLogs == 0) {
			printf("log buffer is empty! Can't remove anything!\n"); 
		} else { 
   
	  // else, we take the word from the log buffer, and store it be be removed from the log buffer
	  // then reset the log's front queue when it reaches the 100th item.
			wordsToLog = BUFF_LOG_ARRAY[LOG_QUEUE_FRONT]; 
			LOG_QUEUE_FRONT = (LOG_QUEUE_FRONT + 1) % LOG_BUF_LEN; 
			amountofLogs--; // -1 the count of phrases in the log buff
		}
		
		printf("Phrase taken out of log buffer to append to log file: %s", wordsToLog); // FOR TESTING
		
    // open and access the log file!
		logFile_ptr = fopen("logFile.txt", "a"); 
		if (logFile_ptr == NULL) { // if the file doesnt exist...
			printf("Cannot open log file!\n"); // error
			exit(1);
		}

	// we put the words taken out of the log buffer to the log file,
	// where fputs returns positive int if the operation is successful
	// and if not, it returns EOF
		if (fputs(wordsToLog, logFile_ptr) >= 0) { 
			printf("Successfully added to log file!\n"); // success!
		} else { 
      // otherwise there was an error adding it ot the log file
			printf("Cannot put word(s) from log buffer to log file.\n"); // print error message
		}
		fclose(logFile_ptr); // close the file pointer
		pthread_mutex_unlock(&log_mutex); // unlock the log mutex
		pthread_cond_signal(&log_cv_pd); // we signal the log buffer as not full anymore (reseting)
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
