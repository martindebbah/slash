CC = gcc
CFLAGS = -Wall -g -pedantic
INCLUDES =-I./include #-I./readline-8.2/prefix/readline/include 
LIB = -L./readline-8.2/prefix/lib
CCLINK = -lreadline
EXEC = slash

.PHONY: clean

all: $(EXEC)

slash: src/slash.c src/commande.c src/cmd.c src/mystring.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(CCLINK)

clean:
	rm -rf $(EXEC)