/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/properties.h"

#include <limits>

#include "m_string.h"  // my_strtoll10
#include "my_dbug.h"
#include "my_sys.h"              // strmake_root
#include "sql/dd/types/table.h"  // enum_row_format
#include "sql/field.h"           // geometry_type
#include "sql/handler.h"         // row_type

namespace dd {

template <typename T>
bool Properties::from_str(const String_type &number, T *value) {
  DBUG_ASSERT(value != nullptr);

  // The target type must be an integer.
  if (!(std::numeric_limits<T>::is_integer)) return true;

  // Do the conversion to an 8 byte signed integer.
  int error_code;
  int64 tmp = 0;
  tmp = my_strtoll10(number.c_str(), nullptr, &error_code);

  // Check for conversion errors, including boundaries for 8 byte integers.
  if (error_code != 0 && error_code != -1) return true;

  // Signs must match.
  if (error_code == -1 && !std::numeric_limits<T>::is_signed) return true;

  // Overflow if positive source, negative result and signed target type.
  if (error_code == 0 && tmp < 0 && std::numeric_limits<T>::is_signed)
    return true;

  // If the target type is less than 8 bytes, check boundaries.
  int64 trg_min = static_cast<int64>(std::numeric_limits<T>::min());
  int64 trg_max = static_cast<int64>(std::numeric_limits<T>::max());
  if (sizeof(T) < 8 && (tmp < trg_min || tmp > trg_max)) return true;

  // Finally, cast to target type.
  *value = static_cast<T>(tmp);
  return false;
}

bool Properties::from_str(const String_type &bool_str, bool *value) {
  DBUG_ASSERT(value != nullptr);

  if (bool_str == "true") {
    *value = true;
    return false;
  }

  if (bool_str == "false" || bool_str == "0") {
    *value = false;
    return false;
  }

  // We already tested for "0" above. Now, check if invalid number.
  uint64 tmp_uint64 = 0;
  int64 tmp_int64 = 0;
  if (from_str(bool_str, &tmp_uint64) && from_str(bool_str, &tmp_int64))
    return true;

  // Valid number, signed or unsigned, != 0 => interpret as true.
  *value = true;

  return false;
}

template <class T>
String_type Properties::to_str(T value) {
  Stringstream_type ostream;
  ostream << value;
  return ostream.str();
}

template <typename Lex_type>
bool Properties::get(const String_type &key, Lex_type *value,
                     MEM_ROOT *mem_root) const {
  DBUG_ASSERT(value != nullptr);
  DBUG_ASSERT(mem_root != nullptr);

  String_type str;
  if (get(key, &str)) return true;

  value->length = str.length();
  value->str =
      static_cast<char *>(strmake_root(mem_root, str.c_str(), str.length()));
  return false;
}

template <typename Value_type>
bool Properties::get(const String_type &key, Value_type *value) const {
  String_type str;
  if (get(key, &str)) return true;

  if (from_str(str, value)) {
    DBUG_ASSERT(false); /* purecov: inspected */
    return true;
  }
  return false;
}

// Doxygen gets confused by this.

/**
 @cond
*/

template bool Properties::get<bool>(const String_type &, bool *) const;

template bool Properties::get<unsigned int>(const String_type &,
                                            unsigned int *) const;

template bool Properties::get<unsigned long>(const String_type &,
                                             unsigned long *) const;

template bool Properties::get<unsigned long long>(const String_type &,
                                                  unsigned long long *) const;

template bool Properties::get<MYSQL_LEX_STRING>(const String_type &,
                                                MYSQL_LEX_STRING *,
                                                MEM_ROOT *) const;

template bool Properties::get<MYSQL_LEX_CSTRING>(const String_type &,
                                                 MYSQL_LEX_CSTRING *,
                                                 MEM_ROOT *) const;

template bool Properties::get<int>(const String_type &, int *) const;

template bool Properties::get<long>(const String_type &, long *) const;

template bool Properties::get<long long>(const String_type &,
                                         long long *) const;

template bool Properties::from_str<int>(const String_type &, int *);

template bool Properties::from_str<unsigned int>(const String_type &,
                                                 unsigned int *);

template bool Properties::from_str<long>(const String_type &, long *);

template bool Properties::from_str<unsigned long>(const String_type &,
                                                  unsigned long *);

template bool Properties::from_str<long long>(const String_type &, long long *);

template bool Properties::from_str<unsigned long long>(const String_type &,
                                                       unsigned long long *);

template String_type Properties::to_str<int>(int);

template String_type Properties::to_str<Field::geometry_type>(
    Field::geometry_type);

template String_type Properties::to_str<long>(long);

template String_type Properties::to_str<long long>(long long);

template String_type Properties::to_str<unsigned long long>(unsigned long long);

template String_type Properties::to_str<bool>(bool);

template String_type Properties::to_str<unsigned int>(unsigned int);

template String_type Properties::to_str<unsigned long>(unsigned long);

template String_type Properties::to_str<char const *>(char const *);

template String_type Properties::to_str<unsigned short>(unsigned short);

template String_type Properties::to_str<row_type>(row_type);

template String_type Properties::to_str<dd::Table::enum_row_format>(
    dd::Table::enum_row_format);

template String_type Properties::to_str<enum_stats_auto_recalc>(
    enum_stats_auto_recalc);

template String_type Properties::to_str<ha_storage_media>(ha_storage_media);

/**
 @endcond
*/

}  // namespace dd
