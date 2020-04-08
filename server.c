#include <stdlib.h>
#include <stdio.h>
#include <pthread.h> // for working with threads, mutexes, and condition variables
#include <string.h> // used for comparing args at start ie. checking for port number vs. dictionary file
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // need for sockaddr_in struct to use with accept()
#include <unistd.h>
#include "server.h"

pthread_t threadPool[NUM_WORKER_THREADS], logThread; // thread pool declaration
pthread_mutex_t job_mutex, log_mutex; // mutex declaration for job and log
pthread_cond_t job_cv_cs, job_cv_pd; // job buffer condition var (consume/produce)
pthread_cond_t log_cv_cs, log_cv_pd; // log buffer condition var (consume/produce)
int BUFF_JOB_ARRAY[JOB_BUF_LEN]; // int array for client connection socket
char BUFF_LOG_ARRAY[LOG_BUF_LEN][PHRASE_SIZE]; // words to be written to log
int lengthJob = JOB_BUF_LEN; // job buffer size
int lengthLog = LOG_BUF_LEN; // log buffer size
int amountOfJobs = 0; // number of items in job buff
int amountofLogs = 0; // number of items in log buff
int JOB_QUEUE_FRONT = -1; // front node of job buff
int JOB_QUEUE_BACK = 0; // rear node
int LOG_QUEUE_FRONT = -1; // front node of log buff
int LOG_QUEUE_BACK = 0; // rear node

int portConnectionNumber = 0; // global declaration portConnectionNumber
char* dictionaryTextFileName = ""; // global declaration dictionaryTextFileName to be used
char dictionaryWordsArray[DICTIONARY_SIZE][MAX_WORD_SIZE]; // operational dictionary to be obtained from dictionary file
int currentWordInDictionary = 0; // keeps track of word count, used when searching for word

struct sockaddr_in client; // struct for client incoming connection socket
int lengthClient = sizeof(client); // size of client's connection
int socketConnection; // existance?
int socketClient; // existance/state

char* CLIENT_MSG = "You're successfully connected to the server, please enter word(s) to check!\n";
char* MSG_REQ = "Please send me more word(s) to check, or press ESC -> ENTER to escape the server.\n";
char* MSG_ESC = "Server is now closed, goodbye!\n";
char ERR_JOB_MUTEX_INIT[] = "***ERROR: Cannot start job mutex.***\n";
char ERR_LOG_MUTEX_INIT[] = "***ERROR: Cannot start log mutex. ***\n";
char ERR_CV_JOB_CONSUME[] = "***ERROR: Cannot initialize condition variable for consume job buffer.***\n";
char ERR_CV_JOB_PRODUCE[] = "***ERROR: Cannot initialize condition variable for produce job buffer.***\n";
char ERR_CV_LOG_CONSUME[] = "***ERROR: Cannot initialize condition variable for consume log buffer.***\n";
char ERR_CV_LOG_PRODUCE[] = "***ERROR: Cannot initialize condition variable for produce log buffer.***\n";
char ERR_NO_PORT[] = "***ERROR: Port cannot be used, please try the default port.***\n";
char ERR_INVALID_ARGS[] = "***ERROR: Cannot run server with the given arguments, try again.***\n";
char ERR_INVALID_SOCKET[] = "***ERROR: Invalid socket, could not establish connection.***\n";
char ERR_JOB_BUFF_FULL[] = "***ERROR: The job buffer is full right now, please wait a second.***\n";
char ERR_LOG_BUFF_FULL[] = "***ERROR: The log buffer is full right now, please wait a second.***\n";

char WORK_THREAD_INIT_SUCCESS[] = "Worker thread successfully created!\n";
char LOG_THREAD_INIT_SUCCESS[] = "Log thread successfully created!\n";
char DICTIONARY_INIT_SUCCESS[] = "Assigned your custom dictionary file successfully!\n";
char USER_ESC[] = "You have escaped the server! If you'd like to continue, please reconnect!\n";

FILE* logFile_ptr; // log file pointer to open log file with

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

		//  inside job mutex, we add the socket client to the job buff
		pthread_mutex_lock(&job_mutex); // we lock the mutex for it too
		while(amountOfJobs == JOB_BUF_LEN) { // while the job buffer is full
			pthread_cond_wait(&job_cv_pd, &job_mutex); // we wait for it to free up to have space
		}

		// socket client goes into job buffer
		if (amountOfJobs == JOB_BUF_LEN) { // if job buffer is full
			write(1,ERR_JOB_BUFF_FULL,strlen(ERR_JOB_BUFF_FULL)); 

		} else {
	  // if the job buffer has no jobs in it, take the first item in the queue and put it at the front (initialize the rear as well)
			if (amountOfJobs == 0) {
				JOB_QUEUE_FRONT = 0;
				JOB_QUEUE_BACK = 0;
			} 
			BUFF_JOB_ARRAY[JOB_QUEUE_BACK] = socketClient; // add our socket client to job buff
			amountOfJobs++; // +1 to socket descriptor count, because we have a successful connection
			JOB_QUEUE_BACK = (JOB_QUEUE_BACK + 1) % JOB_BUF_LEN; // the job queue resets after every 100th job, in order
			// to have a circular process of operations, otherwise the buffer would fill up too fast
		} 

		pthread_mutex_unlock(&job_mutex); // we unlock the job mutex
		pthread_cond_signal(&job_cv_cs); // create a signal that its not empty anymore
		printf("Connection established successfully!\n"); // 
		send(socketClient, CLIENT_MSG, strlen(CLIENT_MSG), 0);  // send out a message to client to prompt them to have a word to 
		// spell check
	}
	return 0;
}


void* workerThreadFunc(void* arg) {
	while (1) { // inf loop
		// take the socket descriptor out of the job buffer to be able to use it
		pthread_mutex_lock(&job_mutex); // lock the job mutex 
    	
		while(amountOfJobs == 0) { // while the buffer is empty
			pthread_cond_wait(&job_cv_cs, &job_mutex); // the worked thread waits for a signal that it IS NOT empty
		}
    	// initalize the socket descriptor to be used with the job buffer
		int socketDesc; 
		// remove it from the job buff
		if (amountOfJobs == 0) { // if the job buffer is empty
			write(1,ERR_JOB_BUFF_FULL,strlen(ERR_JOB_BUFF_FULL));
		} else { // else, remove it from the job buff
			socketDesc = BUFF_JOB_ARRAY[JOB_QUEUE_FRONT]; // store the socket descriptor to process it
			JOB_QUEUE_FRONT = (JOB_QUEUE_FRONT + 1) % JOB_BUF_LEN; // repeated process after 100 tasks
			amountOfJobs--; // socket descriptor removed, -1 to the job count
		}
		pthread_mutex_unlock(&job_mutex); // job mutex is now unlocked 
		pthread_cond_signal(&job_cv_pd); // job buffer is not full anymore because we took something out, signal as so
		// we allocate memory for a single word at a time that we're recieving from the client, by calling calloc
		char* word = calloc(MAX_WORD_SIZE, 1); 
		// we use recv() with our socket descriptor that we took out of our job buffer, which acts as taking the word from the client and storing it for processing
		while(recv(socketDesc, word, MAX_WORD_SIZE, 0)) { 
			if (strlen(word) <= 1) { // if nothing entered, continue and repeat
				continue;
			}
			// exit functionality
			if (word[0] == 27) { // if the user escapes the client (enter ESC), we close the socket descriptors
				printf("Socket descriptor: %d..\n", socketDesc); 
				write(1, USER_ESC, strlen(USER_ESC)); // user escaped server
				write(socketDesc, MSG_ESC, strlen(MSG_ESC)); // let the server know we closed it 
				close(socketDesc); // we close the client socket
				break;
			}
			//// SEARCH FUNCTIONALITY !!! ////////////////////////////////////////////////////
			if (wordSearch(dictionary, word)) { // call our word search (word found)
				strtok(word, "\n"); // every word will automatically have a newline char at the end, so remove it
				word = realloc(word, sizeof(char*)*PHRASE_SIZE);
				 // reallocate the memory to be able to fit the "OK" status at the end of the word and store it within the log
				strcat(word, " OK\n"); // add OK to the end of the word (correct)
			} else { // word not found!
				strtok(word, "\n"); // take out \n
				word = realloc(word, sizeof(char*)*PHRASE_SIZE); // reallocate space to put "WRONG" status at the end and store it within log
				strcat(word, " WRONG\n"); // add WRONG to the end
			}
			write(socketDesc, word, strlen(word)); // write the new phrase with the status to the clinet
			write(socketDesc, MSG_REQ, strlen(MSG_REQ)); // print out MSG_REQ
			// the phrase is now written to the log buff using mutual exclusion
			pthread_mutex_lock(&log_mutex); // we get the log mutex locked
			while(amountofLogs == LOG_BUF_LEN) {// while the log buff is full
				pthread_cond_wait(&log_cv_pd, &log_mutex); // we wait for it to open up
			}
			// add the word plus its status to the log buff
			if (amountofLogs == LOG_BUF_LEN) { // if the log buff is full
				write(1, ERR_LOG_BUFF_FULL, strlen(ERR_LOG_BUFF_FULL)); // say its full
			} else {
		// if the buffer is empty, we load an item into the queue and update the position to front, as well as the rear
				if (amountofLogs == 0) {
					LOG_QUEUE_FRONT = 0;
					LOG_QUEUE_BACK = 0;
				}
				strcpy(BUFF_LOG_ARRAY[LOG_QUEUE_BACK], word); // we add the words and their spell status to the log file
				amountofLogs++; // +1 so we go on to the next word or set of words
				 // we reset the log queue so that the process continues itself and becomes circular (like a path beginning and ending at same place)
				LOG_QUEUE_BACK = (LOG_QUEUE_BACK + 1) % LOG_BUF_LEN;
			}

			pthread_mutex_unlock(&log_mutex); // the log mutex is unlocked
			pthread_cond_signal(&log_cv_cs); // the log buffer is not empty anymore, so we signal it
			free(word); // free the space used by the previous word or words to be used for the next word(s)
			word = calloc(MAX_WORD_SIZE, 1); // and assign the memory for the next word(s)
		}
		close(socketDesc); // descriptor is closed when the client is finished
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
  
	// while i <  currentWordInDictionary - 1 (because start i = 0)
	while(i < dictionaryWordCount - 1) { // iteration through the dictionary
		if (strcmp(dictionary[i], wordToFind) == 0) { // if word found
			return 1; // return word found
		}
		i++; // i++ to check next word
	} 
	return 0; // return word not found
}