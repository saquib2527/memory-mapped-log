CXX=g++

EXEC=bin

GTESTINC=/usr/local/include/gtest
INC=../src
OBJ=${INC}/MemoryMappedFile.o ${INC}/MemoryMappedLog.o
SERINC=../../serializer/src
SEROBJ=${SERINC}/Serializer.o

LIBS=/usr/local/lib/libgtest.a

CXXFLAGS=-g -Wall -std=c++14 -I${GTESTINC} -I${INC}

all: test.out

test.out: test.cpp ${OBJ} ${SEROBJ}
	${CXX} ${CXXFLAGS} -o $@ $^ ${LIBS}
	mkdir -p ${EXEC}; mv $@ ${EXEC}

clean:
	rm -rf ${EXEC}
