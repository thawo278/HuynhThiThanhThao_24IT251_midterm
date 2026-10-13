CC = cc
CFLAGS = -Wall -Wextra -std=c99 -D_NETBSD_SOURCE -g -Iinclude

OBJ = src/main.o src/options.o src/list.o src/sort.o src/display.o

all: myls

myls: $(OBJ)
	$(CC) $(CFLAGS) -o myls $(OBJ)

$(OBJ): include/myls.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) myls

.PHONY: all clean
