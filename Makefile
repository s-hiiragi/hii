SOURCES = calc.cpp node.cpp exprlist.cpp arglist.cpp calc-driver.cpp calc-parser.yy calc-scanner.ll
HEADERS = calc-driver.h node.h arglist.h exprlist.h
OBJ = calc.o node.o exprlist.o arglist.o calc-driver.o calc-parser.o calc-scanner.o
FLEX_OUTPUT = calc-scanner.cc
BISON_OUTPUT = calc-parser.cc calc-parser.hh location.hh position.hh

override CFLAGS += -O2
override CPPFLAGS += -std=c++11

all: calc

#.SUFFIXES:
#.SUFFIXES: .cpp .cc .ll .yy .o

calc: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -lstdc++

.cpp.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

.cc.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(BISON_OUTPUT): calc-parser.yy
	bison -d -ra -ocalc-parser.cc calc-parser.yy

calc-scanner.cc: calc-scanner.ll
	flex -8 -ocalc-scanner.cc calc-scanner.ll

calc-parser.o: $(BISON_OUTPUT)
calc-scanner.o: calc-scanner.cc

depend:
	makedepend -- $(CFLAGS) -- $(SOURCES)

#calc.o: calc-parser.hh
#node.o: location.hh

# DO NOT DELETE

calc.o: calc-driver.h calc-parser.hh stack.hh node.h location.hh position.hh
#node.o: node.h calc-driver.h calc-parser.hh stack.hh location.hh position.hh
node.o: node.h calc-driver.h
exprlist.o: node.h exprlist.h
arglist.o: node.h arglist.h
calc-driver.o: calc-driver.h calc-parser.hh stack.hh node.h exprlist.h arglist.h location.hh
calc-driver.o: position.hh
calc-parser.o: node.h exprlist.h arglist.h calc-driver.h calc-parser.hh stack.hh location.hh
calc-parser.o: position.hh
calc-scanner.o: calc-driver.h calc-parser.hh stack.hh node.h location.hh
calc-scanner.o: position.hh

.PHONY: clean
clean:
	@rm -rf *.o calc $(FLEX_OUTPUT) $(BISON_OUTPUT) *.output stack.hh

# vim: set noexpandtab:
