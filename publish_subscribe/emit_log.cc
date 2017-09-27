#include "../MyTool.h"
#include <ev.h>
#include <iostream>
#include <memory.h>
#include <unistd.h>             // read()
#include <stdlib.h>             // exit()
#include <signal.h>             // signal()
#include <amqpcpp.h>

using namespace std;

const int MAXSIZE = 1024;

struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);
ev_io stdin_watcher;

template<typename T>
shared_ptr<T> make_shared_array(size_t n)
{
  return shared_ptr<T>(new T[n], default_delete<T[]>());
}

void
sig_int(int signo)
{
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  ev_break (EV_A_ EVBREAK_ALL);
  exit(0);
}

static void
stdin_cb(EV_P_ ev_io *w, int revents){
  auto input = make_shared_array<char>(MAXSIZE);
  ssize_t realSize = read(w->fd, input.get(), MAXSIZE);
  input.get()[realSize - 1] = '\0';
  tool.GetChannel()->publish("logs", "", input.get(), realSize - 1);
  cout << "[x] Sent " << input.get() << "\n";
}

int main(int argc, char *argv[])
{
  tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message  << "\n";
    });

  tool.GetChannel()->declareExchange("logs", AMQP::fanout);

  ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
  ev_io_start(loop, &stdin_watcher);

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
