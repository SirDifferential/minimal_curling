EXECUTABLE=minimal_curling

DEFINES=
INCLUDES=

CPP=g++
CXXFLAGS=-std=c++14 -O0 -g -Wall -Wfloat-conversion -Wpedantic -Wno-switch -fsanitize=address -fstack-protector-all
LDFLAGS += -lcurl -fsanitize=address

SOURCES_CXX=./main.cpp

OBJECTS_CXX=$(SOURCES_CXX:.cpp=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS_CXX) $(OBJECTS_C)
	$(CPP) -o $(EXECUTABLE) $(OBJECTS_CXX) $(OBJECTS_C) $(LDFLAGS)

%.o: %.cpp
	$(CPP) $(INCLUDES) $(DEFINES) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f ./*.o
	rm -f ./minimal_curling

