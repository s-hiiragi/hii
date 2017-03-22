PROG     = calc
OUT_DIR  = out
CPPFLAGS = 
CXXFLAGS = -std=c++11

SRCS = $(wildcard *.cpp *.yy *.ll)
DEPS = \
	$(addprefix $(OUT_DIR)/, $(patsubst %.cpp,%.d,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.yy,%.d,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.ll,%.d,$(SRCS)))
OBJS = \
	$(addprefix $(OUT_DIR)/, $(patsubst %.cpp,%.o,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.yy,%.o,$(SRCS))) \
	$(addprefix $(OUT_DIR)/, $(patsubst %.ll,%.o,$(SRCS)))

FLEX_OUTPUT  = calc-scanner.cc
BISON_OUTPUT = calc-parser.cc calc-parser.hh location.hh position.hh

.PHONY: all
all: $(FLEX_OUTPUT) $(BISON_OUTPUT) $(DEPS) $(PROG)

$(FLEX_OUTPUT): calc-scanner.ll
	$(info FLEX_OUTPUT $@)
	flex -8 -o$(patsubst %.ll,%.cc,$<) $<

$(BISON_OUTPUT): calc-parser.yy
	$(info BISON_OUTPUT $@)
	bison -d -ra -o$(patsubst %.yy,%.cc,$<) $<

$(OUT_DIR)/%.d: %.cpp
	@mkdir -p $(OUT_DIR)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM -MF $@ $<

-include $(DEPS)

$(PROG): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

#$(OBJS): %.o: %.c
$(OUT_DIR)/%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -o $@ $<

.PHONY: clean
clean:
	$(RM) -r $(PROG) $(OUT_DIR) $(FLEX_OUTPUT) $(BISON_OUTPUT) *.output

# vim: noexpandtab

