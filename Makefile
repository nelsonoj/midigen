CC=gcc

OS := $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(OS)))
	LDFLAGS=-lm -lncurses -lmenu
	ADDITIONAL_ERRS=-Wno-discarded-qualifiers
else
	LDFLAGS=-lncurses -lmenu
	ADDITIONAL_ERRS=-Wno-incompatible-pointer-types-discards-qualifiers
endif

CFLAGS=-g -Wall -Wcomments -Wextra -pedantic -Wmissing-prototypes -Wshadow -Wwrite-strings -Wno-unused-parameter -Wno-ignored-qualifiers $(ADDITIONAL_ERRS) -std=c99 -Werror

# Targets
all: musicgen

musicgen: music.c interface.c Event.c
	$(CC) $(CFLAGS) $(LDFLAGS) music.c interface.c Event.c -o musicgen

# Actions
.PHONY: test
test:
	./musicgen

.PHONY: clean
clean:
	rm *.o
