#Check if env variable CLANG is set to 1
#If CLANG==1 -> set compiler to CLANG and STDCPP to -lstdc++ to link the C++ standard library explicitly
#Else, default compiler to g++, and keep STDCPP empty
ifeq ($(CLANG), 1)
	CC=clang
	STDCPP=-lstdc++
else
	CC=g++
	STDCPP=
endif

BOOST_DIR = ./boost_1_86_0

#Include Directories
INC = $(BOOST_DIR)/
INC_PARAMS=$(foreach d, $(INC), -I$d)

#Library Directories
LIB_DIRS = $(BOOST_DIR)/libs

#List the source files
SOURCES = FH_Error_Tracker.cpp

#Link the maths library
LIBS = -lm

#Removing warnings on unused typedefs to quiet down boost
COMMON = -std=c++11 -Wall -Wno-unused-local-typedefs $(STDCPP) -L$(LIB_DIRS) $(LIBS) $(INC_PARAMS) -pthread


#Default target, runs the release target to build the optimized release version
all:
	make release

#Build a debug version of the program
debug:
	$(CC) -g -DDEBUG $(COMMON) $(SOURCES) main.cpp -o QuickTradeProcessor

#Build an optimized release version of the program
release:
	g++ -O3 -DNDEBUG $(COMMON) $(SOURCES) main.cpp -o QuickTradeProcessor

#Build a test version of the program
test:
	$(CC) -g -DDEBUG $(COMMON) $(SOURCES) test.cpp -I$(BOOST_DIR) -o UtilityTester

#Build a performance test version with optimizations and debugging enabled
perftest:
	g++ -O3 -g -DDEBUG $(COMMON) $(SOURCES) test.cpp -o UtilityTester

#Build a profiling version of the program
profile:
	g++ -O3 -DNDEBUG -DPROFILE $(COMMON) $(SOURCES) main.cpp -o QuickTradeProcessor

#Compile the source files into a shared library
lib:
	g++ -O3 -DNDEBUG $(COMMON) $(SOURCES) -shared -o libOrderBook.so

#Build a high-optimization profiling version of the program
grind:
	$(CC) -g -O3 -DNDEBUG -DPROFILE $(COMMON) $(SOURCES) main.cpp -o OrderBookProcessor

#Clean-up build artifacts
clean:
	rm -f QuickTradeProcessor
	rm -f UtilityTester
	rm -f *.out
	rm -f *.so