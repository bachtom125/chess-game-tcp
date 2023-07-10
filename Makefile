#Makefile

all: client server

client: client.c
	g++ client.c -o client

server: server-remake/server.cpp
	g++ server-remake/server.cpp -o server 

clean: rm -f client server

new_client: 
	./client 127.0.0.1 3000