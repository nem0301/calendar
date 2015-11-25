CC=gcc

all: server client

server: server.c
	$(CC) -o $@ $^ -l pthread

client: client.c
	$(CC) -o $@ $^ -l pthread
