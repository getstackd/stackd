//
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

#pragma once
#ifndef STACKED_NET_H_
#define STACKED_NET_H_

#include "stackd/core.h"
#include "stackd/net-server.h"
#include "stackd/error.h"

#include <memory>

namespace stackd
{
   class Net
   {
   public:
      Net(Core *core = defaultCore()) : core(core) {};
      ~Net() {};
   
      template <typename T>
      std::unique_ptr<NetServer> createServer(NetServerInterface<T> *interface)
      {
         NetServer *server = new NetServer(core);
         server->bind<T>((T*)interface, interface->delegate());
         return std::unique_ptr<NetServer>( server );
      };
      
      std::unique_ptr<NetServer> createServer(NetServerListener *interface)
      {
         return createServer<NetServerListener>((NetServerInterface<NetServerListener>*)interface);
      }
      
   private:
      Core *core;
   };
}

#endif
