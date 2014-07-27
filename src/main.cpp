//  Copyright (c) 2014 Justin Walsh.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <iostream>
#include <array>

#include "uv.h"
#include "http_parser.h"
#include "boost/context/all.hpp"

#include "stackd/stackd.h"

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

