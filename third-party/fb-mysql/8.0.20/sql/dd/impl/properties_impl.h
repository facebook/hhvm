/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__PROPERTIES_IMPL_INCLUDED
#define DD__PROPERTIES_IMPL_INCLUDED

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/dd/properties.h"   // dd::Properties
#include "sql/dd/string_type.h"  // dd::String_type

namespace dd {

///////////////////////////////////////////////////////////////////////////

/**
  The Properties_impl class implements the Properties interface.

  The key=value pairs are stored in a std::map. An instance can be created
  either by means of the default constructor, which creates an object
  with an empty map, or alternatively, it can be created by means of the
  static parse_properties function with a String_type argument. The string
  is supposed to contain a semicolon separated list of key=value pairs,
  where the characters '=' and ';' also may be part of key or value by
  escaping using the '\' as an escape character. The escape character
  itself must also be escaped if being part of key or value. All characters
  between '=' and ';' are considered part of key or value, whitespace is
  not ignored.

  Escaping is removed during parsing so the strings in the map are not
  escaped. Escaping is only relevant in the context of raw strings that
  are to be parsed, and raw strings that are returned containing all
  key=value pairs.

  Example (note \\ due to escaping of C string literals):
    parse_properties("a=b;b = c")     -> ("a", "b"), ("b ", " c")
    parse_properties("a\\==b;b=\\;c") -> ("a=", "b"), ("b", ";c")

    get("a=") == "b"
    get("b")  == ";c"

  Additional key=value pairs may be added by means of the set function,
  which takes a string argument that is assumed to be unescaped.

  Please also refer to the comments in the file properties.h where the
  interface is defined; the functions in the interface are commented there.
*/

class Properties_impl : public Properties {
 private:
  /* Map containing the actual key-value pairs. */
  Properties::Map m_map;

  /* Set containing the valid keys. An empty set means any key is valid. */
  std::set<String_type> m_keys;

 public:
  Properties_impl() = default;

  /* Constructor accepting a set of valid keys. */
  Properties_impl(const std::set<String_type> &keys) : m_keys(keys) {}

  virtual const Properties_impl *impl() const { return this; }

  virtual iterator begin() { return m_map.begin(); }

  virtual const_iterator begin() const { return m_map.begin(); }

  virtual iterator end() { return m_map.end(); }

  virtual const_iterator end() const { return m_map.end(); }

  virtual size_type size() const { return m_map.size(); }

  virtual bool empty() const { return m_map.empty(); }

  virtual void clear() { return m_map.clear(); }

  virtual bool valid_key(const String_type &key) const {
    return (m_keys.empty() || m_keys.find(key) != m_keys.end());
  }

  virtual bool exists(const String_type &key) const {
    return m_map.find(key) != m_map.end();
  }

  virtual bool remove(const String_type &key) {
    iterator it = m_map.find(key);

    if (it == m_map.end()) return true;

    m_map.erase(it);
    return false;
  }

  /**
    Iterate over all entries in the private hash table. For each
    key value pair, escape both key and value, and append the strings
    to the result. Use '=' to separate key and value, and use ';'
    to separate pairs.

    Invalid keys are not included in the output. However, there should
    never be a situation where invalid keys are present, so we just assert
    that the keys are valid.

    @return string containing all escaped key value pairs
  */
  virtual const String_type raw_string() const;

  /**
    Get the string value for a given key.

    Return true if the operation fails, i.e., if the key does not exist
    or if the key is invalid. Assert that the key exists in debug builds.

    @param      key   key to lookup the value for
    @param[out] value string value
    @return           Operation outcome, false if success, otherwise true
  */
  virtual bool get(const String_type &key, String_type *value) const;

  /**
    Set the key/value. If the key is invalid, a warning is written
    to the error log. Assert that the key exists in debug builds.

    @param  key    Key to set.
    @param  value  Value to set.
    @return        Operation outcome, false if success, otherwise true
  */
  virtual bool set(const String_type &key, const String_type &value);

  /**
    Insert key/value pairs from a different property object.

    The set of valid keys is not copied, instead, the existing
    set in the destination object is used to ignore all invalid
    keys.

    @param properties  Source object.

    @retval  Operation outcome, false if no error, otherwise true.
  */
  virtual bool insert_values(const Properties &properties);

  /**
    Insert key/value pairs from a string.

    Parse the string and add key/value pairs to this object.
    The existing set of valid keys in the destination object
    is used to ignore all invalid keys.

    @param  raw_string  String to be parsed.

    @retval  Operation outcome, false if no error, otherwise true.
  */
  virtual bool insert_values(const String_type &raw_string);

#ifdef EXTRA_CODE_FOR_UNIT_TESTING
  /**
    Extend the set of valid keys after the property object is
    created. This can be used e.g. for the SE private data.

    @pre     There must be a set of valid keys already, or the
             map of key-value pairs must be empty. Otherwise,
             we risk making existing keys invalid, thus hiding
             their values.

    @param   keys    Set of additional keys to insert into
                     the set of valid keys.
  */
  void add_valid_keys(const std::set<String_type> &keys) {
    DBUG_ASSERT(!m_keys.empty() || m_map.empty());
    m_keys.insert(keys.begin(), keys.end());
  }

  /**
    Remove the set of valid keys after the property object is
    created. Convenience method used by unit tests.
  */
  void clear_valid_keys() { m_keys.clear(); }

  /**
    Get valid key at a certain index.

    If the key set is empty, return a string representation of
    the index is returned. If the index is out of bounds, return
    the last key.

    @note    This is needed by unit tests to fill in
             random key/value pairs without breaking the
             check for valid keys.

    @param   index  Index at which to get the valid key.

    @retval  Key at the given index, a string containing the index,
             or the last key.
  */
  const String_type valid_key_at(size_t index) const {
    if (m_keys.empty()) {
      Stringstream_type ostream;
      ostream << index;
      return ostream.str();
    }
    if (m_keys.size() <= index) {
      return *std::next(m_keys.begin(), m_keys.size() - 1);
    }
    return *std::next(m_keys.begin(), index);
  }
#endif
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PROPERTIES_IMPL_INCLUDED
