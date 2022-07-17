# Makefile for myftp client side

myftp: myftp.c command.o stream.o token.o
	gcc -Wall -o myftp myftp.c command.o stream.o token.o

command.o: command.c command.h  token.o ../netprotocol.h
	gcc -Wall -c command.c -o command.o

stream.o: ../stream.c ../stream.h ../netprotocol.h
	gcc -Wall -c ../stream.c -o stream.o

token.o: ../token.c ../token.h
	gcc -Wall -c ../token.c -o token.o

clean:
	rm *.o myftp
