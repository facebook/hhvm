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

#ifndef CODECS_BASE_INCLUDED
#define CODECS_BASE_INCLUDED

#include <utility>
#include "libbinlogevents/include/binary_log.h"

namespace binary_log {
namespace codecs {

/**
  This is the abstract and base class for binary log codecs.

  It defines the codec API. Implementations of this class must
  be stateless.
 */
class Codec {
 public:
  /**
    Member function that shall decode the contents of the given buffer into a
    binary log event.

    @param from the buffer containing the encoded event.
    @param size the length of the data in the buffer.
    @param to the binary log event to populate.

    @return a pair containing the amount of bytes decoded from the buffer and a
            boolean stating whether there was an error or not. False if no
            error, true otherwise.
  */
  virtual std::pair<std::size_t, bool> decode(const unsigned char *from,
                                              std::size_t size,
                                              Binary_log_event &to) const = 0;

  /**
    Member function that shall encode the contents of the given binary log event
    into an in memory buffer.

    @param from the binary log event to encode.
    @param to the buffer where the encoded event should be saved into.
    @param size the size of the buffer.

    @return a pair containing the amount of bytes encoded and whether there was
            an error or not. False if no error, true otherwise.
  */
  virtual std::pair<std::size_t, bool> encode(const Binary_log_event &from,
                                              unsigned char *to,
                                              std::size_t size) const = 0;
  virtual ~Codec() {}
};

}  // namespace codecs
}  // namespace binary_log

#endif
