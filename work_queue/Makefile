CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: new_task worker

new_task: new_task.o
	$(LD) -o new_task $^ $(FLAGS)
worker: worker.o
	$(LD) -o worker $^ $(FLAGS)

clean:
	find . -name "*.o" -exec rm '{}' \;
	rm new_task worker

