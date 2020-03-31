# OTP Encryption and Decryption
Christopher Teufel
teufelc@oregonstate.edu


DESCRIPTION:
compileall will compile all files as specified in the makefile
keygen
receive the directory or files
makefile is only used for ftserver.c

INSTRUCTIONS:

'python ftclient.py [server name] [server port] [command] [data port]' runs the program.
To get the directory listing, enter:
    'python ftclient.py [server name] [server port] -l [data port]'
This should be able to list up to ~100 files.
To get a file, enter:
    'python ftclient.py [server name] [server port] -g [filename] [data port]'
Be sure to include an extension for the filename.
The server will validate that the file exists on the server.
If it exists in the client directory, it will be written as filename_new.txt
