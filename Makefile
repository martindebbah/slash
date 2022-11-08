CC = gcc
CFLAGS = -lreadline -Wall -g -pedantic
EXEC = slash

.PHONY: clean

all: $(EXEC)

slash: src/slash.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(EXEC)