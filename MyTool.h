#ifndef _MYTOOL_H
#define _MYTOOL_H

#include <memory>
#include <amqpcpp.h>
#include <ev.h>
#include <iostream>
#include "MyHandler.h"

class MyTool{
public:
  MyTool(struct ev_loop *loop){
    handlerPtr = make_shared<MyHandler>(loop);
    connectionPtr = make_shared<AMQP::TcpConnection>(handlerPtr.get(),
                                                     AMQP::Address("amqp://guest:guest@localhost/"));

    channelPtr = make_shared<AMQP::TcpChannel>(connectionPtr.get());

  }

  shared_ptr<AMQP::TcpConnection> GetConnection(){return connectionPtr;}
  shared_ptr<AMQP::TcpChannel> GetChannel(){return channelPtr;}
  void TraceError(int a){
    std::cout << a << "\n";
  }
private:
  shared_ptr<MyHandler> handlerPtr;
  shared_ptr<AMQP::TcpConnection> connectionPtr;
  shared_ptr<AMQP::TcpChannel> channelPtr;
  MyTool(const MyTool&);
  MyTool& operator=(const MyTool&);
};

#endif
