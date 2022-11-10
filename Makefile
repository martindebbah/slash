CC = gcc
CFLAGS = -Wall -g -pedantic -I./readline-8.2/prefix/include -I./include -L./readline-8.2/prefix/lib
EXEC = slash

.PHONY: clean

all: $(EXEC)

slash: src/slash.c src/commande.c
	$(CC) $(CFLAGS) $^ -o $@ -lreadline

clean:
	rm -rf $(EXEC)