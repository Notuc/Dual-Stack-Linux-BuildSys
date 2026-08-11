CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra
LDFLAGS ?=

TARGET = logging-service

SRCS = Src/main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
