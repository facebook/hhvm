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

#include "sql/dd/impl/types/object_table_definition_impl.h"  // dd::Object_table_definition

#include "m_string.h"  // my_stpcpy
#include "sql/current_thd.h"
#include "sql/dd/properties.h"     // dd::tables::DD_properties
#include "sql/set_var.h"           // sql_mode_quoted_string...
#include "sql/system_variables.h"  // MODE_LAST

namespace dd {

///////////////////////////////////////////////////////////////////////////

bool Object_table_definition_impl::s_dd_tablespace_encrypted = false;

void Object_table_definition_impl::get_element_properties(
    dd::Properties *properties, const Element_numbers &element_numbers,
    const Element_definitions &element_defs) const {
  DBUG_ASSERT(properties != nullptr);
  DBUG_ASSERT(element_numbers.size() == element_defs.size());
  int count = 0;
  for (auto it : element_numbers) {
    std::unique_ptr<Properties> element(Properties::parse_properties(""));
    Element_definitions::const_iterator element_def =
        element_defs.find(it.second);
    DBUG_ASSERT(element_def != element_defs.end());
    element->set(key(Label::LABEL), it.first);
    element->set(key(Label::POSITION), it.second);
    element->set(key(Label::DEFINITION), element_def->second);

    Stringstream_type ss;
    ss << "elem" << count++;
    properties->set(ss.str(), element->raw_string());
  }
}

bool Object_table_definition_impl::set_element_properties(
    const String_type &prop_str, Element_numbers *element_numbers,
    Element_definitions *element_defs) {
  DBUG_ASSERT(element_numbers != nullptr);
  DBUG_ASSERT(element_defs != nullptr);
  std::unique_ptr<Properties> properties(
      Properties::parse_properties(prop_str));
  /*
    We would normally use a range based loop here, but the developerstudio
    compiler on Solaris does not handle this when the base collection has
    pure virtual begin() and end() functions.
  */
  for (Properties::const_iterator it = properties->begin();
       it != properties->end(); ++it) {
    String_type label;
    int pos = 0;
    String_type def;
    std::unique_ptr<Properties> element(
        Properties::parse_properties(it->second));
    if (element->get(key(Label::LABEL), &label) ||
        element->get(key(Label::POSITION), &pos) ||
        element->get(key(Label::DEFINITION), &def))
      return true;

    add_element(pos, label, def, element_numbers, element_defs);
  }
  return false;
}

void Object_table_definition_impl::add_sql_mode_field(
    int field_number, const String_type &field_name) {
  LEX_STRING sql_mode;
  ulonglong all_sql_mode_mask = MODE_LAST - 1;
  sql_mode_quoted_string_representation(current_thd, all_sql_mode_mask,
                                        &sql_mode);
  DBUG_ASSERT(sql_mode.str);
  dd::String_type sql_modes_in_string(sql_mode.str, sql_mode.length);
  add_field(field_number, field_name,
            "sql_mode SET(" + sql_modes_in_string + ") NOT NULL");
}

String_type Object_table_definition_impl::get_ddl() const {
  /*
    If a DDL statement has been assigned, we return it. Otherwise, we
    create one based on the element maps.
  */
  if (!m_ddl_statement.empty()) return m_ddl_statement;

  Stringstream_type ss;
  ss << "CREATE TABLE ";

  // Output schema name if non-empty.
  if (!m_schema_name.empty()) ss << m_schema_name << ".";

  ss << m_table_name + "(\n";

  // Output fields
  for (auto field : m_field_definitions) {
    if (field != *m_field_definitions.begin()) ss << ",\n";
    ss << "  " << field.second;
  }

  // Output indexes
  for (auto index : m_index_definitions) ss << ",\n  " << index.second;

  // Output foreign keys
  for (auto key : m_foreign_key_definitions) ss << ",\n  " << key.second;

  ss << "\n)";

  // Output options
  for (auto option : m_option_definitions) ss << " " << option.second;

  // We must also append an encryption option if the mysql tablespace
  // is encrypted. This cannot be defined among the common options in
  // Object_table_impl because we don't  know the value until the
  // persisted myssl tablespace object is read from the DD tables.
  if (is_dd_tablespace_encrypted()) ss << " ENCRYPTION='Y'";

  return ss.str();
}

void Object_table_definition_impl::store_into_properties(
    Properties *table_def_properties) const {
  DBUG_ASSERT(table_def_properties != nullptr);
  table_def_properties->set(key(Label::NAME), m_table_name);

  Properties *field_props = Properties::parse_properties("");
  get_element_properties(field_props, m_field_numbers, m_field_definitions);
  table_def_properties->set(key(Label::FIELDS), field_props->raw_string());
  delete field_props;

  Properties *index_props = Properties::parse_properties("");
  get_element_properties(index_props, m_index_numbers, m_index_definitions);
  table_def_properties->set(key(Label::INDEXES), index_props->raw_string());
  delete index_props;

  Properties *fk_props = Properties::parse_properties("");
  get_element_properties(fk_props, m_foreign_key_numbers,
                         m_foreign_key_definitions);
  table_def_properties->set(key(Label::FOREIGN_KEYS), fk_props->raw_string());
  delete fk_props;

  Properties *option_props = Properties::parse_properties("");
  get_element_properties(option_props, m_option_numbers, m_option_definitions);
  table_def_properties->set(key(Label::OPTIONS), option_props->raw_string());
  delete option_props;
}

bool Object_table_definition_impl::restore_from_properties(
    const Properties &table_def_properties) {
  String_type property_str;
  if (table_def_properties.get(key(Label::NAME), &m_table_name) ||
      table_def_properties.get(key(Label::FIELDS), &property_str) ||
      set_element_properties(property_str, &m_field_numbers,
                             &m_field_definitions) ||
      table_def_properties.get(key(Label::INDEXES), &property_str) ||
      set_element_properties(property_str, &m_index_numbers,
                             &m_index_definitions) ||
      table_def_properties.get(key(Label::FOREIGN_KEYS), &property_str) ||
      set_element_properties(property_str, &m_foreign_key_numbers,
                             &m_foreign_key_definitions) ||
      table_def_properties.get(key(Label::OPTIONS), &property_str) ||
      set_element_properties(property_str, &m_option_numbers,
                             &m_option_definitions))
    return true;

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
