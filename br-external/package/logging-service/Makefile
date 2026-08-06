CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra
LDFLAGS ?=

TARGET = logging-service

SRCS = Src/main.cpp

OBJS = $(SRCS:.cpp=.o)

INCLUDES = -IInc

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
