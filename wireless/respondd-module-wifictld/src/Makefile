# standard compliance
CFLAGS         += -std=c99

# warnings
CFLAGS         += -Wall -Wextra -Wformat=2 -Wshadow -Wpointer-arith
CFLAGS         += -pedantic

all: respondd.so

%.c: %.h

respondd.so: respondd.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -lubox -lubus -D_GNU_SOURCE -o $@ $^ $(LDLIBS)

clean:
	rm -rf *.so
