.PHONY: all clean

all: out/1_hello
clean:
	rm -f *.o *.out *~ core

CC=clang++
CXXFLAGS=-std=c++17 
LDFLAGS=-stdlib=libc++ -lconcore

ifeq ($(PROFILING), YES)
	CXXFLAGS+=-DTRACY_ENABLE=1 -I/Users/lucteo/work/others/tracy/
	LDFLAGS+=-lconcore_profiling
endif

out/%: %.cpp
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $@ $<
