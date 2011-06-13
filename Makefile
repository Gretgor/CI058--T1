CPP=g++
CFLAGS=-g -Wall -O0
PROG_NAME=main

ALL: $(PROG_NAME)

$(PROG_NAME): main.o rawsocket.o transmissor.o resto.o
	$(CPP) $(CFLAGS) -o $(PROG_NAME) main.o rawsocket.o transmissor.o resto.o

main.o: main.cpp resto.h rawsocket.h transmissor.h
	$(CPP) $(CFLAGS) -c -o main.o main.cpp

rawsocket.o: rawsocket.cpp rawsocket.h resto.h
	$(CPP) $(CFLAGS) -c -o rawsocket.o rawsocket.cpp

transmissor.o: transmissor.cpp transmissor.h resto.h rawsocket.h
	$(CPP) $(CFLAGS) -c -o transmissor.o transmissor.cpp

resto.o: resto.h
	$(CPP) $(CFLAGS) -c -o resto.o resto.cpp

limpa:
	@rm -rf *.o
	@rm -rf $(PROG_NAME)