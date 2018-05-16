#include "../MyTool.h"
#include <ev.h>
#include <iostream>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include <string>
#include <algorithm>
#include <unistd.h>             // sleep()

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

  tool.GetChannel()->declareQueue("work_queue", AMQP::durable);
  // 公平调度，必须回复ACK的
  tool.GetChannel()->setQos(1);

  tool.GetChannel()->consume("work_queue")
    .onReceived([](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered){
        string data(message.body(), message.bodySize());
        size_t second = count(data.begin(), data.end(), '.');
        cout << "[x] Received " << data << " and sleep " << second << " seconds" << "\n";
        sleep(second);
        cout << "wake up and acknowledge!" << "\n";
        tool.GetChannel()->ack(deliveryTag);
      });

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
