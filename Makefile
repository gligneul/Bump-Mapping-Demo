# PUC-Rio
# INF2610 Real Time Rendering
# Bump Mapping
# Apr 25, 2016
# Author: Gabriel de Quadros Ligneul

target=bumpmapping
cc=g++
opt=-O2
iflags=-I./lib
cflags=-Wall -Werror -std=c++11
lflags=-lGL -lGLEW -lglut
src=$(wildcard *.cpp)
obj=$(patsubst %.cpp,%.o,$(src))

all: $(target)

$(target): $(obj)
	$(cc) $(lflags) -o $@ $^

%.o: %.cpp
	$(cc) $(cflags) $(iflags) $(opt) -c -o $@ $<

depend: $(src)
	$(cc) $(cflags) -MM $^
	
clean:
	rm -rf *.o $(target)

.PHONY: all depend clean

# Generated by `make depend`
main.o: main.cpp ShaderProgram.h
ShaderProgram.o: ShaderProgram.cpp ShaderProgram.h
