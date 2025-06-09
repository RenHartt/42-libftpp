CXX      := g++
AR       := ar
RANLIB   := ranlib
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

SRCDIR   := source
OBJDIR   := obj
LIBDIR   := lib
INCDIR   := include

SRC      := $(wildcard $(SRCDIR)/*.cpp)
OBJ      := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))
TARGET   := $(LIBDIR)/libftpp.a

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(LIBDIR)

install: all
	install -d /usr/local/lib
	install -m 644 $(TARGET) /usr/local/lib/
	install -d /usr/local/include/libftpp
	cp -a $(INCDIR)/*.hpp /usr/local/include/libftpp/
