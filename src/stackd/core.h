//
//  core.h
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#pragma once
#ifndef STACKD_CORE_H_
#define STACKD_CORE_H_

#include <boost/context/all.hpp>

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
      
   private:
      boost::context::fcontext_t context;
            
   private:
      friend Core *defaultCore();
      friend void yeild(bool);
      static Core *active;
      static Core *default_core;
   };
}

#endif
