IDIR = ../include
CC = gcc
CXX = g++
CFLAGS = -Wall -I$(IDIR)

ODIR = ../obj
MKODIR = mkdir -p $(ODIR)
LDIR = ../lib
MKLDIR = mkdir -p $(LDIR)
SDIR = .
BINDIR = ../bin/
MKBDIR = mkdir -p $(BINDIR) 



LIBS = -lm -lcrypto

_DEPS = dedup.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = dedup.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_SRCXX = dedup.cpp 
SRCXX = $(patsubst %,$(SDIR)/%,$(_SRCXX))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(MKODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(MKODIR)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(_OBJ): $(OBJ)
	



.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR)
	rm -f *~ $(INCDIR)/*~
