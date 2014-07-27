//
//  delegate.h
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/22/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

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
   namespace internal {
      struct unwind_coroutine {}; // TODO: Need to throw exception to unwind the stack
      
      class yeild_coroutine
      {
      public:
         yeild_coroutine() : context_main(nullptr), context_active(nullptr) {}
         yeild_coroutine(boost::context::fcontext_t *context_main, boost::context::fcontext_t *context_active)
            : context_main(context_main), context_active(context_active) {}
         
         void operator()(bool terminate = false)
         {
            boost::context::jump_fcontext(context_active, context_main, (intptr_t)terminate);
         }
         
      private:
         boost::context::fcontext_t *context_main;
         boost::context::fcontext_t *context_active;
      };
      
      // TODO: Resuming a context requires that the pointer maintains its type, otherwise the tuple shits on itself
      /*
      class resume_coroutine
      {
      public:
         resume_coroutine() : coroutine_ptr(nullptr), context_main(nullptr), context_active(nullptr) {}
         resume_coroutine(void* coroutine_ptr, boost::context::fcontext_t *context_main, boost::context::fcontext_t *context_active)
         : coroutine_ptr(coroutine_ptr), context_main(context_main), context_active(context_active) {}
         
         bool operator()()
         {
            return (bool) boost::context::jump_fcontext(context_main, context_active, (intptr_t)coroutine_ptr);
         }
         
      private:
         void *coroutine_ptr;
         boost::context::fcontext_t *context_main;
         boost::context::fcontext_t *context_active;
      };
       */
   }
   
   static internal::yeild_coroutine *coroutine_yielder = nullptr;
   inline void yield(bool terminate = false) { if(coroutine_yielder) (*coroutine_yielder)(terminate); }
   
   // Resume may be reserved for internal use only
   //static internal::resume_coroutine *coroutine_resumer = nullptr;
   //inline void resume() { if (coroutine_resumer) (*coroutine_resumer)(); }
   
   
   template<typename... Args>
   class coroutine
   {
   public:
      coroutine() : stack(nullptr), context(nullptr), context_main(nullptr) {}
      ~coroutine() { delete stack; delete context_main; }
      coroutine(std::function<void(Args...)> delegate) : delegate(delegate)
      {
         stack = new std::array<intptr_t, 64*1024>();
         context = boost::context::make_fcontext(stack->data() + stack->size(), stack->size(), &coroutine::dispatch);
         context_main = new boost::context::fcontext_t();
         yielder = internal::yeild_coroutine(context_main, context);
         //resumer = internal::resume_coroutine((void*)this, context_main, context);
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
         //resumer = std::move(other.resumer);
         
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
         //coroutine_resumer = nullptr;
         //return resumer();
         return (bool) boost::context::jump_fcontext(context_main, context, (intptr_t)this);
      }
      
      inline void yield(bool terminate = false)
      {
         coroutine_yielder = nullptr;
         //coroutine_resumer = &resumer;
         yielder(terminate);
         //boost::context::jump_fcontext(context, context_main, (intptr_t)terminate);
      }
      
      template<int ...S>
      void delegator(seq<S...>)
      {
         delegate(std::get<S>(parameters)...);
      }
      
      static void dispatch(intptr_t coroutine_ptr)
      {
         coroutine *self = (coroutine*)coroutine_ptr;
         coroutine_yielder = &self->yielder;
         //coroutine_resumer = &self->resumer;
         
         self->delegator(typename gens<sizeof...(Args)>::type());
         self->yield(true);
      }
      
      std::function<void(Args...)> delegate;
      std::tuple<Args...> parameters;
      
      boost::context::fcontext_t *context;
      boost::context::fcontext_t *context_main;
      std::array<intptr_t, 64*1024> *stack;
      
      internal::yeild_coroutine yielder;
      //internal::resume_coroutine resumer;
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
