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

#ifndef COMPRESSION_NONE_INCLUDED
#define COMPRESSION_NONE_INCLUDED

#include "base.h"

namespace binary_log {
namespace transaction {
namespace compression {

/**
  This compressor does not compress. The only thing that it does
  is to copy the data from the input to the output buffer.
 */
class None_comp : public Compressor {
 public:
  None_comp() = default;

  /**
    No op member function.
   */
  virtual void set_compression_level(unsigned int compression_level) override;

  /**
    Shall get the compressor type code.

    @return the compressor type code.
   */
  virtual type compression_type_code() override;

  /**
    No op member function.

    @return false on success, true otherwise.
   */
  virtual bool open() override;

  /**
    This member function shall simply copy the input buffer to the
    output buffer. It shall grow the output buffer if needed.

    @param data a pointer to the buffer holding the data to compress
    @param length the size of the data to compress.

    @return false on success, true otherwise.
   */
  virtual std::tuple<std::size_t, bool> compress(const unsigned char *data,
                                                 size_t length) override;

  /**
    No op member function.

    @return false on success, true otherwise.
   */
  virtual bool close() override;
};

/**
  This decompressor does not decompress. The only thing that it
  does is to copy the data from the input to the output buffer.
 */
class None_dec : public Decompressor {
 private:
  None_dec &operator=(const None_dec &rhs) = delete;
  None_dec(const None_dec &) = delete;

 public:
  None_dec() = default;

  /**
    Shall return the compression type code.

    @return the compression type code.
   */
  virtual type compression_type_code() override;

  /**
    No op member function.

    @return false on success, true otherwise.
   */
  virtual bool open() override;

  /**
    This member function shall simply copy the input buffer to the
    output buffer. It shall grow the output buffer if needed.

    @param data a pointer to the buffer holding the data to decompress
    @param length the size of the data to decompress.

    @return false on success, true otherwise.
   */
  virtual std::tuple<std::size_t, bool> decompress(const unsigned char *data,
                                                   size_t length) override;

  /**
    No op member function.

    @return false on success, true otherwise.
   */
  virtual bool close() override;
};

}  // namespace compression
}  // namespace transaction
}  // namespace binary_log
#endif