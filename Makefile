CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter $(shell sdl2-config --cflags) -Isrc

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS = $(shell sdl2-config --libs) -framework OpenGL
else
	LDFLAGS = $(shell sdl2-config --libs) -lGL -lpthread -ldl
endif

SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, obj/%.o, $(SRCS))
TARGET = bin/ramacraft

all: $(TARGET)

$(TARGET): $(OBJS) | bin
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

obj:
	mkdir -p obj

clean:
	rm -rf bin obj

run: all
	./$(TARGET)

.PHONY: all clean run
