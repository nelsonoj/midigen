CC=gcc
EXENAME=musicgen

OS := $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(OS)))
	LDFLAGS=-lm -lncurses -lmenu
	ADDITIONAL_ERRS=-Wno-discarded-qualifiers
else
	LDFLAGS=-lncurses -lmenu
	ADDITIONAL_ERRS=-Wno-incompatible-pointer-types-discards-qualifiers
endif

CFLAGS=-Wall -Wcomments -Wextra -pedantic -Wmissing-prototypes -Wshadow -Wwrite-strings -Wno-unused-parameter -Wno-ignored-qualifiers $(ADDITIONAL_ERRS) -std=c99 -Werror

# Targets
all: $(EXENAME)

$(EXENAME): main.c interface.c music.c Event.c
	$(CC) $(CFLAGS) $(LDFLAGS) main.c interface.c music.c Event.c -o $(EXENAME)

# Actions
.PHONY: test
test:
	./$(EXENAME)

.PHONY: clean
clean:
	rm *.o
