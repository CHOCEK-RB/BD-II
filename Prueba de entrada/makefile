CXX = g++
CXXFLAGS = -g -std=c++17

SRCS = main.cpp document.cpp attribute.cpp
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
