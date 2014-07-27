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
#ifndef STACKD_SERVER_H_
#define STACKD_SERVER_H_

#include <iostream>
#include <functional>

#include "stackd/core.h"
#include "stackd/delegate.h"

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
