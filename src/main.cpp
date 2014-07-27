//
//  main.cpp
//  stacked
//
//  Created by Justin Walsh (Axon) on 7/20/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#include <iostream>
#include <array>

#include "uv.h"
#include "http_parser.h"
#include "boost/context/all.hpp"

#include "stackd.h"
#include "delegate.h"

boost::context::fcontext_t og, *coro;
void test(intptr_t param)
{
   int i = 0;
   
   std::cout << "Do Work" << ++i << std::endl;
   boost::context::jump_fcontext(coro, &og, 0);
   std::cout << "Do Work" << ++i << std::endl;
   boost::context::jump_fcontext(coro, &og, 0);
}

class SocketServer : public stackd::NetServerInterface<SocketServer>
{
public:
   stackd::NetServerDelegate<SocketServer> delegate()
   {
      return stackd::NetServerDelegate<SocketServer> { &SocketServer::listening, &SocketServer::connecting, &SocketServer::close, &SocketServer::error };
   }

private:
   void listening(int x) { std::cout << "SocketServer listening on " << x << std::endl; }
   void connecting(float x) { std::cout << "SocketServer connecting" << std::endl; }
   void close(const char * msg) { std::cout << "SocketServer close = " << msg << std::endl; }
   void error(const char * msg) { std::cout << "SocketServer error = " << msg << std::endl; }
};

class StandardNetServer : public stackd::NetServerListener
{
private:
   void listening(int x) { std::cout << "NetServer listening" << std::endl; }
   void connection(float x) { std::cout << "NetServer connecting"<< std::endl; }
   void close(const char *) { std::cout << "NetServer close"<< std::endl; }
   void error(const char *) { std::cout << "NetServer error"<< std::endl; }
};

int main(int argc, const char * argv[])
{
   //std::array<intptr_t, 64*1024> stack;
   //coro = boost::context::make_fcontext(stack.data() + stack.size(), stack.size(), &test);
   //boost::context::jump_fcontext(&og, coro, 0);
   //std::cout << "Again..." << std::endl;
   //boost::context::jump_fcontext(&og, coro, 0);
   //boost::context::jump_fcontext(&og, coro, 0);
   
   stackd::Core core;
   stackd::Net net;
   
   SocketServer serverDelegate;
   auto server = net.createServer<SocketServer>(&serverDelegate);
   server->listen(1234);
   
   StandardNetServer netServer;
   server = net.createServer(&netServer);
   server->listen(8080);
   
   return 0;
}

