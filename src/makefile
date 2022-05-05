CXX=g++

SERINC=../../serializer/src

CXXFLAGS=-g -Wall -std=c++17 -I${SERINC}

all: MemoryMappedFile.o MemoryMappedLog.o

MemoryMappedFile.o: MemoryMappedFile.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $^

MemoryMappedLog.o: MemoryMappedLog.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $^

clean:
	rm -f *.o
