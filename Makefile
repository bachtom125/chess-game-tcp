#Makefile

all: client server

client: client.cpp
	g++ client.cpp -o client 
	
server: server.cpp
	g++ server.cpp -o server 

clean: rm -f client server

new_client: 
	./client 127.0.0.1 3000