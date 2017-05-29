CFLAGS = -O3 -Wall -Wextra -Wpedantic
LIBS = -lxxhash
EMACSDIR = ~/Programs/emacs

xxh.so: emacs-xxhash.o
	ld -shared -o $@ $< $(LIBS)

emacs-xxhash.o: emacs-xxhash.c

%.c %.o:
	gcc $(CFLAGS) -I $(EMACSDIR)/src -fpic -c -o $@ $<
