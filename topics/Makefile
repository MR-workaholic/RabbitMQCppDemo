CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: emit_log_topic receive_logs_topic receive_logs_topic2

emit_log_topic: emit_log_topic.o
	$(LD) -o emit_log_topic $^ $(FLAGS)
receive_logs_topic: receive_logs_topic.o
	$(LD) -o receive_logs_topic $^ $(FLAGS)
receive_logs_topic2: receive_logs_topic2.o
	$(LD) -o receive_logs_topic2 $^ $(FLAGS)

clean:
	find . -name "*.o" -exec rm '{}' \;
	rm emit_log_topic receive_logs_topic receive_logs_topic2

