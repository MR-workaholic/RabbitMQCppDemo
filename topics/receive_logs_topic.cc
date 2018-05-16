#include "../MyTool.h"
#include "../json.hpp"
#include <ev.h>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include <string>
#include <iostream>
#include <amqpcpp.h>

using namespace std;
using json = nlohmann::json;

struct ev_loop* loop = EV_DEFAULT;
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
  tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message  << "\n";
    });

  tool.GetChannel()->declareExchange("logs_topic", AMQP::topic);
  tool.GetChannel()->declareQueue(AMQP::exclusive)
    .onSuccess([&](const string& name, uint32_t messagecount, uint32_t consumercount) {
        if (argc > 1) {
          for (int i = 1; i < argc; i++) {
            string bindKey = string(argv[i]);
            cout << name << "\n";
            tool.GetChannel()->bindQueue("logs_topic", name, bindKey)
              .onSuccess([name, bindKey](){
                  cout << "bing a queue with [logs_topic] to [" << name << "] and the bindKey is " << bindKey << "\n";
                });
          }
        }else{
          tool.GetChannel()->bindQueue("logs_topic", name, "mq_one.debug");
        }
        tool.GetChannel()->consume(name, AMQP::noack)
          .onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
              string data(message.body(), message.bodySize());
              cout << "[x] Received " << data << "\n";
              cout << "[routingkey] " << message.routingkey() << "\n";
              auto dataJson = json::parse(data);
              cout << "facility is " << dataJson["facility"].get<string>() << "\n";
              cout << "severity is " << dataJson["severity"].get<string>() << "\n";
              cout << "data is " << dataJson["data"].get<string>() << "\n";
            });
      });

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
