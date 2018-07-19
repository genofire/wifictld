
all: hostapd wifictld

CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lubox -lubus

DEPS = $(wildcard *.h)

SRC = $(wildcard *.c)

OBJ = $(patsubst %.c, %.o, $(SRC)) hostapd/common.o

hostapd:
	$(MAKE) -C hostapd

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wifictld: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	$(MAKE) -C hostapd clean
	rm -f wifictld *.o
