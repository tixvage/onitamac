all: client server

server: server.c
	gcc -Wall -Wextra -ggdb -o server server.c

client: client.c
	gcc -Wall -Wextra -ggdb -o client client.c -lm -lraylib
