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

#include <compression/base.h>
#include <string>

namespace binary_log {
namespace transaction {
namespace compression {

std::string type_to_string(type t) {
  std::string res;
  switch (t) {
    case binary_log::transaction::compression::type::ZSTD:
      res = "ZSTD";
      break;
    case binary_log::transaction::compression::type::NONE:
      res = "NONE";
      break;
    /* purecov: begin inspected */
    default:
      res = "N/A";
      break;
      /* purecov: end */
  }

  return res;
}

Base_compressor_decompressor::Base_compressor_decompressor() = default;

Base_compressor_decompressor::~Base_compressor_decompressor() = default;

bool Base_compressor_decompressor::set_buffer(unsigned char *buffer,
                                              std::size_t capacity) {
  m_buffer = m_buffer_cursor = buffer;
  m_buffer_capacity = capacity;
  return false;
}

std::tuple<unsigned char *, std::size_t, std::size_t>
Base_compressor_decompressor::get_buffer() {
  std::size_t ret_size = size();
  return std::tuple<unsigned char *, std::size_t, std::size_t>(
      m_buffer, ret_size, m_buffer_capacity);
}

std::size_t Base_compressor_decompressor::size() {
  return (m_buffer_cursor - m_buffer);
}

std::size_t Base_compressor_decompressor::capacity() {
  return m_buffer_capacity;
}

bool Base_compressor_decompressor::reserve(std::size_t bytes) {
  if ((bytes + size()) >= m_buffer_capacity) {
    unsigned int needed_blocks = (bytes / BLOCK_BYTES) + 1;
    unsigned int curr_blocks = (m_buffer_capacity / BLOCK_BYTES) + 1;
    std::size_t old_data_size = size();
    std::size_t new_capacity = (curr_blocks + needed_blocks) * BLOCK_BYTES;
    m_buffer = (unsigned char *)realloc(m_buffer, new_capacity);
    // update the cursor variables
    m_buffer_cursor = m_buffer + old_data_size;
    m_buffer_capacity = new_capacity;
  }

  // out of memory ?
  if (m_buffer == nullptr) return true;

  return false;
}

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
