/*
    Christopher Teufel
    Assignment 4: otp_dec_d
    CS-344
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h> 
#include <time.h>
#include "utility.h"



int main(int argc, char* argv[]){

    // first perform error checking on args
    if(argc<2){
        fprintf(stderr,"otp_dec_d: Not enough args. Use: \'%s port\'\n", argv[0]);
        exit(1);
    }

    // check second arg for non-numbers
    for(int x=0; x<strlen(argv[1]); x++){
        if(argv[1][x] < '0' || argv[1][x] > '9'){
            fprintf(stderr, "otp_dec_d: Only use numbers for port.\n");
            exit(1);
        }
    }

    /* CITATION: socket setup taken from server.c in notes */

    int listenSocketFD, establishedConnectionFD, portNumber;
	socklen_t sizeOfClientInfo;
	//char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

    // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
    // NOTE: all errors are printed to stderr
    // NOTE: program will exit if errors in setting up listening socket
    // NOTE: errors during child proc will exit without affecting parent

    // Create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (listenSocketFD < 0){ // need to print errors to stderr
        fprintf(stderr, "otp_dec_d: Error opening socket\n");
        exit(1);
    }

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
		fprintf(stderr, "otp_dec_d: Error on binding\n");
        exit(1);
    }

    // turn socket on for listening
	if(listen(listenSocketFD, 5) < 0){ 
        fprintf(stderr, "otp_dec_d: Listening error\n");
        exit(1);
    }

    // used for creating temp files
    // was used in earlier confirmation messages
    char* confirmation = "dec";
    char* cipherConfirm = "cipher";
    char* keyConfirm = "key";

    // need this for utility functions
    int confirmStatus;

    // track number of chld procs
    // doesnt have to be global, bulk of program is done in main including fork stuff
    int numProcs = 0;

    // Accept connections in while loop to enable continuous operation and accept calls
    // should only be a max of 5, if higher something went wrong
    while(numProcs < 6){ 

        // pids for proc tracking
        pid_t spawnPID, finishedPID;
        // need extra int for waitpid calls
        int status = 0;

        // check for finished processes at beginning of loop (also at end)
        // will decrease the number of procs each time a child ends
        while((finishedPID = waitpid(-1, &status, WNOHANG))>0){
            numProcs--;
        }

        // again, more fork bomb checking, should not be over 5
        if(numProcs>5){
            printf("FORK BOMB!!! GET HELP!!!\n");
            exit(1);
            break;
            // plshelp
        }else if(numProcs==5){
            // can only have 5 procs, continue to beginning of loop
            continue;
        } 

        /* also from server.c */
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0){
            fprintf(stderr, "otp_dec_d: Accept() error\n");
            continue; // before fork but afer accept() loop, need to start loop over again
        }

        // fork and set up for recieving
        spawnPID = fork();
        switch(spawnPID){
            case -1: // error
                fprintf(stderr, "Fork error. Uh-oh...\n");
                exit(1);
                break;

            case 0: // child process

                // NOTE: all errors in child procs can exit without killing parent

                // hopefully stops the while loop and conditionals
                numProcs=10000;

                // send confirmation message immediatly, client will be expecting it
                confirmStatus = sendConfirmation(establishedConnectionFD, confirmation);
                if(confirmStatus<0){
                    fprintf(stderr, "otp_dec_d: error sending confirmation\n");
                    close(establishedConnectionFD);
                    exit(1); // error sending can exit 1
                }

                // get if we dont get one back, getConfirm returns negative and 
                // we've connected to the wrong client
                int confirmationStat = getConfirmation(establishedConnectionFD, confirmation);
                if(confirmationStat<0){
                    fprintf(stderr, "otp_dec_d: wrong client connected on on port %d\n", portNumber);
                    close(establishedConnectionFD);
                    exit(2); // exit 2 for wrong client
                }

                // unique temp files for each process to hold key and buffer.
                // getpid will give unique value for each proc so no messing with
                //  shared files.
                char cipherFile[20];
                char keyFile[20];
                memset(cipherFile, 0, sizeof(cipherFile));
                memset(keyFile, 0, sizeof(keyFile));
                sprintf(cipherFile, "%s%d.t", cipherConfirm, getpid());
                sprintf(keyFile, "%s%d.t", keyConfirm, getpid());

                // open files for writing, similar error message
                FILE* fhCipher = fopen(cipherFile, "a");
                if(fhCipher==NULL){
                    fprintf(stderr, "otp_dec_d: could not open temp cipher\n");
                    close(establishedConnectionFD);
                    exit(1);
                }
                FILE* fhKey = fopen(keyFile, "a");
                if(fhKey==NULL){
                    fprintf(stderr, "otp_dec_d: could not open temp key\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    exit(1);
                }

                // get ciphertext from otp_dec, send confirmaiton
                // see utility.c for recvFileW, writes to temp file
                int recStat = recvFileW(establishedConnectionFD, fhCipher);
                if(recStat<1){
                    fprintf(stderr, "otp_dec_d: error recieving ciphertext\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    fclose(fhKey);
                    exit(1);
                }

                // send confirmation message to ensure everythings in sync
                confirmStatus = sendConfirmation(establishedConnectionFD, confirmation);
                if(confirmStatus<0){
                    fprintf(stderr, "otp_dec_d: error sending confirmation\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    fclose(fhKey);
                    exit(1);
                }

                // same thing for key
                recStat = recvFileW(establishedConnectionFD, fhKey);
                if(recStat<1){
                    fprintf(stderr, "otp_dec_d: error recieving plaintext\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    fclose(fhKey);
                    exit(1);
                }

                // another confirmation
                confirmStatus = sendConfirmation(establishedConnectionFD, confirmation);
                if(confirmStatus<0){
                    fprintf(stderr, "otp_dec_d: error sending confirmation\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    fclose(fhKey);
                    exit(1);
                }

                // close the files
                fclose(fhCipher);
                fclose(fhKey);

                // re-open files for reading, check for errors
                fhCipher = fopen(cipherFile, "r");
                if(fhCipher==NULL){
                    fprintf(stderr, "otp_dec_d: could not open temp plaintext\n");
                    close(establishedConnectionFD);
                    exit(1);
                }
                fhKey = fopen(keyFile, "r");
                if(fhKey==NULL){
                    fprintf(stderr, "otp_dec_d: could not open temp key\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    exit(1);
                }

                // decrpyt and send to client
                // see sendDec in utility.c
                int sendDecStat = sendDec(establishedConnectionFD, fhCipher, fhKey);
                if(sendDecStat<0){
                    fprintf(stderr, "otp_dec_d: error sending confirmation\n");
                    close(establishedConnectionFD);
                    fclose(fhCipher);
                    fclose(fhKey);
                    exit(1);
                }

                fclose(fhCipher);
                fclose(fhKey);

                // remove temp files and check for errors
                if(remove(cipherFile)!=0){
                    fprintf(stderr, "otp_dec_d: Error removing temp ciphertext\n");
                    exit(1);
                }
                if(remove(keyFile)!=0){
                    fprintf(stderr, "otp_dec_d: Error removing temp key\n");
                    exit(1);
                }
                
                // communication socket closed on client end
                // should end proc here
                exit(0);
                break;

            // parent process
            default:
                numProcs++;
                // connection socket not needed by parent
                close(establishedConnectionFD);          
        }
        
        // check again for finished procs
        while((finishedPID = waitpid(-1, &status, WNOHANG))>0){
            numProcs--;
        }
    }

    // close the listening socket
    close(listenSocketFD);

    return 0;
}