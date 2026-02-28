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

#ifndef DD__RESOURCE_GROUP_IMPL_INCLUDED
#define DD__RESOURCE_GROUP_IMPL_INCLUDED

#include <sys/types.h>
#include <bitset>
#include <new>
#include <string>
#include <vector>

#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/types/resource_group.h"                // dd::Resource_group
#include "sql/resourcegroups/resource_group_sql_cmd.h"  // resourcegroups::Type

namespace dd {

class Open_dictionary_tables_ctx;
class Weak_object;

class Resource_group_impl : public Entity_object_impl, public Resource_group {
 public:
  Resource_group_impl();
  Resource_group_impl(const Resource_group_impl &);

 public:
  const Object_table &object_table() const override;
  static void register_tables(Open_dictionary_tables_ctx *otx);

  bool validate() const override;
  bool restore_attributes(const Raw_record &r) override;
  bool store_attributes(Raw_record *r) override;
  void debug_print(String_type &outb) const override;

  const resourcegroups::Type &resource_group_type() const override {
    return m_type;
  }
  void set_resource_group_type(const resourcegroups::Type &type) override {
    m_type = type;
  }

  bool resource_group_enabled() const override { return m_enabled; }
  void set_resource_group_enabled(bool enabled) override {
    m_enabled = enabled;
  }

  const std::bitset<CPU_MASK_SIZE> &cpu_id_mask() const override {
    return m_cpu_id_mask;
  }

  void set_cpu_id_mask(
      const std::vector<resourcegroups::Range> &vcpu_vec) override {
    m_cpu_id_mask.reset();
    for (auto vcpu_range : vcpu_vec) {
      for (auto bit_pos = vcpu_range.m_start; bit_pos <= vcpu_range.m_end;
           ++bit_pos)
        m_cpu_id_mask.set(bit_pos);
    }
  }

  int thread_priority() const override { return m_thread_priority; }

  void set_thread_priority(int priority) override {
    m_thread_priority = priority;
  }

  // Fix "inherits ... via dominance" warnings
  virtual Entity_object_impl *impl() override {
    return Entity_object_impl::impl();
  }
  virtual const Entity_object_impl *impl() const override {
    return Entity_object_impl::impl();
  }
  virtual Object_id id() const override { return Entity_object_impl::id(); }
  virtual bool is_persistent() const override {
    return Entity_object_impl::is_persistent();
  }
  virtual const String_type &name() const override {
    return Entity_object_impl::name();
  }
  virtual void set_name(const String_type &name) override {
    Entity_object_impl::set_name(name);
  }

 private:
  String_type m_resource_group_name;
  resourcegroups::Type m_type;
  bool m_enabled;
  std::bitset<CPU_MASK_SIZE> m_cpu_id_mask;
  int m_thread_priority;

  Resource_group *clone() const override {
    return new Resource_group_impl(*this);
  }
};

}  // namespace dd
#endif  // DD__RESOURCE_GROUP_IMPL_INCLUDED
