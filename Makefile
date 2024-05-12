CC = gcc
CFLAGS = -Wall -Wextra

all: echo_server echo_client

echo_server: echo_server.o
	$(CC) $(CFLAGS) echo_server.o -o echo_server

echo_client: echo_client.o
	$(CC) $(CFLAGS) echo_client.o -o echo_client

echo_server.o: echo_server.c
	$(CC) $(CFLAGS) -c echo_server.c -o echo_server.o

echo_client.o: echo_client.c
	$(CC) $(CFLAGS) -c echo_client.c -o echo_client.o

clean:
	rm -f echo_server.o echo_client.o echo_server echo_client