CC = gcc
CFLAGS = -Wall -g -pedantic
INCLUDES = -I./include
CCLINK = -lreadline
EXEC = slash

.PHONY: clean

all: $(EXEC)

slash: src/slash.c src/commande.c src/cmd.c src/mystring.c src/string_list.c src/redirection.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(CCLINK)

clean:
	rm -rf $(EXEC)