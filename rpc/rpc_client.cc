/*
输入格式是：n m
n代表clientid，m代表para
 */
#include "../MyTool.h"
#include "../json.hpp"
#include <ev.h>
#include <ev++.h>
#include <iostream>
#include <amqpcpp.h>
#include <stdlib.h>             // exit()
#include <signal.h>             // signal()
#include <string>
#include <time.h>
#include <sys/time.h>           // gettimeofday()
#include <vector>
#include <memory>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;
using Envelope = AMQP::Envelope;
using Message = AMQP::Message;

class RpcClient;
typedef function<void(shared_ptr<RpcClient>)> cbfun;

static shared_ptr<RpcClient> rpcclient;
struct ev_loop* loop = EV_DEFAULT;
MyTool tool(loop);
ev_io stdin_watcher;
size_t cid, para;


void
sig_int(int signo){
  cout << "close the connection" << "\n";
  tool.GetConnection()->close();
  ev_break(EV_A_ EVBREAK_ALL);
  exit(0);
}

class RpcClient{
public:
  RpcClient(uint8_t id = 0):
    _clientid(id),
    _initFinished(false),
    _evtimer(loop)
  {
    // 声明topic交换机
    tool.GetChannel()->declareExchange("logs_topic", AMQP::topic);
    cout << "start new queue..."  << "\n";
    // 声明一个随机队列
    tool.GetChannel()->declareQueue(AMQP::exclusive)
      .onSuccess([this](const string &name, uint32_t messagecount, uint32_t consumercount) {
          cout << "queue name is " << name << "\n";
          this->_queueName = name;
          // 在topic下，还是需要绑定队列名字与消息键的
          tool.GetChannel()->bindQueue("logs_topic", this->_queueName, this->_queueName)
            .onSuccess([this]() {
                this->_initFinished = true;
              });


          tool.GetChannel()->consume(this->_queueName, AMQP::noack)
            .onReceived([this](const Message &message, uint64_t deliveryTag, bool redelivered) {
                if (_uuid != "" && _uuid == message.correlationID()) {
                  string data(message.body(), message.bodySize());
                  cout << "[x] Received " << data << " from routingkey " << message.routingkey() << "\n";
                  json responseJson = json::parse(data);
                  for(auto& item : responseJson["results"]) {
                    (this->_results).push_back(item);
                  }
                  // 合法的，但是内存会被释放，因为退出函数后引用计数是0，被释放；退出这个lamdba后再次释放 内存，出现double free or corruption错误
                  // (this->_func)(shared_ptr<RpcClient>(this));
                  // 解决方法是：（哈，还是不行，用this初始化智能指针不能提高原智能指针的引用计数）
                  // shared_ptr<RpcClient> tempHandler(this);
                  // (this->_func)(tempHandler);
                  //  解决办法：传入全局变量
                  (this->_func)(rpcclient);
                }else {
                  cout << "uuid wrong!!!"  << "\n";
                }
              });
        });

  }

  void recall(size_t para) {
    _uuid = to_string(static_cast<unsigned>(_clientid)) + to_string(GetMSTimestamp());
    _results.clear();
    _para = para;
    if (getInitFinished()) {
      publishreq(_para);
    }else {
      _evtimer.set<RpcClient, &RpcClient::timer_cb>(this);
      // 0.5秒后继续检查一次
      // _evtimer.set(0.5, 0.0);
      _evtimer.start(0.5, 0.0);
    }
  }

  void setclientid(uint8_t newid) {
    _clientid = newid;
  }

  void setcbfun(cbfun func) {
    _func = func;
  }

  vector<int> getresults() {
    return _results;
  }

  bool getInitFinished() {
    return _initFinished;
  }


  ~RpcClient(){
    cout << "delete RpcClient cid is " << static_cast<uint32_t>(_clientid) << "\n";
  }

  static void stdin_cb(EV_P_ ev_io *w, int revents);

private:
  uint8_t _clientid;
  string _uuid;
  string _queueName;
  vector<int> _results;
  cbfun _func;
  bool _initFinished;
  ev::timer _evtimer;
  size_t _para;

  unsigned long GetMSTimestamp()
  {
    struct timeval t_val;
    gettimeofday(&t_val, NULL);
    return t_val.tv_sec * 1000 + t_val.tv_usec / 1000; // 毫秒级别时间戳
  }

  void publishreq(size_t para) {
    json requestJson;
    requestJson.emplace("method", "Fibonacci");
    requestJson.emplace("para", para);

    // 一定要先将dump()保存到string变量上才执行envelope构造函数
    string reqStr = requestJson.dump();
    cout << "[x] Sent request " << reqStr << " uuid is " << _uuid << " queuename is " << _queueName << "\n";
    Envelope envelope(reqStr.data(), reqStr.size());
    envelope.setReplyTo(_queueName);
    envelope.setCorrelationID(_uuid);

    tool.GetChannel()->publish("logs_topic", "rpc_service", envelope);
  }

  void timer_cb(ev::timer &w, int revents) {
    if (getInitFinished()) {
      publishreq(_para);
      w.stop();
    }else {
      // 0.5秒后继续检查一次
      w.stop();
      w.start(0.5, 0.0);
    }
  }
};


void stdin_cb(EV_P_ ev_io *w, int revents) {
  // 获取参数
  cin >> cid >> para;

  if (cid != 0) {
    rpcclient = make_shared<RpcClient>(cid);
    // cout << rpcclient.use_count()  << "\n";

    // 自定义结果处理函数
    rpcclient->setcbfun([](shared_ptr<RpcClient> handler) {
        // cout << handler.use_count() << "\n";
        for(auto item : handler->getresults()) {
          cout << item  << " ";
        }
        cout << "\n";
      });
    // 延迟确保rpcclient初始化后才进行recall调用，不能使用挂起系统的同步定时函数，可以使用ev_timer
    // sleep(3);
    // rpcclient->recall(para);
    // ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 1.0);
    // ev_timer_start (loop, &timeout_watcher);
    // but 现在想将recall改成非阻塞的
    rpcclient->recall(para);

  }else {
    rpcclient->recall(para);
  }

}


int main(int argc, char *argv[])
{
  tool.GetChannel()->onError([](const char* message){
      cout << "channel error: " << message  << "\n";
    });

  // 下面的while循环是不对的，这是一个异步程序，不用试图用同步的思路去写代码，等待一个异步回调后才调用下一次的recall函数
  // 没有运行ev_run(loop, 0);这句的话，MQ相关的代码一句都不会执行，要run才能开始启动程序的
  // 因此采用从终端读入的思路去启动客户端吧
  // int n(10);
  // while (n--) {
  //   cout << "create a new client"  << "\n";


  //   // RpcClient rpcclient(n);
  //   cout << "start recall..." << rpcclient.use_count() << "\n";
  //   rpcclient->recall(10+n);
  //   // sleep(100);
  // }

  ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
  ev_io_start(loop, &stdin_watcher);

  signal(SIGINT, sig_int);
  ev_run(loop, 0);

  return 0;
}
