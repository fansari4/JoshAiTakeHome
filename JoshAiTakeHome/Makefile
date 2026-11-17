CXX := g++
CXXFLAGS := -std=c++14 -O2 -Wall -Wextra -I. -Iinclude
TARGET := light_monitor

all: $(TARGET)

$(TARGET): main.cpp pch.h
    $(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

run: $(TARGET)
    ./$(TARGET)

clean:
    rm -f $(TARGET)

.PHONY: all run clean
