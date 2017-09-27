#include "../MyTool.h"
#include <ev.h>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include <string>
#include <fstream>
#include <amqpcpp.h>

using namespace std;

struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);
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
  tool.GetChannel()->declareExchange("logs", AMQP::fanout);

  tool.GetChannel()->declareQueue(AMQP::exclusive)
    .onSuccess([](const string& name, uint32_t messagecount, uint32_t consumercount){
        cout << "queue name is " << name << " messagecount is " << messagecount << " consumercount is " << consumercount << "\n";
        tool.GetChannel()->bindQueue("logs", name, "");
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
