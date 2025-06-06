OBJS=config.o main.o util.o morse.o parser.o input.o audio.o video.o
EXE=mediamorse
CFLAGS=-g -I/usr/include/cairo
LDFLAGS=-lm -lcairo

all: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(EXE) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

ctags:
	ctags -R *

config.o: config.c util.h config.h
main.o: main.c config.h util.h token.h
util.o: util.c
morse.o: morse.c morse.h token.h
parser.o: parser.c config.h morse.h parser.h input.h token.h
input.o: input.c config.h input.h
audio.o: audio.c audio.h config.h token.h
video.o: video.c video.h config.h token.h

