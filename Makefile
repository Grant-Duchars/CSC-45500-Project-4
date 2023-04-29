all: server client

server: project4server.c
	gcc project4server.c -o project4server -lpthread

client: project4client.c
	gcc project4client.c -o project4client -lpthread