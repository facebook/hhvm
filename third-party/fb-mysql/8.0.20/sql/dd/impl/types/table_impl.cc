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

#include "sql/dd/impl/types/table_impl.h"

#include <string.h>
#include <set>
#include <sstream>
#include <string>

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"                         // ER_*
#include "sql/current_thd.h"                      // current_thd
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::bootstrap::DD_bootstrap_ctx
#include "sql/dd/impl/dictionary_impl.h"          // Dictionary_impl
#include "sql/dd/impl/properties_impl.h"          // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"           // Raw_record
#include "sql/dd/impl/raw/raw_record_set.h"       // Raw_record_set
#include "sql/dd/impl/raw/raw_table.h"            // Raw_table
#include "sql/dd/impl/sdi_impl.h"                 // sdi read/write functions
#include "sql/dd/impl/tables/check_constraints.h"  // Check_constraints
#include "sql/dd/impl/tables/columns.h"            // Columns
#include "sql/dd/impl/tables/foreign_keys.h"       // Foreign_keys
#include "sql/dd/impl/tables/indexes.h"            // Indexes
#include "sql/dd/impl/tables/schemata.h"           // Schemata
#include "sql/dd/impl/tables/table_partitions.h"   // Table_partitions
#include "sql/dd/impl/tables/tables.h"             // Tables
#include "sql/dd/impl/tables/triggers.h"           // Triggers
#include "sql/dd/impl/transaction_impl.h"          // Open_dictionary_tables_ctx
#include "sql/dd/impl/types/check_constraint_impl.h"  // Check_constraint_impl
#include "sql/dd/impl/types/foreign_key_impl.h"       // Foreign_key_impl
#include "sql/dd/impl/types/index_impl.h"             // Index_impl
#include "sql/dd/impl/types/partition_impl.h"         // Partition_impl
#include "sql/dd/impl/types/trigger_impl.h"           // Trigger_impl
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"   // dd::String_type
#include "sql/dd/types/column.h"  // Column
#include "sql/dd/types/foreign_key.h"
#include "sql/dd/types/index.h"
#include "sql/dd/types/partition.h"
#include "sql/dd/types/weak_object.h"
#include "sql/sql_class.h"

using dd::tables::Check_constraints;
using dd::tables::Foreign_keys;
using dd::tables::Indexes;
using dd::tables::Table_partitions;
using dd::tables::Tables;
using dd::tables::Triggers;

namespace dd {

class Sdi_rcontext;
class Sdi_wcontext;

///////////////////////////////////////////////////////////////////////////
// Table_impl implementation.
///////////////////////////////////////////////////////////////////////////

Table_impl::Table_impl()
    : m_se_private_id(INVALID_OBJECT_ID),
      m_se_private_data(),
      m_row_format(RF_FIXED),
      m_is_temporary(false),
      m_partition_type(PT_NONE),
      m_default_partitioning(DP_NONE),
      m_subpartition_type(ST_NONE),
      m_default_subpartitioning(DP_NONE),
      m_indexes(),
      m_foreign_keys(),
      m_partitions(),
      m_triggers(),
      m_check_constraints(),
      m_collation_id(INVALID_OBJECT_ID),
      m_tablespace_id(INVALID_OBJECT_ID) {}

Table_impl::~Table_impl() { delete_container_pointers(m_foreign_key_parents); }

///////////////////////////////////////////////////////////////////////////

bool Table_impl::validate() const {
  if (Abstract_table_impl::validate()) return true;

  if (m_collation_id == INVALID_OBJECT_ID) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Collation ID not set.");
    return true;
  }

  if (m_engine.empty()) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), DD_table::instance().name().c_str(),
             "Engine name is not set.");
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::load_foreign_key_parents(Open_dictionary_tables_ctx *otx) {
  /*
    Read information about FKs where this table is the parent.
    The relevant tables are already opened.
  */

  // 1. Read the parent's schema name based on schema_id.
  Raw_table *schema_table = otx->get_table<dd::Schema>();
  DBUG_ASSERT(schema_table);
  Primary_id_key schema_pk(schema_id());

  std::unique_ptr<Raw_record_set> schema_rs;
  if (schema_table->open_record_set(&schema_pk, schema_rs)) return true;

  Raw_record *schema_rec = schema_rs->current_record();
  DBUG_ASSERT(schema_rec);
  if (schema_rec == nullptr) return true;

  // 2. Build a key for searching the FK table.
  Table_reference_range_key parent_ref_key(
      tables::Foreign_keys::INDEX_K_REF_CATALOG_REF_SCHEMA_REF_TABLE,
      tables::Foreign_keys::FIELD_REFERENCED_TABLE_CATALOG,
      String_type(Dictionary_impl::default_catalog_name()),
      tables::Foreign_keys::FIELD_REFERENCED_TABLE_SCHEMA,
      schema_rec->read_str(tables::Schemata::FIELD_NAME),
      tables::Foreign_keys::FIELD_REFERENCED_TABLE, name());

  // 3. Get the FK record set where this table is parent.
  Raw_table *foreign_key_table = otx->get_table<dd::Foreign_key>();
  DBUG_ASSERT(foreign_key_table);

  std::unique_ptr<Raw_record_set> child_fk_rs;
  if (foreign_key_table->open_record_set(&parent_ref_key, child_fk_rs))
    return true;

  Raw_record *child_fk_rec = child_fk_rs->current_record();
  while (child_fk_rec) {
    // 4.1 Get the child table record based on the child table id.
    Primary_id_key child_pk(
        child_fk_rec->read_int(tables::Foreign_keys::FIELD_TABLE_ID));
    Raw_table *tables_table = otx->get_table<dd::Table>();
    DBUG_ASSERT(tables_table);

    std::unique_ptr<Raw_record_set> child_table_rs;
    if (tables_table->open_record_set(&child_pk, child_table_rs)) return true;

    Raw_record *child_table = child_table_rs->current_record();
    DBUG_ASSERT(child_table);
    if (child_table == nullptr) return true;

    /*
       4.2 Filter out child tables belonging to different SEs.
           This is not supported at the moment and we don't want
           such FKs to show up as Foreign_key_parent objects.
    */
    if (my_strcasecmp(
            system_charset_info,
            child_table->read_str(tables::Tables::FIELD_ENGINE).c_str(),
            m_engine.c_str()) != 0) {
      if (child_fk_rs->next(child_fk_rec)) return true;
      continue;
    }

    // 5. Get the child schema record based on schema id from the table record.
    schema_pk.update(child_table->read_int(tables::Tables::FIELD_SCHEMA_ID));
    schema_rs.reset(nullptr);  // Must end index read to allow new index read.
    if (schema_table->open_record_set(&schema_pk, schema_rs)) return true;

    schema_rec = schema_rs->current_record();
    DBUG_ASSERT(schema_rec);
    if (schema_rec == nullptr) return true;

    // 6. Collect the relevant information.
    Foreign_key_parent *fk_parent = add_foreign_key_parent();
    fk_parent->set_child_schema_name(
        schema_rec->read_str(tables::Schemata::FIELD_NAME));
    fk_parent->set_child_table_name(
        child_table->read_str(tables::Tables::FIELD_NAME));
    fk_parent->set_fk_name(
        child_fk_rec->read_str(tables::Foreign_keys::FIELD_NAME));

    Foreign_key::enum_rule update_rule = static_cast<Foreign_key::enum_rule>(
        child_fk_rec->read_int(tables::Foreign_keys::FIELD_UPDATE_RULE));

    fk_parent->set_update_rule(update_rule);

    Foreign_key::enum_rule delete_rule = static_cast<Foreign_key::enum_rule>(
        child_fk_rec->read_int(tables::Foreign_keys::FIELD_DELETE_RULE));

    fk_parent->set_delete_rule(delete_rule);

    // 7. Get next child record.
    if (child_fk_rs->next(child_fk_rec)) return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::reload_foreign_key_parents(THD *thd) {
  /*
     Use READ UNCOMMITTED isolation, so this method works correctly when
     called from the middle of atomic DDL statements.
   */
  dd::Transaction_ro trx(thd, ISO_READ_UNCOMMITTED);

  // Register and open tables.
  trx.otx.register_tables<dd::Table>();
  if (trx.otx.open_tables()) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  // Delete and reload the foreign key parents.
  delete_container_pointers(m_foreign_key_parents);

  return load_foreign_key_parents(&trx.otx);
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::restore_children(Open_dictionary_tables_ctx *otx) {
  // NOTE: the order of restoring collections is important because:
  //   - Index-objects reference Column-objects
  //     (thus, Column-objects must be loaded before Index-objects).
  //   - Foreign_key-objects reference both Index-objects and Column-objects.
  //     (thus, both Indexes and Columns must be loaded before FKs).
  //   - Partitions should be loaded at the end, as it refers to
  //     indexes.

  /*
    Do not load check constraints if upgrade is from the DD version before
    check constraints support. Check constraint support is introduced in 80016.
  */
  bool skip_check_constraints =
      (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80016));

  return (
      Abstract_table_impl::restore_children(otx) ||
      m_indexes.restore_items(this, otx, otx->get_table<Index>(),
                              Indexes::create_key_by_table_id(this->id())) ||
      m_foreign_keys.restore_items(
          this, otx, otx->get_table<Foreign_key>(),
          Foreign_keys::create_key_by_table_id(this->id()),
          Foreign_key_order_comparator()) ||
      m_partitions.restore_items(
          this, otx, otx->get_table<Partition>(),
          Table_partitions::create_key_by_parent_partition_id(
              this->id(), dd::INVALID_OBJECT_ID),
          // Sort partitions first on level and then on number.
          Partition_order_comparator()) ||
      m_triggers.restore_items(this, otx, otx->get_table<Trigger>(),
                               Triggers::create_key_by_table_id(this->id()),
                               Trigger_order_comparator()) ||
      load_foreign_key_parents(otx) ||
      (!skip_check_constraints &&
       m_check_constraints.restore_items(
           this, otx, otx->get_table<Check_constraint>(),
           Check_constraints::create_key_by_table_id(this->id()),
           Check_constraint_order_comparator())));
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::store_triggers(Open_dictionary_tables_ctx *otx) {
  /*
    There is a requirement to keep the collection items in
    following order.  The reason is,

    Suppose we are updating a dd::Table object with,
      a) We already have a trigger 't1' with ID 1.
      b) We added a new trigger 't2' added preceding to 't1'.
    We have a row for a) in (DD) disk with action_order=1.

    The expectation is that row b) should have action_order=1
    and row a) should have action_order=2.

    If we try to store row b) first with action_order=1, then
    there is possibility violating the constraint
      "UNIQUE KEY (table_id, event_type,
                   action_timing, action_order)"
    because row a) might also contain the same event_type and
    action_timing as that of b). And we would fail inserting
    row b).

    This demands us to drop all the triggers which are already
    present on disk and then store any new triggers.  This
    would not violate the above unique constraint.

    However we should avoid trying to drop triggers if no triggers
    existed before. Such an attempt will lead to index lookup which
    might cause acquisition of gap lock on index supremum in InnoDB.
    This might lead to deadlock if two independent CREATE TRIGGER
    are executed concurrently and both acquire gap locks on index
    supremum first and then try to insert their records into this gap.
  */
  bool needs_delete = m_triggers.has_removed_items();

  if (!needs_delete) {
    /* Check if there are any non-new Trigger objects. */
    for (const Trigger *trigger : *triggers()) {
      if (trigger->id() != INVALID_OBJECT_ID) {
        needs_delete = true;
        break;
      }
    }
  }

  if (needs_delete) {
    if (m_triggers.drop_items(otx, otx->get_table<Trigger>(),
                              Triggers::create_key_by_table_id(this->id())))
      return true;

    /*
      In-case a trigger is dropped, we need to avoid dropping it
      second time. So clear all the removed items.
    */
    m_triggers.clear_removed_items();
  }

  // Store the items.
  return m_triggers.store_items(otx);
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::store_children(Open_dictionary_tables_ctx *otx) {
  /*
    Do not store check constraints if upgrade is from the DD version before
    check constraints support. Check constraint support is introduced in 80016.
  */
  bool skip_check_constraints =
      (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80016));

  return Abstract_table_impl::store_children(otx) ||
         // Note that indexes has to be stored first, as
         // partitions refer indexes.
         m_indexes.store_items(otx) || m_foreign_keys.store_items(otx) ||
         m_partitions.store_items(otx) || store_triggers(otx) ||
         (!skip_check_constraints && m_check_constraints.store_items(otx));
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::drop_children(Open_dictionary_tables_ctx *otx) const {
  // Note that partition collection has to be dropped first
  // as it has foreign key to indexes.

  return m_check_constraints.drop_items(
             otx, otx->get_table<Check_constraint>(),
             Check_constraints::create_key_by_table_id(this->id())) ||
         m_triggers.drop_items(otx, otx->get_table<Trigger>(),
                               Triggers::create_key_by_table_id(this->id())) ||
         m_partitions.drop_items(
             otx, otx->get_table<Partition>(),
             Table_partitions::create_key_by_table_id(this->id())) ||
         m_foreign_keys.drop_items(
             otx, otx->get_table<Foreign_key>(),
             Foreign_keys::create_key_by_table_id(this->id())) ||
         m_indexes.drop_items(otx, otx->get_table<Index>(),
                              Indexes::create_key_by_table_id(this->id())) ||
         Abstract_table_impl::drop_children(otx);
}

/////////////////////////////////////////////////////////////////////////

bool Table_impl::restore_attributes(const Raw_record &r) {
  {
    enum_table_type table_type =
        static_cast<enum_table_type>(r.read_int(Tables::FIELD_TYPE));

    if (table_type != enum_table_type::BASE_TABLE) return true;
  }

  if (Abstract_table_impl::restore_attributes(r)) return true;

  m_comment = r.read_str(Tables::FIELD_COMMENT);
  m_row_format = (enum_row_format)r.read_int(Tables::FIELD_ROW_FORMAT);

  // Partitioning related fields (NULL -> enum value 0!)

  m_partition_type =
      (enum_partition_type)r.read_int(Tables::FIELD_PARTITION_TYPE, 0);

  m_default_partitioning = (enum_default_partitioning)r.read_int(
      Tables::FIELD_DEFAULT_PARTITIONING, 0);

  m_subpartition_type =
      (enum_subpartition_type)r.read_int(Tables::FIELD_SUBPARTITION_TYPE, 0);

  m_default_subpartitioning = (enum_default_partitioning)r.read_int(
      Tables::FIELD_DEFAULT_SUBPARTITIONING, 0);

  // Special cases dealing with NULL values for nullable fields

  m_se_private_id = dd::tables::Tables::read_se_private_id(r);

  m_collation_id = r.read_ref_id(Tables::FIELD_COLLATION_ID);
  m_tablespace_id = r.read_ref_id(Tables::FIELD_TABLESPACE_ID);

  set_se_private_data(r.read_str(Tables::FIELD_SE_PRIVATE_DATA, ""));

  m_engine = r.read_str(Tables::FIELD_ENGINE);

  // m_last_checked_for_upgrade_version added in 80012
  if (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80013)) {
    m_last_checked_for_upgrade_version_id = 0;
  } else {
    m_last_checked_for_upgrade_version_id =
        r.read_int(Tables::FIELD_LAST_CHECKED_FOR_UPGRADE_VERSION_ID, 0);
  }
  m_partition_expression = r.read_str(Tables::FIELD_PARTITION_EXPRESSION, "");
  m_partition_expression_utf8 =
      r.read_str(Tables::FIELD_PARTITION_EXPRESSION_UTF8, "");
  m_subpartition_expression =
      r.read_str(Tables::FIELD_SUBPARTITION_EXPRESSION, "");
  m_subpartition_expression_utf8 =
      r.read_str(Tables::FIELD_SUBPARTITION_EXPRESSION_UTF8, "");

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::store_attributes(Raw_record *r) {
  //
  // Special cases dealing with NULL values for nullable fields
  //   - Store NULL if version is not set
  //     Eg: USER_VIEW or SYSTEM_VIEW may not have version set
  //   - Store NULL if se_private_id is not set
  //     Eg: A non-innodb table may not have se_private_id
  //   - Store NULL if collation id is not set
  //     Eg: USER_VIEW will not have collation id set.
  //   - Store NULL if tablespace id is not set
  //     Eg: A non-innodb table may not have tablespace
  //   - Store NULL in options if there are no key=value pairs
  //   - Store NULL in se_private_data if there are no key=value pairs
  //   - Store NULL in partition type if not set.
  //   - Store NULL in partition expression if not set.
  //   - Store NULL in default partitioning if not set.
  //   - Store NULL in subpartition type if not set.
  //   - Store NULL in subpartition expression if not set.
  //   - Store NULL in default subpartitioning if not set.
  //

  // Temporary table definitions are never persisted.
  DBUG_ASSERT(!m_is_temporary);

  // Store last_checked_for_upgrade_version_id only if we're not upgrading
  if (!bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80013) &&
      r->store(Tables::FIELD_LAST_CHECKED_FOR_UPGRADE_VERSION_ID,
               m_last_checked_for_upgrade_version_id)) {
    return true;
  }

  // Store field values
  return Abstract_table_impl::store_attributes(r) ||
         r->store(Tables::FIELD_ENGINE, m_engine) ||
         r->store_ref_id(Tables::FIELD_COLLATION_ID, m_collation_id) ||
         r->store(Tables::FIELD_COMMENT, m_comment) ||
         r->store(Tables::FIELD_SE_PRIVATE_DATA, m_se_private_data) ||
         r->store(Tables::FIELD_SE_PRIVATE_ID, m_se_private_id,
                  m_se_private_id == (Object_id)-1) ||
         r->store(Tables::FIELD_ROW_FORMAT, m_row_format) ||
         r->store_ref_id(Tables::FIELD_TABLESPACE_ID, m_tablespace_id) ||
         r->store(Tables::FIELD_PARTITION_TYPE, m_partition_type,
                  m_partition_type == PT_NONE) ||
         r->store(Tables::FIELD_PARTITION_EXPRESSION, m_partition_expression,
                  m_partition_expression.empty()) ||
         r->store(Tables::FIELD_PARTITION_EXPRESSION_UTF8,
                  m_partition_expression_utf8,
                  m_partition_expression_utf8.empty()) ||
         r->store(Tables::FIELD_DEFAULT_PARTITIONING, m_default_partitioning,
                  m_default_partitioning == DP_NONE) ||
         r->store(Tables::FIELD_SUBPARTITION_TYPE, m_subpartition_type,
                  m_subpartition_type == ST_NONE) ||
         r->store(Tables::FIELD_SUBPARTITION_EXPRESSION,
                  m_subpartition_expression,
                  m_subpartition_expression.empty()) ||
         r->store(Tables::FIELD_SUBPARTITION_EXPRESSION_UTF8,
                  m_subpartition_expression_utf8,
                  m_subpartition_expression_utf8.empty()) ||
         r->store(Tables::FIELD_DEFAULT_SUBPARTITIONING,
                  m_default_subpartitioning,
                  m_default_subpartitioning == DP_NONE);
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::serialize(Sdi_wcontext *wctx, Sdi_writer *w) const {
  // Temporary table definitions are never persisted.
  DBUG_ASSERT(!m_is_temporary);

  w->StartObject();
  Abstract_table_impl::serialize(wctx, w);
  write(w, m_se_private_id, STRING_WITH_LEN("se_private_id"));
  write(w, m_engine, STRING_WITH_LEN("engine"));
  write(w, m_last_checked_for_upgrade_version_id,
        STRING_WITH_LEN("last_checked_for_upgrade_version_id"));
  write(w, m_comment, STRING_WITH_LEN("comment"));
  write_properties(w, m_se_private_data, STRING_WITH_LEN("se_private_data"));
  write_enum(w, m_row_format, STRING_WITH_LEN("row_format"));
  write_enum(w, m_partition_type, STRING_WITH_LEN("partition_type"));
  write(w, m_partition_expression, STRING_WITH_LEN("partition_expression"));
  write(w, m_partition_expression_utf8,
        STRING_WITH_LEN("partition_expression_utf8"));
  write_enum(w, m_default_partitioning,
             STRING_WITH_LEN("default_partitioning"));
  write_enum(w, m_subpartition_type, STRING_WITH_LEN("subpartition_type"));
  write(w, m_subpartition_expression,
        STRING_WITH_LEN("subpartition_expression"));
  write(w, m_subpartition_expression_utf8,
        STRING_WITH_LEN("subpartition_expression_utf8"));
  write_enum(w, m_default_subpartitioning,
             STRING_WITH_LEN("default_subpartitioning"));
  serialize_each(wctx, w, m_indexes, STRING_WITH_LEN("indexes"));
  serialize_each(wctx, w, m_foreign_keys, STRING_WITH_LEN("foreign_keys"));
  serialize_each(wctx, w, m_check_constraints,
                 STRING_WITH_LEN("check_constraints"));
  serialize_each(wctx, w, m_partitions, STRING_WITH_LEN("partitions"));
  write(w, m_collation_id, STRING_WITH_LEN("collation_id"));
  serialize_tablespace_ref(wctx, w, m_tablespace_id,
                           STRING_WITH_LEN("tablespace_ref"));
  w->EndObject();
}

///////////////////////////////////////////////////////////////////////////

bool Table_impl::deserialize(Sdi_rcontext *rctx, const RJ_Value &val) {
  Abstract_table_impl::deserialize(rctx, val);
  read(&m_se_private_id, val, "se_private_id");
  read(&m_engine, val, "engine");
  read(&m_last_checked_for_upgrade_version_id, val,
       "last_checked_for_upgrade_version_id");
  read(&m_comment, val, "comment");
  read_properties(&m_se_private_data, val, "se_private_data");
  read_enum(&m_row_format, val, "row_format");
  read_enum(&m_partition_type, val, "partition_type");
  read(&m_partition_expression, val, "partition_expression");
  read(&m_partition_expression_utf8, val, "partition_expression_utf8");
  read_enum(&m_default_partitioning, val, "default_partitioning");
  read_enum(&m_subpartition_type, val, "subpartition_type");
  read(&m_subpartition_expression, val, "subpartition_expression");
  read(&m_subpartition_expression_utf8, val, "subpartition_expression_utf8");
  read_enum(&m_default_subpartitioning, val, "default_subpartitioning");

  // Note! Deserialization of ordinal position cross-referenced
  // objects (i.e. Index and Column) must happen before deserializing
  // objects which refrence these objects:
  // Foreign_key_element -> Column,
  // Foreign_key         -> Index,
  // Index_element       -> Column,
  // Partition_index     -> Index
  // Otherwise the cross-references will not be deserialized correctly
  // (as we don't know the address of the referenced Column or Index
  // object).

  deserialize_each(
      rctx, [this]() { return add_index(); }, val, "indexes");

  deserialize_each(
      rctx, [this]() { return add_foreign_key(); }, val, "foreign_keys");
  deserialize_each(
      rctx, [this]() { return add_check_constraint(); }, val,
      "check_constraints");
  deserialize_each(
      rctx, [this]() { return add_partition(); }, val, "partitions");
  read(&m_collation_id, val, "collation_id");
  return deserialize_tablespace_ref(rctx, &m_tablespace_id, val,
                                    "tablespace_id");
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::debug_print(String_type &outb) const {
  String_type s;
  Abstract_table_impl::debug_print(s);

  dd::Stringstream_type ss;
  ss << "TABLE OBJECT: { " << s << "m_engine: " << m_engine << "; "
     << "m_last_checked_for_upgrade_version_id: "
     << m_last_checked_for_upgrade_version_id << "; "
     << "m_collation: {OID: " << m_collation_id << "}; "
     << "m_comment: " << m_comment << "; "
     << "m_se_private_data " << m_se_private_data.raw_string() << "; "
     << "m_se_private_id: {OID: " << m_se_private_id << "}; "
     << "m_row_format: " << m_row_format << "; "
     << "m_is_temporary: " << m_is_temporary << "; "
     << "m_tablespace: {OID: " << m_tablespace_id << "}; "
     << "m_partition_type " << m_partition_type << "; "
     << "m_default_partitioning " << m_default_partitioning << "; "
     << "m_partition_expression " << m_partition_expression << "; "
     << "m_partition_expression_utf8 " << m_partition_expression_utf8 << "; "
     << "m_subpartition_type " << m_subpartition_type << "; "
     << "m_default_subpartitioning " << m_default_subpartitioning << "; "
     << "m_subpartition_expression " << m_subpartition_expression << "; "
     << "m_subpartition_expression_utf8 " << m_subpartition_expression_utf8
     << "; "
     << "m_partitions: " << m_partitions.size() << " [ ";

  {
    for (const Partition *i : partitions()) {
      String_type sp;
      i->debug_print(sp);
      ss << sp << " | ";
    }
  }

  ss << "] m_indexes: " << m_indexes.size() << " [ ";

  {
    for (const Index *i : indexes()) {
      String_type si;
      i->debug_print(si);
      ss << si << " | ";
    }
  }

  ss << "] m_foreign_keys: " << m_foreign_keys.size() << " [ ";

  {
    for (const Foreign_key *fk : foreign_keys()) {
      String_type sfk;
      fk->debug_print(sfk);
      ss << sfk << " | ";
    }
  }

  ss << "] m_check_constraints: " << m_check_constraints.size() << " [ ";

  {
    for (const Check_constraint *cc : check_constraints()) {
      String_type scc;
      cc->debug_print(scc);
      ss << scc << " | ";
    }
  }

  ss << "] m_triggers: " << m_triggers.size() << " [ ";

  {
    for (const Trigger *trig : triggers()) {
      String_type st;
      trig->debug_print(st);
      ss << st << " | ";
    }
  }
  ss << "] ";

  ss << " }";

  outb = ss.str();
}

///////////////////////////////////////////////////////////////////////////
// Index collection.
///////////////////////////////////////////////////////////////////////////

Index *Table_impl::add_index() {
  Index_impl *i = new (std::nothrow) Index_impl(this);
  m_indexes.push_back(i);
  return i;
}

///////////////////////////////////////////////////////////////////////////

Index *Table_impl::add_first_index() {
  Index_impl *i = new (std::nothrow) Index_impl(this);
  m_indexes.push_front(i);
  return i;
}

///////////////////////////////////////////////////////////////////////////

Index *Table_impl::get_index(Object_id index_id) {
  for (Index *i : m_indexes) {
    if (i->id() == index_id) return i;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////
// Foreign key collection.
///////////////////////////////////////////////////////////////////////////

Foreign_key *Table_impl::add_foreign_key() {
  Foreign_key_impl *fk = new (std::nothrow) Foreign_key_impl(this);
  m_foreign_keys.push_back(fk);
  return fk;
}

///////////////////////////////////////////////////////////////////////////
// Foreign key parent collection.
///////////////////////////////////////////////////////////////////////////

Foreign_key_parent *Table_impl::add_foreign_key_parent() {
  Foreign_key_parent *fk_parent = new (std::nothrow) Foreign_key_parent();
  m_foreign_key_parents.push_back(fk_parent);
  return fk_parent;
}

///////////////////////////////////////////////////////////////////////////
// Partition collection.
///////////////////////////////////////////////////////////////////////////

Partition *Table_impl::add_partition() {
  Partition_impl *i = new (std::nothrow) Partition_impl(this);
  m_partitions.push_back(i);

  return i;
}

///////////////////////////////////////////////////////////////////////////

Partition *Table_impl::get_partition(Object_id partition_id) {
  for (Partition *i : m_partitions) {
    if (i->id() == partition_id) return i;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////
// Trigger collection.
///////////////////////////////////////////////////////////////////////////

uint Table_impl::get_max_action_order(Trigger::enum_action_timing at,
                                      Trigger::enum_event_type et) const {
  uint max_order = 0;
  for (const Trigger *trig : triggers()) {
    if (trig->action_timing() == at && trig->event_type() == et) max_order++;
  }

  return max_order;
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::reorder_action_order(Trigger::enum_action_timing at,
                                      Trigger::enum_event_type et) {
  uint new_order = 1;
  for (Trigger *trigger : *triggers()) {
    if (trigger->action_timing() == at && trigger->event_type() == et)
      trigger->set_action_order(new_order++);
  }
}

///////////////////////////////////////////////////////////////////////////

Trigger_impl *Table_impl::create_trigger() {
  Trigger_impl *trigger = new (std::nothrow) Trigger_impl(this);
  if (trigger == nullptr) return nullptr;

  THD *thd = current_thd;
  trigger->set_created(thd->query_start_timeval_trunc(2));
  trigger->set_last_altered(thd->query_start_timeval_trunc(2));

  return trigger;
}

///////////////////////////////////////////////////////////////////////////

Trigger *Table_impl::add_trigger(Trigger::enum_action_timing at,
                                 Trigger::enum_event_type et) {
  Trigger_impl *trigger = create_trigger();
  if (trigger == nullptr) return nullptr;

  m_triggers.push_back(trigger);
  trigger->set_action_timing(at);
  trigger->set_event_type(et);
  trigger->set_action_order(get_max_action_order(at, et));

  return trigger;
}

///////////////////////////////////////////////////////////////////////////

const Trigger *Table_impl::get_trigger(const char *name) const {
  const uchar *src_trg_name = pointer_cast<const uchar *>(name);
  size_t src_trg_name_len = strlen(name);
  for (const Trigger *trigger : triggers()) {
    if (!my_strnncoll(dd::tables::Triggers::name_collation(), src_trg_name,
                      src_trg_name_len,
                      pointer_cast<const uchar *>(trigger->name().c_str()),
                      trigger->name().length()))
      return trigger;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////

Trigger *Table_impl::add_trigger_following(const Trigger *trigger,
                                           Trigger::enum_action_timing at,
                                           Trigger::enum_event_type et) {
  DBUG_ASSERT(trigger != nullptr && trigger->action_timing() == at &&
              trigger->event_type() == et);

  // Allocate new Trigger object.
  Trigger_impl *new_trigger = create_trigger();
  if (new_trigger == nullptr) return nullptr;

  Trigger_collection::iterator it =
      m_triggers.find(dynamic_cast<const Trigger_impl *>(trigger));

  if (++it != m_triggers.end())
    m_triggers.insert(it, new_trigger);
  else
    m_triggers.push_back(new_trigger);

  new_trigger->set_action_timing(at);
  new_trigger->set_event_type(et);

  reorder_action_order(at, et);
  return new_trigger;
}

///////////////////////////////////////////////////////////////////////////

Trigger *Table_impl::add_trigger_preceding(const Trigger *trigger,
                                           Trigger::enum_action_timing at,
                                           Trigger::enum_event_type et) {
  DBUG_ASSERT(trigger != nullptr && trigger->action_timing() == at &&
              trigger->event_type() == et);

  Trigger_impl *new_trigger = create_trigger();
  if (new_trigger == nullptr) return nullptr;

  new_trigger->set_action_timing(at);
  new_trigger->set_event_type(et);

  Trigger_collection::iterator it =
      m_triggers.find(dynamic_cast<const Trigger_impl *>(trigger));
  m_triggers.insert(it, new_trigger);

  reorder_action_order(at, et);

  return new_trigger;
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::copy_triggers(const Table *tab_obj) {
  DBUG_ASSERT(tab_obj != nullptr);

  for (const Trigger *trig : tab_obj->triggers()) {
    /*
      Reset the trigger primary key ID, so that a new row is
      created for them, when the object is stored. Following is
      the issue if we don't do that.

      * When the triggers are copied by dd::Table::copy_triggers(),
        it retained the old trigger ID's. This is fine in theory
        to re-use ID. But see below points.

      * thd->dd_client()->update() updates the dd::Table object which
        contains the moved triggers. The DD framework would insert
        these triggers with same old trigger ID in mysql.triggers.id.
        This too is fine.

      * After inserting a row, we set dd::Trigger_impl::m_id
        only if a new id m_table->file->insert_id_for_cur_row was
        generated. The problem here is that there was no new row ID
        generated as we did retain old mysql.triggers.id. Hence we
        end-up marking the dd::Trigger_impl::m_id as INVALID_OBJECT_ID.
        Note that the value stored in DD is now difference than the
        value in in-memory dd::Trigger_impl object.

      * Later if the same object is updated (may be rename operation)
        then as the dd::Trigger_impl::m_id is INVALID_OBJECT_ID, we
        end-up creating a duplicate row which already exists.

      So, It is not necessary to retain the old trigger ID's, the
      dd::Table::copy_triggers() API now sets the ID's of cloned
      trigger objects to INVALID_OBJECT_ID. This will work fine as the
      m_table->file->insert_id_for_cur_row gets generated as expected
      and the trigger metadata on DD table mysql.triggers and in-memory
      DD object dd::Trigger_impl would both be same.
    */
    Trigger_impl *new_trigger =
        new Trigger_impl(*dynamic_cast<const Trigger_impl *>(trig), this);
    DBUG_ASSERT(new_trigger != nullptr);

    new_trigger->set_id(INVALID_OBJECT_ID);

    m_triggers.push_back(new_trigger);
  }
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::drop_all_triggers() { m_triggers.remove_all(); }

///////////////////////////////////////////////////////////////////////////

void Table_impl::drop_trigger(const Trigger *trigger) {
  DBUG_ASSERT(trigger != nullptr);
  dd::Trigger::enum_action_timing at = trigger->action_timing();
  dd::Trigger::enum_event_type et = trigger->event_type();

  m_triggers.remove(
      dynamic_cast<Trigger_impl *>(const_cast<Trigger *>(trigger)));

  reorder_action_order(at, et);
}

///////////////////////////////////////////////////////////////////////////
// Check constraint collection.
///////////////////////////////////////////////////////////////////////////

Check_constraint *Table_impl::add_check_constraint() {
  Check_constraint_impl *cc = new (std::nothrow) Check_constraint_impl(this);
  if (cc != nullptr) m_check_constraints.push_back(cc);
  return cc;
}

///////////////////////////////////////////////////////////////////////////

Partition *Table_impl::get_partition(const String_type &name) {
  for (Partition *i : m_partitions) {
    if (i->name() == name) return i;
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////

bool Table::update_aux_key(Aux_key *key, const String_type &engine,
                           Object_id se_private_id) {
  if (se_private_id != INVALID_OBJECT_ID)
    return Tables::update_aux_key(key, engine, se_private_id);

  return true;
}

///////////////////////////////////////////////////////////////////////////

void Table_impl::register_tables(Open_dictionary_tables_ctx *otx) {
  otx->add_table<Tables>();

  otx->register_tables<Schema>();
  otx->register_tables<Column>();
  otx->register_tables<Index>();
  otx->register_tables<Foreign_key>();
  otx->register_tables<Partition>();
  otx->register_tables<Trigger>();
  /*
    Do not register check constraint table if upgrade is from the DD version
    before check constraints support. Check constraint is introduced in 8.0.15.
  */
  if (!bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80016))
    otx->register_tables<Check_constraint>();
}

///////////////////////////////////////////////////////////////////////////

Table_impl::Table_impl(const Table_impl &src)
    : Weak_object(src),
      Abstract_table_impl(src),
      m_se_private_id(src.m_se_private_id),
      m_engine(src.m_engine),
      m_comment(src.m_comment),
      m_last_checked_for_upgrade_version_id{
          src.m_last_checked_for_upgrade_version_id},
      m_se_private_data(src.m_se_private_data),
      m_row_format(src.m_row_format),
      m_is_temporary(src.m_is_temporary),
      m_partition_type(src.m_partition_type),
      m_partition_expression(src.m_partition_expression),
      m_partition_expression_utf8(src.m_partition_expression_utf8),
      m_default_partitioning(src.m_default_partitioning),
      m_subpartition_type(src.m_subpartition_type),
      m_subpartition_expression(src.m_subpartition_expression),
      m_subpartition_expression_utf8(src.m_subpartition_expression_utf8),
      m_default_subpartitioning(src.m_default_subpartitioning),
      m_indexes(),
      m_foreign_keys(),
      m_partitions(),
      m_triggers(),
      m_check_constraints(),
      m_collation_id(src.m_collation_id),
      m_tablespace_id(src.m_tablespace_id) {
  m_indexes.deep_copy(src.m_indexes, this);
  m_foreign_keys.deep_copy(src.m_foreign_keys, this);
  for (auto fk_parent : src.m_foreign_key_parents)
    m_foreign_key_parents.push_back(new (std::nothrow)
                                        Foreign_key_parent(*fk_parent));
  m_partitions.deep_copy(src.m_partitions, this);
  m_triggers.deep_copy(src.m_triggers, this);
  m_check_constraints.deep_copy(src.m_check_constraints, this);
}
}  // namespace dd
