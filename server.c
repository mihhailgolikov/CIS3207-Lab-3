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

	// make new work and log threads

	// work thread
	for (int i = 0; i < NUM_WORKER_THREADS; i++) {
		if(pthread_create(&threadPool[i], NULL, workerThreadFunc, NULL) == 0) { // pthread_create() = 0 = worker thread successfully created
			
			write(1,WORK_THREAD_INIT_SUCCESS,strlen(WORK_THREAD_INIT_SUCCESS)); 
			
		}
	}
  // log thread
	if (pthread_create(&logThread, NULL, logThreadFunc, NULL) == 0) { // pthread_create() = 0 = log thread successfully created
		
		write(1,LOG_THREAD_INIT_SUCCESS,strlen(LOG_THREAD_INIT_SUCCESS)); 
		
	}

	// if no port or dictionary name specified, use defaults
	if (argc == 1){
		// write(1,ERR_NO_PORT,strlen(ERR_NO_PORT)); 
		// use DEFAULT_PORT, DEFAULT_DICTIONARY
		portConnectionNumber = DEFAULT_PORT;
		dictionaryTextFileName = DEFAULT_DICTIONARY;
		
		printf("The dictionary txt file used: %s\n", dictionaryTextFileName); 
		
	} else if (argc == 2) { // check if the second argument is a port number or a file name,
		// if port number, the dictionary remains default
		if (strstr(argv[1], ".txt") == NULL){ // second arg is a text file so we have a custom dictionary
			portConnectionNumber = atoi(argv[1]); 
			dictionaryTextFileName = DEFAULT_DICTIONARY;
			
			write(1,DICTIONARY_INIT_SUCCESS,strlen(DICTIONARY_INIT_SUCCESS)); 

		} 
		// if it is a dictionary file name, we set it as the dictionary and leave the default port
		if (strstr(argv[1], ".txt") != NULL) { // is a text file, so dictionary specified
			portConnectionNumber = DEFAULT_PORT;
			dictionaryTextFileName = argv[1];
			
			printf("Assigned custom port number successfully!\n"); // FOR TESTING
			
		}
		printf("You entered 2 arguments! Where port number is %d and dictionary is %s\n", portConnectionNumber, dictionaryTextFileName);
	} else if (argc == 3) {
		if ((strstr(argv[1], ".txt") == NULL) && (strstr(argv[2], ".txt") != NULL)) {
			// 2nd arg is a dictionary, 3rd arg is custom port
			portConnectionNumber = atoi(argv[1]);
			dictionaryTextFileName = argv[2]; 
	
		} else if ((strstr(argv[1], ".txt") != NULL) && (strstr(argv[2], ".txt") == NULL)) {
			// 2nd arg is custom port, 3rd arg is dictionary
			dictionaryTextFileName = argv[1]; // set  accordingly
			portConnectionNumber = atoi(argv[2]);
		} else {
			write(1,ERR_INVALID_ARGS,strlen(ERR_INVALID_ARGS)); 
			return -1;
		}
	} else { 
		write(1,ERR_INVALID_ARGS,strlen(ERR_INVALID_ARGS)); 
		return -1;
	}

	
	FILE* dictionaryTextFileName_ptr = fopen(dictionaryTextFileName, "r"); // open custom dictionary file
	if (dictionaryTextFileName_ptr == NULL) { // if not able to open
		printf("Cannot open dictionary file!\n"); // error
		return -1; // return -1 to exit program
	} 
	else { // successful read, store the words from the dictionary file into the program's dictionary array.
		int i = 0;
	// store each word from the dictionary file to the dictionary array, fgets gets rid of the null char at the end, so no problem
	// we also stop reading the dictionary file when it hits the end
		while((fgets(dictionaryWordsArray[i], sizeof(dictionaryWordsArray[i]), dictionaryTextFileName_ptr) != NULL) && (i < (DICTIONARY_SIZE - 1))) {  
			currentWordInDictionary++;
			i++;
		}
		fclose(dictionaryTextFileName_ptr); // close file pointer
	}

	// the ports below and above these numbers do not exist, so we print out an error.
	if (portConnectionNumber < 1024 || portConnectionNumber > 65535){
		write(1,ERR_NO_PORT,strlen(ERR_NO_PORT)); 
		return -1;
	}
	// get socket descriptor to listen for incoming connection
	socketConnection = open_listenfd(portConnectionNumber);
	if (socketConnection == -1){ // invalid connection
		write(1,ERR_INVALID_SOCKET,strlen(ERR_INVALID_SOCKET)); 
		return -1; // unsucessful
	}

	// inf loop processing incoming connections
	while(1) { 
		
		// the accept function waits until there is a user connection to the server, and writes the info into the sockaddr_in client struct.
		// if we have a successful connection, we make a second socket descriptor for another new possible connection
		// initial one is still listening and waiting for connections, new one is communicating with user for new connection established.
		if ((socketClient = accept(socketConnection, (struct sockaddr*)&client, &lengthClient)) == -1){
			write(1,ERR_INVALID_SOCKET,strlen(ERR_INVALID_SOCKET)); 
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
