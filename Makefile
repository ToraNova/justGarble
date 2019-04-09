# ***********************************************
#                    JustGarble
#
# Forked from Justine's version
# ***********************************************

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR   = test
SAMPLEDIR = sample
MAINDIR = main
OBJECTFULL = obj/*.o

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

TESTS := $(wildcard $(TESTDIR)/*.c)
TESTP := $(TESTS:$(TESTDIR)/%.c=$(BINDIR)/%.testbin) # generate the list of target

SAMPLES := $(wildcard $(SAMPLEDIR)/*.c)
SAMPLEP := $(SAMPLES:$(SAMPLEDIR)/%.c=$(BINDIR)/%.sample) # generate the list of target

MAINS := $(wildcard $(MAINDIR)/*.c)
MAINP := $(MAINS:$(MAINDIR)/%.c=$(BINDIR)/%.out) # generate the list of target

IDIR = include
CC=gcc
CFLAGS= -O3 -lm -lcrypto -lgnutlsxx -lgnutls -lrt -lpthread -maes -msse4 -lssl -lmsgpackc -march=native -I$(IDIR)

AES = AES_MBDPI
OT = ot_AES
rm = rm --f
ls = ls

.PHONY: all
all:	tests samples AES
	@echo 'make done'
	@$(ls) $(BINDIR)

AES: $(OBJECTS) $(MAINDIR)/$(AES).c
	$(CC) $(OBJECTFULL) $(MAINDIR)/$(AES).c -o $(BINDIR)/$(AES).out $(LIBS) $(CFLAGS)

# TODO FIX THIS CPP BUILD !!
OT: $(OBJECTS)
	$(CC) $(OBJECTFULL) $(TESTDIR)/ot_AES.cpp -o $(BINDIR)/OT_test $(LIBS) $(CLAGS)

.PHONY: samples
samples: $(SAMPLEP)
	@echo $(SAMPLEP)
	@echo 'Samples compiled'

$(SAMPLEP) : $(BINDIR)/%.sample: $(SAMPLEDIR)/%.c $(OBJECTS)
	$(CC) $(OBJECTFULL) $< -o $@ $(LIBS) $(CFLAGS)

.PHONY: tests
tests:	$(TESTP)
	@echo $(TESTP)
	@echo 'Tests Compiled'

$(TESTP) : $(BINDIR)/%.testbin : $(TESTDIR)/%.c $(OBJECTS)
	$(CC) $(OBJECTFULL) $< -o $@ $(LIBS) $(CFLAGS)

$(OBJECTS):	$(OBJDIR)/%.o :	$(SRCDIR)/%.c
	$(CC) -c $< -o $@ $(LIBS) $(CFLAGS)


.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(BINDIR)/*
