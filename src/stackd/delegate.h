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
   class coroutine_context
   {
      template<typename... Args> friend class coroutine;
      
   public:
      coroutine_context() : terminated(true), stack(nullptr), context(nullptr) {}
      ~coroutine_context() { delete stack; }
      coroutine_context(void* cptr, void (*handler)(intptr_t));
      coroutine_context(coroutine_context&& other);
      coroutine_context& operator=(coroutine_context&& other);
      
      coroutine_context(coroutine_context const&) = delete;
      coroutine_context& operator=(coroutine_context const&) = delete;
      
      bool active();
      bool resume();
      void yield(bool terminate = false);
      
   protected:
      struct coroutine_start
      {
         coroutine_start() : coroutine(0), context(nullptr) {};
         void* coroutine;
         coroutine_context* context;
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
      extern coroutine_context* active_context;
   }
   
   // Coroutine with parameters and a delegate, wrapper for the delegate function to turn into a coroutine
   template<typename... Args>
   class coroutine
   {      
   public:
      coroutine() = default;
      coroutine(std::function<void(Args...)> delegate) : delegate(delegate) {};
      
      coroutine(coroutine&& other)
      {
         delegate = std::move(other.delegate);
         parameters = std::move(other.parameters);
      }
      
      coroutine& operator=(coroutine&& other)
      {
         if (this != &other)
         {
            delegate = std::move(other.delegate);
            parameters = std::move(other.parameters);
         }
         return *this;
      }
      
      std::shared_ptr<coroutine_context> operator()(Args... args)
      {
         parameters = std::make_tuple(args...);
         coroutine_context* context = new coroutine_context(this, &coroutine::dispatch);
         context->resume();
         return std::shared_ptr<coroutine_context>(context);
      }
      
   private:
      template<int ...S>
      void delegator(seq<S...>)
      {
         delegate(std::get<S>(parameters)...);
      }
      
      static void dispatch(intptr_t start)
      {
         auto self = (coroutine<Args...>*)((coroutine_context::coroutine_start*)(start))->coroutine;
         auto context = ((coroutine_context::coroutine_start*)(start))->context;
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
