CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: emit_log receive_logs receive_logs_file

emit_log: emit_log.o
	$(LD) -o emit_log $^ $(FLAGS)
receive_logs: receive_logs.o
	$(LD) -o receive_logs $^ $(FLAGS)
receive_logs_file: receive_logs_file.o
	$(LD) -o receive_logs_file $^ $(FLAGS)


clean:
	find . -name "*.o" -exec rm '{}' \;
	rm emit_log receive_logs receive_logs_file

