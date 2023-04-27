/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <exception>

#include "my_thread.h"
#include "mysql/components/services/my_thread_bits.h"

namespace my_boost {

class thread {
 public:
  template <typename TCallable>
  thread(TCallable start) {
    context<TCallable> *new_context = new context<TCallable>(start);

    if (my_thread_create(&m_thread, nullptr, context<TCallable>::entry_point,
                         new_context)) {
      throw std::exception();
    }
  }

  void join();

 private:
  my_thread_handle m_thread;

  template <typename TCallable>
  class context {
   public:
    context(TCallable callable) : m_callable(callable) {}

    static void *entry_point(void *context_raw) {
      context *this_context = (context *)context_raw;
      this_context->m_callable();
      delete this_context;
      return nullptr;
    }

   private:
    TCallable m_callable;
  };
};

}  // namespace my_boost

#endif
