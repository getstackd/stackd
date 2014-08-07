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

#include "stackd/core.h"

namespace stackd
{
   Core* defaultCore()
   {
      return Core::default_core;
   }
      
   Core::Core()
   {
      if (Core::default_core == 0) Core::default_core = this;
      
      uv_loop = new uv_loop_t;
      uv_loop_init(uv_loop);
   }
   
   Core::~Core()
   {
      uv_loop_close(uv_loop);
      delete uv_loop;
   }
   
   void Core::activate()
   {
      Core::active = this;
   }
   
   void Core::deactivate()
   {
      Core::active = 0;
   }
   
   uv_loop_t* Core::loop()
   {
      return uv_loop;
   }
   
   int Core::run()
   {
      return uv_run(uv_loop, UV_RUN_DEFAULT);
   }
   
   uint32_t Core::uv_timers_index = 0;
   std::array<uv_timer_t, 2048> Core::uv_timers;
   std::unordered_map<uv_timer_t*, std::shared_ptr<coroutine_context>> Core::uv_binding;
   void Core::on_continuation(uv_timer_t *timer)
   {
      auto context = uv_binding[timer];
      bool terminated = context->resume();
      if (terminated) {
         uv_binding.erase(timer);
         uv_timer_stop(timer);
      }
   }
   
   Core* Core::active = 0;
   Core* Core::default_core = 0;
}