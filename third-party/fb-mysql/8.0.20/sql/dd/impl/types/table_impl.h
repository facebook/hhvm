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

#ifndef DD__TABLE_IMPL_INCLUDED
#define DD__TABLE_IMPL_INCLUDED

#include <sys/types.h>
#include <memory>
#include <new>
#include <string>

#include "my_inttypes.h"
#include "mysql_version.h"  // MYSQL_VERSION_ID
#include "sql/dd/impl/properties_impl.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/abstract_table_impl.h"  // dd::Abstract_table_impl
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/check_constraint.h"  // dd::Check_constraint
#include "sql/dd/types/foreign_key.h"       // dd::Foreign_key
#include "sql/dd/types/index.h"             // dd::Index
#include "sql/dd/types/partition.h"         // dd::Partition
#include "sql/dd/types/table.h"             // dd:Table
#include "sql/dd/types/trigger.h"           // dd::Trigger

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Foreign_key;
class Index;
class Object_table;
class Open_dictionary_tables_ctx;
class Partition;
class Properties;
class Sdi_rcontext;
class Sdi_wcontext;
class Trigger_impl;
class Weak_object;
class Object_table;

class Table_impl : public Abstract_table_impl, virtual public Table {
 public:
  Table_impl();

  virtual ~Table_impl();

 public:
  /////////////////////////////////////////////////////////////////////////
  // enum_table_type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_table_type type() const { return enum_table_type::BASE_TABLE; }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  virtual void debug_print(String_type &outb) const;

 private:
  /**
    Store the trigger object in DD table.

    @param otx  current Open_dictionary_tables_ctx

    @returns
     false on success.
     true on failure.
  */
  bool store_triggers(Open_dictionary_tables_ctx *otx);

 public:
  /////////////////////////////////////////////////////////////////////////
  // is_temporary.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_temporary() const { return m_is_temporary; }
  virtual void set_is_temporary(bool is_temporary) {
    m_is_temporary = is_temporary;
  }

  /////////////////////////////////////////////////////////////////////////
  // collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id collation_id() const { return m_collation_id; }

  virtual void set_collation_id(Object_id collation_id) {
    m_collation_id = collation_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // tablespace.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id tablespace_id() const { return m_tablespace_id; }

  virtual void set_tablespace_id(Object_id tablespace_id) {
    m_tablespace_id = tablespace_id;
  }

  virtual bool is_explicit_tablespace() const {
    bool is_explicit = false;
    if (options().exists("explicit_tablespace"))
      options().get("explicit_tablespace", &is_explicit);
    return is_explicit;
  }

  /////////////////////////////////////////////////////////////////////////
  // engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const { return m_engine; }

  virtual void set_engine(const String_type &engine) { m_engine = engine; }

  /////////////////////////////////////////////////////////////////////////
  // row_format
  /////////////////////////////////////////////////////////////////////////

  virtual enum_row_format row_format() const { return m_row_format; }

  virtual void set_row_format(enum_row_format row_format) {
    m_row_format = row_format;
  }

  /////////////////////////////////////////////////////////////////////////
  // comment
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

  /////////////////////////////////////////////////////////////////////////
  // last_checked_for_upgrade_version_id
  /////////////////////////////////////////////////////////////////////////
  virtual uint last_checked_for_upgrade_version_id() const {
    return m_last_checked_for_upgrade_version_id;
  }

  virtual void mark_as_checked_for_upgrade() {
    m_last_checked_for_upgrade_version_id = MYSQL_VERSION_ID;
  }

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const {
    return m_se_private_data;
  }

  virtual Properties &se_private_data() { return m_se_private_data; }

  virtual bool set_se_private_data(const String_type &se_private_data_raw) {
    return m_se_private_data.insert_values(se_private_data_raw);
  }

  virtual bool set_se_private_data(const Properties &se_private_data) {
    return m_se_private_data.insert_values(se_private_data);
  }

  /////////////////////////////////////////////////////////////////////////
  // se_private_id.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id se_private_id() const { return m_se_private_id; }

  virtual void set_se_private_id(Object_id se_private_id) {
    m_se_private_id = se_private_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // Partition type
  /////////////////////////////////////////////////////////////////////////

  virtual enum_partition_type partition_type() const {
    return m_partition_type;
  }

  virtual void set_partition_type(enum_partition_type partition_type) {
    m_partition_type = partition_type;
  }

  /////////////////////////////////////////////////////////////////////////
  // default_partitioning
  /////////////////////////////////////////////////////////////////////////

  virtual enum_default_partitioning default_partitioning() const {
    return m_default_partitioning;
  }

  virtual void set_default_partitioning(
      enum_default_partitioning default_partitioning) {
    m_default_partitioning = default_partitioning;
  }

  /////////////////////////////////////////////////////////////////////////
  // partition_expression
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &partition_expression() const {
    return m_partition_expression;
  }

  virtual void set_partition_expression(
      const String_type &partition_expression) {
    m_partition_expression = partition_expression;
  }

  /////////////////////////////////////////////////////////////////////////
  // partition_expression_utf8
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &partition_expression_utf8() const {
    return m_partition_expression_utf8;
  }

  virtual void set_partition_expression_utf8(
      const String_type &partition_expression_utf8) {
    m_partition_expression_utf8 = partition_expression_utf8;
  }

  /////////////////////////////////////////////////////////////////////////
  // subpartition_type
  /////////////////////////////////////////////////////////////////////////

  virtual enum_subpartition_type subpartition_type() const {
    return m_subpartition_type;
  }

  virtual void set_subpartition_type(enum_subpartition_type subpartition_type) {
    m_subpartition_type = subpartition_type;
  }

  /////////////////////////////////////////////////////////////////////////
  // default_subpartitioning
  /////////////////////////////////////////////////////////////////////////

  virtual enum_default_partitioning default_subpartitioning() const {
    return m_default_subpartitioning;
  }

  virtual void set_default_subpartitioning(
      enum_default_partitioning default_subpartitioning) {
    m_default_subpartitioning = default_subpartitioning;
  }

  /////////////////////////////////////////////////////////////////////////
  // subpartition_expression
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &subpartition_expression() const {
    return m_subpartition_expression;
  }

  virtual void set_subpartition_expression(
      const String_type &subpartition_expression) {
    m_subpartition_expression = subpartition_expression;
  }

  /////////////////////////////////////////////////////////////////////////
  // subpartition_expression_utf8
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &subpartition_expression_utf8() const {
    return m_subpartition_expression_utf8;
  }

  virtual void set_subpartition_expression_utf8(
      const String_type &subpartition_expression_utf8) {
    m_subpartition_expression_utf8 = subpartition_expression_utf8;
  }

  /////////////////////////////////////////////////////////////////////////
  // Index collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Index *add_index();

  virtual Index *add_first_index();

  virtual const Index_collection &indexes() const { return m_indexes; }

  virtual Index_collection *indexes() { return &m_indexes; }

  const Index *get_index(Object_id index_id) const {
    return const_cast<Table_impl *>(this)->get_index(index_id);
  }

  Index *get_index(Object_id index_id);

  /////////////////////////////////////////////////////////////////////////
  // Foreign key collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Foreign_key *add_foreign_key();

  virtual const Foreign_key_collection &foreign_keys() const {
    return m_foreign_keys;
  }

  virtual Foreign_key_collection *foreign_keys() { return &m_foreign_keys; }

  /////////////////////////////////////////////////////////////////////////
  // Foreign key parent collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Foreign_key_parent *add_foreign_key_parent();

 private:
  bool load_foreign_key_parents(Open_dictionary_tables_ctx *otx);

 public:
  virtual bool reload_foreign_key_parents(THD *thd);

  virtual const Foreign_key_parent_collection &foreign_key_parents() const {
    return m_foreign_key_parents;
  }

  /////////////////////////////////////////////////////////////////////////
  // Partition collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Partition *add_partition();

  virtual const Partition_collection &partitions() const {
    return m_partitions;
  }

  virtual Partition_collection *partitions() { return &m_partitions; }

  virtual const Partition_leaf_vector &leaf_partitions() const {
    return m_leaf_partitions;
  }

  virtual Partition_leaf_vector *leaf_partitions() {
    return &m_leaf_partitions;
  }

  // non-virtual
  void add_leaf_partition(Partition *p) { m_leaf_partitions.push_back(p); }

  const Partition *get_partition(Object_id partition_id) const {
    return const_cast<Table_impl *>(this)->get_partition(partition_id);
  }

  Partition *get_partition(Object_id partition_id);

  Partition *get_partition(const String_type &name);

  // Fix "inherits ... via dominance" warnings
  virtual Entity_object_impl *impl() { return Entity_object_impl::impl(); }
  virtual const Entity_object_impl *impl() const {
    return Entity_object_impl::impl();
  }
  virtual Object_id id() const { return Entity_object_impl::id(); }
  virtual bool is_persistent() const {
    return Entity_object_impl::is_persistent();
  }
  virtual const String_type &name() const { return Entity_object_impl::name(); }
  virtual void set_name(const String_type &name) {
    Entity_object_impl::set_name(name);
  }
  virtual Object_id schema_id() const {
    return Abstract_table_impl::schema_id();
  }
  virtual void set_schema_id(Object_id schema_id) {
    Abstract_table_impl::set_schema_id(schema_id);
  }
  virtual uint mysql_version_id() const {
    return Abstract_table_impl::mysql_version_id();
  }
  virtual const Properties &options() const {
    return Abstract_table_impl::options();
  }
  virtual Properties &options() { return Abstract_table_impl::options(); }
  virtual bool set_options(const Properties &options) {
    return Abstract_table_impl::set_options(options);
  }
  virtual bool set_options(const String_type &options_raw) {
    return Abstract_table_impl::set_options(options_raw);
  }
  virtual ulonglong created(bool convert_time) const {
    return Abstract_table_impl::created(convert_time);
  }
  virtual void set_created(ulonglong created) {
    Abstract_table_impl::set_created(created);
  }
  virtual ulonglong last_altered(bool convert_time) const {
    return Abstract_table_impl::last_altered(convert_time);
  }
  virtual void set_last_altered(ulonglong last_altered) {
    Abstract_table_impl::set_last_altered(last_altered);
  }
  virtual Column *add_column() { return Abstract_table_impl::add_column(); }
  virtual const Column_collection &columns() const {
    return Abstract_table_impl::columns();
  }
  virtual Column_collection *columns() {
    return Abstract_table_impl::columns();
  }
  const Column *get_column(Object_id column_id) const {
    return Abstract_table_impl::get_column(column_id);
  }
  Column *get_column(Object_id column_id) {
    return Abstract_table_impl::get_column(column_id);
  }
  const Column *get_column(const String_type &name) const {
    return Abstract_table_impl::get_column(name);
  }
  Column *get_column(const String_type &name) {
    return Abstract_table_impl::get_column(name);
  }
  virtual bool update_aux_key(Aux_key *key) const {
    return Table::update_aux_key(key);
  }
  virtual enum_hidden_type hidden() const {
    return Abstract_table_impl::hidden();
  }
  virtual void set_hidden(enum_hidden_type hidden) {
    Abstract_table_impl::set_hidden(hidden);
  }

  /////////////////////////////////////////////////////////////////////////
  // Trigger collection.
  /////////////////////////////////////////////////////////////////////////

  virtual bool has_trigger() const { return (m_triggers.size() > 0); }

  virtual const Trigger_collection &triggers() const { return m_triggers; }

  virtual Trigger_collection *triggers() { return &m_triggers; }

  virtual void copy_triggers(const Table *tab_obj);

  virtual Trigger *add_trigger(Trigger::enum_action_timing at,
                               Trigger::enum_event_type et);

  virtual const Trigger *get_trigger(const char *name) const;

  virtual Trigger *add_trigger_following(const Trigger *trigger,
                                         Trigger::enum_action_timing at,
                                         Trigger::enum_event_type et);

  virtual Trigger *add_trigger_preceding(const Trigger *trigger,
                                         Trigger::enum_action_timing at,
                                         Trigger::enum_event_type et);

  virtual void drop_trigger(const Trigger *trigger);

  virtual void drop_all_triggers();

 private:
  uint get_max_action_order(Trigger::enum_action_timing at,
                            Trigger::enum_event_type et) const;

  void reorder_action_order(Trigger::enum_action_timing at,
                            Trigger::enum_event_type et);

  Trigger_impl *create_trigger();

 public:
  /////////////////////////////////////////////////////////////////////////
  // Check constraints.
  /////////////////////////////////////////////////////////////////////////

  virtual Check_constraint *add_check_constraint();

  virtual const Check_constraint_collection &check_constraints() const {
    return m_check_constraints;
  }

  virtual Check_constraint_collection *check_constraints() {
    return &m_check_constraints;
  }

 private:
  // Fields.

  Object_id m_se_private_id;

  String_type m_engine;
  String_type m_comment;

  // Setting this to 0 means that every table will be checked by CHECK
  // TABLE FOR UPGRADE once, even if it was created in this version.
  // If we instead initialize to MYSQL_VERSION_ID, it will only run
  // CHECK TABLE FOR UPGRADE after a real upgrade.
  uint m_last_checked_for_upgrade_version_id = 0;
  Properties_impl m_se_private_data;
  enum_row_format m_row_format;
  bool m_is_temporary;

  // - Partitioning related fields.

  enum_partition_type m_partition_type;
  String_type m_partition_expression;
  String_type m_partition_expression_utf8;
  enum_default_partitioning m_default_partitioning;

  enum_subpartition_type m_subpartition_type;
  String_type m_subpartition_expression;
  String_type m_subpartition_expression_utf8;
  enum_default_partitioning m_default_subpartitioning;

  // References to tightly-coupled objects.

  Index_collection m_indexes;
  Foreign_key_collection m_foreign_keys;
  Foreign_key_parent_collection m_foreign_key_parents;
  Partition_collection m_partitions;
  Partition_leaf_vector m_leaf_partitions;
  Trigger_collection m_triggers;
  Check_constraint_collection m_check_constraints;

  // References to other objects.

  Object_id m_collation_id;
  Object_id m_tablespace_id;

  Table_impl(const Table_impl &src);
  Table_impl *clone() const { return new Table_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__TABLE_IMPL_INCLUDED
