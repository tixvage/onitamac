all: game

game: main.c
	gcc -std=c11 -Wall -Wextra -ggdb -o game main.c -lm -lraylib
