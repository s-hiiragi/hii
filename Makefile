PROG     = hii
OUT_DIR  = out

# for debug
# BISONFLAGS = --warnings --debug --report=all --report-file="$(OUT_DIR)/parser.output"
BISONFLAGS = --warnings --report=all --report-file="$(OUT_DIR)/parser.output"
# for release
#BISONFLAGS = --warnings

# for debug
#CPPFLAGS = -DYYERROR_VERBOSE
# for release
CPPFLAGS = -DYYERROR_VERBOSE -DNDEBUG

# for debug
#CXXFLAGS = -std=c++17 -g3 -O0
# for release
CXXFLAGS = -std=c++17

SRCS       = $(wildcard *.cpp)
FLEX_SRCS  = $(wildcard *.ll)
BISON_SRCS = $(wildcard *.yy)

DEPS = \
	$(addprefix $(OUT_DIR)/, $(patsubst %.cpp,%.d,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.ll, %.d,$(FLEX_SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.yy, %.d,$(BISON_SRCS)))

OBJS = \
	$(addprefix $(OUT_DIR)/, $(patsubst %.cpp,%.o,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.ll, %.o,$(FLEX_SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.yy, %.o,$(BISON_SRCS)))

FLEX_OUTPUT  = scanner.cc
BISON_OUTPUT = parser.cc parser.hh location.hh position.hh stack.hh

.PHONY: all
all: $(PROG)

-include $(DEPS)

$(PROG): $(FLEX_OUTPUT) $(BISON_OUTPUT) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

$(FLEX_OUTPUT): scanner.ll
	$(info FLEX_OUTPUT $@)
	flex -8 -o $(patsubst %.ll,%.cc,$<) $<

$(BISON_OUTPUT): parser.yy
	$(info BISON_OUTPUT $@)
	@mkdir -p $(OUT_DIR)
	bison $(BISONFLAGS) -o $(patsubst %.yy,%.cc,$<) $<

$(OUT_DIR)/%.o: %.cpp
	@mkdir -p $(OUT_DIR)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -o $@ $<

$(OUT_DIR)/scanner.o: scanner.cc
	@mkdir -p $(OUT_DIR)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -o $@ $<

$(OUT_DIR)/parser.o: parser.cc
	@mkdir -p $(OUT_DIR)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -o $@ $<

.PHONY: clean
clean:
	$(RM) -r $(PROG) $(OUT_DIR) $(FLEX_OUTPUT) $(BISON_OUTPUT)

# vim: noexpandtab

