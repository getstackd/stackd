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
#ifndef STACKD_DELEGATE_H_
#define STACKD_DELEGATE_H_

#include <boost/context/all.hpp>
#include <functional>
#include <tuple>
#include <array>

#include "stackd/core.h"


namespace /* tuple sequence helper */
{
   template<int ...> struct seq {};
   template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
   template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };
}

namespace stackd
{
   namespace /* internal */
   {
      struct unwind_coroutine {};
      
      class yeild_coroutine
      {
      public:
         yeild_coroutine() : context_main(nullptr), context_active(nullptr) {}
         yeild_coroutine(boost::context::fcontext_t *context_main, boost::context::fcontext_t *context_active)
            : context_main(context_main), context_active(context_active) {}
         
         void operator()(bool terminate = false)
         {
            terminate ? throw unwind_coroutine() : boost::context::jump_fcontext(context_active, context_main, (intptr_t)terminate);
         }
         
      private:
         boost::context::fcontext_t *context_main;
         boost::context::fcontext_t *context_active;
      };
   }
   
   static yeild_coroutine *coroutine_yielder = nullptr;
   inline void yield(bool terminate = false) { terminate ? throw unwind_coroutine() : (*coroutine_yielder)(); }
   
   
   template<typename... Args>
   class coroutine
   {
      friend Core;
      
   public:
      coroutine() : stack(nullptr), context(nullptr), context_main(nullptr) {}
      ~coroutine() { delete stack; delete context_main; }
      coroutine(std::function<void(Args...)> delegate) : delegate(delegate)
      {
         stack = new std::array<intptr_t, 64*1024>();
         context = boost::context::make_fcontext(stack->data() + stack->size(), stack->size(), &coroutine::dispatch);
         context_main = new boost::context::fcontext_t();
         yielder = yeild_coroutine(context_main, context);
      }
      
      coroutine& operator=(coroutine&& other)
      {
         delegate = std::move(other.delegate);
         parameters = std::move(other.parameters);
         
         stack = other.stack;
         other.stack = nullptr;
         
         context = other.context;
         other.context = nullptr;
         
         context_main = other.context_main;
         other.context_main = nullptr;
         
         yielder = std::move(other.yielder);
         
         return *this;
      }
      
      bool operator()(Args... args)
      {
         parameters = std::make_tuple(args...);
         return call();
      }
      
   private:
      inline bool call()
      {
         coroutine_yielder = &yielder;
         return (bool) boost::context::jump_fcontext(context_main, context, (intptr_t)this);
      }
      
      inline void yield(bool terminate = false)
      {
         coroutine_yielder = nullptr;
         boost::context::jump_fcontext(context, context_main, (intptr_t)terminate);
      }
      
      template<int ...S>
      void delegator(seq<S...>)
      {
         delegate(std::get<S>(parameters)...);
      }
      
      static void dispatch(intptr_t coroutine_ptr)
      {
         coroutine *self = (coroutine*)coroutine_ptr;
         try {
            coroutine_yielder = &self->yielder;
            self->delegator(typename gens<sizeof...(Args)>::type());
         } catch (unwind_coroutine const&) {}
         
         self->yield(true);
      }
      
      std::function<void(Args...)> delegate;
      std::tuple<Args...> parameters;
      
      boost::context::fcontext_t *context;
      boost::context::fcontext_t *context_main;
      std::array<intptr_t, 64*1024> *stack;
      
      yeild_coroutine yielder;
   };
   
   template <class T, typename... Args>
   class delegate
   {
   public:
      typedef void (T::*Member)(Args...);
      delegate(Member function) : callable(function) {};
      
      std::function<void(Args...)> bind(void* instance)
      {
         Member function = callable;
         return [instance, function](Args... args) {
            T* p = static_cast<T*>(instance);
            return (p->*function)(args...);
         };
      }
      
      Member callable;
   };
}

#endif
