CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = -lssl -lcrypto

SRC_DIR := ./
SRCS := $(wildcard $(SRC_DIR)*.cpp)
OBJS := $(SRCS:.cpp=.o)

TARGET = imapcl

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$< vc$(ARGS)

.PHONY: all clean run