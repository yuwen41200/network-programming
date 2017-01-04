CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic

all: server client

server: server.cpp fnbio.cpp
	$(CC) $(CFLAGS) -o server server.cpp fnbio.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	rm server client
