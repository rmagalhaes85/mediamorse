OBJS=config.o main.o util.o parser.o input.o
EXE=mediamorse
CFLAGS=-g

all: $(OBJS)
	gcc $(OBJS) $(CFLAGS) -o $(EXE)

clean:
	$(RM) $(OBJS)

ctags:
	ctags -R *

config.o: config.c util.h
main.o: main.c config.h util.h
util.o: util.c
parser.o: parser.c config.h
input.o: input.c config.h
