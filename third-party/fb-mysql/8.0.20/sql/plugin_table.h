#ifndef PLUGIN_TABLE_INCLUDED
#define PLUGIN_TABLE_INCLUDED

/*
   Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/string_type.h"  // String_type
#include "sql/sql_list.h"        // List

/**
  Class to hold information regarding a table to be created on
  behalf of a plugin. The class stores the name, definition, options
  and optional tablespace of the table. The definition should not contain the
  'CREATE TABLE name' prefix.

  @note The data members are not owned by the class, and will not
        be deleted when this instance is deleted.
*/
class Plugin_table {
 private:
  const char *m_schema_name;
  const char *m_table_name;
  const char *m_table_definition;
  const char *m_table_options;
  const char *m_tablespace_name;

 public:
  Plugin_table(const char *schema_name, const char *table_name,
               const char *definition, const char *options,
               const char *tablespace_name)
      : m_schema_name(schema_name),
        m_table_name(table_name),
        m_table_definition(definition),
        m_table_options(options),
        m_tablespace_name(tablespace_name) {}

  virtual ~Plugin_table() = default;

  const char *get_schema_name() const { return m_schema_name; }

  const char *get_name() const { return m_table_name; }

  const char *get_table_definition() const { return m_table_definition; }

  const char *get_table_options() const { return m_table_options; }

  const char *get_tablespace_name() const { return m_tablespace_name; }

  virtual const char *get_object_type() const { return "TABLE"; }

  dd::String_type get_qualified_name() const {
    dd::Stringstream_type ss;
    if (m_schema_name != nullptr) ss << m_schema_name << ".";
    ss << m_table_name;

    return ss.str();
  }

  virtual dd::String_type get_ddl() const {
    dd::Stringstream_type ss;
    ss << "CREATE TABLE " << get_qualified_name();
    ss << "(\n" << m_table_definition << ")";
    ss << m_table_options;

    if (m_tablespace_name != nullptr)
      ss << " "
         << "TABLESPACE=" << m_tablespace_name;

    return ss.str();
  }
};

/**
  Class to hold information regarding a view to be created on behalf of
  a plugin. The class stores the name, definition, and view options.
  The definition should not contain the 'CREATE VIEW name' prefix.

  @note The data members are not owned by the class, and will not
        be deleted when this instance is deleted.
*/
class Plugin_view : public Plugin_table {
 public:
  Plugin_view(const char *schema_name, const char *table_name,
              const char *definition, const char *options)
      : Plugin_table(schema_name, table_name, definition, options, nullptr) {}

  const char *get_object_type() const override { return "VIEW"; }

  dd::String_type get_ddl() const override {
    dd::Stringstream_type ss;
    ss << "CREATE " << get_table_options();
    ss << " VIEW " << get_qualified_name();
    ss << " AS " << get_table_definition();

    return ss.str();
  }
};

/**
  Class to hold information regarding a predefined tablespace
  created by a storage engine. The class stores the name, options,
  se_private_data, comment and engine of the tablespace. A list of
  of the tablespace files is also stored.

  @note The data members are not owned by the class, and will not
        be deleted when this instance is deleted.
*/
class Plugin_tablespace {
 public:
  class Plugin_tablespace_file {
   private:
    const char *m_name;
    const char *m_se_private_data;

   public:
    Plugin_tablespace_file(const char *name, const char *se_private_data)
        : m_name(name), m_se_private_data(se_private_data) {}

    const char *get_name() const { return m_name; }

    const char *get_se_private_data() const { return m_se_private_data; }
  };

 private:
  const char *m_name;
  const char *m_options;
  const char *m_se_private_data;
  const char *m_comment;
  const char *m_engine;
  List<const Plugin_tablespace_file> m_files;

 public:
  Plugin_tablespace(const char *name, const char *options,
                    const char *se_private_data, const char *comment,
                    const char *engine)
      : m_name(name),
        m_options(options),
        m_se_private_data(se_private_data),
        m_comment(comment),
        m_engine(engine) {}

  void add_file(const Plugin_tablespace_file *file) { m_files.push_back(file); }

  const char *get_name() const { return m_name; }

  const char *get_options() const { return m_options; }

  const char *get_se_private_data() const { return m_se_private_data; }

  const char *get_comment() const { return m_comment; }

  const char *get_engine() const { return m_engine; }

  const List<const Plugin_tablespace_file> &get_files() const {
    return m_files;
  }
};

#endif  // PLUGIN_TABLE_INCLUDED
