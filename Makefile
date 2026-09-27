CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O2 -g -I/opt/homebrew/opt/sdl2-compat/include
LDFLAGS = -L/opt/homebrew/opt/sdl2-compat/lib -lSDL2
TARGET = xtemu
BIOSGEN = biosgen
SOURCES = main.c memory.c hardware.c display.c
BIOSGEN_SOURCES = biosgen.c
OBJECTS = $(SOURCES:.c=.o)
BIOSGEN_OBJS = $(BIOSGEN_SOURCES:.c=.o)

.PHONY: all clean $(TARGET) $(BIOSGEN)

all: $(TARGET) $(BIOSGEN)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BIOSGEN): $(BIOSGEN_OBJS)
	$(CC) $(BIOSGEN_OBJS) -o $(BIOSGEN) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(BIOSGEN_OBJS) $(TARGET) $(BIOSGEN)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)