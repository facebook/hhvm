/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UUID_H_INCLUDED
#define UUID_H_INCLUDED 1

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <utility>

#include "template_utils.h"

namespace binary_log {

/**
  @struct  Uuid

  This is a POD.  It has to be a POD because it is a member of
  Sid_map::Node which is stored in HASH in mysql-server code.
  The structure contains the following components.
  <table>
  <caption>Structure gtid_info</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>byte</td>
    <td>unsigned char array</td>
    <td>This stores the Uuid of the server on which transaction
        is originated</td>
  </tr>
  </table>
*/

struct Uuid {
  /// Set to all zeros.
  void clear() { memset(bytes, 0, BYTE_LENGTH); }
  /// Copies the given 16-byte data to this UUID.
  void copy_from(const unsigned char *data) {
    memcpy(bytes, data, BYTE_LENGTH);
  }
  /// Copies the given UUID object to this UUID.
  void copy_from(const Uuid &data) {
    copy_from(pointer_cast<const unsigned char *>(data.bytes));
  }
  /// Copies the given UUID object to this UUID.
  void copy_to(unsigned char *data) const { memcpy(data, bytes, BYTE_LENGTH); }
  /// Returns true if this UUID is equal the given UUID.
  bool equals(const Uuid &other) const {
    return memcmp(bytes, other.bytes, BYTE_LENGTH) == 0;
  }
  /**
    Returns true if parse() would succeed, but doesn't store the result.

     @param string String that needs to be checked.
     @param len    Length of that string.

     @retval true  valid string.
     @retval false invalid string.
  */
  static bool is_valid(const char *string, size_t len);

  /**
    Stores the UUID represented by a string of the form
    XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX or
    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX or
    {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    in this object.

     @param string String to be parsed and stored.
     @param len    Length of that string.

     @retval   0   success.
     @retval  >0   failure.
  */
  int parse(const char *string, size_t len);

  /**
    Parses the UUID passed as argument in in_string and functions and writes
    the binary representation in out_binary_string.
    Depends on UUID's read_section method and the constants for text length.

     @param[in] in_string           String to be parsed.
     @param[in] len                 Length of that string.
     @param[out] out_binary_string  String where the binary UUID will be stored

     @retval   0   success.
     @retval  >0   failure.
  */
  static int parse(const char *in_string, size_t len,
                   const unsigned char *out_binary_string);
  /**
    Helper method used to validate and parse one section of a uuid.
    If the last parameter, out_binary_str, is NULL then the function will
    just validate the section.

     @param[in]      section_len      Length of the section to be parsed.
     @param[in,out]  section_str      Pointer to a string containing the
                                      section. It will be updated during the
                                      execution as the string is parsed.
     @param[out]     out_binary_str   String where the section will be stored
                                      in binary format. If null, the function
                                      will just validate the input string.

     @retval  false   success.
     @retval  true    failure.
  */
  static bool read_section(int section_len, const char **section_str,
                           const unsigned char **out_binary_str);
  /** The number of bytes in the data of a Uuid. */
  static const size_t BYTE_LENGTH = 16;
  /** The data for this Uuid. */
  unsigned char bytes[BYTE_LENGTH];
  /**
    Generates a 36+1 character long representation of this UUID object
    in the given string buffer.

    @retval 36 - the length of the resulting string.
  */
  size_t to_string(char *buf) const;
  /// Convert the given binary buffer to a UUID
  static size_t to_string(const unsigned char *bytes_arg, char *buf);
  void print() const {
    char buf[TEXT_LENGTH + 1];
    to_string(buf);
    printf("%s\n", buf);
  }
  /// The number of bytes in the textual representation of a Uuid.
  static const size_t TEXT_LENGTH = 36;
  /// The number of bits in the data of a Uuid.
  static const size_t BIT_LENGTH = 128;
  static const int NUMBER_OF_SECTIONS = 5;
  static const int bytes_per_section[NUMBER_OF_SECTIONS];
  static const int hex_to_byte[256];
};

struct Hash_Uuid {
  size_t operator()(const Uuid &uuid) const {
    return std::hash<std::string>()(
        std::string(pointer_cast<const char *>(uuid.bytes), Uuid::BYTE_LENGTH));
  }
};

inline bool operator==(const Uuid &a, const Uuid &b) { return a.equals(b); }

}  // namespace binary_log

#endif  // UUID_H_INCLUDED
