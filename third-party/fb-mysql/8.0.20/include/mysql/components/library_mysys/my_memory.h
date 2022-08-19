/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */
#ifndef COMPONENT_MEMORY_H
#define COMPONENT_MEMORY_H

/**
  @file components/library_mysys/my_memory.h
*/

#include "mysql/components/services/psi_memory_bits.h"

/**
  Below functions are used by the components. And these functions will
  be in a static library(liblibrary_mysys.a) and the library is statically
  linked whenever any component needs these function.
*/

/**
 Allocates size bytes of memory.

 @param key P_S key used for memory instrumentation
 @param size size bytes to allocate the memory
 @param flags used at the time of allocation
*/
extern "C" void *my_malloc(PSI_memory_key key, size_t size, int flags);

/**
 Frees the memory pointed by the ptr.

 @param ptr memory address to be freed
*/
extern "C" void my_free(void *ptr);
#endif  // COMPONENT_MEMORY_H
