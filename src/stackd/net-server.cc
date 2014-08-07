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

#include "stackd/net-server.h"

#include <iostream>
#include "uv.h"

namespace stackd
{
   NetServer::NetServer(Core *core) : core(core)
   {
      NetServer::uv_binding[(uv_handle_t*)&tcp_server] = this;
      uv_tcp_init(core->loop(), &tcp_server);
   }
   
   NetServer::~NetServer()
   {
      close();
   }
   
   
   void NetServer::listen(int port)
   {
      listen(port, 1234);
   }
   
   void NetServer::listen(int port, int address)
   {
      //struct sockaddr_in bind_addr;
      //uv_ip4_addr("0.0.0.0", port, &bind_addr);
      //uv_tcp_bind(&tcp_server, (sockaddr*)&bind_addr, 0);
      //int error = uv_listen((uv_stream_t*)&tcp_server, 128, NetServer::on_connection);
      
      core->execute<int>(&onListening, port);
   }
   
   void NetServer::close()
   {
      uv_close((uv_handle_t*)&tcp_server, NetServer::on_close);
      core->execute<const char*>(&onClose, "success");
   }
   
   std::unordered_map<uv_handle_t*, NetServer*> NetServer::uv_binding;
   void NetServer::on_connection(uv_stream_t *server, int status)
   {
      NetServer *net_server = uv_binding[(uv_handle_t*)server];
      //net_server->onNewConnection(server, status);
   }
   
   void NetServer::on_close(uv_handle_t *server)
   {
      NetServer *net_server = uv_binding[(uv_handle_t*)server];
   }
}