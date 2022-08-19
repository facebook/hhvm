/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/properties_impl.h"

#include <stddef.h>
#include <iterator>
#include <new>

#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/dd/impl/utils.h"  // eat_pairs

namespace dd {

///////////////////////////////////////////////////////////////////////////

/**
  Parse the submitted string for properties on the format
  "key=value;key=value;...". Create new property object and add
  the properties to the map in the object.

  @param raw_properties  string containing list of key=value pairs
  @return                pointer to new Property_impl object
    @retval NULL         if an error occurred
*/

Properties *Properties::parse_properties(const String_type &raw_properties) {
  Properties *tmp = new (std::nothrow) Properties_impl();
  String_type::const_iterator it = raw_properties.begin();

  if (eat_pairs(it, raw_properties.end(), tmp)) {
    delete tmp;
    tmp = nullptr;
  }

  return tmp;
}

const String_type Properties_impl::raw_string() const {
  String_type str("");
  str.reserve(16 * m_map.size());

  // Iterate over all valid map entries.
  for (auto &it : m_map) {
    DBUG_ASSERT(valid_key(it.first));
    if (valid_key(it.first)) {
      escape(&str, it.first);
      str.append("=");
      escape(&str, it.second);
      str.append(";");
    }
  }
  return str;
}

bool Properties_impl::get(const String_type &key, String_type *value) const {
  DBUG_ASSERT(value != nullptr);
  if (!valid_key(key)) {
    LogErr(WARNING_LEVEL, ER_INVALID_PROPERTY_KEY, key.c_str());
    DBUG_ASSERT(false);
    return true;
  }
  const_iterator it = m_map.find(key);
  if (it == m_map.end()) {
    // Key is not present.
    DBUG_ASSERT(false); /* purecov: inspected */
    return true;
  }
  *value = it->second;
  return false;
}

bool Properties_impl::set(const String_type &key, const String_type &value) {
  if (!valid_key(key)) {
    LogErr(WARNING_LEVEL, ER_INVALID_PROPERTY_KEY, key.c_str());
    DBUG_ASSERT(false);
    return true;
  }
  if (!key.empty()) m_map[key] = value;
  return false;
}

bool Properties_impl::insert_values(const Properties &properties) {
  // The precondition is that this object is empty
  DBUG_ASSERT(empty());
  std::copy_if(properties.begin(), properties.end(),
               std::inserter(m_map, m_map.begin()),
               [&](const Map::value_type &p) { return valid_key(p.first); });
  return false;
}

bool Properties_impl::insert_values(const String_type &raw_string) {
  // The precondition is that this object is empty
  DBUG_ASSERT(empty());

  /*
    Parse string and set key values. In the 'eat_pairs()' function,
    invalid keys will be silently ignored.
  */
  String_type::const_iterator it = raw_string.begin();
  if (eat_pairs(it, raw_string.end(), this)) {
    clear();
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
