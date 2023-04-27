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

#ifndef DD__OBJECT_KEYS_INCLUDED
#define DD__OBJECT_KEYS_INCLUDED

#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_inttypes.h"
#include "sql/dd/impl/object_key.h"  // dd::Object_key
#include "sql/dd/object_id.h"        // dd::Object_id
#include "sql/dd/string_type.h"
#include "template_utils.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Raw_table;
struct Raw_key;

///////////////////////////////////////////////////////////////////////////

// NOTE: the current naming convention is as follows:
// - use '_key' suffix to name keys identifying 0 or 1 row;
// - use '_range_key' suffix to name keys identifying 0 or N rows.

///////////////////////////////////////////////////////////////////////////

// Key type to be used for keys that are not supported by an object type.
class Void_key : public Object_key {
 public:
  Void_key() {}

 public:
  /* purecov: begin inspected */
  virtual Raw_key *create_access_key(Raw_table *) const { return nullptr; }
  /* purecov: end */

  /* purecov: begin inspected */
  virtual String_type str() const { return ""; }
  /* purecov: end */

  // We need a comparison operator since the type will be used
  // as a template type argument.
  /* purecov: begin inspected */
  bool operator<(const Void_key &rhs) const { return this < &rhs; }
  /* purecov: end */
};

///////////////////////////////////////////////////////////////////////////

// Entity_object-id primary key for global objects.
class Primary_id_key : public Object_key {
 public:
  Primary_id_key() {}

  Primary_id_key(Object_id object_id) : m_object_id(object_id) {}

  // Update a preallocated instance.
  void update(Object_id object_id) { m_object_id = object_id; }

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

  bool operator<(const Primary_id_key &rhs) const {
    return m_object_id < rhs.m_object_id;
  }

 private:
  Object_id m_object_id;
};

///////////////////////////////////////////////////////////////////////////

// Entity_object-id partial key for looking for containing objects.
class Parent_id_range_key : public Object_key {
 public:
  Parent_id_range_key(int id_index_no, int id_column_no, Object_id object_id)
      : m_id_index_no(id_index_no),
        m_id_column_no(id_column_no),
        m_object_id(object_id) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_id_index_no;
  int m_id_column_no;
  Object_id m_object_id;
};

///////////////////////////////////////////////////////////////////////////

// Entity_object-name key for global objects.
class Global_name_key : public Object_key {
 public:
  Global_name_key() {}

  Global_name_key(int name_column_no, const String_type &object_name,
                  const CHARSET_INFO *cs)
      : m_name_column_no(name_column_no),
        m_object_name(object_name),
        m_cs(cs) {}

  // Update a preallocated instance.
  void update(int name_column_no, const String_type &object_name,
              const CHARSET_INFO *cs) {
    m_name_column_no = name_column_no;
    m_object_name = object_name;
    m_cs = cs;
  }

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  /* purecov: begin inspected */
  virtual String_type str() const { return m_object_name; }
  /* purecov: end */

  bool operator<(const Global_name_key &rhs) const {
    return (my_strnncoll(m_cs,
                         pointer_cast<const uchar *>(m_object_name.c_str()),
                         m_object_name.length(),
                         pointer_cast<const uchar *>(rhs.m_object_name.c_str()),
                         rhs.m_object_name.length()) < 0);
  }

 private:
  int m_name_column_no;
  String_type m_object_name;
  // Collation used for the name in the table.
  const CHARSET_INFO *m_cs;
};

///////////////////////////////////////////////////////////////////////////

// Entity_object-name key for objects which are identified within a container.
class Item_name_key : public Object_key {
 public:
  Item_name_key() {}

  Item_name_key(int container_id_column_no, Object_id container_id,
                int name_column_no, const String_type &object_name,
                const CHARSET_INFO *cs)
      : m_container_id_column_no(container_id_column_no),
        m_name_column_no(name_column_no),
        m_container_id(container_id),
        m_object_name(object_name),
        m_cs(cs) {}

  // Update a preallocated instance.
  void update(int container_id_column_no, Object_id container_id,
              int name_column_no, const String_type &object_name,
              const CHARSET_INFO *cs) {
    m_container_id_column_no = container_id_column_no;
    m_name_column_no = name_column_no;
    m_container_id = container_id;
    m_object_name = object_name;
    m_cs = cs;
  }

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

  bool operator<(const Item_name_key &rhs) const {
    if (m_container_id != rhs.m_container_id)
      return (m_container_id < rhs.m_container_id);

    return (my_strnncoll(m_cs,
                         pointer_cast<const uchar *>(m_object_name.c_str()),
                         m_object_name.length(),
                         pointer_cast<const uchar *>(rhs.m_object_name.c_str()),
                         rhs.m_object_name.length()) < 0);
  }

 private:
  int m_container_id_column_no;
  int m_name_column_no;

  Object_id m_container_id;
  String_type m_object_name;
  // Collation used for the name in the table.
  const CHARSET_INFO *m_cs;
};

///////////////////////////////////////////////////////////////////////////

// TODO: find a better name.
class Se_private_id_key : public Object_key {
 public:
  Se_private_id_key() {}

  /* purecov: begin deadcode */
  Se_private_id_key(int index_no, int engine_column_no,
                    const String_type &engine, int private_id_column_no,
                    Object_id private_id)
      : m_index_no(index_no),
        m_engine_column_no(engine_column_no),
        m_engine(engine),
        m_private_id_column_no(private_id_column_no),
        m_private_id(private_id) {}
  /* purecov: end */

  // Update a preallocated instance.
  void update(int index_no, int engine_column_no, const String_type &engine,
              int private_id_column_no, Object_id private_id) {
    m_index_no = index_no;
    m_engine_column_no = engine_column_no;
    m_engine = engine;
    m_private_id_column_no = private_id_column_no;
    m_private_id = private_id;
  }

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

  bool operator<(const Se_private_id_key &rhs) const {
    return m_private_id < rhs.m_private_id
               ? true
               : rhs.m_private_id < m_private_id ? false
                                                 : m_engine < rhs.m_engine;
  }

 private:
  int m_index_no;

  int m_engine_column_no;
  String_type m_engine;

  int m_private_id_column_no;
  Object_id m_private_id;
};

///////////////////////////////////////////////////////////////////////////

class Composite_pk : public Object_key {
 public:
  Composite_pk(int index_no, uint first_column_no, ulonglong first_id,
               uint second_column_no, ulonglong second_id)
      : m_index_no(index_no),
        m_first_column_no(first_column_no),
        m_first_id(first_id),
        m_second_column_no(second_column_no),
        m_second_id(second_id) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_first_column_no;
  ulonglong m_first_id;

  int m_second_column_no;
  ulonglong m_second_id;
};

///////////////////////////////////////////////////////////////////////////

class Composite_char_key : public Object_key {
 public:
  Composite_char_key(int index_no, uint first_column_no,
                     const String_type &first_name, uint second_column_no,
                     const String_type &second_name)
      : m_index_no(index_no),
        m_first_column_no(first_column_no),
        m_first_name(first_name),
        m_second_column_no(second_column_no),
        m_second_name(second_name) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_first_column_no;
  String_type m_first_name;

  int m_second_column_no;
  String_type m_second_name;
};

///////////////////////////////////////////////////////////////////////////

class Composite_4char_key : public Object_key {
 public:
  Composite_4char_key(int index_no, uint first_column_no,
                      const String_type &first_name, uint second_column_no,
                      const String_type &second_name, uint third_column_no,
                      const String_type &third_name, uint fourth_column_no,
                      const String_type &fourth_name)
      : m_index_no(index_no),
        m_first_column_no(first_column_no),
        m_first_name(first_name),
        m_second_column_no(second_column_no),
        m_second_name(second_name),
        m_third_column_no(third_column_no),
        m_third_name(third_name),
        m_fourth_column_no(fourth_column_no),
        m_fourth_name(fourth_name) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_first_column_no;
  String_type m_first_name;

  int m_second_column_no;
  String_type m_second_name;

  int m_third_column_no;
  String_type m_third_name;

  int m_fourth_column_no;
  String_type m_fourth_name;
};

///////////////////////////////////////////////////////////////////////////

class Composite_obj_id_3char_key : public Object_key {
 public:
  Composite_obj_id_3char_key(int index_no, uint id_column_no, Object_id id,
                             uint first_column_no,
                             const String_type &first_name,
                             uint second_column_no,
                             const String_type &second_name,
                             uint third_column_no,
                             const String_type &third_name)
      : m_index_no(index_no),
        m_id_column_no(id_column_no),
        m_id(id),
        m_first_column_no(first_column_no),
        m_first_name(first_name),
        m_second_column_no(second_column_no),
        m_second_name(second_name),
        m_third_column_no(third_column_no),
        m_third_name(third_name) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_id_column_no;
  Object_id m_id;

  int m_first_column_no;
  String_type m_first_name;

  int m_second_column_no;
  String_type m_second_name;

  int m_third_column_no;
  String_type m_third_name;
};

///////////////////////////////////////////////////////////////////////////

// Range key to find index statistics entries by table name.
// in mysql.index_stats.
class Index_stat_range_key : public Object_key {
 public:
  Index_stat_range_key(int index_no, int schema_name_column_no,
                       const String_type &schema_name, int table_name_column_no,
                       const String_type &table_name)
      : m_index_no(index_no),
        m_schema_name_column_no(schema_name_column_no),
        m_schema_name(schema_name),
        m_table_name_column_no(table_name_column_no),
        m_table_name(table_name) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_schema_name_column_no;
  String_type m_schema_name;

  int m_table_name_column_no;
  String_type m_table_name;
};

///////////////////////////////////////////////////////////////////////////

class Routine_name_key : public Object_key {
 public:
  Routine_name_key() {}

  Routine_name_key(int index_no, int container_id_column_no,
                   Object_id container_id, int type_column_no, uint type,
                   int name_column_no, const String_type &object_name,
                   const CHARSET_INFO *cs)
      : m_index_no(index_no),
        m_container_id_column_no(container_id_column_no),
        m_type_column_no(type_column_no),
        m_name_column_no(name_column_no),
        m_container_id(container_id),
        m_type(type),
        m_object_name(object_name),
        m_cs(cs) {}

  // Update a preallocated instance.
  void update(int index_no, int container_id_column_no, Object_id container_id,
              int type_column_no, uint type, int name_column_no,
              const String_type &object_name, const CHARSET_INFO *cs) {
    m_index_no = index_no;
    m_container_id_column_no = container_id_column_no;
    m_type_column_no = type_column_no;
    m_name_column_no = name_column_no;
    m_container_id = container_id;
    m_type = type;
    m_object_name = object_name;
    m_cs = cs;
  }

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

  bool operator<(const Routine_name_key &rhs) const;

 private:
  int m_index_no;
  int m_container_id_column_no;
  int m_type_column_no;
  int m_name_column_no;

  Object_id m_container_id;
  uint m_type;
  String_type m_object_name;
  // Collation used for the routine name in the table.
  const CHARSET_INFO *m_cs;
};

///////////////////////////////////////////////////////////////////////////

// Range key to find rows using catalog/schema/table name.
class Table_reference_range_key : public Object_key {
 public:
  Table_reference_range_key(int index_no, int catalog_name_column_no,
                            const String_type &catalog_name,
                            int schema_name_column_no,
                            const String_type &schema_name,
                            int table_name_column_no,
                            const String_type &table_name)
      : m_index_no(index_no),
        m_catalog_name_column_no(catalog_name_column_no),
        m_catalog_name(catalog_name),
        m_schema_name_column_no(schema_name_column_no),
        m_schema_name(schema_name),
        m_table_name_column_no(table_name_column_no),
        m_table_name(table_name) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_catalog_name_column_no;
  String_type m_catalog_name;

  int m_schema_name_column_no;
  String_type m_schema_name;

  int m_table_name_column_no;
  String_type m_table_name;
};

///////////////////////////////////////////////////////////////////////////

// Range key to find sub partition entries by table id and parent partition
// id in mysql.partitions.
class Sub_partition_range_key : public Object_key {
 public:
  Sub_partition_range_key(int index_no, int table_id_column_no,
                          const Object_id table_id,
                          int parent_partition_id_column_no,
                          const Object_id parent_partition_id)
      : m_index_no(index_no),
        m_table_id_column_no(table_id_column_no),
        m_table_id(table_id),
        m_parent_partition_id_column_no(parent_partition_id_column_no),
        m_parent_partition_id(parent_partition_id) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_index_no;

  int m_table_id_column_no;
  Object_id m_table_id;

  int m_parent_partition_id_column_no;
  Object_id m_parent_partition_id;
};

///////////////////////////////////////////////////////////////////////////
}  // namespace dd
#endif  // DD__OBJECT_KEYS_INCLUDED
