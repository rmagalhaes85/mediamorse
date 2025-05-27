OBJS=config.o main.o util.o

all: $(OBJS)
	gcc $(OBJS) -g -o mediamorse

ctags:
	ctags -R *

config.o: config.c config.h util.h
main.o: main.c config.h util.h
util.o: util.c util.h

