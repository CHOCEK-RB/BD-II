CXX = g++
CXXFLAGS = -g -std=c++17

SRCS = main.cpp megatron.cpp readCSV.cpp utils.cpp const.cpp 
OBJS = $(SRCS:.cpp=.o)
TARGET = main
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
clean:
	rm -f *.o $(TARGET)
	rm -f *.o


