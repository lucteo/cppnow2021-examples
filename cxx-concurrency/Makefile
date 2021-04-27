.PHONY: all clean

all: out/1_hello
clean:
	rm -f *.o *.out *~ core

CC=/usr/local/bin/g++-10
CXXFLAGS=-std=c++2a -I/usr/local/include/ -DUSE_CXX2020=1
LDFLAGS=-lconcore -L/usr/local/lib/

ifeq ($(PROFILING), YES)
	CXXFLAGS+=-DTRACY_ENABLE=1 -I/Users/lucteo/work/others/tracy/
	LDFLAGS+=-lconcore_profiling
endif

out/%: %.cpp
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $@ $<
