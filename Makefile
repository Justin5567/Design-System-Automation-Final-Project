CXX = g++
CXXFLAGS =  -std=c++17 -Wall

TARGET = dpll
SRC = *.cpp
HEADER = *.h

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) $(HEADER) #target: dependence
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)