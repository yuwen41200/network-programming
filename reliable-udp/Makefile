CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -Wpedantic

all: sender1 receiver1 sender2 receiver2 sender3 receiver3

sender1: sender1.cpp
	$(CC) $(CFLAGS) -o sender1 sender1.cpp

receiver1: receiver1.cpp
	$(CC) $(CFLAGS) -o receiver1 receiver1.cpp

sender2: sender2.cpp
	$(CC) $(CFLAGS) -o sender2 sender2.cpp

receiver2: receiver2.cpp
	$(CC) $(CFLAGS) -o receiver2 receiver2.cpp

sender3: sender3.cpp
	$(CC) $(CFLAGS) -o sender3 sender3.cpp

receiver3: receiver3.cpp
	$(CC) $(CFLAGS) -o receiver3 receiver3.cpp

clean:
	rm sender1 receiver1 sender2 receiver2 sender3 receiver3
