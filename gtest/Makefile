.PHONY : clean

CC := g++
CPPFLAGS := -Iinclude
CXXFLAGS := -std=c++11 -O0 -pthread
LDFLAGS := -Llib -pthread
LDLIBS := -lgtest -lgmock

project: bin/main.o bin/test_suite.o bin/undertest.o 
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

bin/test_suite.o: src/test_suite.cpp include/mock_dependency.hpp
bin/undertest.o: src/undertest.cpp include/undertest.hpp include/dependency.hpp

$(addprefix bin/, $(addsuffix .o, $(notdir $(basename $(wildcard src/*))) )): include/common.hpp

clean:
	rm -rf bin/*

bin/%.o:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ src/$*.cpp

