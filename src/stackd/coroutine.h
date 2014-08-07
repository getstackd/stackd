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
#ifndef STACKD_COROUTINE_H_
#define STACKD_COROUTINE_H_

#include <boost/context/all.hpp>
#include <functional>
#include <tuple>
#include <array>
#include <memory>

namespace /* tuple sequence helper */
{
   template<int ...> struct seq {};
   template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
   template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };
}

namespace stackd
{
   // A movable context for mainting the state of each coroutine
   class coroutine
   {
      template<typename... Args> friend class coroutine_delegate;
      
   public:
      coroutine() : terminated(true), stack(nullptr), context(nullptr) {}
      ~coroutine() { delete stack; }
      coroutine(void* cptr, void (*handler)(intptr_t));
      coroutine(coroutine&& other);
      coroutine& operator=(coroutine&& other);
      
      coroutine(coroutine const&) = delete;
      coroutine& operator=(coroutine const&) = delete;
      
      bool active();
      bool resume();
      void yield(bool terminate = false);
      
   protected:
      struct coroutine_start
      {
         coroutine_start() : coroutine_delegate(0), context(nullptr) {};
         void* coroutine_delegate;
         coroutine* context;
      } start;
      
   private:
      bool terminated;
      std::array<intptr_t, 64 * 1024> *stack;
      boost::context::fcontext_t *context;
      boost::context::fcontext_t context_main;
   };
   
   // API for maintaining coroutine state
   extern void yield(bool terminate = false);
   namespace internal
   {
      struct unwind_coroutine {};
      extern coroutine* active_context;
   }
   
   // Coroutine with parameters and a delegate, wrapper for the delegate function to turn into a coroutine
   template<typename... Args>
   class coroutine_delegate
   {      
   public:
      coroutine_delegate() = default;
      coroutine_delegate(std::function<void(Args...)> delegate) : delegate(delegate) {};
      
      coroutine_delegate(coroutine_delegate&& other)
      {
         delegate = std::move(other.delegate);
         parameters = std::move(other.parameters);
      }
      
      coroutine_delegate& operator=(coroutine_delegate&& other)
      {
         if (this != &other)
         {
            delegate = std::move(other.delegate);
            parameters = std::move(other.parameters);
         }
         return *this;
      }
      
      std::shared_ptr<coroutine> operator()(Args... args)
      {
         parameters = std::make_tuple(args...);
         coroutine* context = new coroutine(this, &coroutine_delegate::dispatch);
         context->resume();
         return std::shared_ptr<coroutine>(context);
      }
      
   private:
      template<int ...S>
      void delegator(seq<S...>)
      {
         delegate(std::get<S>(parameters)...);
      }
      
      static void dispatch(intptr_t start)
      {
         auto self = (coroutine_delegate<Args...>*)((coroutine::coroutine_start*)(start))->coroutine_delegate;
         auto context = ((coroutine::coroutine_start*)(start))->context;
         try {
            self->delegator(typename gens<sizeof...(Args)>::type());
         } catch (internal::unwind_coroutine const&) {}
         
         context->yield(true);
      }
      
      std::function<void(Args...)> delegate;
      std::tuple<Args...> parameters;
   };
   
   // Late binding delegate to a member function
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
