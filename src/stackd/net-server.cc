//
//  net-server.cpp
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#include "net-server.h"
#include <iostream>

namespace stackd
{
   NetServer::NetServer(Core *core) : core(core)
   {
      
   }
   
   NetServer::~NetServer()
   {
      close();
   }
   
   
   void NetServer::listen(int port)
   {
      listen(port, 0);
   }
   
   void NetServer::listen(int port, int address)
   {
      onListening(port);
   }
   
   void NetServer::close()
   {
      onClose("success");
   }
}