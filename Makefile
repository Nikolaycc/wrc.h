CC = gcc
CFLAGS = -I .

TESTSRC = test/ifc.c
TESTCMD = $(CC) $(CFLAGS) -o ifc test/ifc.c;

TESTSRC += test/cap.c
TESTCMD += $(CC) $(CFLAGS) -o cap test/cap.c;

test: $(TESTSRC)
	$(TESTCMD)