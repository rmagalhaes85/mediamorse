OBJS=config.o main.o util.o morse.o parser.o input.o audio.o
EXE=mediamorse
CFLAGS=-g
LDFLAGS=-lm

all: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(EXE) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

ctags:
	ctags -R *

config.o: config.c util.h config.h
main.o: main.c config.h util.h
util.o: util.c
morse.o: morse.c morse.h
parser.o: parser.c config.h morse.h parser.h input.h
input.o: input.c config.h input.h
audio.o: audio.c audio.h config.h
