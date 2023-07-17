#Makefile

all: client server

client: client2.cpp
	g++ client2.cpp -o client 
	
server: server.cpp
	g++ server.cpp -o server 

clean: rm -f client server

new_client: 
	./client 127.0.0.1 3000