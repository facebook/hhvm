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

#ifndef DD__RESOURCE_GROUP_INCLUDED
#define DD__RESOURCE_GROUP_INCLUDED

#include <bitset>  // std::bitset
#include <memory>  // std::unique_ptr

#include "sql/dd/types/entity_object.h"        // dd::Entity_object
#include "sql/dd/types/entity_object_table.h"  // dd::Entity_object_table
#include "sql/resourcegroups/resource_group_basic_types.h"  // Range, Type

struct MDL_key;

namespace dd {

class Object_type;
class Primary_id_key;
class Resource_group_impl;
class Global_name_key;
class Void_key;

namespace tables {
class Resource_groups;
}

static constexpr int CPU_MASK_SIZE = 1024;
class Resource_group : virtual public Entity_object {
 public:
  typedef Resource_group_impl Impl;
  typedef Resource_group Cache_partition;
  typedef tables::Resource_groups DD_table;
  typedef Primary_id_key Id_key;
  typedef Global_name_key Name_key;
  typedef Void_key Aux_key;

 public:
  ~Resource_group() override {}

  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }
  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_name_key(key, name());
  }
  static bool update_name_key(Name_key *key, const String_type &name);

  virtual bool update_aux_key(Aux_key *) const { return true; }

  virtual const resourcegroups::Type &resource_group_type() const = 0;
  virtual void set_resource_group_type(const resourcegroups::Type &type) = 0;

  virtual bool resource_group_enabled() const = 0;
  virtual void set_resource_group_enabled(bool enabled) = 0;

  virtual const std::bitset<CPU_MASK_SIZE> &cpu_id_mask() const = 0;
  virtual void set_cpu_id_mask(
      const std::vector<resourcegroups::Range> &vcpu_vec) = 0;

  virtual int thread_priority() const = 0;
  virtual void set_thread_priority(int priority) = 0;

  virtual Resource_group *clone() const = 0;

  static void create_mdl_key(const String_type &name, MDL_key *key);
};
}  // namespace dd

#endif  // DD__RESOURCE_GROUP_INCLUDED
