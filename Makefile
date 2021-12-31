CROSS_COMPILE	?= 
ARCH		?= x86
KERNEL_DIR	?= /usr/src/linux

CXX		:= $(CROSS_COMPILE)g++
CFLAGS		:= -I /usr/include/opencv4 -W -Wall -fpermissive
LDFLAGS		:= -g

all: main

main.o: main.cpp
	$(CXX) -c $(CFLAGS) $^

main: main.o
	$(CXX) $(LDFLAGS) -o $@ $^ `pkg-config --libs opencv4`

clean:
	rm -f *.o
	rm -f main
