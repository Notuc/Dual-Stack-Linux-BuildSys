CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra
LDFLAGS ?=

TARGET = daemon

SRCS = Src/main.cpp \
       Drivers/BME280/bme280.cpp \
       Drivers/CAN/can_interface.cpp

OBJS = $(SRCS:.cpp=.o)

INCLUDES = -IDrivers/BME280 \
           -IDrivers/CAN

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -li2c -lpthread -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
