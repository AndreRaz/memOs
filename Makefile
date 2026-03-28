CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -g
LIBS    = -lsqlite3

SRC     = src/main.c src/db.c src/memory.c src/commands.c src/output.c
OBJ     = $(SRC:.c=.o)
TARGET  = mnemos

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) data/mnemos.db

run: all
	./$(TARGET)

test: all
	@chmod +x test/test.sh
	@bash test/test.sh

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(TARGET) stats

.PHONY: all clean run test valgrind