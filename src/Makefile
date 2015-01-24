IDIR = ../include
CC = gcc
CXX = g++
DBGFLAGS = -pg
CFLAGS = -Wall -I$(IDIR)
CXXFLAGS = $(DBGFLAGS) $(CFLAGS) -std=c++11 


ODIR = ../obj
MKODIR = mkdir -p $(ODIR)
LDIR = ../lib
MKLDIR = mkdir -p $(LDIR)
SDIR = .
BINDIR = ../bin/
MKBDIR = mkdir -p $(BINDIR) 



LIBS = -lm -lcrypto

_DEPS = hash.h index.h dedup.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hash.o index.o dedup.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_SRCXX = hash.cpp index.cpp dedup.cpp 
SRCXX = $(patsubst %,$(SDIR)/%,$(_SRCXX))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(MKODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(MKODIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(_OBJ): $(OBJ)
	



.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR)
	rm -f *~ $(INCDIR)/*~
