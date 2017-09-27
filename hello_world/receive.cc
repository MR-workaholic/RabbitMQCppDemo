#include <iostream>
#include <ev.h>
#include <amqpcpp.h>
#include <string>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include "../MyTool.h"

using namespace std;

struct ev_loop *loop = EV_DEFAULT;
MyTool tool(loop);

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

  auto successCB = [](const string& consumertag){
    cout << "consumer operator start"  << "\n";
  };

  auto receiveCB = [](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered){
    string data(message.body(), message.bodySize());
    cout << "recevie the message: " << data << "\n";
    cout << "from the exchange : " << message.exchange() << " and the routingkey : " << message.routingkey()  << "\n";

    // acknowledge the message
    // channel.ack(deliveryTag)

  };

  tool.GetChannel()->consume("hello", AMQP::noack)
    .onReceived(receiveCB)
    .onSuccess(successCB);

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
