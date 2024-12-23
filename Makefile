include ../Make.defines
CC = gcc
CXX = g++
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lpthread

all: client

servtest: servtest.c
	${CC} ${CFLAGS} -o $@ servtest.c ${LIBS}

client: client.o ui.o
	$(CXX) client.o ui.o -o client $(LIBS) $(LDFLAGS)
	
client.o: client.c ui.h
	$(CC) $(CFLAGS) -c client.c -o client.o $(LIBS)

ui.o: ui.cpp ui.h
	$(CXX) $(CFLAGS) -c ui.cpp -o ui.o $(LDFLAGS)

serv: serv.o
	${CC} ${CFLAGS} -o $@ serv.o ${LIBS}

clean:
	rm -f *.o client
