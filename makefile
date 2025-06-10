CXX      := g++
AR       := ar
ranlib   := ranlib
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS  := -Llib -lftpp

SRCDIR   := src
OBJDIR   := obj
LIBDIR   := lib
INCDIR   := include
TESTDIR  := tests
BINDIR   := bin

SRC      := $(wildcard $(SRCDIR)/*.cpp)
OBJ      := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))
LIB      := $(LIBDIR)/libftpp.a

TESTS    := $(wildcard $(TESTDIR)/*.cpp)
TESTBINS := $(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%,$(TESTS))

.PHONY: all lib tests clean

all: lib tests

lib: $(LIB)

$(LIB): $(OBJ)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^
	$(ranlib) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests: $(TESTBINS)

$(BINDIR)/%: $(TESTDIR)/%.cpp lib
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJDIR) $(LIBDIR) $(BINDIR)
