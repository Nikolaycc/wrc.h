CC = gcc
CFLAGS = -ggdb -I .

TESTSRC = test/ifc.c
TESTCMD = $(CC) $(CFLAGS) -o bin/ifc test/ifc.c;

TESTSRC += test/cap.c
TESTCMD += $(CC) $(CFLAGS) -o bin/cap test/cap.c;

.PHONY: all test

all: test

test: $(TESTSRC)
	$(TESTCMD)
