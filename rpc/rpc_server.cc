#include <iostream>
#include "../MyTool.h"
#include "../json.hpp"
#include <ev.h>
#include <amqpcpp.h>
#include <vector>
#include <stdlib.h>             // exit()
#include <signal.h>             // signal()
#include <string>

using namespace std;
using json = nlohmann::json;
using Envelope = AMQP::Envelope;
using Message = AMQP::Message;


struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);

void
sig_int(int signo){
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  ev_break(EV_A_ EVBREAK_ALL);
  exit(0);
}

class RpcServer{
public:
  RpcServer():
    _queueName("rpc_service")
  {
    cout << "create service..."  << "\n";
    tool.GetChannel()->declareExchange("logs_topic", AMQP::topic);
    tool.GetChannel()->declareQueue(_queueName);
    tool.GetChannel()->bindQueue("logs_topic", _queueName, _queueName);
    tool.GetChannel()->setQos(1);
  }

  void StartService() {
    cout << "start service..." << "\n";
    tool.GetChannel()->consume("rpc_service")
      .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
          // 获取请求数据
          cout << "begin handle request" << "\n";
          string data(message.body(), message.bodySize());
          cout << data << "\n";
          json dataJson = json::parse(data);
          size_t para = dataJson.at("para").get<size_t>();
          // 处理RPC请求
          vector<int> res = this->fibonacci(para);
          json rspJson;
          json resultsJson = json::array();
          for (auto item : res) {
            resultsJson.push_back(item);
          }
          rspJson.emplace("results", resultsJson);
          string rspStr = rspJson.dump();
          Envelope envelope(rspStr.c_str(), rspStr.size());
          envelope.setCorrelationID(message.correlationID());

          // 发送请求
          tool.GetChannel()->publish("logs_topic", message.replyTo(), envelope);
          cout << "[x] sent response: " << rspStr << " to queue:" << message.replyTo() << " uuid is:" << message.correlationID() << "\n";

          // 回复ACK
          cout << "finish rpc service" << "\n";
          tool.GetChannel()->ack(deliveryTag);
        });
  }

private:
  vector<int> fibonacci(size_t n) {
    vector<int> results;
    if (n == 1) {
      results.push_back(1);
    }else if (n == 2) {
      results.push_back(1);
      results.push_back(1);
    }else if (n > 2) {
      results.push_back(1);
      results.push_back(1);
      while (results.size() < n) {
        size_t len = results.size();
        results.push_back(results.at(len - 2) + results.at(len - 1));
      }
    }
    if (results.empty()) {
      results.push_back(0);
    }
    return results;
  }

  string _queueName;
};

int main(int argc, char *argv[])
{
   tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message << "\n";
    });

   RpcServer rpcserver;
   rpcserver.StartService();

   signal(SIGINT, sig_int);
   ev_run(loop, 0);

   return 0;
}
