/*
    Christopher Teufel
    Assignment 4: Utility header files
    CS-344
    Started to repeat functions so I decided put all utility functions into
    header file and leave the main programs pretty simple.
    This includes any functions that are common to both programs.
    Function definitions are in utility.c.
*/

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// all info will be sent/recvd in chunks
#define CHUNK_SIZE 512

// enc/dec single char
char encryptChar(char messageChar, char keyChar);
char decryptChar(char cipherChar, char keyChar);

// for checking number of chars in files
int getNumChars(FILE* fh);

// for sending/recving the file for writing
void sendFile(int sock, FILE* fh);
int recvFileW(int sock, FILE* fh);

// send/recv info for printing
void recvFileP(int sock);
int sendEnc(int sock, FILE* fhPlain, FILE* fhKey);
int sendDec(int sock, FILE* fhCipher, FILE* fhKey);

// confirmatioin messages to ensure proper client/server
// connection and that everything is in sync
int sendConfirmation(int sock, char* message);
int getConfirmation(int sock, char* message);

#endif