#include "server.h"

int main(int argc, char *argv) {
    //char* word = "sdfsdf";

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
