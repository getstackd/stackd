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

#include "stackd/delegate.h"
#include <boost/context/all.hpp>

namespace stackd
{
   coroutine_context* internal::active_context = nullptr;
   void yield(bool terminate)
   {
      if (internal::active_context == nullptr) return;
      
      if (terminate) throw internal::unwind_coroutine();
      internal::active_context->yield(terminate);
   }
   
   coroutine_context::coroutine_context(void* cptr, void (*handler)(intptr_t))
   {
      terminated = false;
      start.coroutine = cptr;
      start.context = this;
      stack = new std::array<intptr_t, 64 * 1024>();
      context = boost::context::make_fcontext(stack->data() + stack->size(), stack->size(), handler);
   }
   
   coroutine_context::coroutine_context(coroutine_context&& other)
   {
      delete stack;
      stack = other.stack;
      other.stack = nullptr;
      
      delete context;
      context = std::move(other.context);
      other.context = nullptr;
      
      context_main = std::move(context_main);
      
      start.coroutine = other.start.coroutine;
      start.context = this;
      
      terminated = other.terminated;
   }
   
   coroutine_context& coroutine_context::operator=(coroutine_context&& other)
   {
      if (this != &other)
      {
         delete stack;
         stack = other.stack;
         other.stack = nullptr;
         
         delete context;
         context = std::move(other.context);
         other.context = nullptr;
         
         context_main = std::move(context_main);
         
         start.coroutine = other.start.coroutine;
         start.context = this;
         
         terminated = other.terminated;
      }
      return *this;
   }
   
   bool coroutine_context::active()
   {
      return !terminated;
   }
   
   bool coroutine_context::resume()
   {
      if (terminated) return terminated;
      
      internal::active_context = this;
      terminated = (bool)boost::context::jump_fcontext(&context_main, context, (intptr_t)&start);
      return terminated;
   }
   
   void coroutine_context::yield(bool terminate)
   {
      if (terminated) return;
      
      internal::active_context = nullptr;
      boost::context::jump_fcontext(context, &context_main, (intptr_t)terminate);
   }
}