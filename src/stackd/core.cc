//
//  core.cpp
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#include "core.h"

namespace stackd
{
   Core* defaultCore()
   {
      return Core::default_core;
   }
   
   void yeild(bool terminate)
   {
      if (Core::active == 0) return;
   }
   
   Core::Core()
   {
      if (Core::default_core == 0) Core::default_core = this;
   }
   
   Core::~Core()
   {
      
   }
   
   void Core::activate()
   {
      Core::active = this;
   }
   
   void Core::deactivate()
   {
      Core::active = 0;
   }
   
   Core* Core::active = 0;
   Core* Core::default_core = 0;
}