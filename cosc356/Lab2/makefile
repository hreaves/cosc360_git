CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = cache_sim
SRC = cache_sim.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
