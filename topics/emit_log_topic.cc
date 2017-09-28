#include "../MyTool.h"
#include <ev.h>
#include <iostream>
#include <memory.h>
#include <unistd.h>             // read()
#include <stdlib.h>             // exit()
#include <signal.h>             // signal()
#include <amqpcpp.h>
#include <string>
#include <vector>
#include <algorithm>


using namespace std;

const int MAXSIZE = 1024;

struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);
ev_io stdin_watcher;
vector<string> facilityList = {"server_one", "server_two", "mq_one", "mq_two"};
vector<string> severityList = {"debug", "info", "error"};

template<typename T>
shared_ptr<T> make_shared_array(size_t size){
  return shared_ptr<T>(new T[size], default_delete<T[]>());
}

void
sig_int(int signo){
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  ev_break(EV_A_ EVBREAK_ALL);
  exit(0);
}

static void
stdin_cb(EV_P_ ev_io* w, int revents){
  auto input = make_shared_array<char>(MAXSIZE);
  ssize_t dataSize = read(w->fd, input.get(), MAXSIZE);
  string data(input.get(), dataSize - 1);

  size_t pos_one = data.find(".");
  if (pos_one != string::npos) {
    string facility = data.substr(0, pos_one - 0);
    if (find(facilityList.begin(), facilityList.end(), facility) != facilityList.end()) {
      size_t pos_two = data.find(":");
      if (pos_two != string::npos && pos_two > pos_one) {
        string severity = data.substr(pos_one + 1, pos_two - pos_one - 1);
        if (find(severityList.begin(), severityList.end(), severity) != severityList.end()) {
          string routingKey = facility + "." + severity;
          tool.GetChannel()->publish("logs_topic", routingKey, data);
          cout << "[x] Sent " << data << " and the routingkey is " << routingKey << "\n";
        }
      }
    }
  }
}


int main(int argc, char *argv[])
{
  tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message << "\n";
    });

  tool.GetChannel()->declareExchange("logs_topic", AMQP::topic);

  ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
  ev_io_start(loop, &stdin_watcher);

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
