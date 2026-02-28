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

#include "sql/dd/impl/types/resource_group_impl.h"

#include "sql/dd/dd_utility.h"                   // normalize_string()
#include "sql/dd/impl/raw/object_keys.h"         // Primary_id_keys
#include "sql/dd/impl/raw/raw_record.h"          // Raw_record
#include "sql/dd/impl/tables/resource_groups.h"  // Resource_groups
#include "sql/dd/impl/transaction_impl.h"        // Open_dictionary_tables_ctx

using dd::tables::Resource_groups;

namespace dd {

// Resource_group_impl implementation

Resource_group_impl::Resource_group_impl()
    : m_resource_group_name(""),
      m_type(resourcegroups::Type::SYSTEM_RESOURCE_GROUP),
      m_enabled(false),
      m_thread_priority(0) {}

Resource_group_impl::Resource_group_impl(const Resource_group_impl &src)
    : Weak_object(src),
      Entity_object_impl(src),
      m_resource_group_name(src.m_resource_group_name),
      m_type(src.m_type),
      m_enabled(src.m_enabled),
      m_cpu_id_mask(src.m_cpu_id_mask),
      m_thread_priority(src.m_thread_priority) {}

bool Resource_group_impl::validate() const { return false; }

/**
  Check if the string contain characters of 0 and 1.

  @returns true if characters of string are either 0 or 1 else false.
*/

static inline bool is_valid_cpu_mask_str(const String_type &str) {
  for (uint i = 0; i < str.size(); i++)
    if (str[i] != '0' && str[i] != '1') return false;
  return true;
}

bool Resource_group_impl::restore_attributes(const Raw_record &r) {
  restore_id(r, Resource_groups::FIELD_ID);
  restore_name(r, Resource_groups::FIELD_RESOURCE_GROUP_NAME);

  m_type = static_cast<resourcegroups::Type>(
      r.read_int(Resource_groups::FIELD_RESOURCE_GROUP_TYPE));

  m_enabled = r.read_bool(Resource_groups::FIELD_RESOURCE_GROUP_ENABLED);

  // convert bitmap values.
  String_type cpu_id_mask_str = r.read_str(Resource_groups::FIELD_CPU_ID_MASK);

  if (cpu_id_mask_str.size() > CPU_MASK_SIZE ||
      !is_valid_cpu_mask_str(cpu_id_mask_str))
    return true;

  m_cpu_id_mask = std::bitset<CPU_MASK_SIZE>(cpu_id_mask_str);

  m_thread_priority = r.read_int(Resource_groups::FIELD_THREAD_PRIORITY);

  return false;
}

bool Resource_group_impl::store_attributes(Raw_record *r) {
  return store_id(r, Resource_groups::FIELD_ID) ||
         store_name(r, Resource_groups::FIELD_RESOURCE_GROUP_NAME) ||
         r->store(Resource_groups::FIELD_RESOURCE_GROUP_TYPE,
                  static_cast<int>(m_type)) ||
         r->store(Resource_groups::FIELD_RESOURCE_GROUP_ENABLED, m_enabled) ||
         r->store(Resource_groups::FIELD_CPU_ID_MASK,
                  String_type(m_cpu_id_mask.to_string().c_str())) ||
         r->store(Resource_groups::FIELD_THREAD_PRIORITY, m_thread_priority);
}

void Resource_group_impl::debug_print(String_type &outb) const {
  dd::Stringstream_type ss;
  ss << "id: {OID: " << id() << "}; "
     << "Resource group name: " << m_resource_group_name << ";"
     << "CPU ID Mask: " << m_cpu_id_mask.to_string() << ";"
     << "Resource group type: " << (int)m_type << " ; "
     << "Thread priority: " << m_thread_priority << "; ]";

  outb = ss.str();
}

const Object_table &Resource_group_impl::object_table() const {
  return DD_table::instance();
}

void Resource_group_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Resource_groups>();
}

bool Resource_group::update_id_key(Id_key *key, Object_id id) {
  key->update(id);
  return false;
}

bool Resource_group::update_name_key(Name_key *key, const String_type &name) {
  // Resource group names are case insensitive
  char lc_name[NAME_LEN + 1];
  my_stpncpy(lc_name, name.c_str(), NAME_LEN);
  my_casedn_str(system_charset_info, lc_name);
  lc_name[NAME_LEN] = '\0';
  return Resource_groups::update_object_key(key, lc_name);
}

///////////////////////////////////////////////////////////////////////////

void Resource_group::create_mdl_key(const String_type &name, MDL_key *mdl_key) {
  /*
    Normalize the resource group name so that key comparison for case and accent
    insensitive names yields the correct result.
  */
  char normalized_name[NAME_CHAR_LEN * 2];
  size_t len = normalize_string(DD_table::name_collation(), name,
                                normalized_name, sizeof(normalized_name));

  mdl_key->mdl_key_init(MDL_key::RESOURCE_GROUPS, "", normalized_name, len,
                        name.c_str());
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
