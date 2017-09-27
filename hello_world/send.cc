#include <iostream>
#include <ev.h>
#include <amqpcpp.h>
#include <memory>
#include <unistd.h>             // read()
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include "../MyTool.h"


const int MAXSIZE = 1024;


using namespace std;

struct ev_loop *loop = EV_DEFAULT;
MyTool tool(loop);
ev_io stdin_watcher;


template<typename T>
shared_ptr<T> make_shared_array(size_t size){
  return shared_ptr<T>(new T[size], default_delete<T[]>());
}


static void
stdin_cb(EV_P_ ev_io *w, int revents)
{
  shared_ptr<char> input = make_shared_array<char>(MAXSIZE);
  ssize_t realSize = read(w->fd, input.get(), MAXSIZE);
  input.get()[realSize - 1] = '\0';
  tool.GetChannel()->publish("", "hello", input.get(), realSize-1);
  cout << " [x] Sent " << input.get() << "\n";
}

void
sig_int(int signo)
{
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  ev_break (EV_A_ EVBREAK_ALL);
  exit(0);

}

int main(int argc, char *argv[])
{

  tool.GetChannel()->onError([](const char* message) {
      cout << "channel error: " << message << "\n";
    });

  tool.GetChannel()->declareQueue("hello");

  ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
  ev_io_start(loop, &stdin_watcher);

  signal(SIGINT, sig_int);
  ev_run(loop, 0);


  return 0;
}
