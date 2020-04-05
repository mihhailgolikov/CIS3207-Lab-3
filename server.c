#include "server.h"

int main(int argc, char *argv) {
    //char* word = "sdfsdf";

}


int processWrd(char* word) { //compare the spelling of the word within the dictionary
    // check the dictionary file through every word if there is a match for the given word
    for (int i = 0; i < dictionarySize; i++) {
        if (strcmp(word, dictionary[i]) == 0)
            return 1;
    }
    return 0;
}


int isWrdANum(char* num) { // check if the word is a number (no dictionary needed)
    for (int i = 0, len = strlen(num); i < len; i++)
    {
        if (!isdigit(num[i])) // if its a digit, return 0
            return 0;
    }
    return 1; // otherwise return yes
}