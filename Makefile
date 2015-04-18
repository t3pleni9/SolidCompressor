IDIR = ./include
LDIR = ./lib
SLDIR = ./slib
MKSLDIR = mkdir -p $(SLDIR)
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



LIBS = -lm -lcrypto -lrsync -lzd 

_DEPS = hash.h index.h dedup.h diff.h solidlib.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hash.o index.o dedup.o diff.o solidlib.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c  $(DEPS)
	$(MKODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(MKODIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS) 

install: $(OBJ)
	$(MKBDIR)
	$(CC) main.c -o $(BINDIR)main  obj/solidlib.o  $(CFLAGS) $(LIBS)


$(_OBJ): $(OBJ)	

.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR) $(SLDIR)
	rm -f *~ $(INCDIR)/*~ *.tmp temp *.out index
