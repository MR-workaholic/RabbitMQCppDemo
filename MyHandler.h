#ifndef _MYHANDLER_H
#define _MYHANDLER_H

#include <ev.h>
#include <amqpcpp/libev.h>
#include <iostream>
using namespace std;

// class AMQP::TcpConnection;
// class AMQP::LibEvHandler;

class MyHandler : public AMQP::LibEvHandler{
public:
  MyHandler(struct ev_loop *loop) : LibEvHandler(loop) {}
  virtual void onError(AMQP::TcpConnection *connection, const char* message){
    cout << "Error : " << message << "\n";
  }
private:
  MyHandler(const MyHandler& val);
  MyHandler& operator=(const MyHandler& val);
};

#endif
