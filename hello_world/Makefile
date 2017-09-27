CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: send receive

send: send.o
	$(LD) -o send $^ $(FLAGS)
receive: receive.o
	$(LD) -o receive $^ $(FLAGS)

clean:
	find . -name "*.o" -exec rm '{}' \;
	rm send receive

