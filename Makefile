CFLAGS=-g -Wall
CC=clang
SRCS=tokenizer.c seashell.c
OBJS=tokenizer.o seashell.o
LDFLAGS=
LIBS=

all:    seashell

$(SRCS):
	$(CC) $(CFLAGS) -c $*.c

seashell: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o seashell $(OBJS)

clean:
	rm -f *.o seashell