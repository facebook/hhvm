/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <compression/none.h>
#include <string.h>

namespace binary_log {
namespace transaction {
namespace compression {

/*
   ****************************************************************
   Compressor
   ****************************************************************
 */

void None_comp::set_compression_level(unsigned int) {} /* purecov: inspected */

type None_comp::compression_type_code() {
  return NONE; /* purecov: inspected */
}

bool None_comp::open() { return false; } /* purecov: inspected */

bool None_comp::close() { return false; } /* purecov: inspected */

std::tuple<std::size_t, bool> None_comp::compress(const unsigned char *buffer,
                                                  std::size_t length) {
  /* purecov: begin inspected */
  auto min_capacity = length + (m_buffer_cursor - m_buffer);
  if (min_capacity > m_buffer_capacity)
    if (reserve(min_capacity)) std::make_tuple(length, true);

  memcpy(m_buffer_cursor, buffer, length);
  m_buffer_cursor += length;
  return std::make_tuple(0, false);
  /* purecov: end */
}

/*
   ****************************************************************
   Decompressor
   ****************************************************************
 */

type None_dec::compression_type_code() { return NONE; } /* purecov: inspected */

bool None_dec::open() { return false; } /* purecov: inspected */

bool None_dec::close() { return false; } /* purecov: inspected */

std::tuple<std::size_t, bool> None_dec::decompress(const unsigned char *buffer,
                                                   std::size_t length) {
  /* purecov: begin inspected */
  auto min_capacity = length + (m_buffer_cursor - m_buffer);
  if (min_capacity > m_buffer_capacity)
    if (reserve(min_capacity)) std::make_tuple(length, true);

  memcpy(m_buffer_cursor, buffer, length);
  m_buffer_cursor += length;
  return std::make_tuple(0, false);
  /* purecov: end */
}

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
