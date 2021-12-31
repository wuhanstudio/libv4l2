CROSS_COMPILE	?= 
ARCH		?= x86
KERNEL_DIR	?= /usr/src/linux

CXX			:= $(CROSS_COMPILE)g++
CFLAGS		:= -W -Wall -fpermissive
LDFLAGS		:= -g

all: uvc-gadget

uvc-gadget.o: uvc-gadget.cpp
	$(CXX) -c $(CFLAGS) $^

uvc-gadget: uvc-gadget.o
	$(CXX) $(LDFLAGS) -o $@ $^ `pkg-config --libs opencv`

clean:
	rm -f *.o
	rm -f uvc-gadget
