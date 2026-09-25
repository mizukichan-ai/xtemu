CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O2 -g -I/opt/homebrew/opt/sdl2-compat/include
LDFLAGS = -L/opt/homebrew/opt/sdl2-compat/lib -lSDL2
TARGET = xtemu
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)