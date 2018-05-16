CXX = g++ -c
LD = g++
DEBUG = -g -Wall
FLAGS = -std=c++11 -lamqpcpp -lpthread -L/usr/local/lib -lev

%.o: %.cc
	$(CXX) $(DEBUG) -o $*.o $< $(FLAGS)

all: rpc_client rpc_server

rpc_client: rpc_client.o
	$(LD) -o rpc_client $^ $(FLAGS)
rpc_server: rpc_server.o
	$(LD) -o rpc_server $^ $(FLAGS)

clean:
	find . -name "*.o" -exec rm '{}' \;
	rm rpc_client rpc_server

