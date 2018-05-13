all: server client

server: server.c
	gcc -o server -Wall -pthread server.c
client: client.c
	gcc -o client -Wall client.c
