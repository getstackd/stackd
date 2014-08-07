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
#ifndef STACKD_CORE_H_
#define STACKD_CORE_H_

#include <list>
#include <unordered_map>
#include "uv.h"
#include "stackd/coroutine.h"

namespace stackd
{
   // Public state management
   class Core;
   Core* defaultCore();
   
   // The main context and memory manager
   class Core
   {
   public:
      Core();
      ~Core();
      
      void activate();     // Call to activate the stack snapshot
      void deactivate();   // Release the stack snapshot
      
      template<typename... Args>
      void execute(coroutine_delegate<Args...>* continuation, Args... args)
      {
         auto context = (*continuation)(args...);
         if (context->active()) {
            uv_timer_t *timer = &uv_timers[uv_timers_index++];
            uv_timer_init(uv_loop, timer);
            uv_timer_start(timer, on_continuation, 0, 1);
            uv_binding[timer] = context;
         }
      }
      
      uv_loop_t* loop();
      int run();
      
   protected:
      static uint32_t uv_timers_index;
      static std::array<uv_timer_t, 2048> uv_timers;
      static std::unordered_map<uv_timer_t*, std::shared_ptr<coroutine>> uv_binding;
      static void on_continuation(uv_timer_t *timer);
      
   private:
      uv_loop_t *uv_loop;
      
      friend Core *defaultCore();
      static Core *active;
      static Core *default_core;
   };
}

#endif
