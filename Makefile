# Makefile for the Multi-Branch Banking Management System
#
# Usage:
#   make        build bank.exe from all modules
#   make run    build then run
#   make clean  remove build artefacts

CXX      := g++
CXXFLAGS := -g -std=c++17 -Wall
TARGET   := bank.exe

SOURCES  := main.cpp utils.cpp validation.cpp accounts.cpp branch.cpp \
            customer.cpp teller.cpp admin.cpp menus.cpp
OBJECTS  := $(SOURCES:.cpp=.o)
HEADERS  := records.h utils.h validation.h accounts.h branch.h \
            customer.h teller.h admin.h menus.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Each object depends on all headers (simple and safe for a project this size).
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all run clean
