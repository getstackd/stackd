//
//  net.h
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

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
