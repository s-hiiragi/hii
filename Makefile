SOURCES = calc.cpp node.cpp calc-driver.cpp calc-parser.yy calc-scanner.ll
HEADERS = calc-driver.h node.h
OBJ = calc.o node.o calc-driver.o calc-parser.o calc-scanner.o
FLEX_OUTPUT = calc-scanner.cc
BISON_OUTPUT = calc-parser.cc calc-parser.hh location.hh position.hh

CFLAGS = -O2

all: calc

convert: $(SOURCES) $(HEADERS)

.SUFFIXES:
.SUFFIXES: .cpp .cc .ll .yy .o

calc: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -lstdc++

.cpp.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cc.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(BISON_OUTPUT): calc-parser.yy
	bison -d -ra -ocalc-parser.cc calc-parser.yy

calc-scanner.cc: calc-scanner.ll
	flex -8 -ocalc-scanner.cc calc-scanner.ll

calc-parser.o: $(BISON_OUTPUT)
calc-scanner.o: calc-scanner.cc

calc.cpp: ../calc.cpp
	nkf -w --unix $< > $@

node.cpp: ../node.cpp
	nkf -w --unix $< > $@

calc-driver.cpp: ../calc-driver.cpp
	nkf -w --unix $< > $@

calc-parser.yy: ../calc-parser.yy
	nkf -w --unix $< > $@

calc-scanner.ll: ../calc-scanner.ll
	nkf -w --unix $< > $@

calc-driver.h: ../calc-driver.h
	nkf -w --unix $< > $@

node.h: ../node.h
	nkf -w --unix $< > $@

depend:
	makedepend -- $(CFLAGS) -- $(SOURCES)

calc.o: calc-parser.hh
node.o: location.hh

# DO NOT DELETE

calc.o: calc-driver.h calc-parser.hh stack.hh node.h location.hh position.hh
node.o: node.h calc-driver.h calc-parser.hh stack.hh location.hh position.hh
calc-driver.o: calc-driver.h calc-parser.hh stack.hh node.h location.hh
calc-driver.o: position.hh
calc-parser.o: node.h calc-driver.h calc-parser.hh stack.hh location.hh
calc-parser.o: position.hh
calc-scanner.o: calc-driver.h calc-parser.hh stack.hh node.h location.hh
calc-scanner.o: position.hh

.PHONY: clean
clean:
	@rm -rf *.o calc $(FLEX_OUTPUT) $(BISON_OUTPUT) *.output stack.hh

