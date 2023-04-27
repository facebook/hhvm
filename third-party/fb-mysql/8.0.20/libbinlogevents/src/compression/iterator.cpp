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

#include <byteorder.h>
#include <compression/factory.h>
#include <compression/iterator.h>

namespace binary_log {
namespace transaction {
namespace compression {

/* purecov: begin inspected */
Iterable_buffer::Iterable_buffer(const char *input_buffer,
                                 size_t input_buffer_size)
    : Iterable_buffer(input_buffer, input_buffer_size, input_buffer_size,
                      CompType::NONE) {}
/* purecov: end */

Iterable_buffer::Iterable_buffer(
    const char *input_buffer, size_t input_buffer_size,
    size_t decompressed_buffer_size,
    binary_log::transaction::compression::type comp_algo)
    : m_compressed_buffer(input_buffer),
      m_compressed_buffer_size(input_buffer_size),
      m_decompressed_buffer_size(decompressed_buffer_size) {
  if (comp_algo != CompType::NONE) {
    auto res{false};
    auto left{0};
    m_decoder = Factory::build_decompressor(comp_algo);
    auto ptr = (unsigned char *)malloc(m_decompressed_buffer_size);
    m_decoder->set_buffer((unsigned char *)ptr, m_decompressed_buffer_size);

    // We decompress everything in one go.
    m_decoder->open();
    std::tie(left, res) = m_decoder->decompress(
        (const unsigned char *)m_compressed_buffer, m_compressed_buffer_size);
    m_decoder->close();

    // Some error happened. Was not able to successfully decompress everything
    if (res || left > 0) {
      /* purecov: begin inspected */
      free(const_cast<char *>(m_decompressed_buffer));
      m_decompressed_buffer = nullptr;
      m_decompressed_buffer_size = 0;
      /* purecov: end */
    } else {
      // may have been realloc'ed in the decompressor
      std::tie(ptr, m_decompressed_buffer_size, std::ignore) =
          m_decoder->get_buffer();
      m_decompressed_buffer = (const char *)ptr;
    }
  } else {
    /* purecov: begin inspected */
    m_decompressed_buffer = m_compressed_buffer;
    m_decompressed_buffer_size = m_compressed_buffer_size;
    /* purecov: end */
  }
}

Iterable_buffer::~Iterable_buffer() {
  if (m_decompressed_buffer != m_compressed_buffer) {
    auto ptr{const_cast<char *>(m_decompressed_buffer)};
    free(ptr);
    m_decompressed_buffer = nullptr;
  }
}

Iterable_buffer::iterator::iterator(Iterable_buffer &parent)
    : m_target{&parent} {
  m_reader = std::make_unique<binary_log::Event_reader>(
      m_target->m_decompressed_buffer, m_target->m_decompressed_buffer_size);
  m_reader->go_to(0);
}

Iterable_buffer::iterator::iterator(const iterator &rhs) { (*this) = rhs; }
Iterable_buffer::iterator::iterator(iterator &&rhs) { (*this) = rhs; }

Iterable_buffer::iterator::~iterator() {}

Iterable_buffer::iterator &Iterable_buffer::iterator::operator=(
    const Iterable_buffer::iterator &rhs) {
  m_target = rhs.m_target;
  if (rhs.m_reader != nullptr) {
    m_reader = std::make_unique<binary_log::Event_reader>(
        m_target->m_decompressed_buffer, m_target->m_decompressed_buffer_size);
    m_reader->go_to(rhs.m_reader->position());
  }
  return (*this);
}

/* purecov: begin inspected */
Iterable_buffer::iterator &Iterable_buffer::iterator::operator=(
    Iterable_buffer::iterator &&rhs) {
  m_target = rhs.m_target;
  m_reader.swap(rhs.m_reader);

  rhs.m_target = nullptr;

  return (*this);
}
/* purecov: end */

Iterable_buffer::iterator &Iterable_buffer::iterator::operator++() {
  // advance the previous buffer length
  if (has_next_buffer()) {
    auto ptr = m_reader->ptr();
    m_reader->forward(EVENT_LEN_OFFSET);
    uint32_t event_len = m_reader->read_and_letoh<uint32_t>();
    m_reader->go_to((ptr - m_reader->buffer()) + event_len);
  }

  // now check again if we have reached the end
  if (!has_next_buffer()) m_reader.reset(nullptr);

  return (*this);
}

Iterable_buffer::iterator::reference Iterable_buffer::iterator::operator*()
    const {
  if (has_next_buffer()) return m_reader->ptr();
  return nullptr; /* purecov: inspected */
}
/* purecov: begin inspected */
Iterable_buffer::iterator Iterable_buffer::iterator::operator++(int) {
  Iterable_buffer::iterator to_return = (*this);
  ++(*this);
  return to_return;
}
/* purecov: end */

/* purecov: begin inspected */
Iterable_buffer::iterator::pointer Iterable_buffer::iterator::operator->()
    const {
  return this->operator*();
}
/* purecov: end */

bool Iterable_buffer::iterator::operator==(
    Iterable_buffer::iterator rhs) const {
  return m_reader == rhs.m_reader ||
         (m_reader != nullptr && rhs.m_reader != nullptr &&
          m_reader->position() == rhs.m_reader->position());
}

bool Iterable_buffer::iterator::operator!=(
    Iterable_buffer::iterator rhs) const {
  return !((*this) == rhs);
}

bool Iterable_buffer::iterator::has_next_buffer() const {
  if (m_reader->has_error() ||
      !m_reader->can_read(LOG_EVENT_MINIMAL_HEADER_LEN))
    return false;

  return true;
}

Iterable_buffer::iterator Iterable_buffer::begin() {
  Iterable_buffer::iterator begin{*this};
  if (begin.has_next_buffer())
    return begin;
  else
    return Iterable_buffer::iterator{}; /* purecov: inspected */
}

Iterable_buffer::iterator Iterable_buffer::end() {
  return Iterable_buffer::iterator{};
}

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
