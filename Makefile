IDIR = ./include
LDIR = ./lib
SLDIR = ./slib
MKSLDIR = mkdir -p $(SLDIR)
CC = gcc
CXX = g++
DBGFLAGS = -pg
CFLAGS = -fPIC -Wall -I$(IDIR) -L$(LDIR)  -D_FILE_OFFSET_BITS=64 
CXXFLAGS = $(DBGFLAGS) $(CFLAGS) -std=c++11 -fpic  -D_FILE_OFFSET_BITS=64 

ODIR = ./obj
MKODIR = mkdir -p $(ODIR)
SDIR = ./src
BINDIR = ./bin/
MKBDIR = mkdir -p $(BINDIR) 



LIB = -lsolidComp
DEPLIBS = -lm -lcrypto -lfuzzy -lzd -lz -lpthread -lbz2

_DEPS = scons.h streamc.h hash.h index.h dedup.h diff.h solidlib.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = scons.o streamc.o hash.o index.o dedup.o diff.o solidlib.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(MKODIR)
	$(CC) -c  -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(MKODIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS) 

install: lib
	$(MKBDIR)
	$(CC) main.c -Wl,-R$(SLDIR) -L$(SLDIR) -o $(BINDIR)main $(CFLAGS) $(LIB)

lib: $(OBJ)
	$(MKSLDIR)
	$(CXX) -shared -Wl,-soname,libsolidComp.so.1 -o $(SLDIR)/libsolidComp.so.1 $^ -lc $(DEPLIBS) 
	cd $(SLDIR); \
	ln -sf libsolidComp.so.1 libsolidComp.so 

test: $(OBJ)
	$(MKBDIR)
	$(CXX) main.c -o $(BINDIR)main $^ $(CXXFLAGS) $(DEPLIBS)

$(_OBJ): $(OBJ)	

.PHONY: clean

clean:
	rm -rf $(ODIR) $(BINDIR) $(SLDIR)
	rm -f *~ $(INCDIR)/*~ *.tmp temp *.out index
