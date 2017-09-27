CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: emit_log_direct receive_logs_direct receive_logs_file_direct

emit_log_direct: emit_log_direct.o
	$(LD) -o emit_log_direct $^ $(FLAGS)
receive_logs_direct: receive_logs_direct.o
	$(LD) -o receive_logs_direct $^ $(FLAGS)
receive_logs_file_direct: receive_logs_file_direct.o
	$(LD) -o receive_logs_file_direct $^ $(FLAGS)


clean:
	find . -name "*.o" -exec rm '{}' \;
	rm emit_log_direct receive_logs_direct receive_logs_file_direct

