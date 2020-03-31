/*
    Christopher Teufel
    Assignment 4: keygen
    CS-344
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <time.h>

#define NUM_CHARS 27


/*
    Whole program will be encompassed in main.
    Outputs a random key specified in the argument by the user.
    If no argument is given, an error message is shown.
*/
int main(int argc, char* argv[]){
    // always start with seeding rand()
    srand(time(NULL));

    // possible chars for the key
    // will generate an index of chars[] and print the char
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int ind, i;

    // check number of args
    if(argc<2){
        fprintf(stderr, "Incorrect number of args. Use \'keygen [length]\'\n");
        exit(1);
    }

    // check second arg for non-numbers
    // check each char in arg with for loop
    for(i=0; i<strlen(argv[1]); i++){

        // can use ascii values
        if(argv[1][i] < '0' || argv[1][i] > '9'){
            fprintf(stderr, "Only use numbers for length.\n");
            exit(1);
        }
    }

    // convert arg to integer
    int len = atoi(argv[1]);


    // simply print out randomly chosen char from the string
    // can be redirected from command line
    for(i=0; i<len; i++){
        ind = rand()%NUM_CHARS;
        fprintf(stdout, "%c", chars[ind]);
    }
    // include the newline
    fprintf(stdout, "\n");

    return 0;
}
