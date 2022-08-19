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

#ifndef DD_TABLES__DD_PROPERTIES_INCLUDED
#define DD_TABLES__DD_PROPERTIES_INCLUDED

#include <sys/types.h>
#include <map>
#include <string>

#include "sql/dd/impl/properties_impl.h"  // dd::Properties_impl
#include "sql/dd/impl/types/object_table_impl.h"
#include "sql/dd/string_type.h"

class THD;

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

class DD_properties : public Object_table_impl {
 public:
  DD_properties();

  enum enum_fields { FIELD_PROPERTIES };

  static DD_properties &instance();

  /**
    The 'mysql.dd_properties' table will store key=value pairs. The valid
    keys are predefined, and represented in an internal map as pairs of
    'key name','type'. This is used in get() and set() to verify that the
    key is valid, and that the type of the value we are getting or setting
    is correct.

    One of the keys is 'SYSTEM_TABLES'. This is itself a property object
    where the keys are names of DD tables, and the values are property
    objects containing information about each table. These property objects
    will also have a set of predefined keys. These are defined by an
    enumeration, and there is a function that returns keys for each
    enumeration value. The purpose of this is to avoid directly entering
    keys in the source code, which is prone to mistyping etc.

    The two structures of valid keys are separate because they are used for
    different purposes. The top level keys defined in the internal map are
    used when properties at the top level are being looked up. The keys for
    the DD tables are used to lookup in the property object which is associated
    with a table DD name.
  */

  /**
    Enumeration used to lookup the valid keys for the DD table properties.
  */
  enum class DD_property { ID, DATA, SPACE_ID, IDX, COL, DEF };

  /**
    Property key names for DD table properties.

    @param label   Enumeration label of the key.

    @return Key character string.
  */
  static const char *dd_key(DD_property label) {
    switch (label) {
      case DD_property::ID:
        return "id";
      case DD_property::DATA:
        return "data";
      case DD_property::SPACE_ID:
        return "space_id";
      case DD_property::IDX:
        return "idx";
      case DD_property::COL:
        return "col";
      case DD_property::DEF:
        return "def";
      default:
        DBUG_ASSERT(false);
        return "";
    }
  }

  /**
    Get the integer value of the property key.

    Will validate the key before retrieving the value.

    @param       thd     Thread context.
    @param       key     The key representing the property.
    @param [out] value   Corresponding value, if it exists, otherwise
                         undefined.
    @param [out] exists  Will be 'false' if key is not present.

    @returns false on success otherwise true.
  */
  bool get(THD *thd, const String_type &key, uint *value, bool *exists);

  /**
    Set the integer value of the property key.

    Will validate the key before setting the value.

    @param thd    Thread context.
    @param key    The key representing the property.
    @param value  The value to be stored for 'key'.

    @returns false on success otherwise true.
  */
  bool set(THD *thd, const String_type &key, uint value);

  /**
    Get the character string value of the property key.

    Will validate the key before retrieving the value.

    @param       thd     Thread context.
    @param       key     The key representing the property.
    @param [out] value   Corresponding value, if it exists, otherwise
                         undefined.
    @param [out] exists  Will be 'false' if key is not present.

    @returns false on success otherwise true.
  */
  bool get(THD *thd, const String_type &key, String_type *value, bool *exists);

  /**
    Set the character string value of the property key.

    Will validate the key before setting the value.

    @param thd    Thread context.
    @param key    The key representing the property.
    @param value  The value to be stored for 'key'.

    @returns false on success otherwise true.
  */
  bool set(THD *thd, const String_type &key, const String_type &value);

  /**
    Get the properties object associated with a key.

    Will validate the key before retrieving the properties. Used to
    get hold of SE private data for the DD tables.

    @param       thd        Thread context.
    @param       key        Key name.
    @param [out] properties Properties object associated with the key.
    @param [out] exists     Will be 'false' if key is not present.

    @returns false on success otherwise true.
  */
  bool get(THD *thd, const String_type &key,
           std::unique_ptr<Properties> *properties, bool *exists);

  /**
    Set the properties object associated with a key.

    Will validate the key before setting the properties. Used to
    store SE private data for the DD tables.

    @param thd        Thread context.
    @param key        Key name.
    @param properties Properties object associated with the key.

    @returns false on success otherwise true.
  */
  bool set(THD *thd, const String_type &key, const dd::Properties &properties);

  /**
    Remove a property key.

    @param thd        Thread context.
    @param key        Key name.

    @returns false on success otherwise true.
  */
  bool remove(THD *thd, const String_type &key);

 private:
  // A cache of the table contents.
  Properties_impl m_properties;

  // Definitions of the valid property types. Used for internal validation.
  enum class Property_type { UNSIGNED_INT_32, CHARACTER_STRING, PROPERTIES };

  // Map from valid property keys to types. Used for internal validation.
  std::map<String_type, Property_type> m_property_desc;

  /**
    Initialize the cached properties by reading from disk.

    @param thd         Thread context.

    @returns false on success otherwise true.
  */
  bool init_cached_properties(THD *thd);

  /**
    Flush the cached properties to disk.

    @param thd         Thread context.

    @returns false on success otherwise true.
  */
  bool flush_cached_properties(THD *thd);

  /**
    Get the value of the property key.

    Will initialize the cached properties stored in the DD_properties instance
    if not already done. No key validation is done, this is expected to be
    done before this function is called.

    @param       thd     Thread context.
    @param       key     The key representing the property.
    @param [out] value   Corresponding value, if it exists, otherwise
                         undefined.
    @param [out] exists  Will be 'false' if key is not present.

    @returns false on success otherwise true.
  */
  bool unchecked_get(THD *thd, const String_type &key, String_type *value,
                     bool *exists);

  /**
    Set the value of the property key.

    Will initialize the cached properties stored in the DD_properties instance
    if not already done. No key validation is done, this is expected to be
    done before this function is called. Properties are flushed to disk after
    the cache is updated.

    @param thd    Thread context.
    @param key    The key representing the property.
    @param value  The value to be stored for 'key'.

    @returns false on success otherwise true.
  */
  bool unchecked_set(THD *thd, const String_type &key,
                     const String_type &value);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__DD_PROPERTIES_INCLUDED
