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

#ifndef DD__PROPERTIES_INCLUDED
#define DD__PROPERTIES_INCLUDED

#include <map>

#include "sql/dd/string_type.h"  // String_type, Stringstream_type

struct MEM_ROOT;

namespace dd {

///////////////////////////////////////////////////////////////////////////

/**
  The Properties class defines an interface for storing key=value pairs,
  where both key and value may be UTF-8 strings.

  The interface contains functions for testing whether a key exists,
  replacing or removing key=value pairs, iteration etc. The interface
  also defines template functions for converting between strings and
  various primitive types.

  Please note that in debug builds, the get() functions will assert that
  the key exists. This is to make sure that non-existing keys are
  handled explicitly at the client side.

  The raw_string() function returns a semicolon separated list of all
  key=value pairs. Characters '=' and ';' that are part of key or value
  are escaped using the '\' as an escape character. The escape character
  itself must also be escaped if being part of key or value.

  Examples of usage:

    Add key=value:
      p->set("akey=avalue");

    Add a numeric value:
      p->set("intvalue", 1234);

    Get values:
      int32 num;
      p->get("intvalue", &num);

    Get raw string:
      String_type mylist= p->raw_string();

  Further comments can be found in the files properties_impl.{h,cc}
  where the interface is implemented.
 */

class Properties {
 public:
  /**
    Convert a string to a value of an integral type. Verify correct
    sign, check for overflow and conversion errors.

    @tparam     T      Value type.
    @param      number String containing integral value to convert.
    @param[out] value  Converted value.

    @return            operation status.
      @retval true     if an error occurred
      @retval false    if success
  */
  template <typename T>
  static bool from_str(const String_type &number, T *value);

  /**
    Convert string to bool. Valid values are "true", "false", and
    decimal numbers, where "0" will be taken to mean false, and
    numbers != 0 will be taken to mean true.

    @param      bool_str String containing boolean value to convert.
    @param[out] value    Converted value.

    @return            operation status.
      @retval true     if an error occurred
      @retval false    if success
  */
  static bool from_str(const String_type &bool_str, bool *value);

  /**
    Convert a value of an integral type (including bool) to a string.
    Create an output stream and write the value.

    @tparam  T       Value type.
    @param   value   Actual value to convert.

    @return          string containing representation of the value.
  */
  template <class T>
  static String_type to_str(T value);

  /**
    Parse the submitted string for properties on the format
    "key=value;key=value;...". Create new property object and add
    the properties to the map in the object.

    @param raw_properties  string containing list of key=value pairs
    @return                pointer to new Property_impl object
      @retval NULL         if an error occurred
  */
  static Properties *parse_properties(const String_type &raw_properties);

  typedef std::map<String_type, String_type> Map;
  typedef std::map<String_type, String_type>::size_type size_type;
  typedef Map::iterator iterator;
  typedef Map::const_iterator const_iterator;

  virtual iterator begin() = 0;
  virtual const_iterator begin() const = 0;

  virtual iterator end() = 0;
  virtual const_iterator end() const = 0;

  /**
    Insert keys and values from a different property object.

    @note The valid keys in the target object are used to filter out invalid
          keys, which will be silently ignored. The set of valid keys in the
          source object is not copied.

    @pre The 'this' object shall be empty.

    @param properties Object which will have its properties
                      copied to 'this' object.

    @return           operation outcome, false if success, otherwise true.
  */
  virtual bool insert_values(const Properties &properties) = 0;

  /**
    Insert keys and values from a raw string.

    Invalid keys will be silently ignored, using the set of valid keys in
    the target object as a filter. The source is a string, so it has no
    definition of valid keys.

    @pre The 'this' object shall be empty.

    @param raw_string String with key/value pairs which will be
                      parsed and added to the 'this' object.

    @return           operation outcome, false if success, otherwise true.
  */
  virtual bool insert_values(const String_type &raw_string) = 0;

  /**
    Get the number of key=value pairs.

    @note Invalid keys that are present will also be reflected in the count.

    @return number of key=value pairs
  */
  virtual size_type size() const = 0;

  /**
    Are there any key=value pairs?

    @return true if there is no key=value pair, else false.
  */
  virtual bool empty() const = 0;

  /**
    Remove all key=value pairs.
  */
  virtual void clear() = 0;

  /**
    Check if the submitted key is valid.

    @param key Key to be checked.

    @retval true if the key is valid, otherwise false.
  */
  virtual bool valid_key(const String_type &key) const = 0;

  /**
    Check for the existence of a key=value pair given the key.

    @param key Key to be checked.

    @return true if the given key exists, false otherwise.
  */
  virtual bool exists(const String_type &key) const = 0;

  /**
    Remove the key=value pair for the given key if it exists.
    Otherwise, do nothing.

    @param key key to lookup.

    @return    false if the given key existed, true otherwise.
  */
  virtual bool remove(const String_type &key) = 0;

  /**
    Create a string containing all key=value pairs as a semicolon
    separated list. Key and value are separated by '='. The '=' and
    ';' characters are escaped using '\' if part of key or value, hence,
    the escape character '\' must also be escaped.

    @return a string listing all key=value pairs.
  */
  virtual const String_type raw_string() const = 0;

  /**
    Get the string value for a given key.

    Return true (assert in debug builds) if the operation fails, i.e.,
    if the key does not exist or if the key is invalid.

    @param      key   Key to lookup the value for.
    @param[out] value String value.

    @return           operation outcome, false if success, otherwise true.
  */
  virtual bool get(const String_type &key, String_type *value) const = 0;

  /**
    Get the lex string value for a given key.

    Return true (assert in debug builds) if the operation fails, i.e.,
    if the key does not exist or if the key is invalid.

    @tparam     Lex_type Type of LEX string.
    @param      key      Key to lookup the value for.
    @param[out] value    LEX_STRING or LEX_CSTRING value.
    @param[in]  mem_root MEM_ROOT to allocate string.

    @return              operation outcome, false if success, otherwise true.
  */
  template <typename Lex_type>
  bool get(const String_type &key, Lex_type *value, MEM_ROOT *mem_root) const;

  /**
    Get the string value for the key and convert it to the appropriate type.

    Return true (assert in debug builds) if the operation fails, i.e.,
    if the key does not exist or is invalid, or if the type conversion fails.

    @tparam     Value_type  Type of the value to get.
    @param      key         Key to lookup.
    @param[out] value       Value of appropriate type.

    @return                 operation outcome, false if success, otherwise true.
  */
  template <typename Value_type>
  bool get(const String_type &key, Value_type *value) const;

  /**
    Add a new key=value pair where the value is a string. If the key already
    exists, the associated value will be replaced by the new value argument.

    Return true (assert in debug builds) if the operation fails, i.e.,
    if the key is invalid.

    @param key   Key to map to a value.
    @param value String value to be associated with the key.

    @return      operation outcome, false if success, otherwise true.
  */
  virtual bool set(const String_type &key, const String_type &value) = 0;

  /**
    Convert the value to a string and set it for the given key.

    Return true (assert in debug builds) if the operation fails, i.e.,
    if the key is invalid.

    @tparam     Value_type  Type of the value to set.
    @param      key         Key to assign to.
    @param[out] value       Value of appropriate type.

    @return                 operation outcome, false if success, otherwise true.
  */
  template <typename Value_type>
  bool set(const String_type &key, Value_type value) {
    return set(key, to_str(value));
  }

  Properties() = default;

  Properties(const Properties &) = default;

  Properties &operator=(const Properties &) = delete;

  virtual ~Properties() {}
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PROPERTIES_INCLUDED
