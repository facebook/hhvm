/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "event_reader.h"
#include <string>
#include "mysql_com.h"  // net_field_length_ll, net_field_length_size

namespace binary_log {

void Event_reader::set_error(const char *error) {
  BAPI_ASSERT(error != nullptr);
  BAPI_PRINT("debug", ("Event_reader::set_error(%s)", error));
  m_error = error;
}

void Event_reader::set_length(unsigned long long length) {
  PRINT_READER_STATUS("Event_reader::set_length");
  if (length < m_length) {
    BAPI_PRINT("debug", ("Event_reader::set_length(%llu)", length));
    set_error("Buffer length cannot shrink");
  } else {
    m_limit = m_length = length;
  }
}

void Event_reader::shrink_limit(unsigned long long bytes) {
  PRINT_READER_STATUS("Event_reader::shrink_limit");
  if (bytes > m_limit || position() > m_limit - bytes) {
    BAPI_PRINT("debug", ("Event_reader::shrink_limit(%llu)", bytes));
    set_error("Unable to shrink buffer limit");
  } else
    m_limit = m_limit - bytes;
}

const char *Event_reader::ptr(unsigned long long length) {
  PRINT_READER_STATUS("Event_reader::ptr");
  BAPI_PRINT("debug", ("Event_reader::ptr(%llu)", length));
  if (!can_read(length)) {
    set_error("Cannot point to out of buffer bounds");
    return nullptr;
  }
  const char *ret_ptr = m_ptr;
  m_ptr = m_ptr + length;
  return ret_ptr;
}

const char *Event_reader::go_to(unsigned long long position) {
  PRINT_READER_STATUS("Event_reader::go_to");
  if (position >= m_limit) {
    BAPI_PRINT("debug", ("Event_reader::go_to(%llu)", position));
    set_error("Cannot point to out of buffer bounds");
    return nullptr;
  }
  m_ptr = m_buffer + position;
  return m_ptr;
}

void Event_reader::alloc_and_memcpy(unsigned char **destination, size_t length,
                                    int flags) {
  PRINT_READER_STATUS("Event_reader::alloc_and_memcpy");
  if (!can_read(length)) {
    BAPI_PRINT("debug", ("Event_reader::alloc_and_copy(%zu)", length));
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  BAPI_ASSERT(*destination == nullptr);
  *destination = static_cast<unsigned char *>(bapi_malloc(length, flags));
  if (!*destination) {
    BAPI_PRINT("debug", ("Event_reader::alloc_and_copy(%zu)", length));
    set_error("Out of memory");
    return;
  }
  ::memcpy(*destination, m_ptr, length);
  m_ptr = m_ptr + length;
}

void Event_reader::alloc_and_strncpy(char **destination, size_t length,
                                     int flags) {
  PRINT_READER_STATUS("Event_reader::alloc_and_strncpy");
  if (!can_read(length)) {
    BAPI_PRINT("debug", ("Event_reader::alloc_and_strncpy(%zu)", length));
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  BAPI_ASSERT(*destination == nullptr);
  *destination = static_cast<char *>(bapi_malloc(length + 1, flags));
  if (!*destination) {
    BAPI_PRINT("debug", ("Event_reader::alloc_and_strncpy(%zu)", length));
    set_error("Out of memory");
    return;
  }
  strncpy(*destination, m_ptr, length);
  (*destination)[length] = '\0';
  m_ptr = m_ptr + length;
}

void Event_reader::read_str_at_most_255_bytes(const char **destination,
                                              uint8_t *lenght) {
  PRINT_READER_STATUS("Event_reader::read_str_at_most_255_bytes");
  if (!can_read(sizeof(uint8_t))) {
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  ::memcpy(lenght, m_ptr, sizeof(uint8_t));
  m_ptr = m_ptr + sizeof(uint8_t);

  BAPI_PRINT("debug",
             ("Event_reader::read_str_at_most_255_bytes(%u)", *lenght));
  if (!can_read(*lenght)) {
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  *destination = m_ptr;
  m_ptr = m_ptr + *lenght;
}

uint64_t Event_reader::net_field_length_ll() {
  PRINT_READER_STATUS("Event_reader::net_field_length_ll");
  if (!can_read(sizeof(uint8_t))) {
    set_error("Cannot read from out of buffer bounds");
    return 0;
  }
  // It is safe to read the first byte of the transaction_length
  unsigned char *ptr_length;
  ptr_length = reinterpret_cast<unsigned char *>(const_cast<char *>(m_ptr));
  unsigned int length_size = net_field_length_size(ptr_length);
  BAPI_PRINT("debug", ("Event_reader::read_net_field_length_ll(): "
                       "expect to read length with %u byte(s)",
                       length_size));
  if (!can_read(length_size)) {
    set_error("Cannot read from out of buffer bounds");
    return 0;
  }
  // It is safe to read the full transaction_length from the buffer
  uint64_t value = ::net_field_length_ll(&ptr_length);
  m_ptr = m_ptr + length_size;
  return value;
}

void Event_reader::read_data_set(uint32_t set_len,
                                 std::list<const char *> *set) {
  PRINT_READER_STATUS("Event_reader::read_data_set");
  uint16_t len = 0;
  for (uint32_t i = 0; i < set_len; i++) {
    len = read_and_letoh<uint16_t>();
    if (m_error) break;
    const char *hash = strndup<const char *>(len);
    if (m_error) break;
    set->push_back(hash);
  }
}

void Event_reader::read_data_map(uint32_t map_len,
                                 std::map<std::string, std::string> *map) {
  PRINT_READER_STATUS("Event_reader::read_data_map");
  BAPI_ASSERT(map->empty());
  for (uint32_t i = 0; i < map_len; i++) {
    uint16_t key_len = read_and_letoh<uint16_t>();
    if (m_error) break;
    if (!can_read(key_len)) {
      set_error("Cannot read from out of buffer bounds");
      break;
    }
    std::string key(m_ptr, key_len);
    m_ptr += key_len;

    uint32_t value_len = read_and_letoh<uint32_t>();
    if (m_error) break;
    if (!can_read(value_len)) {
      set_error("Cannot read from out of buffer bounds");
      break;
    }
    std::string value(m_ptr, value_len);
    m_ptr += value_len;

    (*map)[key] = value;
  }
}

void Event_reader::strncpyz(char *destination, size_t max_length,
                            size_t dest_length) {
  PRINT_READER_STATUS("Event_reader::strncpyz");
  if (!can_read(max_length)) {
    BAPI_PRINT("debug",
               ("Event_reader::strncpy(%zu, %zu)", max_length, dest_length));
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  strncpy(destination, m_ptr, max_length);
  destination[dest_length - 1] = 0;
  m_ptr = m_ptr + strlen(destination) + 1;
}

void Event_reader::assign(std::vector<uint8_t> *vector, size_t length) {
  PRINT_READER_STATUS("Event_reader::assign");
  BAPI_ASSERT(vector->empty());
  if (!can_read(length)) {
    set_error("Cannot read from out of buffer bounds");
    return;
  }
  try {
    vector->assign(m_ptr, m_ptr + length);
  } catch (const std::bad_alloc &e) {
    vector->clear();
    set_error("std::bad_alloc");
  }
  BAPI_ASSERT(vector->size() == length);
  m_ptr = m_ptr + length;
}
}  // end namespace binary_log
