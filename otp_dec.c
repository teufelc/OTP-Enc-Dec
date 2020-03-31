/*
    Christopher Teufel
    Assignment 4: otp_dec
    CS-344
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "utility.h"

int main(int argc, char* argv[]){

    // check arguments
    if (argc<4){
        fprintf(stderr,"otp_dec: Not enough args. Use: \'%s ciphertext key port\'\n", argv[0]);
        exit(1);
    }

    // check port arg for non-numbers
    for(int x=0; x<strlen(argv[3]); x++){
        if(argv[3][x] < '0' || argv[3][x] > '9'){
            fprintf(stderr, "otp_dec: Only use numbers for port.\n");
            exit(1);
        }
    }

    // used for checking file lengths
    int cipherLen, keyLen;
    
    // check if the args are different files
    if(strcmp(argv[1], argv[2])==0){
        fprintf(stderr, "otp_dec: You must use two different files for ciphertext and key.\n");
        exit(1);
    }

    // check if ciphertext exists 
    FILE* fhCipher = fopen(argv[1], "r");
    if(fhCipher==NULL){
        fprintf(stderr, "otp_dec: Error opening %s\n", argv[1]);
        exit(1);
    }

    // check if key exists
    FILE* fhKey = fopen(argv[2], "r");
    if(fhKey==NULL){
        fprintf(stderr, "otp_dec: Error opening %s\n", argv[2]);
        exit(1);
    }

    // check ciphertext for bad chars, see utility.c for function
    cipherLen = getNumChars(fhCipher);
    if(cipherLen<0){
        fprintf(stderr, "otp_dec: %s contained bad chars. Use only capital letters or spaces with a newline at the end.\n", argv[1]);
        fclose(fhCipher);
        fclose(fhKey);
        exit(1);
    }

    // check key for bad chars, see utility.c for function
    keyLen = getNumChars(fhKey);
    if(keyLen<0){
        fprintf(stderr, "otp_dec: %s contained bad chars. Use only capital letters or spaces with a newline at the end\n", argv[2]);
        fclose(fhCipher);
        fclose(fhKey);
        exit(1);
    }

    // check to see if the ciphertext is longer than key
    if(cipherLen>keyLen){
        fprintf(stderr, "otp_dec: %s is too short. It must be as long or longer than %s\n", argv[2], argv[1]);
        fclose(fhCipher);
        fclose(fhKey);
        exit(1);
    }

    // close the files to reset the file pointer
    if(fseek(fhCipher, 0, SEEK_SET) != 0){
        fprintf(stderr, "otp_dec: fseek() plaintext error\n");
        exit(1);
    }

    if(fseek(fhKey, 0, SEEK_SET) != 0){
        fprintf(stderr, "otp_dec: fseek() key error\n");
        exit(1);
    }

    /* CITATION: socket setup taken from client.c in notes */

    int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	//char buffer[256];

    // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number

    // use localhost
	serverHostInfo = gethostbyname("localhost");
	if (serverHostInfo == NULL) { 
        fprintf(stderr, "otp_dec: CLIENT: ERROR, no such host\n"); 
        exit(1); 
    }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0){
        fprintf(stderr, "otp_dec: Error opening socket\n");
        exit(1);
    }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		fprintf(stderr, "otp_dec: CLIENT: ERROR connecting");
        exit(1);
    }

    // get confirmation
    char* confimationMessage = "dec";

    // recieve confirmation message, see utility.c
    int confirmation = getConfirmation(socketFD, confimationMessage);
    if(confirmation<0){
        fprintf(stderr, "otp_dec: Could not confirm connection to otp_dec_d on port %d\n", portNumber);
        
        // closing the socket on error will signal to server that the server that
        // the confirmation was no good
        // it wont get the next confirmation message and the socket will be closed
        // on its end as well

        close(socketFD);
        exit(2);
    }

    // need to send confirmation back to client to let them know to continue
    confirmation = sendConfirmation(socketFD, confimationMessage);
    if(confirmation<0){
        fprintf(stderr, "otp_dec: error sending confirmation back\n");
        exit(1);
    }

    // send the plaintext to the daemon, see sendFile in utility.c
    sendFile(socketFD, fhCipher);

    // get confirmation to make sure everything is in sync
    confirmation = getConfirmation(socketFD, confimationMessage);
    if(confirmation<0){
        fprintf(stderr, "otp_dec: Could not confirm connection to otp_dec_d on port %d\n", portNumber);
        // need to properly close socket
        close(socketFD);
        exit(1);
    }

    // send key, same as above
    sendFile(socketFD, fhKey);

    // get confirmation, same as above
    confirmation = getConfirmation(socketFD, confimationMessage);
    if(confirmation<0){
        fprintf(stderr, "otp_dec: Could not confirm connection to otp_dec_d on port %d\n", portNumber);
        // need to properly close socket
        close(socketFD);
        exit(1);
    }

    // close the files
    fclose(fhCipher);
    fclose(fhKey);

    // get and print decrypted file, see utility.c
    recvFileP(socketFD);

    // close the socket
    close(socketFD);

    
    return 0;
}