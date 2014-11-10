OBJS   = btext.o main.o
TARGET = sinething
CFLAGS = `sdl-config --cflags` -pedantic -Wall -Wextra -std=gnu99 -ggdb
#CFLAGS = `sdl-config --cflags` -pedantic -Wall -Wextra -std=gnu99
LDFLAGS = `sdl-config --libs` -lm

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
