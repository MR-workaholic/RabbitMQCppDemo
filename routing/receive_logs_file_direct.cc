#include "../MyTool.h"
#include <ev.h>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include <string>
#include <iostream>
#include <amqpcpp.h>
#include <vector>
#include <fstream>

using namespace std;

struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);
vector<string> logLevels = {"error"};
ofstream file;

void
sig_int(int signo)
{
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  file.close();
  ev_break (EV_A_ EVBREAK_ALL);
  exit(0);
}

int main(int argc, char *argv[])
{
  file.open("logs.txt", ios_base::out | ios_base::app);
  tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message  << "\n";
    });

  tool.GetChannel()->declareExchange("logs_direct", AMQP::direct);
  tool.GetChannel()->declareQueue(AMQP::exclusive)
    .onSuccess([](const string& name, uint32_t messagecount, uint32_t consumercount){
        for_each(logLevels.begin(), logLevels.end(), [&name](const string& logLevel){
            tool.GetChannel()->bindQueue("logs_direct", name, logLevel);
          });
        tool.GetChannel()->consume(name, AMQP::noack)
          .onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
              string data(message.body(), message.bodySize());
              file << data << endl;
            });
      });

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
