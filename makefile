INC_DIRS := -I./includes
CC=g++ -static-libstdc++ -std=c++14 -fpermissive 
CFLAGS=-c -Wall -Ofast -march=native 
LDFLAGS=-pthread
CPP_FILES=$(wildcard src/*.cpp)
OBJ_FILES=$(CPP_FILES:.cpp=.o)
EXECUTABLE=ethergen

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES) 
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 

%.o: %.cpp
	$(CC) $(CFLAGS) ${INC_DIRS} -o $@ $<

clean:
	rm -f ethergen ./*.o ./src/*.o 