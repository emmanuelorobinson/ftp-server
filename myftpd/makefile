# Makefile for server

myftpd: myftpd.c commandd.o stream.o helper.o
	gcc -Wall -o myftpd myftpd.c commandd.o stream.o helper.o
	
commandd.o: commandd.c commandd.h ../netprotocol.h
	gcc -Wall -c commandd.c -o commandd.o

stream.o: ../stream.c ../stream.h ../netprotocol.h
	gcc -Wall -c ../stream.c -o stream.o

helper.o: ../helper.c ../helper.h ../netprotocol.h
	gcc -Wall -c ../helper.c -o helper.o

clean:
	rm *.o myftpd
