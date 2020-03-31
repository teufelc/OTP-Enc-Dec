CC = gcc
CFLAGS = -g -Wall -std=gnu99

all : keygen otp_enc_d otp_dec_d otp_enc otp_dec

keygen : keygen.c
	$(CC) $(CFLAGS) keygen.c -o keygen

otp_enc_d : otp_enc_d.o utility.o
	$(CC) $(CFLAGS) -o $@ $^

otp_enc : otp_enc.o utility.o
	$(CC) $(CFLAGS) -o $@ $^

otp_dec_d : otp_dec_d.o utility.o
	$(CC) $(CFLAGS) -o $@ $^

otp_dec : otp_dec.o utility.o
	$(CC) $(CFLAGS) -o $@ $^

otp_enc_d.o : otp_enc_d.c utility.h

otp_enc.o : otp_enc.c utility.h

otp_dec_d.o : otp_dec_d.c utility.h

otp_dec.o : otp_dec.c utility.h

utility.o : utility.c utility.h

clean :
	-rm *.o
	-rm otp_enc_d
	-rm otp_dec_d
	-rm otp_enc
	-rm otp_dec
	-rm keygen
	-rm mytestresults
