//
//  net-server.h
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#ifndef STACKD_SERVER_H_
#define STACKD_SERVER_H_

#include <iostream>
#include <functional>

#include "core.h"
#include "delegate.h"

namespace stackd
{
   class Net;
   
   // NetServer Delegate struct for maintaining function pointers to the proper handlers
   template <typename T>
   struct NetServerDelegate
   {
      delegate<T, int> listening;
      delegate<T, float> connection;
      delegate<T, const char *> close;
      delegate<T, const char *> error;
   };
   
   
   // NetServerInterface ensures we have the proper delegate wired into a class that want's to handle requests
   template <typename T>
   class NetServerInterface
   {
   public:
      virtual NetServerDelegate<T> delegate() = 0;
   };
   
   
   // Default implementation to rely on classic inheritance for defining your net server
   class NetServerListener : public NetServerInterface<NetServerListener>
   {
      virtual void listening(int) = 0;
      virtual void connection(float) = 0;
      virtual void close(const char *) = 0;
      virtual void error(const char *) = 0;
    
   public:
      NetServerDelegate<NetServerListener> delegate()
      {
         return  { &NetServerListener::listening, &NetServerListener::connection, &NetServerListener::close, &NetServerListener::error };
      }
   };
   
   // The net server class that controls server flow
   class NetServer
   {
   public:
      ~NetServer();
      void listen(int port);
      void listen(int port, int address);
      void close();
      
   protected:
      friend Net;
      NetServer(Core *core);
      
      template <class T>
      void bind(T* delegate, NetServerDelegate<T> delegates)
      {
         onListening = coroutine<int>(delegates.listening.bind(delegate));
         onConnecting = coroutine<float>(delegates.connection.bind(delegate));
         onClose = coroutine<const char *>(delegates.close.bind(delegate));
         onError = coroutine<const char *>(delegates.error.bind(delegate));
      };
      
   private:
      Core *core;
      coroutine<int> onListening;
      coroutine<float> onConnecting;
      coroutine<const char *> onClose;
      coroutine<const char *> onError;
   };
}

#endif
