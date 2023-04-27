/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MEM_ROOT_DEQUE_H
#define MEM_ROOT_DEQUE_H

#include <deque>
#include <initializer_list>
#include <utility>

#include "sql/mem_root_allocator.h"

/// A utility for having an std::deque which stores its elements on a MEM_ROOT.
template <class T>
class mem_root_deque : public std::deque<T, Mem_root_allocator<T>> {
 private:
  using super = std::deque<T, Mem_root_allocator<T>>;

 public:
  explicit mem_root_deque(MEM_ROOT *mem_root)
      : super(Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(typename super::size_type count, const T &value,
                 MEM_ROOT *mem_root)
      : super(count, value, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(typename super::size_type count, MEM_ROOT *mem_root)
      : super(count, Mem_root_allocator<T>(mem_root)) {}

  template <class InputIt>
  mem_root_deque(InputIt first, InputIt last, MEM_ROOT *mem_root)
      : super(first, last, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(const mem_root_deque &other) : super(other) {}
  mem_root_deque(const mem_root_deque &other, MEM_ROOT *mem_root)
      : super(other, Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(mem_root_deque &&other) : super(std::move(other)) {}
  mem_root_deque(mem_root_deque &&other, MEM_ROOT *mem_root)
      : super(std::move(other), Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque(std::initializer_list<T> init, MEM_ROOT *mem_root)
      : super(std::move(init), Mem_root_allocator<T>(mem_root)) {}

  mem_root_deque &operator=(const mem_root_deque &arg) = default;
};

#endif  // MEM_ROOT_DEQUE_H
