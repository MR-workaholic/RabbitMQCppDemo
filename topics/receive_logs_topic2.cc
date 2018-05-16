#include "../MyTool.h"
#include "../json.hpp"
#include <ev.h>
#include <signal.h>             // signal()
#include <stdlib.h>             // exit()
#include <string>
#include <iostream>
#include <fstream>
#include <amqpcpp.h>

using namespace std;
using json = nlohmann::json;
const int MAXSIZE = 1024;

struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);

typedef struct
{
  uint8_t a;
  uint8_t b;
  char c[MAXSIZE];

}dataType;


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

  ifstream jsonFile("receive_logs_topic2_key.json");
  json keys;
  jsonFile >> keys;
  cout << setw(4) << keys << "\n";
  json keys1 = keys["keys1"];
  json keys2 = keys["keys2"];

  tool.GetChannel()->declareExchange("logs_topic", AMQP::topic);
  tool.GetChannel()->declareQueue(AMQP::exclusive)
    .onSuccess([&](const string& name, uint32_t messagecount, uint32_t consumercount) {
        if (keys1.size() > 0) {
          for (json::iterator i = keys1.begin(); i != keys1.end(); i++) {
            string bindKey = (*i).get<string>();
            // cout << name << "\n";
            tool.GetChannel()->bindQueue("logs_topic", name, bindKey)
              .onSuccess([name, bindKey](){
                  cout << "bing a queue with [logs_topic] to [" << name << "] and the bindKey is " << bindKey << "\n";
                });
          }
        }else{
          tool.GetChannel()->bindQueue("logs_topic", name, "server_one.error");
        }
        tool.GetChannel()->consume(name, AMQP::noack)
          .onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
              string data(message.body(), message.bodySize());
              cout << "[HDR1] Received " << data << "\n";
              cout << "[routingkey] " << message.routingkey() << "\n";
              auto dataJson = json::parse(data);
              cout << "facility is " << dataJson["facility"].get<string>() << "\n";
              cout << "severity is " << dataJson["severity"].get<string>() << "\n";
              cout << "data is " << dataJson["data"].get<string>() << "\n";
              dataType tempStruct;
              memcpy(&tempStruct, dataJson["data"].get<string>().c_str(), sizeof(dataType));
              cout << "a is " << (uint16_t)tempStruct.a << "\n";
              cout << "b is " << (uint16_t)tempStruct.b << "\n";
              cout << tempStruct.c << "\n";
            });
      });

  tool.GetChannel()->declareQueue(AMQP::exclusive)
    .onSuccess([&](const string& name, uint32_t messagecount, uint32_t consumercount){
        if (keys2.size() > 0) {
          for (json::iterator i = keys2.begin(); i != keys2.end(); i++) {
            string bindKey = (*i).get<string>();
            // cout << name << "\n";
            tool.GetChannel()->bindQueue("logs_topic", name, bindKey)
              .onSuccess([name, bindKey](){
                  cout << "bing a queue with [logs_topic] to [" << name << "] and the bindKey is " << bindKey << "\n";
                });
          }
        }else{
          tool.GetChannel()->bindQueue("logs_topic", name, "mq_one.error");
        }
        tool.GetChannel()->consume(name, AMQP::noack)
          .onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
              string data(message.body(), message.bodySize());
              cout << "[HDR2] Received " << data << "\n";
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
