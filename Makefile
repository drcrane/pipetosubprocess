CFLAGS=-g -std=c11 -O2 -Wall -Isrc -Iinclude $(OPTFLAGS)
CXXFLAGS=-g -O2 -std=c++11 -Wall -Isrc -Iinclude $(OPTFLAGS)
LDLIBS=$(OPTLIBS)

CC=gcc
CXX=g++
LD=gcc

SRCDIR=src/
OBJDIR=obj/
RESDIR=res/
# removes the directory and returns a list of files with .c extension
# SOURCES=$(wildcard src/**/*.c src/*.c)
CSOURCES=$(wildcard $(SRCDIR)*.c)
COBJECTS=$(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CSOURCES))
CPPSOURCES=$(wildcard $(SRCDIR)*.cpp)
CPPOBJECTS=$(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(CPPSOURCES))
OBJECTS=$(COBJECTS) $(CPPOBJECTS)
BINARIES=bin/
OPTLIBS=

APP_DIR=appsrc/
APP_SRC=$(wildcard $(APP_DIR)*.c)
APPS=$(patsubst %.c,%,$(APP_SRC))

TEST_DIR=testsrc/
TEST_SRC=$(wildcard $(TEST_DIR)*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

# dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: $(TARGET)

$(CPPOBJECTS): $(OBJDIR)%.o : $(SRCDIR)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(COBJECTS): $(OBJDIR)%.o : $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

#$(RESOURCES): $(OBJDIR)%.o : $(RESDIR)%.rc
#	windres --codepage 65001 -i $< -o $@
	
%: %.c $(OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) $(RESOURCES) $< $(LDLIBS) -o $@

%: %.cpp $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) $(RESOURCES) $< $(LDLIBS) -o $@

build:
	@mkdir -p $(BINARIES)
	@mkdir -p $(OBJDIR)

# The Unit Tests
.PHONY: tests
tests: LDLIBS = $(OPTLIBS)
tests: $(TESTS)
	echo "not running tests... takes too long."
#	sh ./testsrc/runtests.sh

testsclean:
	rm -f $(TESTS)

appsclean:
	@rm -f $(APPS)

# The Applications
.PHONY: apps
apps: build
apps: LDLIBS = $(OPTLIBS)
apps: $(OBJECTS) $(APPS)

# The Cleaner
clean:
	rm -rf $(BINARIES) $(OBJECTS) $(TESTS) $(APPS)
	rm -f testsrc/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

