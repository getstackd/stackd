//
//  error.h
//  stackd
//
//  Created by Justin Walsh (Axon) on 7/21/14.
//  Copyright (c) 2014 Justin Walsh. All rights reserved.
//

#pragma once
#ifndef STACKD_ERROR_H_
#define STACKD_ERROR_H_

namespace stackd
{
   struct error
   {
      int id;
      const char *message;
   };
}

#endif
