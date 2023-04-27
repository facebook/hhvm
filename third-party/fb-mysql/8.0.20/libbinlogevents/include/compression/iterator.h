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

#ifndef COMPRESSION_ITERATOR_INCLUDED
#define COMPRESSION_ITERATOR_INCLUDED

#include <memory>
#include "libbinlogevents/include/control_events.h"
#include "libbinlogevents/include/event_reader.h"

namespace binary_log {
namespace transaction {
namespace compression {

/**
  This is a commodity iterator over a buffer containing binlog
  events inside. This buffer may be compressed, in which case,
  this class shall decompress the buffer itself and provide the
  iterator.

  If iterating over a compressed buffer, this iterator shall
  allocate memory internally to handle this. However, the ownership
  of this memory is never handed over to any external entity and
  as such, it is freed when this iterator is destroyed.
 */
class Iterable_buffer {
 private:
  using CompType = binary_log::transaction::compression::type;

 public:
  /**
     Iterable buffer over an input buffer.

     @param buffer input buffer
     @param buffer_size the input buffer size
   */
  Iterable_buffer(const char *buffer, size_t buffer_size);

  /**
     Iterable buffer over a compressed input buffer.

     @param buffer input buffer
     @param buffer_size the input buffer size
     @param uncompressed_size the size of the data uncompressed
     @param comp_algo the compression algorythm (e.g., ZSTD)
   */
  Iterable_buffer(const char *buffer, size_t buffer_size,
                  size_t uncompressed_size, CompType comp_algo);

  /**
    Deletes the iterator and frees any buffers allocated during
    while the iterator was in use.
   */
  virtual ~Iterable_buffer();

  Iterable_buffer(const Iterable_buffer &rhs) = delete;
  Iterable_buffer(Iterable_buffer &&rhs) = delete;
  Iterable_buffer &operator=(const Iterable_buffer &rhs) = delete;
  Iterable_buffer &operator=(Iterable_buffer &&rhs) = delete;

  /**
    Iterator class to allow iterating over a compressed log event buffer.

    In order to fully understand this class implementation, please, check the
    documentation on the Iterator concept requirements within the C++ standard
    and the STL definition.
   */
  class iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = const char *;
    using pointer = const char *;
    using reference = const char *;
    using iterator_category = std::forward_iterator_tag;

    explicit iterator(Iterable_buffer &parent);
    explicit iterator() = default;
    /**
      Copy constructor.

      @param rhs object instance we pretend to copy from.
     */
    iterator(const iterator &rhs);
    /**
      Move constructor.

      @param rhs object instance we pretend to move from.
     */
    iterator(iterator &&rhs);
    virtual ~iterator();

    iterator &operator=(iterator &&rhs);

    // BASIC ITERATOR METHODS //
    iterator &operator=(const iterator &rhs);
    iterator &operator++();
    reference operator*() const;
    // END / BASIC ITERATOR METHODS //
    // INPUT ITERATOR METHODS //
    iterator operator++(int);
    pointer operator->() const;
    bool operator==(iterator rhs) const;
    bool operator!=(iterator rhs) const;
    // END / INPUT ITERATOR METHODS //

    // OUTPUT ITERATOR METHODS //
    // reference operator*() const; <- already defined
    // iterator operator++(int); <- already defined
    // END / OUTPUT ITERATOR METHODS //
    // FORWARD ITERATOR METHODS //
    // Enable support for both input and output iterator <- already enabled
    // END / FORWARD ITERATOR METHODS //

   private:
    bool has_next_buffer() const;
    Iterable_buffer *m_target{nullptr};
    std::unique_ptr<binary_log::Event_reader> m_reader{nullptr};

    friend class Iterable_buffer;
  };

  iterator begin();
  iterator end();

  friend class iterator;

 private:
  const char *m_compressed_buffer{nullptr};
  size_t m_compressed_buffer_size{0};
  std::unique_ptr<Decompressor> m_decoder{nullptr};
  const char *m_decompressed_buffer{nullptr};
  size_t m_decompressed_buffer_size{0};
};

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log

#endif
