IDIR = ./include
LDIR = ./lib
SLDIR = ./slib
MKSLDIR = mkdir -p $(SLDIR)
CC = gcc
CXX = g++
DBGFLAGS = -pg
CFLAGS = -fPIC -Wall -I$(IDIR) -L$(LDIR)
CXXFLAGS = $(DBGFLAGS) $(CFLAGS) -std=c++11 -fpic

ODIR = ./obj
MKODIR = mkdir -p $(ODIR)
SDIR = ./src
BINDIR = ./bin/
MKBDIR = mkdir -p $(BINDIR) 



LIBS = -lsolidComp -lm -lcrypto -lrsync -lzd 

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

install: lib
	$(MKBDIR)
	$(CC) main.c -Wl,-R$(SLDIR) -L$(SLDIR) -o $(BINDIR)main $(CFLAGS) $(LIBS) 

lib: $(OBJ)
	$(MKSLDIR)
	$(CXX) -shared -Wl,-soname,libsolidComp.so.1 -o $(SLDIR)/libsolidComp.so.1 $^ -lc
	cd $(SLDIR); \
	ln -sf libsolidComp.so.1 libsolidComp.so

$(_OBJ): $(OBJ)	

.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR) $(SLDIR)
	rm -f *~ $(INCDIR)/*~ *.tmp temp *.out index
