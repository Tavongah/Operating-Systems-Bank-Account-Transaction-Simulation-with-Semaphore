CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE
TARGET = assign2
SOURCES = assign2.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) -lpthread

clean:
	rm -f $(TARGET)

.PHONY: clean