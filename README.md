# OTP Encryption and Decryption Using Socket Programming
Christopher Teufel

teufelc@oregonstate.edu

NOTES: The project was written and tested on a linux server with GCC, should only be run on linux.
Only capital letters should be used in the plaintext files.
The plaintext files should only contain a newline charater at the end.


FILE DESCRIPTIONS:
compileall will compile all files as specified in the makefile

keygen.c is used to create the key for encryption and decryption

opt_enc_d.c runs in the background on a specified port
otp_enc.c sends a plaintext file as well as a key to otp_enc_d
otp_enc_d encrypts the file and sends the cipher to otp_enc
otp_enc recieves the cipher and prints to stdout

opt_dec_d.c runs in the background on a specified port
otp_dec.c sends a cipher file as well as a key to otp_dec_d
otp_dec_d decrypts the file and sends the plaintext file to otp_dec
otp_dec recieves the plaintext and prints to stdout

utility.c and utility.h are used for utility functions


INSTRUCTIONS:
'compileall' from the command line will compile all files as specified in the makefile

'keygen keylength > mykey' will create a plaintext key stored in mykey that is keylength characters in length

'otp_enc_d port1 &' enables the encryption daemon to listen on port1 in the background

'otp_dec_d port2 &' enables the decryption daemon to listen on port2 in the background

'otp_enc ptext mykey port1 > ctext' encrypts a plaintext file 'ptext' using mykey by sending the file to the daemon
listening on port 1 and prints the resulting cipher to 'ctext'.

'otp_dec ctext mykey port2 > dtext' decrypts a cipher file 'ctext' using mykey by sending the file to the daemon
listening on port 2 and prints the resulting plaintext to 'dtext'.
