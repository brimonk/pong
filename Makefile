all:
	gcc -Wall pong.c -o pong -lncurses

clean:
	rm pong
