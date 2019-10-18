CC      = g++
CFLAGS  = -std=c++11 -Wall
SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:%.cc=%.o)

all: dragonshell

dragonshell: $(OBJECTS)
	$(CC) -o dragonshell $(OBJECTS)

compile: $(OBJECTS)

%.o: %.cc
	${CC} ${CFLAGS} -c $^

clean:
	@rm -f *.o dragonshell

compress:
	zip dragonshell.zip README.md Makefile *.cc *.h

