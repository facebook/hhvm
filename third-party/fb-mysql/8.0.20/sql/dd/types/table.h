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

#ifndef DD__TABLE_INCLUDED
#define DD__TABLE_INCLUDED

#include "mysql_version.h"  // MYSQL_VERSION_ID

#include "sql/dd/sdi_fwd.h"               // Sdi_wcontext
#include "sql/dd/types/abstract_table.h"  // dd::Abstract_table
#include "sql/dd/types/foreign_key.h"     // IWYU pragma: keep
#include "sql/dd/types/index.h"           // IWYU pragma: keep
#include "sql/dd/types/trigger.h"         // dd::Trigger::enum_*

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Check_constraint;
class Partition;
class Table_impl;
class Trigger;

///////////////////////////////////////////////////////////////////////////

class Table : virtual public Abstract_table {
 public:
  typedef Table_impl Impl;
  typedef Collection<Index *> Index_collection;
  typedef Collection<Foreign_key *> Foreign_key_collection;
  typedef std::vector<Foreign_key_parent *> Foreign_key_parent_collection;
  typedef Collection<Partition *> Partition_collection;
  typedef Collection<Trigger *> Trigger_collection;
  typedef Collection<Check_constraint *> Check_constraint_collection;

  /*
    The type Partition_collection object 'own' the Partition* object. That
    means that the object Partition* would be deleted when the
    Partition_collection is deleted. However the Partition_leaf_vector type
    does not 'own' the Partition* object and points to one of element
    owned by Partition_collection. Deleting Partition_leaf_vector will not
    delete the Partition* objects pointed by it.
  */
  typedef std::vector<Partition *> Partition_leaf_vector;

  // We need a set of functions to update a preallocated se private id key,
  // which requires special handling for table objects.
  virtual bool update_aux_key(Aux_key *key) const {
    return update_aux_key(key, engine(), se_private_id());
  }

  static bool update_aux_key(Aux_key *key, const String_type &engine,
                             Object_id se_private_id);

 public:
  virtual ~Table() {}

 public:
  enum enum_row_format {
    RF_FIXED = 1,
    RF_DYNAMIC,
    RF_COMPRESSED,
    RF_REDUNDANT,
    RF_COMPACT,
    RF_PAGED
  };

  /* Keep in sync with subpartition type for forward compatibility.*/
  enum enum_partition_type {
    PT_NONE = 0,
    PT_HASH,
    PT_KEY_51,
    PT_KEY_55,
    PT_LINEAR_HASH,
    PT_LINEAR_KEY_51,
    PT_LINEAR_KEY_55,
    PT_RANGE,
    PT_LIST,
    PT_RANGE_COLUMNS,
    PT_LIST_COLUMNS,
    PT_AUTO,
    PT_AUTO_LINEAR,
  };

  enum enum_subpartition_type {
    ST_NONE = 0,
    ST_HASH,
    ST_KEY_51,
    ST_KEY_55,
    ST_LINEAR_HASH,
    ST_LINEAR_KEY_51,
    ST_LINEAR_KEY_55
  };

  /* Also used for default subpartitioning. */
  enum enum_default_partitioning { DP_NONE = 0, DP_NO, DP_YES, DP_NUMBER };

 public:
  /////////////////////////////////////////////////////////////////////////
  // is_temporary.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_temporary() const = 0;
  virtual void set_is_temporary(bool is_temporary) = 0;

  /////////////////////////////////////////////////////////////////////////
  // collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id collation_id() const = 0;
  virtual void set_collation_id(Object_id collation_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // tablespace.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id tablespace_id() const = 0;
  virtual void set_tablespace_id(Object_id tablespace_id) = 0;
  virtual bool is_explicit_tablespace() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const = 0;
  virtual void set_engine(const String_type &engine) = 0;

  /////////////////////////////////////////////////////////////////////////
  // row_format
  /////////////////////////////////////////////////////////////////////////
  virtual enum_row_format row_format() const = 0;
  virtual void set_row_format(enum_row_format row_format) = 0;

  /////////////////////////////////////////////////////////////////////////
  // comment
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;
  virtual void set_comment(const String_type &comment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last_checked_for_upgrade_version_id api
  /////////////////////////////////////////////////////////////////////////

  virtual uint last_checked_for_upgrade_version_id() const = 0;
  virtual void mark_as_checked_for_upgrade() = 0;

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const = 0;

  virtual Properties &se_private_data() = 0;
  virtual bool set_se_private_data(const String_type &se_private_data_raw) = 0;
  virtual bool set_se_private_data(const Properties &se_private_data) = 0;

  /////////////////////////////////////////////////////////////////////////
  // se_private_id.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id se_private_id() const = 0;
  virtual void set_se_private_id(Object_id se_private_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Partition related.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_partition_type partition_type() const = 0;
  virtual void set_partition_type(enum_partition_type partition_type) = 0;

  virtual enum_default_partitioning default_partitioning() const = 0;
  virtual void set_default_partitioning(
      enum_default_partitioning default_partitioning) = 0;

  virtual const String_type &partition_expression() const = 0;
  virtual void set_partition_expression(
      const String_type &partition_expression) = 0;

  virtual const String_type &partition_expression_utf8() const = 0;
  virtual void set_partition_expression_utf8(
      const String_type &partition_expression) = 0;

  virtual enum_subpartition_type subpartition_type() const = 0;
  virtual void set_subpartition_type(
      enum_subpartition_type subpartition_type) = 0;

  virtual enum_default_partitioning default_subpartitioning() const = 0;
  virtual void set_default_subpartitioning(
      enum_default_partitioning default_subpartitioning) = 0;

  virtual const String_type &subpartition_expression() const = 0;
  virtual void set_subpartition_expression(
      const String_type &subpartition_expression) = 0;

  virtual const String_type &subpartition_expression_utf8() const = 0;
  virtual void set_subpartition_expression_utf8(
      const String_type &subpartition_expression) = 0;

  /** Dummy method to be able to use Partition and Table interchangeably
  in templates. */
  const Table &table() const { return *this; }
  Table &table() { return *this; }

  /////////////////////////////////////////////////////////////////////////
  // Index collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Index *add_index() = 0;

  virtual Index *add_first_index() = 0;

  virtual const Index_collection &indexes() const = 0;

  virtual Index_collection *indexes() = 0;

  /////////////////////////////////////////////////////////////////////////
  // Foreign key collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Foreign_key *add_foreign_key() = 0;

  virtual const Foreign_key_collection &foreign_keys() const = 0;

  virtual Foreign_key_collection *foreign_keys() = 0;

  /////////////////////////////////////////////////////////////////////////
  // Foreign key parent collection.
  /////////////////////////////////////////////////////////////////////////

  // The Foreign_key_parent_collection represents a list of tables that
  // have a foreign key referencing this table. It is constructed when
  // the dd::Table object is fetched from disk, and it can be reloaded
  // from the DD tables on demand using 'reload_foreign_key_parents()'.

  virtual const Foreign_key_parent_collection &foreign_key_parents() const = 0;

  virtual bool reload_foreign_key_parents(THD *thd) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Partition collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Partition *add_partition() = 0;

  virtual const Partition_collection &partitions() const = 0;

  virtual Partition_collection *partitions() = 0;

  /*
    This API to list only the leaf partition entries of a table. This depicts
    the real physical partitions. In case of table with no sub-partitions,
    this API returns list of all partitions. In case of table with
    sub-partitions, the API lists only the sub-partition entries.

    @return Partition_leaf_vector  - List of pointers to dd::Partition objects.
  */
  virtual const Partition_leaf_vector &leaf_partitions() const = 0;

  virtual Partition_leaf_vector *leaf_partitions() = 0;

  /////////////////////////////////////////////////////////////////////////
  // Trigger collection.
  /////////////////////////////////////////////////////////////////////////

  /**
    Check if table has any triggers.

    @return true  - if there are triggers on the table installed.
    @return false - if not.
  */

  virtual bool has_trigger() const = 0;

  /**
    Get const reference to Trigger_collection.

    @return Trigger_collection& - Const reference to a collection of triggers.
  */

  virtual const Trigger_collection &triggers() const = 0;

  /**
    Get non-const pointer to Trigger_collection.

    @return Trigger_collection* - Pointer to collection of triggers.
  */

  virtual Trigger_collection *triggers() = 0;

  /**
    Copy all the triggers from another dd::Table object.

    @param tab_obj Pointer to Table from which the triggers are copied.
  */

  virtual void copy_triggers(const Table *tab_obj) = 0;

  /**
    Add new trigger to the table.

    @param at      - Action timing of the trigger to be added.
    @param et      - Event type of the trigger to be added.

    @return Trigger* - Pointer to new Trigger that is added to table.
  */

  virtual Trigger *add_trigger(Trigger::enum_action_timing at,
                               Trigger::enum_event_type et) = 0;

  /**
    Get dd::Trigger object for the given trigger name.

    @return Trigger* - Pointer to Trigger.
  */

  virtual const Trigger *get_trigger(const char *name) const = 0;

  /**
    Add new trigger just after the trigger specified in argument.

    @param trigger - dd::Trigger object after which the new
                     trigger should be created.
    @param at      - Action timing of the trigger to be added.
    @param et      - Event type of the trigger to be added.

    @return Trigger* - Pointer to newly created dd::Trigger object.
  */

  virtual Trigger *add_trigger_following(const Trigger *trigger,
                                         Trigger::enum_action_timing at,
                                         Trigger::enum_event_type et) = 0;

  /**
    Add new trigger just before the trigger specified in argument.

    @param trigger - dd::Trigger object before which the new
                     trigger should be created.
    @param at      - Action timing of the trigger to be added.
    @param et      - Event type of the trigger to be added.

    @return Trigger* - Pointer to newly created dd::Trigger object.
  */

  virtual Trigger *add_trigger_preceding(const Trigger *trigger,
                                         Trigger::enum_action_timing at,
                                         Trigger::enum_event_type et) = 0;

  /**
    Drop the given trigger object.

    The method returns void, as we just remove the trigger from
    dd::Collection (dd::Table_impl::m_triggers) in memory. The
    trigger will be removed from mysql.triggers DD table when the
    dd::Table object is stored/updated.

    @param trigger - dd::Trigger object to be dropped.
  */

  virtual void drop_trigger(const Trigger *trigger) = 0;

  /**
    Drop all the trigger on this dd::Table object.

    The method returns void, as we just remove the trigger from
    dd::Collection (dd::Table_impl::m_triggers) in memory. The
    trigger will be removed from mysql.triggers DD table when the
    dd::Table object is stored/updated.
  */

  virtual void drop_all_triggers() = 0;

  /////////////////////////////////////////////////////////////////////////
  // Check constraint collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Check_constraint *add_check_constraint() = 0;

  virtual const Check_constraint_collection &check_constraints() const = 0;

  virtual Check_constraint_collection *check_constraints() = 0;

 public:
  /**
    Allocate a new object graph and invoke the copy contructor for
    each object.

    @return pointer to dynamically allocated copy
  */
  virtual Table *clone() const = 0;

  /**
    Converts *this into json.

    Converts all member variables that are to be included in the sdi
    into json by transforming them appropriately and passing them to
    the rapidjson writer provided.

    @param wctx opaque context for data needed by serialization
    @param w rapidjson writer which will perform conversion to json

  */

  virtual void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const = 0;

  /**
    Re-establishes the state of *this by reading sdi information from
    the rapidjson DOM subobject provided.

    Cross-references encountered within this object are tracked in
    sdictx, so that they can be updated when the entire object graph
    has been established.

    @param rctx stores book-keeping information for the
    deserialization process
    @param val subobject of rapidjson DOM containing json
    representation of this object
  */

  virtual bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) = 0;
};

///////////////////////////////////////////////////////////////////////////

inline bool is_checked_for_upgrade(const Table &t) {
  return t.last_checked_for_upgrade_version_id() == MYSQL_VERSION_ID;
}
}  // namespace dd

#endif  // DD__TABLE_INCLUDED
