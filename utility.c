/*
    Christopher Teufel
    Assignment 4: Utility functions definitions
    CS-344
    Definitions for utility functions in utility.h.
    This includes any functions that are common to both programs.
    Function declarations are in utility.h.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "utility.h"

/*
    Function that takes a character of a message and of a 
    key and gives the encrypted char as specified in the hw. 
    Originally an alphabet char array was used but not needed.
    Alphabet can be thought of as a modified ascii table. The encrypted
    char is the char corresponding to the addition of the key and message char, 
    wrapped around.
    Spaces are in a different location in the actual ASCII table so they need
    to be treated differently.
    Without the space char we would be able to do simple char addition/subtraction.
*/
char encryptChar(char messageChar, char keyChar){

    // numbers would be the indicies of 'ABCDE...XYZ ' from 0 to 26
    int messageNum, keyNum, encryptNum;

    // direct conversion, at a different place in ASCII table
    if (messageChar==' '){
        messageNum = 26;
    } else{
        // find ascii number relative to 'A'
        messageNum = messageChar - 'A';
    }

    // same as above, but for the key character
    if(keyChar==' '){
        keyNum = 26;
    } else{
        keyNum = keyChar - 'A';
    }

    // get the addition of the numbers
    // Same as wrapping around the table and
    // starting at the beginning if over 26
    encryptNum = (messageNum + keyNum) % 27;

    // alphabet arg not needed.
    if(encryptNum==26){
        return ' ';
    } else{
        return 'A' + encryptNum;
    }
}

/*
    Function that takes a character of a cipher and a key and gives the 
    decrypted char as specified in the hw. 
    Originally an alphabet char array was used but not needed.
    Alphabet can be thought of as a modified ascii table. The encrypted
    char is the char corresponding to the addition of the key and message char, 
    wrapped around.
*/
char decryptChar(char cipherChar, char keyChar){

    // numbers would be the indicies of ABCDE..
    int cipherNum, keyNum, decryptNum;

    // same as encryptchar above
    if(cipherChar==' '){
        cipherNum = 26;
    } else{
        cipherNum = cipherChar - 'A';
    }

    // again, same as above
    if(keyChar==' '){
        keyNum = 26;
    } else{
        keyNum = keyChar - 'A';
    }

    // get the number, cant use mod with negative in c
    decryptNum = (cipherNum-keyNum);
    if(decryptNum < 0){ decryptNum += 27;}

    if(decryptNum==26){
        return ' ';
    } else{
        return 'A' + decryptNum;
    }
}

/*
    Utility function that gets the number of chars in a file and checks
    for bad chars. Good chars are only capital letters and space char.
    Used for checking that key is at least as long as the plaintext
    or cipher files before sending them to the daemons.
    Also checks if the file has ANY bad chars in them, including the key,
    according to the HW assignment.
    Takes a FILE* pointer as an arg and returns the number of chars
    as an int or -1 if there is a bad char.
    File must be opened for reading beforehand.
    It doesn't include the newline as part of the char count but makes sure
    that the newline is at the end of each file. Since it will only be used 
    for comparison, it does not make a differnce.
*/
int getNumChars(FILE* fh){

    int count = 0;
    char ch;

    // loops get total number of chars and checks for bad chars
    do{

        // get the next char in file
        ch = getc(fh);

        // file can only have newline at the end, must check for EOF next
        if(ch=='\n'){
            ch = getc(fh);

            // if next char is newline, then return the count properly
            if(ch==EOF){
                return count;
            } 
            else{ // else there is a newline not at the end, return -1
                return -1;
            }
        }


        // file needs to be only chars and spaces
        if((ch<'A' || ch>'Z') && ch!=' '){
            return -1;
        }

        // increment the count if all checks are good
        count++;


    } while(ch!=EOF); // it wasn't working as a regular while loop

    // just to be sure
    return count;
}

/*
    Function to send a file of an indeterminite length.
    Takes the final connection socket and filename as
    args. Sends the file in chunks until the end of the 
    file is reached. 
    Socket must be open and ready for use.
    Taken from assignment for networking class.
    Used by clients to send files to daemons for writing
    to temp files. Not used to send enc/dec data, I made
    seperate functions so there would be less temp files.
    Since its only used by clients, it can exit(1).
    Using a loop means you don't need to do extra work to 
    send the lengths of the files back and forth.
*/
void sendFile(int sock, FILE* fh){

    // check that the file is open
    if(fh==NULL){
        fprintf(stderr, "Error calling sendFile, the file needs to be opened.\n");
        exit(1);
    }

    // read buffer bigger than chunk size to be sure
    char readBuffer[CHUNK_SIZE+1];

    // bytes read and sent
    int bR, bS;

    // flag to end the loop
    int endFileFlag = 1;

    // read and send file in chunks with a loop until end of file is reached
    while(endFileFlag){

        // clear out read buffer and read CHUNK_SIZE into the buffer
        // needs to be '\0', was getting garbage with just 0
        memset(readBuffer, '\0', sizeof(readBuffer));
        bR = fread(readBuffer, 1, CHUNK_SIZE, fh);
        if(bR<0){
            fprintf(stderr, "Error reading file in sending\n");
            break;
        }
        
        // looks for end of file, will be last chunk
        // newline will still be included in sent file so 
        //  theres no worry about adding/removing it
        if(strstr(readBuffer, "\n")!=NULL){
            endFileFlag = 0;
        }

        // send chunks regardless of end of file or not
        // receiver will be reading in chunks of same size
        // unitl theres a newline
        bS = send(sock, readBuffer, CHUNK_SIZE, 0);
        if(bS < 0){
            fprintf(stderr, "Error sending data");
            exit(1);
        }

        // printf("bytes read from file: %d\n", bR);
        // printf("bytes sent: %d\n", bS);

    }

}

/*
    Function to receive a file of an indeterminite length.
    Takes the final connection socket and filename as
    args. Receives chunks of data on an opened socket
    and writes to file given by filename.
    Receives full chunk to clear socket but doesn't write
    full chunk in case of last chunk having null bytes.
    Receives until sender sends 0 bytes.
    Socket must be open and ready for use.
    Taken from assignment for networking class.
    Looks pretty much the same as sendFile.
    Used by daemons to get the plain/cipher and
    keys. Will return -1 on error and 1 on 
    success, so daemon can approprately handle
    error.
    Not used for getting the enc/dec data.
*/
int recvFileW(int sock, FILE* fh){

    // check that the files open
    if(fh==NULL){
        fprintf(stderr, "Error calling recvFileW, file needs to be opened.\n");
        return -1;
    }

    // buffer and bytes recvd, bytes written
    char buffer[CHUNK_SIZE+1];
    int bR, bW;

    // to find end of file
    int endFileFlag=1;

    while(endFileFlag){
        memset(buffer, '\0', sizeof(buffer));

        // receive data on socket
        bR = recv(sock, buffer, CHUNK_SIZE, 0);
        if(bR<0){ // error recieving
            fprintf(stderr, "Error receiving data\n");
            return -1;
        } // if no more bytes the connection was broken
        else if(bR==0){break;}
            
        // end recving when theres a newline present
        // will still read newline into file
        if(strstr(buffer, "\n")!=NULL){
            endFileFlag = 0;
        }

        // write to temp file
        // use strlen to avoid writing null bytes
        bW = fwrite(buffer, 1, strlen(buffer), fh);
        if(bW<0){
            fprintf(stderr, "Daemon error writing to file\n");
            return -1;
        }        
    }
    return 1;
}

/*
    Function used by daemon to encrpyt data and send
    to the client. Reads from the two temp files 
    char-by-char and encrypts until the end is found.
    Reads the encrpyted data into buffer of CHUNK_SIZE
    and sends the chunk to the client.
    Returns 1 on success and -1 on error so daemon can 
    handle error.
    Also sends the newline so theres no need to worry about
    taking off and adding back on the newline.
*/
int sendEnc(int sock, FILE* fhPlain, FILE* fhKey){

    // check for null files
    if(fhKey==NULL || fhPlain==NULL){
        fprintf(stderr, "Error calling sendEnc, files needs to be opened.\n");
        return -1;
    }

    // char variables
    char p, k;
    int endFlag=1; //flag for loop
    int i, bS; // bS is bytes sent
    char buffer[CHUNK_SIZE+1]; // buffer bigger than chunk

    // loop and send chunks until end of file
    while(endFlag){

        // reset buffer
        memset(buffer, '\0', sizeof(buffer));

        // read in CHUNK_SIZE chars to buffer with for loop
        for(i=0; i<CHUNK_SIZE; i++){

            // get chars individually, one at a time
            p = getc(fhPlain);
            k = getc(fhKey);

            // add newline to buffer, last chunk to be sent
            if(p=='\n'){
                buffer[i] = p;
                endFlag = 0;
                break; // breaks out of for loop, not while
            }
            else{
                // else add encrypted char to buffer normally
                // see utility.c for function
                buffer[i] = encryptChar(p, k);
            }
        }

        // send chunk, including newline and null bytes
        bS = send(sock, buffer, CHUNK_SIZE, 0);
        if(bS < 0){
            fprintf(stderr, "Error sending data");
            return -1;
        }
    }
    // success
    return 1;
}

/*
    Decrypts ands sends in same fashion as above.  
    See sendEnc.   
*/
int sendDec(int sock, FILE* fhCipher, FILE* fhKey){

    // chuck for null files
    if(fhKey==NULL || fhCipher==NULL){
        fprintf(stderr, "Error calling sendDec, files needs to be opened.\n");
        return -1;
    }

    // variables and flags
    char c, k;
    int endFlag=1;
    int i, bS;
    char buffer[CHUNK_SIZE+1];

    // read chunks into buffer in loop
    while(endFlag){

        // reset buffer
        memset(buffer, '\0', sizeof(buffer));

        // read CHUNK_SIZE chars into buffer
        for(i=0; i<CHUNK_SIZE; i++){

            // get chars 
            c = getc(fhCipher);
            k = getc(fhKey);

            // add newline to buffer, last chunk
            if(c=='\n'){
                buffer[i] = c;
                endFlag = 0;
                break; // breaks out of for loop, not while
            }
            else{
                // else add decrypted char to buffer
                buffer[i] = decryptChar(c, k);
            }
        }

        // send chunk including newline and null bytes in last
        bS = send(sock, buffer, CHUNK_SIZE, 0);
        if(bS < 0){
            fprintf(stderr, "Error sending data");
            return -1;
        }
    }
    //success
    return 1;
    
}

// Used for printing out to the recieved file to stdout
// Used by client
/*
    Function to receive encrypted or decrypted data and print to 
    stdout. All it takes it the comm socket.
    Since it is used by the client, it does not need any special
    error handling.
    Simply recvs chunks in loop and prints chunks to stdout.
    Similar structure to above but without complexity of recving
    and writing to a temp file.
*/
void recvFileP(int sock){

    // buffer and bytes recvd
    char buffer[CHUNK_SIZE+1];
    int bR;

    // to find end of file
    int endFlag=1;

    // loops until end is found
    while(endFlag){
        // reset buffer each time
        memset(buffer, '\0', sizeof(buffer));

        // receive data on socketof chunk_size
        bR = recv(sock, buffer, CHUNK_SIZE, 0);
        if(bR<0){ // error recieving
            fprintf(stderr, "Error receiving data\n");
            exit(1);
        } // should not happen but this means the socket has closed
        else if(bR==0){break;}
            
        // newline signals that its the last chunk
        if(strstr(buffer, "\n")!=NULL){
            endFlag = 0;
        }

        // simple print to stdout, can be redirected.
        // Null bytes will not be printed but the newline
        //   that was included will be.
        fprintf(stdout, "%s", buffer);      
    }

}

/*  Utility function to make sure that the client and
    server are communicating with each other properly 
    and everything is in sync.
    Sends the confirmation message the the client/server
    is expecting to ensure that it is the correct partner
    for communication.
    If there is trouble in sending, for example the comm
    socket it close, then it returns -1.
    On a successfil send, returns 1.
    Takes the socket and message as args.
    Only simple 3 letter messages will be sent each time 
    to simplify the size sending.
    To be paired on other end with getConfirmation.
*/
int sendConfirmation(int sock, char* message){

    // bytes sent
    int bS;
    // message copied into buffer so be safe
    char buffer[sizeof(message)];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, message);

    // only simple 3 letter messages to be send
    bS = send(sock, buffer, sizeof(buffer), 0);
    if(bS < 0){

        // closed socket would produce error
        return -1;
    }

    // success
    return 1;

}

/*
    Utility function to receive confirmation message
    and ensure that the message is the expected one.
    Gets the simple message and compares it to one 
    preovided by caller, returning -1 for wrong
    message and 1 for correct one.
    Also returns -1 on errors or closed socket.
    Again, only simple 3 letter messages are used to simplify
    size of recv.
    To be paired with sendConfirmation.
    Takes the socket and expected message as args.
    
    NOTE: does not differentiate between the wrong message
    being sent, a closed socket, or a recv error.
*/
int getConfirmation(int sock, char* message){

    // variables and buffers
    int bR, cmp;
    char buffer[sizeof(message)];
    memset(buffer, 0, sizeof(buffer));

    // should be the same size as message
    bR = recv(sock, buffer, sizeof(buffer), 0);
    if(bR<0){
        return -1; // return -1 on error or closed socket
    } else if(bR == 0){
        return -1;
    } else{
        // check to see the messages are the same
        cmp = strcmp(buffer, message);
        if(cmp==0){  
            return 1;  // returns 1 for same message
        } else{
            return -1; // -1 for wrong message
        }
    }

}