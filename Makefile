CXX      = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -I include
LDFLAGS  = -lncurses -lpthread
TARGET   = zelda
SRCS     = src/main.cpp src/renderer.cpp src/scores.cpp

all: data $(TARGET)

$(TARGET): $(SRCS) include/game.h include/renderer.h include/scores.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

data:
	mkdir -p data

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run data
