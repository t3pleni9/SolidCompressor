IDIR = ./include
LDIR = ./lib
MKLDIR = mkdir -p $(LDIR)
CC = gcc
CXX = g++
DBGFLAGS = -pg
CFLAGS = -Wall -I$(IDIR) -L$(LDIR)
CXXFLAGS = $(DBGFLAGS) $(CFLAGS) -std=c++11 

ODIR = ./obj
MKODIR = mkdir -p $(ODIR)
SDIR = ./src
BINDIR = ./bin/
MKBDIR = mkdir -p $(BINDIR) 



LIBS = -lm -lcrypto -lrsync

_DEPS = hash.h index.h dedup.h delta.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hash.o index.o dedup.o delta.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_SRCXX = hash.cpp index.cpp dedup.cpp delta.cpp
SRCXX = $(patsubst %,$(SDIR)/%,$(_SRCXX))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(MKODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(MKODIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS) 



test: $(OBJ)
	$(MKBDIR)
	$(CXX) main.cpp -o $(BINDIR)main $^ $(CXXFLAGS) $(LIBS)

$(_OBJ): $(OBJ)	

.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR)
	rm -f *~ $(INCDIR)/*~ *.tmp temp *.out index
