CFLAGS = -Wall -Werror -ggdb3
all: lets-talk

lets-talk: lets-talk.c
	gcc $(CFLAGS) -g -o lets-talk lets-talk.c list.c -lpthread

clean: 
	rm -f *.o lets-talk
	
valgrind:
	valgrind ./lets-talk 3000 127.0.0.1 3001