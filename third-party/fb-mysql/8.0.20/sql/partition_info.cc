/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/partition_info.h"  // LIST_PART_ENTRY

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "mysql/psi/psi_base.h"
#include "mysql/udf_registration_types.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // *_ACL
#include "sql/create_field.h"
#include "sql/derror.h"  // ER_THD
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/partitioning/partition_handler.h"  // PART_DEF_NAME, Partition_share
#include "sql/set_var.h"
#include "sql/sql_base.h"   // fill_record
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // test_if_data_home_dir
#include "sql/sql_partition.h"
#include "sql/sql_tablespace.h"  // validate_tablespace_name
#include "sql/system_variables.h"
#include "sql/table.h"                     // TABLE_LIST
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thr_malloc.h"
#include "sql/trigger_chain.h"  // Trigger_chain
#include "sql/trigger_def.h"
#include "sql_string.h"
#include "varlen_sort.h"

using std::string;

// TODO: Create ::get_copy() for getting a deep copy.

partition_info *partition_info::get_clone(THD *thd, bool reset /* = false */) {
  DBUG_TRACE;
  List_iterator<partition_element> part_it(partitions);
  partition_element *part;
  partition_info *clone = new (thd->mem_root) partition_info(*this);
  if (!clone) {
    mem_alloc_error(sizeof(partition_info));
    return nullptr;
  }
  new (&(clone->read_partitions)) MY_BITMAP;
  new (&(clone->lock_partitions)) MY_BITMAP;
  clone->bitmaps_are_initialized = false;
  clone->partitions.empty();
  clone->temp_partitions.empty();

  while ((part = (part_it++))) {
    List_iterator<partition_element> subpart_it(part->subpartitions);
    partition_element *subpart;
    partition_element *part_clone =
        new (thd->mem_root) partition_element(*part);
    if (!part_clone) {
      mem_alloc_error(sizeof(partition_element));
      return nullptr;
    }

    /* Explicitly copy the tablespace name, use the thd->mem_root. */
    if (part->tablespace_name != nullptr)
      part_clone->tablespace_name =
          strmake_root(thd->mem_root, part->tablespace_name,
                       strlen(part->tablespace_name) + 1);

    /*
      Mark that RANGE and LIST values needs to be fixed so that we don't
      use old values. fix_column_value_functions would evaluate the values
      from Item expression.
    */
    if (reset) {
      clone->defined_max_value = false;
      List_iterator<part_elem_value> list_it(part_clone->list_val_list);
      while (part_elem_value *list_value = list_it++) {
        part_column_list_val *col_val = list_value->col_val_array;
        for (uint i = 0; i < num_columns; col_val++, i++) {
          col_val->fixed = 0;
        }
      }
    }

    part_clone->subpartitions.empty();
    while ((subpart = (subpart_it++))) {
      partition_element *subpart_clone =
          new (thd->mem_root) partition_element(*subpart);
      if (!subpart_clone) {
        mem_alloc_error(sizeof(partition_element));
        return nullptr;
      }

      /* Explicitly copy the tablespace name, use the thd->mem_root. */
      if (subpart->tablespace_name != nullptr)
        subpart_clone->tablespace_name =
            strmake_root(thd->mem_root, subpart->tablespace_name,
                         strlen(subpart->tablespace_name) + 1);

      part_clone->subpartitions.push_back(subpart_clone);
    }
    clone->partitions.push_back(part_clone);
  }
  return clone;
}

partition_info *partition_info::get_full_clone(THD *thd) {
  partition_info *clone;
  DBUG_TRACE;
  clone = get_clone(thd);
  if (!clone) return nullptr;
  memcpy(&clone->read_partitions, &read_partitions, sizeof(read_partitions));
  memcpy(&clone->lock_partitions, &lock_partitions, sizeof(lock_partitions));
  clone->bitmaps_are_initialized = bitmaps_are_initialized;
  return clone;
}

/**
  Mark named [sub]partition to be used/locked.

  @param part_name  Partition name to match.
  @param length     Partition name length.

  @return Success if partition found
    @retval true  Partition found
    @retval false Partition not found
*/

bool partition_info::add_named_partition(const char *part_name, size_t length) {
  PART_NAME_DEF *part_def;
  Partition_share *part_share;
  DBUG_TRACE;
  DBUG_ASSERT(table && table->s && table->s->ha_share);
  part_share = static_cast<Partition_share *>((table->s->ha_share));
  DBUG_ASSERT(part_share->partition_name_hash != nullptr);
  auto part_name_hash = part_share->partition_name_hash.get();
  DBUG_ASSERT(!part_name_hash->empty());

  part_def = find_or_nullptr(*part_name_hash, string(part_name, length));
  if (!part_def) {
    my_error(ER_UNKNOWN_PARTITION, MYF(0), part_name, table->alias);
    return true;
  }

  if (part_def->is_subpart) {
    bitmap_set_bit(&read_partitions, part_def->part_id);
  } else {
    if (is_sub_partitioned()) {
      /* Mark all subpartitions in the partition */
      uint j, start = part_def->part_id;
      uint end = start + num_subparts;
      for (j = start; j < end; j++) bitmap_set_bit(&read_partitions, j);
    } else
      bitmap_set_bit(&read_partitions, part_def->part_id);
  }
  DBUG_PRINT("info", ("Found partition %u is_subpart %d for name %s",
                      part_def->part_id, part_def->is_subpart, part_name));
  return false;
}

/**
  Mark named [sub]partition to be used/locked.
*/

bool partition_info::set_named_partition_bitmap(const char *part_name,
                                                size_t length) {
  DBUG_TRACE;
  bitmap_clear_all(&read_partitions);
  if (add_named_partition(part_name, length)) return true;
  bitmap_copy(&lock_partitions, &read_partitions);
  return false;
}

/**
  Prune away partitions not mentioned in the PARTITION () clause,
  if used.

  @return Operation status
    @retval false Success
    @retval true  Failure
*/
bool partition_info::set_read_partitions(List<String> *partition_names) {
  DBUG_TRACE;
  if (!partition_names || !partition_names->elements) {
    return true;
  }

  uint num_names = partition_names->elements;
  List_iterator<String> partition_names_it(*partition_names);
  uint i = 0;
  /*
    TODO: When adding support for FK in partitioned tables, the referenced
    table must probably lock all partitions for read, and also write depending
    of ON DELETE/UPDATE.
  */
  bitmap_clear_all(&read_partitions);

  /* No check for duplicate names or overlapping partitions/subpartitions. */

  DBUG_PRINT("info", ("Searching through partition_name_hash"));
  do {
    String *part_name_str = partition_names_it++;
    if (add_named_partition(part_name_str->c_ptr(), part_name_str->length()))
      return true;
  } while (++i < num_names);
  return false;
}

/**
  Set read/lock_partitions bitmap over non pruned partitions

  @param table_list   Possible TABLE_LIST which can contain
                      list of partition names to query

  @return Operation status
    @retval false  OK
    @retval true   Failed to allocate memory for bitmap or list of partitions
                   did not match

  @note OK to call multiple times without the need for free_bitmaps.
*/

bool partition_info::set_partition_bitmaps(TABLE_LIST *table_list) {
  DBUG_TRACE;

  DBUG_ASSERT(bitmaps_are_initialized);
  DBUG_ASSERT(table);
  is_pruning_completed = false;
  if (!bitmaps_are_initialized) return true;

  if (table_list && table_list->partition_names &&
      table_list->partition_names->elements) {
    if (table->s->db_type()->partition_flags() & HA_USE_AUTO_PARTITION) {
      /*
        Don't allow PARTITION () clause on a NDB tables yet.
        TODO: Add partition name handling to NDB/partition_info.
        which is currently ha_partition specific.
      */
      my_error(ER_PARTITION_CLAUSE_ON_NONPARTITIONED, MYF(0));
      return true;
    }
    if (set_read_partitions(table_list->partition_names)) return true;
  } else {
    bitmap_set_all(&read_partitions);
    DBUG_PRINT("info", ("Set all partitions"));
  }
  bitmap_copy(&lock_partitions, &read_partitions);
  DBUG_ASSERT(bitmap_get_first_set(&lock_partitions) != MY_BIT_NONE);
  return false;
}

/**
  Checks if possible to do prune partitions on insert.

  @param thd           Thread context
  @param duplic        How to handle duplicates
  @param update        In case of ON DUPLICATE UPDATE, default function fields
  @param update_fields In case of ON DUPLICATE UPDATE, which fields to update
  @param fields        Listed fields
  @param empty_values  True if values is empty (only defaults)
  @param[out] prune_needs_default_values  Set on return if copying of default
                                          values is needed
  @param[out] can_prune_partitions        Enum showing if possible to prune
  @param[in,out] used_partitions          If possible to prune the bitmap
                                          is initialized and cleared

  @return Operation status
    @retval false  Success
    @retval true   Failure
*/

bool partition_info::can_prune_insert(THD *thd, enum_duplicates duplic,
                                      COPY_INFO &update,
                                      List<Item> &update_fields,
                                      List<Item> &fields, bool empty_values,
                                      enum_can_prune *can_prune_partitions,
                                      bool *prune_needs_default_values,
                                      MY_BITMAP *used_partitions) {
  *can_prune_partitions = PRUNE_NO;
  DBUG_ASSERT(bitmaps_are_initialized);
  DBUG_TRACE;

  if (table->s->db_type()->partition_flags() & HA_USE_AUTO_PARTITION)
    return false; /* Should not insert prune NDB tables */

  /*
    If under LOCK TABLES pruning will skip start_stmt instead of external_lock
    for unused partitions.

    Cannot prune if there are BEFORE INSERT triggers that changes any
    partitioning column, since they may change the row to be in another
    partition.
  */
  if (table->triggers) {
    Trigger_chain *trigger_chain =
        table->triggers->get_triggers(TRG_EVENT_INSERT, TRG_ACTION_BEFORE);

    if (trigger_chain &&
        trigger_chain->has_updated_trigger_fields(&full_part_field_set))
      return false;
  }

  /*
    Can't prune partitions over generated columns, as their values are
    calculated much later.
  */
  if (table->vfield) {
    Field **fld;
    for (fld = table->vfield; *fld; fld++) {
      if (bitmap_is_set(&full_part_field_set, (*fld)->field_index))
        return false;
    }
  }

  /*
    Can't prune partitions over generated default expresssions, as their values
    are calculated much later.
  */
  if (table->gen_def_fields_ptr) {
    Field **fld;
    for (fld = table->gen_def_fields_ptr; *fld; fld++) {
      if (bitmap_is_set(&full_part_field_set, (*fld)->field_index))
        return false;
    }
  }

  if (table->found_next_number_field) {
    /*
      If the field is used in the partitioning expression, we cannot prune.
      TODO: If all rows have not null values and
      is not 0 (with NO_AUTO_VALUE_ON_ZERO sql_mode), then pruning is possible!
    */
    if (bitmap_is_set(&full_part_field_set,
                      table->found_next_number_field->field_index))
      return false;
  }

  /*
    If updating a field in the partitioning expression, we cannot prune.

    Note: TIMESTAMP_AUTO_SET_ON_INSERT is handled by converting Item_null
    to the start time of the statement. Which will be the same as in
    write_row(). So pruning of TIMESTAMP DEFAULT CURRENT_TIME will work.
    But TIMESTAMP_AUTO_SET_ON_UPDATE cannot be pruned if the timestamp
    column is a part of any part/subpart expression.
  */
  if (duplic == DUP_UPDATE) {
    /*
      Cannot prune if any field in the partitioning expression can
      be updated by ON DUPLICATE UPDATE.
    */
    if (update.function_defaults_apply_on_columns(&full_part_field_set))
      return false;

    /*
      TODO: add check for static update values, which can be pruned.
    */
    if (is_fields_in_part_expr(update_fields)) return false;

    /*
      Cannot prune if there are BEFORE UPDATE triggers that changes any
      partitioning column, since they may change the row to be in another
      partition.
    */
    if (table->triggers) {
      Trigger_chain *trigger_chain =
          table->triggers->get_triggers(TRG_EVENT_UPDATE, TRG_ACTION_BEFORE);

      if (trigger_chain &&
          trigger_chain->has_updated_trigger_fields(&full_part_field_set))
        return false;
    }
  }

  /*
    If not all partitioning fields are given,
    we also must set all non given partitioning fields
    to get correct defaults.
    TODO: If any gain, we could enhance this by only copy the needed default
    fields by
      1) check which fields needs to be set.
      2) only copy those fields from the default record.
  */
  *prune_needs_default_values = false;
  if (fields.elements) {
    if (!is_full_part_expr_in_fields(fields))
      *prune_needs_default_values = true;
  } else if (empty_values) {
    *prune_needs_default_values = true;  // like 'INSERT INTO t () VALUES ()'
  } else {
    /*
      In case of INSERT INTO t VALUES (...) we must get values for
      all fields in table from VALUES (...) part, so no defaults
      are needed.
    */
  }

  /*
    Pruning possible, have to initialize the used_partitions bitmap.
    This also clears all bits.
  */
  if (init_partition_bitmap(used_partitions, thd->mem_root)) return true;

  /*
    If no partitioning field in set (e.g. defaults) check pruning only once.
  */
  if (fields.elements && !is_fields_in_part_expr(fields))
    *can_prune_partitions = PRUNE_DEFAULTS;
  else
    *can_prune_partitions = PRUNE_YES;

  return false;
}

/**
  Mark the partition, the record belongs to, as used.

  @param fields           Fields to set
  @param values           Values to use
  @param info             COPY_INFO used for default values handling
  @param copy_default_values  True if we should copy default values
  @param used_partitions  Bitmap to set

  @returns Operational status
    @retval false  Success
    @retval true   Failure
  A return value of 'true' may indicate conversion error,
  so caller must check thd->is_error().
*/

bool partition_info::set_used_partition(List<Item> &fields, List<Item> &values,
                                        COPY_INFO &info,
                                        bool copy_default_values,
                                        MY_BITMAP *used_partitions) {
  THD *thd = table->in_use;
  uint32 part_id;
  longlong func_value;

  DBUG_ASSERT(thd);

  /* Only allow checking of constant values */
  List_iterator_fast<Item> v(values);
  Item *item;

  while ((item = v++)) {
    if (!item->const_item()) return true;
  }

  if (copy_default_values) restore_record(table, s->default_values);

  if (fields.elements || !values.elements) {
    if (fill_record(thd, table, fields, values, &full_part_field_set, nullptr,
                    false))
      return true;
  } else {
    if (fill_record(thd, table, table->field, values, &full_part_field_set,
                    nullptr, false))
      return true;
  }

  /*
    Evaluate DEFAULT functions like CURRENT_TIMESTAMP.
    TODO: avoid setting non partitioning fields default value, to avoid
    overhead. Not yet done, since mostly only one DEFAULT function per
    table, or at least very few such columns.
  */
  if (info.function_defaults_apply_on_columns(&full_part_field_set))
    info.set_function_defaults(table);

  {
    /*
      This function is used in INSERT; 'values' are supplied by user,
      or are default values, not values read from a table, so read_set is
      irrelevant.
    */
    my_bitmap_map *old_map = dbug_tmp_use_all_columns(table, table->read_set);
    const int rc = get_partition_id(this, &part_id, &func_value);
    dbug_tmp_restore_column_map(table->read_set, old_map);
    if (rc) return true;
  }

  DBUG_PRINT("info", ("Insert into partition %u", part_id));
  bitmap_set_bit(used_partitions, part_id);
  return false;
}

/*
  Create a memory area where default partition names are stored and fill it
  up with the names.

  SYNOPSIS
    create_default_partition_names()
    num_parts                       Number of partitions
    start_no                        Starting partition number
    subpart                         Is it subpartitions

  RETURN VALUE
    A pointer to the memory area of the default partition names

  DESCRIPTION
    A support routine for the partition code where default values are
    generated.
    The external routine needing this code is check_partition_info
*/

#define MAX_PART_NAME_SIZE 8

char *partition_info::create_default_partition_names(uint num_parts_arg,
                                                     uint start_no) {
  char *ptr = (char *)sql_calloc(num_parts_arg * MAX_PART_NAME_SIZE);
  char *move_ptr = ptr;
  uint i = 0;
  DBUG_TRACE;

  if (likely(ptr != nullptr)) {
    do {
      sprintf(move_ptr, "p%u", (start_no + i));
      move_ptr += MAX_PART_NAME_SIZE;
    } while (++i < num_parts_arg);
  } else {
    mem_alloc_error(num_parts_arg * MAX_PART_NAME_SIZE);
  }
  return ptr;
}

/*
  Generate a version string for partition expression
  This function must be updated every time there is a possibility for
  a new function of a higher version number than 5.5.0.

  SYNOPSIS
    set_show_version_string()
  RETURN VALUES
    None
*/
void partition_info::set_show_version_string(String *packet) {
  int version = 0;
  if (column_list)
    packet->append(STRING_WITH_LEN("\n/*!50500"));
  else {
    if (part_expr)
      part_expr->walk(&Item::intro_version, enum_walk::POSTFIX,
                      (uchar *)&version);
    if (subpart_expr)
      subpart_expr->walk(&Item::intro_version, enum_walk::POSTFIX,
                         (uchar *)&version);
    if (version == 0) {
      /* No new functions in partition function */
      packet->append(STRING_WITH_LEN("\n/*!50100"));
    } else {
      packet->append(STRING_WITH_LEN("\n/*!"));
      packet->append_longlong(version);
    }
  }
}

/*
  Create a unique name for the subpartition as part_name'sp''subpart_no'
  SYNOPSIS
    create_default_subpartition_name()
    subpart_no                  Number of subpartition
    part_name                   Name of partition
  RETURN VALUES
    >0                          A reference to the created name string
    0                           Memory allocation error
*/

char *partition_info::create_default_subpartition_name(uint subpart_no,
                                                       const char *part_name) {
  size_t size_alloc = strlen(part_name) + MAX_PART_NAME_SIZE;
  char *ptr = (char *)sql_calloc(size_alloc);
  DBUG_TRACE;

  if (likely(ptr != nullptr)) {
    snprintf(ptr, size_alloc, "%ssp%u", part_name, subpart_no);
  } else {
    mem_alloc_error(size_alloc);
  }
  return ptr;
}

/*
  Set up all the default partitions not set-up by the user in the SQL
  statement. Also perform a number of checks that the user hasn't tried
  to use default values where no defaults exists.

  SYNOPSIS
    set_up_default_partitions()
    file                A reference to a handler of the table
    info                Create info
    start_no            Starting partition number

  RETURN VALUE
    true                Error, attempted default values not possible
    false               Ok, default partitions set-up

  DESCRIPTION
    The routine uses the underlying handler of the partitioning to define
    the default number of partitions. For some handlers this requires
    knowledge of the maximum number of rows to be stored in the table.
    This routine only accepts HASH and KEY partitioning and thus there is
    no subpartitioning if this routine is successful.
    The external routine needing this code is check_partition_info
*/

bool partition_info::set_up_default_partitions(Partition_handler *part_handler,
                                               HA_CREATE_INFO *info,
                                               uint start_no) {
  uint i;
  char *default_name;
  bool result = true;
  DBUG_TRACE;

  if (part_type != partition_type::HASH) {
    const char *error_string;
    if (part_type == partition_type::RANGE)
      error_string = partition_keywords[PKW_RANGE].str;
    else
      error_string = partition_keywords[PKW_LIST].str;
    my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), error_string);
    goto end;
  }

  if (num_parts == 0) {
    if (!part_handler) {
      num_parts = 1;
    } else {
      num_parts = part_handler->get_default_num_partitions(info);
    }
    if (num_parts == 0) {
      my_error(ER_PARTITION_NOT_DEFINED_ERROR, MYF(0), "partitions");
      goto end;
    }
  }

  if (unlikely(num_parts > MAX_PARTITIONS)) {
    my_error(ER_TOO_MANY_PARTITIONS_ERROR, MYF(0));
    goto end;
  }
  if (unlikely((!(default_name =
                      create_default_partition_names(num_parts, start_no)))))
    goto end;
  i = 0;
  do {
    partition_element *part_elem = new (*THR_MALLOC) partition_element();
    if (likely(part_elem != nullptr && (!partitions.push_back(part_elem)))) {
      part_elem->engine_type = default_engine_type;
      part_elem->partition_name = default_name;
      default_name += MAX_PART_NAME_SIZE;
    } else {
      mem_alloc_error(sizeof(partition_element));
      goto end;
    }
  } while (++i < num_parts);
  result = false;
end:
  return result;
}

/*
  Set up all the default subpartitions not set-up by the user in the SQL
  statement. Also perform a number of checks that the default partitioning
  becomes an allowed partitioning scheme.

  SYNOPSIS
    set_up_default_subpartitions()
    file                A reference to a handler of the table
    info                Create info

  RETURN VALUE
    true                Error, attempted default values not possible
    false               Ok, default partitions set-up

  DESCRIPTION
    The routine uses the underlying handler of the partitioning to define
    the default number of partitions. For some handlers this requires
    knowledge of the maximum number of rows to be stored in the table.
    This routine is only called for RANGE or LIST partitioning and those
    need to be specified so only subpartitions are specified.
    The external routine needing this code is check_partition_info
*/

bool partition_info::set_up_default_subpartitions(
    Partition_handler *part_handler, HA_CREATE_INFO *info) {
  uint i, j;
  bool result = true;
  partition_element *part_elem;
  List_iterator<partition_element> part_it(partitions);
  DBUG_TRACE;

  if (num_subparts == 0) {
    if (!part_handler) {
      num_subparts = 1;
    } else {
      num_subparts = part_handler->get_default_num_partitions(info);
    }
  }
  if (unlikely((num_parts * num_subparts) > MAX_PARTITIONS)) {
    my_error(ER_TOO_MANY_PARTITIONS_ERROR, MYF(0));
    goto end;
  }
  i = 0;
  do {
    part_elem = part_it++;
    j = 0;
    do {
      partition_element *subpart_elem =
          new (*THR_MALLOC) partition_element(part_elem);
      if (likely(subpart_elem != nullptr &&
                 (!part_elem->subpartitions.push_back(subpart_elem)))) {
        char *ptr =
            create_default_subpartition_name(j, part_elem->partition_name);
        if (!ptr) goto end;
        subpart_elem->engine_type = default_engine_type;
        subpart_elem->partition_name = ptr;
      } else {
        mem_alloc_error(sizeof(partition_element));
        goto end;
      }
    } while (++j < num_subparts);
  } while (++i < num_parts);
  result = false;
end:
  return result;
}

/*
  Support routine for check_partition_info

  SYNOPSIS
    set_up_defaults_for_partitioning()
    file                A reference to a handler of the table
    info                Create info
    start_no            Starting partition number

  RETURN VALUE
    true                Error, attempted default values not possible
    false               Ok, default partitions set-up

  DESCRIPTION
    Set up defaults for partition or subpartition (cannot set-up for both,
    this will return an error.
*/

bool partition_info::set_up_defaults_for_partitioning(
    Partition_handler *part_handler, HA_CREATE_INFO *info, uint start_no) {
  DBUG_TRACE;

  if (!default_partitions_setup) {
    default_partitions_setup = true;
    if (use_default_partitions)
      return set_up_default_partitions(part_handler, info, start_no);
    if (is_sub_partitioned() && use_default_subpartitions)
      return set_up_default_subpartitions(part_handler, info);
  }
  return false;
}

/*
  Support routine for check_partition_info

  SYNOPSIS
    find_duplicate_field
    no parameters

  RETURN VALUE
    Erroneus field name  Error, there are two fields with same name
    NULL                 Ok, no field defined twice

  DESCRIPTION
    Check that the user haven't defined the same field twice in
    key or column list partitioning.
*/
char *partition_info::find_duplicate_field() {
  char *field_name_outer, *field_name_inner;
  List_iterator<char> it_outer(part_field_list);
  uint num_fields = part_field_list.elements;
  uint i, j;
  DBUG_TRACE;

  for (i = 0; i < num_fields; i++) {
    field_name_outer = it_outer++;
    List_iterator<char> it_inner(part_field_list);
    for (j = 0; j < num_fields; j++) {
      field_name_inner = it_inner++;
      if (i >= j) continue;
      if (!(my_strcasecmp(system_charset_info, field_name_outer,
                          field_name_inner))) {
        return field_name_outer;
      }
    }
  }
  return nullptr;
}

/**
  @brief Get part_elem and part_id from partition name

  @param partition_name Name of partition to search for.

  @param [out] part_id   Id of found partition or NOT_A_PARTITION_ID.

  @retval Pointer to part_elem of [sub]partition, if not found NULL

  @note Since names of partitions AND subpartitions must be unique,
  this function searches both partitions and subpartitions and if name of
  a partition is given for a subpartitioned table, part_elem will be
  the partition, but part_id will be NOT_A_PARTITION_ID and file_name not set.
*/
partition_element *partition_info::get_part_elem(const char *partition_name,
                                                 uint32 *part_id) {
  List_iterator<partition_element> part_it(partitions);
  uint i = 0;
  DBUG_TRACE;
  DBUG_ASSERT(part_id);
  *part_id = NOT_A_PARTITION_ID;
  do {
    partition_element *part_elem = part_it++;
    if (is_sub_partitioned()) {
      List_iterator<partition_element> sub_part_it(part_elem->subpartitions);
      uint j = 0;
      do {
        partition_element *sub_part_elem = sub_part_it++;
        if (!my_strcasecmp(system_charset_info, sub_part_elem->partition_name,
                           partition_name)) {
          *part_id = j + (i * num_subparts);
          return sub_part_elem;
        }
      } while (++j < num_subparts);

      /* Naming a partition (first level) on a subpartitioned table. */
      if (!my_strcasecmp(system_charset_info, part_elem->partition_name,
                         partition_name))
        return part_elem;
    } else if (!my_strcasecmp(system_charset_info, part_elem->partition_name,
                              partition_name)) {
      *part_id = i;
      return part_elem;
    }
  } while (++i < num_parts);
  return nullptr;
}

/*
  A support function to check partition names for duplication in a
  partitioned table

  SYNOPSIS
    find_duplicate_name()

  RETURN VALUES
    NULL               Has unique part and subpart names
    !NULL              Pointer to duplicated name

  DESCRIPTION
    Checks that the list of names in the partitions doesn't contain any
    duplicated names.
*/

const char *partition_info::find_duplicate_name() {
  collation_unordered_set<string> partition_names{system_charset_info,
                                                  PSI_INSTRUMENT_ME};
  uint max_names;
  List_iterator<partition_element> parts_it(partitions);
  partition_element *p_elem;

  DBUG_TRACE;

  /*
    TODO: If table->s->ha_part_data->partition_name_hash.elements is > 0,
    then we could just return NULL, but that has not been verified.
    And this only happens when in ALTER TABLE with full table copy.
  */

  max_names = num_parts;
  if (is_sub_partitioned()) max_names += num_parts * num_subparts;
  while ((p_elem = (parts_it++))) {
    const char *partition_name = p_elem->partition_name;
    if (!partition_names.insert(partition_name).second) return partition_name;

    if (!p_elem->subpartitions.is_empty()) {
      List_iterator<partition_element> subparts_it(p_elem->subpartitions);
      partition_element *subp_elem;
      while ((subp_elem = (subparts_it++))) {
        const char *subpartition_name = subp_elem->partition_name;
        if (!partition_names.insert(subpartition_name).second)
          return subpartition_name;
      }
    }
  }
  return nullptr;
}

/*
  Check that the partition/subpartition is setup to use the correct
  storage engine
  SYNOPSIS
    check_engine_condition()
    p_elem                   Partition element
    table_engine_set         Have user specified engine on table level
    inout::engine_type       Current engine used
    inout::first             Is it first partition
  RETURN VALUE
    true                     Failed check
    false                    Ok
  DESCRIPTION
    Specified engine for table and partitions p0 and pn
    Must be correct both on CREATE and ALTER commands
    table p0 pn res (0 - OK, 1 - FAIL)
        -  -  - 0
        -  -  x 1
        -  x  - 1
        -  x  x 0
        x  -  - 0
        x  -  x 0
        x  x  - 0
        x  x  x 0
    i.e:
    - All subpartitions must use the same engine
      AND it must be the same as the partition.
    - All partitions must use the same engine
      AND it must be the same as the table.
    - if one does NOT specify an engine on the table level
      then one must either NOT specify any engine on any
      partition/subpartition OR for ALL partitions/subpartitions
    Note:
    When ALTER a table, the engines are already set for all levels
    (table, all partitions and subpartitions). So if one want to
    change the storage engine, one must specify it on the table level

*/

static bool check_engine_condition(partition_element *p_elem,
                                   bool table_engine_set,
                                   handlerton **engine_type, bool *first) {
  DBUG_TRACE;

  DBUG_PRINT("enter", ("p_eng %s t_eng %s t_eng_set %u first %u state %u",
                       ha_resolve_storage_engine_name(p_elem->engine_type),
                       ha_resolve_storage_engine_name(*engine_type),
                       table_engine_set, *first, p_elem->part_state));
  if (*first && !table_engine_set) {
    *engine_type = p_elem->engine_type;
    DBUG_PRINT("info", ("setting table_engine = %s",
                        ha_resolve_storage_engine_name(*engine_type)));
  }
  *first = false;
  if ((table_engine_set &&
       (p_elem->engine_type != (*engine_type) && p_elem->engine_type)) ||
      (!table_engine_set && p_elem->engine_type != (*engine_type))) {
    return true;
  }

  return false;
}

/*
  Check engine mix that it is correct
  Current limitation is that all partitions and subpartitions
  must use the same storage engine.
  SYNOPSIS
    check_engine_mix()
    inout::engine_type       Current engine used
    table_engine_set         Have user specified engine on table level
  RETURN VALUE
    true                     Error, mixed engines
    false                    Ok, no mixed engines
  DESCRIPTION
    Current check verifies only that all handlers are the same.
    Later this check will be more sophisticated.
    (specified partition handler ) specified table handler
    (NDB, NDB) NDB           OK
    (MYISAM, MYISAM) -       OK
    (MYISAM, -)      -       NOT OK
    (MYISAM, -)    MYISAM    OK
    (- , MYISAM)   -         NOT OK
    (- , -)        MYISAM    OK
    (-,-)          -         OK
    (NDB, MYISAM) *          NOT OK
*/

bool partition_info::check_engine_mix(handlerton *engine_type,
                                      bool table_engine_set) {
  handlerton *old_engine_type = engine_type;
  bool first = true;
  uint n_parts = partitions.elements;
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("in: engine_type = %s, table_engine_set = %u",
              ha_resolve_storage_engine_name(engine_type), table_engine_set));
  if (n_parts) {
    List_iterator<partition_element> part_it(partitions);
    uint i = 0;
    do {
      partition_element *part_elem = part_it++;
      DBUG_PRINT("info",
                 ("part = %d engine = %s table_engine_set %u", i,
                  ha_resolve_storage_engine_name(part_elem->engine_type),
                  table_engine_set));
      if (is_sub_partitioned() && part_elem->subpartitions.elements) {
        uint n_subparts = part_elem->subpartitions.elements;
        uint j = 0;
        List_iterator<partition_element> sub_it(part_elem->subpartitions);
        do {
          partition_element *sub_elem = sub_it++;
          DBUG_PRINT("info",
                     ("sub = %d engine = %s table_engie_set %u", j,
                      ha_resolve_storage_engine_name(sub_elem->engine_type),
                      table_engine_set));
          if (check_engine_condition(sub_elem, table_engine_set, &engine_type,
                                     &first))
            goto error;
        } while (++j < n_subparts);
        /* ensure that the partition also has correct engine */
        if (check_engine_condition(part_elem, table_engine_set, &engine_type,
                                   &first))
          goto error;
      } else if (check_engine_condition(part_elem, table_engine_set,
                                        &engine_type, &first))
        goto error;
    } while (++i < n_parts);
  }
  DBUG_PRINT("info",
             ("engine_type = %s", ha_resolve_storage_engine_name(engine_type)));
  if (!engine_type) engine_type = old_engine_type;
  if (engine_type->flags & HTON_NO_PARTITION) {
    my_error(ER_PARTITION_MERGE_ERROR, MYF(0));
    return true;
  }
  DBUG_PRINT("info", ("out: engine_type = %s",
                      ha_resolve_storage_engine_name(engine_type)));
  return false;
error:
  /*
    Mixed engines not yet supported but when supported it will need
    the partition handler
  */
  return true;
}

/*
  This routine allocates an array for all range constants to achieve a fast
  check what partition a certain value belongs to. At the same time it does
  also check that the range constants are defined in increasing order and
  that the expressions are constant integer expressions.

  SYNOPSIS
    check_range_constants()
    thd                          Thread object

  RETURN VALUE
    true                An error occurred during creation of range constants
    false               Successful creation of range constant mapping

  DESCRIPTION
    This routine is called from check_partition_info to get a quick error
    before we came too far into the CREATE TABLE process. It is also called
    from fix_partition_func every time we open the .frm file. It is only
    called for RANGE PARTITIONed tables.
*/

bool partition_info::check_range_constants(THD *thd) {
  partition_element *part_def;
  bool first = true;
  uint i;
  List_iterator<partition_element> it(partitions);
  int result = true;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("RANGE with %d parts, column_list = %u", num_parts, column_list));

  if (column_list) {
    part_column_list_val *loc_range_col_array;
    part_column_list_val *current_largest_col_val = nullptr;
    uint num_column_values = part_field_list.elements;
    uint size_entries = sizeof(part_column_list_val) * num_column_values;
    range_col_array =
        (part_column_list_val *)sql_calloc(num_parts * size_entries);
    if (unlikely(range_col_array == nullptr)) {
      mem_alloc_error(num_parts * size_entries);
      goto end;
    }
    loc_range_col_array = range_col_array;
    i = 0;
    do {
      part_def = it++;
      {
        List_iterator<part_elem_value> list_val_it(part_def->list_val_list);
        part_elem_value *range_val = list_val_it++;
        part_column_list_val *col_val = range_val->col_val_array;
        DBUG_ASSERT(part_def->list_val_list.elements == 1);

        if (fix_column_value_functions(thd, range_val, i)) goto end;
        memcpy(loc_range_col_array, (const void *)col_val, size_entries);
        loc_range_col_array += num_column_values;
        if (!first) {
          if (!compare_column_values(current_largest_col_val, col_val))
            goto range_not_increasing_error;
        }
        current_largest_col_val = col_val;
      }
      first = false;
    } while (++i < num_parts);
  } else {
    longlong current_largest = 0;
    longlong part_range_value;
    bool signed_flag = !part_expr->unsigned_flag;

    range_int_array =
        (longlong *)(*THR_MALLOC)->Alloc(num_parts * sizeof(longlong));
    if (unlikely(range_int_array == nullptr)) {
      mem_alloc_error(num_parts * sizeof(longlong));
      goto end;
    }
    i = 0;
    do {
      part_def = it++;
      if ((i != (num_parts - 1)) || !defined_max_value) {
        part_range_value = part_def->range_value;
        if (!signed_flag) part_range_value -= 0x8000000000000000ULL;
      } else
        part_range_value = LLONG_MAX;

      if (!first) {
        if (unlikely(current_largest > part_range_value) ||
            (unlikely(current_largest == part_range_value) &&
             (part_range_value < LLONG_MAX || i != (num_parts - 1) ||
              !defined_max_value)))
          goto range_not_increasing_error;
      }
      range_int_array[i] = part_range_value;
      current_largest = part_range_value;
      first = false;
    } while (++i < num_parts);
  }
  result = false;
end:
  return result;

range_not_increasing_error:
  my_error(ER_RANGE_NOT_INCREASING_ERROR, MYF(0));
  goto end;
}

/*
  Compare two lists of column values in RANGE/LIST partitioning
  SYNOPSIS
    compare_column_values()
    first                    First column list argument
    second                   Second column list argument
  RETURN VALUES
    true                     first < second
    false                    first >= second
*/

static bool partition_info_compare_column_values(
    const part_column_list_val *first, const part_column_list_val *second) {
  for (Field **field = first->part_info->part_field_array; *field;
       field++, first++, second++) {
    /*
      If both are maxvalue, they are equal (don't check the rest of the parts).
      Otherwise, maxvalue > *.
    */
    if (first->max_value || second->max_value)
      return first->max_value < second->max_value;

    // NULLs sort before non-NULLs.
    if (first->null_value != second->null_value) return first->null_value;

    // For non-NULLs, compare the actual fields.
    if (!first->null_value) {
      int res = (*field)->cmp(first->column_value.field_image,
                              second->column_value.field_image);
      if (res != 0) return res < 0;
    }
  }
  return false;
}

bool partition_info::compare_column_values(
    const part_column_list_val *first_arg,
    const part_column_list_val *second_arg) {
  return partition_info_compare_column_values(first_arg, second_arg);
}

/*
  This routine allocates an array for all list constants to achieve a fast
  check what partition a certain value belongs to. At the same time it does
  also check that there are no duplicates among the list constants and that
  that the list expressions are constant integer expressions.

  SYNOPSIS
    check_list_constants()
    thd                            Thread object

  RETURN VALUE
    true                  An error occurred during creation of list constants
    false                 Successful creation of list constant mapping

  DESCRIPTION
    This routine is called from check_partition_info to get a quick error
    before we came too far into the CREATE TABLE process. It is also called
    from fix_partition_func every time we open the .frm file. It is only
    called for LIST PARTITIONed tables.
*/

bool partition_info::check_list_constants(THD *thd) {
  uint size_entries, num_column_values;
  uint list_index = 0;
  part_elem_value *list_value;
  bool result = true;
  longlong calc_value;
  partition_element *part_def;
  bool found_null = false;
  void *ptr;
  List_iterator<partition_element> list_func_it(partitions);
  DBUG_TRACE;

  num_list_values = 0;
  /*
    We begin by calculating the number of list values that have been
    defined in the first step.

    We use this number to allocate a properly sized array of structs
    to keep the partition id and the value to use in that partition.
    In the second traversal we assign them values in the struct array.

    Finally we sort the array of structs in order of values to enable
    a quick binary search for the proper value to discover the
    partition id.
    After sorting the array we check that there are no duplicates in the
    list.
  */

  uint part_id = 0;
  do {
    part_def = list_func_it++;
    if (part_def->has_null_value) {
      if (found_null) {
        my_error(ER_MULTIPLE_DEF_CONST_IN_LIST_PART_ERROR, MYF(0));
        goto end;
      }
      has_null_value = true;
      has_null_part_id = part_id;
      found_null = true;
    }
    List_iterator<part_elem_value> list_val_it1(part_def->list_val_list);
    while (list_val_it1++) num_list_values++;
  } while (++part_id < num_parts);
  list_func_it.rewind();
  num_column_values = part_field_list.elements;
  size_entries = column_list
                     ? (num_column_values * sizeof(part_column_list_val))
                     : sizeof(LIST_PART_ENTRY);
  ptr = sql_calloc((num_list_values + 1) * size_entries);
  if (unlikely(ptr == nullptr)) {
    mem_alloc_error(num_list_values * size_entries);
    goto end;
  }
  if (column_list) {
    part_column_list_val *loc_list_col_array;
    loc_list_col_array = (part_column_list_val *)ptr;
    list_col_array = (part_column_list_val *)ptr;
    part_id = 0;
    do {
      part_def = list_func_it++;
      List_iterator<part_elem_value> list_val_it2(part_def->list_val_list);
      while ((list_value = list_val_it2++)) {
        part_column_list_val *col_val = list_value->col_val_array;
        if (unlikely(fix_column_value_functions(thd, list_value, part_id))) {
          return true;
        }
        memcpy(loc_list_col_array, (const void *)col_val, size_entries);
        loc_list_col_array += num_column_values;
      }
    } while (++part_id < num_parts);

    varlen_sort(list_col_array,
                list_col_array + num_list_values * num_column_values,
                size_entries, partition_info_compare_column_values);

    for (uint i = 1; i < num_list_values; ++i) {
      if (!partition_info_compare_column_values(
              &list_col_array[num_column_values * (i - 1)],
              &list_col_array[num_column_values * i])) {
        my_error(ER_MULTIPLE_DEF_CONST_IN_LIST_PART_ERROR, MYF(0));
        goto end;
      }
    }
  } else {
    list_array = (LIST_PART_ENTRY *)ptr;
    part_id = 0;
    /*
      Fix to be able to reuse signed sort functions also for unsigned
      partition functions.
    */
    ulonglong type_add =
        (part_expr->unsigned_flag ? 0x8000000000000000ULL : 0ULL);

    do {
      part_def = list_func_it++;
      List_iterator<part_elem_value> list_val_it2(part_def->list_val_list);
      while ((list_value = list_val_it2++)) {
        calc_value = list_value->value | type_add;
        list_array[list_index].list_value = calc_value;
        list_array[list_index++].partition_id = part_id;
      }
    } while (++part_id < num_parts);

    LIST_PART_ENTRY *list_array_end = list_array + num_list_values;
    std::sort(list_array, list_array_end,
              [](const LIST_PART_ENTRY &a, const LIST_PART_ENTRY &b) {
                return a.list_value < b.list_value;
              });
    if (std::adjacent_find(
            list_array, list_array_end,
            [](const LIST_PART_ENTRY &a, const LIST_PART_ENTRY &b) {
              return a.list_value == b.list_value;
            }) != list_array_end) {
      my_error(ER_MULTIPLE_DEF_CONST_IN_LIST_PART_ERROR, MYF(0));
      goto end;
    }
  }
  DBUG_ASSERT(fixed);
  result = false;
end:
  return result;
}

/**
  Check if we allow DATA/INDEX DIRECTORY, if not warn and set them to NULL.

  @param thd  THD also containing sql_mode (looks from MODE_NO_DIR_IN_CREATE).
  @param part_elem partition_element to check.
*/
static void warn_if_dir_in_part_elem(THD *thd, partition_element *part_elem) {
  if (thd->variables.sql_mode & MODE_NO_DIR_IN_CREATE) {
    if (part_elem->data_file_name)
      push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_OPTION_IGNORED,
                          ER_THD(thd, WARN_OPTION_IGNORED), "DATA DIRECTORY");
    if (part_elem->index_file_name)
      push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_OPTION_IGNORED,
                          ER_THD(thd, WARN_OPTION_IGNORED), "INDEX DIRECTORY");
    part_elem->data_file_name = part_elem->index_file_name = nullptr;
  }
}

/*
  This code is used early in the CREATE TABLE and ALTER TABLE process.

  SYNOPSIS
    check_partition_info()
    thd                 Thread object
    eng_type            Return value for used engine in partitions
    file                A reference to a handler of the table
    info                Create info
    add_or_reorg_part   Is it ALTER TABLE ADD/REORGANIZE command

  RETURN VALUE
    true                 Error, something went wrong
    false                Ok, full partition data structures are now generated

  DESCRIPTION
    We will check that the partition info requested is possible to set-up in
    this version. This routine is an extension of the parser one could say.
    If defaults were used we will generate default data structures for all
    partitions.

*/

bool partition_info::check_partition_info(THD *thd, handlerton **eng_type,
                                          handler *file, HA_CREATE_INFO *info,
                                          bool add_or_reorg_part) {
  handlerton *table_engine = default_engine_type;
  uint i, tot_partitions;
  bool result = true, table_engine_set;
  const char *same_name;
  DBUG_TRACE;

  DBUG_PRINT("info", ("default table_engine = %s",
                      ha_resolve_storage_engine_name(table_engine)));
  if (!add_or_reorg_part) {
    int err = 0;

    /* Check for partition expression. */
    if (!list_of_part_fields) {
      DBUG_ASSERT(part_expr);
      err = part_expr->walk(&Item::check_partition_func_processor,
                            enum_walk::POSTFIX, nullptr);
    }

    /* Check for sub partition expression. */
    if (!err && is_sub_partitioned() && !list_of_subpart_fields) {
      DBUG_ASSERT(subpart_expr);
      err = subpart_expr->walk(&Item::check_partition_func_processor,
                               enum_walk::POSTFIX, nullptr);
    }

    if (err) {
      my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
      goto end;
    }
    if (thd->lex->sql_command == SQLCOM_CREATE_TABLE && fix_parser_data(thd))
      goto end;
  }
  if (unlikely(!is_sub_partitioned() &&
               !(use_default_subpartitions && use_default_num_subpartitions))) {
    my_error(ER_SUBPARTITION_ERROR, MYF(0));
    goto end;
  }
  if (unlikely(is_sub_partitioned() &&
               (!(part_type == partition_type::RANGE ||
                  part_type == partition_type::LIST)))) {
    /* Only RANGE and LIST partitioning can be subpartitioned */
    my_error(ER_SUBPARTITION_ERROR, MYF(0));
    goto end;
  }
  if (unlikely(set_up_defaults_for_partitioning(file->get_partition_handler(),
                                                info, (uint)0))) {
    goto end;
  }
  if (!(tot_partitions = get_tot_partitions())) {
    my_error(ER_PARTITION_NOT_DEFINED_ERROR, MYF(0), "partitions");
    goto end;
  }
  if (unlikely(tot_partitions > MAX_PARTITIONS)) {
    my_error(ER_TOO_MANY_PARTITIONS_ERROR, MYF(0));
    goto end;
  }
  /*
    if NOT specified ENGINE = <engine>:
      If Create, always use create_info->db_type
      else, use previous tables db_type
      either ALL or NONE partition should be set to
      default_engine_type when not table_engine_set
      Note: after a table is created its storage engines for
      the table and all partitions/subpartitions are set.
      So when ALTER it is already set on table level
  */
  if (info && info->used_fields & HA_CREATE_USED_ENGINE) {
    table_engine_set = true;
    table_engine = info->db_type;
    DBUG_PRINT("info", ("Using table_engine = %s",
                        ha_resolve_storage_engine_name(table_engine)));
  } else {
    table_engine_set = false;
    if (thd->lex->sql_command != SQLCOM_CREATE_TABLE) {
      table_engine_set = true;
      DBUG_PRINT("info", ("No create, table_engine = %s",
                          ha_resolve_storage_engine_name(table_engine)));
    }
  }

  if (part_field_list.elements > 0 && (same_name = find_duplicate_field())) {
    my_error(ER_SAME_NAME_PARTITION_FIELD, MYF(0), same_name);
    goto end;
  }
  if ((same_name = find_duplicate_name())) {
    my_error(ER_SAME_NAME_PARTITION, MYF(0), same_name);
    goto end;
  }
  i = 0;
  {
    List_iterator<partition_element> part_it(partitions);
    uint num_parts_not_set = 0;
    uint prev_num_subparts_not_set = num_subparts + 1;
    do {
      partition_element *part_elem = part_it++;
      warn_if_dir_in_part_elem(thd, part_elem);
      if (!is_sub_partitioned()) {
        if (part_elem->engine_type == nullptr) {
          num_parts_not_set++;
          part_elem->engine_type = default_engine_type;
        }
        Ident_name_check ident_check_status = check_table_name(
            part_elem->partition_name, strlen(part_elem->partition_name));
        if (ident_check_status == Ident_name_check::WRONG) {
          my_error(ER_WRONG_PARTITION_NAME, MYF(0));
          goto end;
        } else if (ident_check_status == Ident_name_check::TOO_LONG) {
          my_error(ER_TOO_LONG_IDENT, MYF(0));
          goto end;
        }
        DBUG_PRINT("info",
                   ("part = %d engine = %s", i,
                    ha_resolve_storage_engine_name(part_elem->engine_type)));
      } else {
        uint j = 0;
        uint num_subparts_not_set = 0;
        List_iterator<partition_element> sub_it(part_elem->subpartitions);
        partition_element *sub_elem;
        do {
          sub_elem = sub_it++;
          warn_if_dir_in_part_elem(thd, sub_elem);
          Ident_name_check ident_check_status = check_table_name(
              sub_elem->partition_name, strlen(sub_elem->partition_name));
          if (ident_check_status == Ident_name_check::WRONG) {
            my_error(ER_WRONG_PARTITION_NAME, MYF(0));
            goto end;
          } else if (ident_check_status == Ident_name_check::TOO_LONG) {
            my_error(ER_TOO_LONG_IDENT, MYF(0));
            goto end;
          }
          if (sub_elem->engine_type == nullptr) {
            if (part_elem->engine_type != nullptr)
              sub_elem->engine_type = part_elem->engine_type;
            else {
              sub_elem->engine_type = default_engine_type;
              num_subparts_not_set++;
            }
          }
          DBUG_PRINT("info",
                     ("part = %d sub = %d engine = %s", i, j,
                      ha_resolve_storage_engine_name(sub_elem->engine_type)));
        } while (++j < num_subparts);

        if (prev_num_subparts_not_set == (num_subparts + 1) &&
            (num_subparts_not_set == 0 || num_subparts_not_set == num_subparts))
          prev_num_subparts_not_set = num_subparts_not_set;

        if (!table_engine_set &&
            prev_num_subparts_not_set != num_subparts_not_set) {
          DBUG_PRINT("info", ("num_subparts_not_set = %u num_subparts = %u",
                              num_subparts_not_set, num_subparts));
          my_error(ER_MIX_HANDLER_ERROR, MYF(0));
          goto end;
        }

        if (part_elem->engine_type == nullptr) {
          if (num_subparts_not_set == 0)
            part_elem->engine_type = sub_elem->engine_type;
          else {
            num_parts_not_set++;
            part_elem->engine_type = default_engine_type;
          }
        }
      }
    } while (++i < num_parts);
    if (!table_engine_set && num_parts_not_set != 0 &&
        num_parts_not_set != num_parts) {
      DBUG_PRINT("info", ("num_parts_not_set = %u num_parts = %u",
                          num_parts_not_set, num_subparts));
      my_error(ER_MIX_HANDLER_ERROR, MYF(0));
      goto end;
    }
  }
  if (unlikely(check_engine_mix(table_engine, table_engine_set))) {
    my_error(ER_MIX_HANDLER_ERROR, MYF(0));
    goto end;
  }

  DBUG_ASSERT(table_engine == default_engine_type);

  if (eng_type) *eng_type = table_engine;

  /*
    We need to check all constant expressions that they are of the correct
    type and that they are increasing for ranges and not overlapping for
    list constants.
  */

  if (add_or_reorg_part) {
    if (unlikely(
            (part_type == partition_type::RANGE &&
             check_range_constants(thd)) ||
            (part_type == partition_type::LIST && check_list_constants(thd))))
      goto end;
  }
  result = false;
end:
  return result;
}

/*
  Print error for no partition found

  SYNOPSIS
    print_no_partition_found()
    thd                          Thread handle
    table                        Table object

  RETURN VALUES
*/

void partition_info::print_no_partition_found(THD *thd, TABLE *table_arg) {
  char buf[100];
  const char *buf_ptr = buf;
  TABLE_LIST table_list;

  table_list.db = table_arg->s->db.str;
  table_list.table_name = table_arg->s->table_name.str;

  if (check_single_table_access(thd, SELECT_ACL, &table_list, true)) {
    my_message(ER_NO_PARTITION_FOR_GIVEN_VALUE,
               ER_THD(thd, ER_NO_PARTITION_FOR_GIVEN_VALUE_SILENT), MYF(0));
  } else {
    if (column_list)
      buf_ptr = "from column_list";
    else {
      my_bitmap_map *old_map =
          dbug_tmp_use_all_columns(table_arg, table_arg->read_set);
      if (part_expr->null_value)
        buf_ptr = "NULL";
      else
        longlong10_to_str(err_value, buf, part_expr->unsigned_flag ? 10 : -10);
      dbug_tmp_restore_column_map(table_arg->read_set, old_map);
    }
    my_error(ER_NO_PARTITION_FOR_GIVEN_VALUE, MYF(0), buf_ptr);
  }
}

/*
  Set fields related to partition expression

  @param start_token    Start of partition function string
  @param item_ptr       Pointer to item tree
  @param end_token      End of partition function string
  @param is_subpart     Subpartition indicator

  @retval true          Memory allocation error or
                        Invalid character string
  @retval false         Success
*/

bool partition_info::set_part_expr(char *start_token, Item *item_ptr,
                                   char *end_token, bool is_subpart) {
  size_t expr_len = end_token - start_token;
  char *func_string = (char *)sql_memdup(start_token, expr_len);

  if (!func_string) {
    mem_alloc_error(expr_len);
    return true;
  }

  if (is_invalid_string(LEX_CSTRING{func_string, expr_len},
                        system_charset_info))
    return true;

  if (is_subpart) {
    list_of_subpart_fields = false;
    subpart_expr = item_ptr;
    subpart_func_string = func_string;
    subpart_func_len = expr_len;
  } else {
    list_of_part_fields = false;
    part_expr = item_ptr;
    part_func_string = func_string;
    part_func_len = expr_len;
  }
  return false;
}

/*
  Check that partition fields and subpartition fields are not too long

  SYNOPSIS
    check_partition_field_length()

  RETURN VALUES
    true                             Total length was too big
    false                            Length is ok
*/

bool partition_info::check_partition_field_length() {
  uint store_length = 0;
  uint i;
  DBUG_TRACE;

  for (i = 0; i < num_part_fields; i++)
    store_length += get_partition_field_store_length(part_field_array[i]);
  if (store_length > MAX_KEY_LENGTH) return true;
  store_length = 0;
  for (i = 0; i < num_subpart_fields; i++)
    store_length += get_partition_field_store_length(subpart_field_array[i]);
  if (store_length > MAX_KEY_LENGTH) return true;
  return false;
}

/*
  Set up buffers and arrays for fields requiring preparation
  SYNOPSIS
    set_up_charset_field_preps()

  RETURN VALUES
    true                             Memory Allocation error
    false                            Success

  DESCRIPTION
    Set up arrays and buffers for fields that require special care for
    calculation of partition id. This is used for string fields with
    variable length or string fields with fixed length that isn't using
    the binary collation.
*/

bool partition_info::set_up_charset_field_preps() {
  Field *field, **ptr;
  uchar **char_ptrs;
  unsigned i;
  size_t size;
  uint tot_fields = 0;
  uint tot_part_fields = 0;
  uint tot_subpart_fields = 0;
  DBUG_TRACE;

  if (!(part_type == partition_type::HASH && list_of_part_fields) &&
      check_part_func_fields(part_field_array, false)) {
    ptr = part_field_array;
    /* Set up arrays and buffers for those fields */
    while ((field = *(ptr++))) {
      if (field_is_partition_charset(field)) {
        tot_part_fields++;
        tot_fields++;
      }
    }
    size = tot_part_fields * sizeof(char *);
    if (!(char_ptrs = (uchar **)sql_calloc(size))) goto error;
    part_field_buffers = char_ptrs;
    if (!(char_ptrs = (uchar **)sql_calloc(size))) goto error;
    restore_part_field_ptrs = char_ptrs;
    size = (tot_part_fields + 1) * sizeof(Field *);
    if (!(char_ptrs = (uchar **)(*THR_MALLOC)->Alloc(size))) goto error;
    part_charset_field_array = (Field **)char_ptrs;
    ptr = part_field_array;
    i = 0;
    while ((field = *(ptr++))) {
      if (field_is_partition_charset(field)) {
        uchar *field_buf;
        size = field->pack_length();
        if (!(field_buf = (uchar *)sql_calloc(size))) goto error;
        part_charset_field_array[i] = field;
        part_field_buffers[i++] = field_buf;
      }
    }
    part_charset_field_array[i] = nullptr;
  }
  if (is_sub_partitioned() && !list_of_subpart_fields &&
      check_part_func_fields(subpart_field_array, false)) {
    /* Set up arrays and buffers for those fields */
    ptr = subpart_field_array;
    while ((field = *(ptr++))) {
      if (field_is_partition_charset(field)) {
        tot_subpart_fields++;
        tot_fields++;
      }
    }
    size = tot_subpart_fields * sizeof(char *);
    if (!(char_ptrs = (uchar **)sql_calloc(size))) goto error;
    subpart_field_buffers = char_ptrs;
    if (!(char_ptrs = (uchar **)sql_calloc(size))) goto error;
    restore_subpart_field_ptrs = char_ptrs;
    size = (tot_subpart_fields + 1) * sizeof(Field *);
    if (!(char_ptrs = (uchar **)(*THR_MALLOC)->Alloc(size))) goto error;
    subpart_charset_field_array = (Field **)char_ptrs;
    ptr = subpart_field_array;
    i = 0;
    while ((field = *(ptr++))) {
      uchar *field_buf = nullptr;

      if (!field_is_partition_charset(field)) continue;
      size = field->pack_length();
      if (!(field_buf = (uchar *)sql_calloc(size))) goto error;
      subpart_charset_field_array[i] = field;
      subpart_field_buffers[i++] = field_buf;
    }
    subpart_charset_field_array[i] = nullptr;
  }
  return false;
error:
  mem_alloc_error(size);
  return true;
}

/*
  Check if path does not contain mysql data home directory
  for partition elements with data directory and index directory

  SYNOPSIS
    check_partition_dirs()
    part_info               partition_info struct

  RETURN VALUES
    0   ok
    1   error
*/

bool check_partition_dirs(partition_info *part_info) {
  if (!part_info) return false;

  partition_element *part_elem;
  const char *file_name;
  List_iterator<partition_element> part_it(part_info->partitions);
  while ((part_elem = part_it++)) {
    if (part_elem->subpartitions.elements) {
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      partition_element *subpart_elem;
      while ((subpart_elem = sub_it++)) {
        if (test_if_data_home_dir(subpart_elem->data_file_name)) {
          file_name = subpart_elem->data_file_name;
          goto err;
        }
        if (test_if_data_home_dir(subpart_elem->index_file_name)) {
          file_name = subpart_elem->index_file_name;
          goto err;
        }
      }
    } else {
      if (test_if_data_home_dir(part_elem->data_file_name)) {
        file_name = part_elem->data_file_name;
        goto err;
      }
      if (test_if_data_home_dir(part_elem->index_file_name)) {
        file_name = part_elem->index_file_name;
        goto err;
      }
    }
  }
  return false;

err:
  my_error(ER_WRONG_VALUE, MYF(0), "path", file_name);
  return true;
}

/**
  Check what kind of error to report.

  @param use_subpart_expr Use the subpart_expr instead of part_expr
*/

void partition_info::report_part_expr_error(bool use_subpart_expr) {
  Item *expr = part_expr;
  DBUG_TRACE;
  if (use_subpart_expr) expr = subpart_expr;

  if (expr->type() == Item::FIELD_ITEM) {
    partition_type type = part_type;
    bool list_of_fields = list_of_part_fields;
    Item_field *item_field = (Item_field *)expr;
    /*
      The expression consists of a single field.
      It must be of integer type unless KEY or COLUMNS partitioning.
    */
    if (use_subpart_expr) {
      type = subpart_type;
      list_of_fields = list_of_subpart_fields;
    }
    if (!column_list && item_field->field &&
        item_field->field->result_type() != INT_RESULT &&
        !(type == partition_type::HASH && list_of_fields)) {
      my_error(ER_FIELD_TYPE_NOT_ALLOWED_AS_PARTITION_FIELD, MYF(0),
               item_field->item_name.ptr());
      return;
    }
  }
  if (use_subpart_expr)
    my_error(ER_PARTITION_FUNC_NOT_ALLOWED_ERROR, MYF(0), "SUBPARTITION");
  else
    my_error(ER_PARTITION_FUNC_NOT_ALLOWED_ERROR, MYF(0), "PARTITION");
}

/**
  Check if fields are in the partitioning expression.

  @param fields  List of Items (fields)

  @return True if any field in the fields list is used by a partitioning expr.
    @retval true  At least one field in the field list is found.
    @retval false No field is within any partitioning expression.
*/

bool partition_info::is_fields_in_part_expr(List<Item> &fields) {
  List_iterator<Item> it(fields);
  Item *item;
  Item_field *field;
  DBUG_TRACE;
  while ((item = it++)) {
    field = item->field_for_view_update();
    DBUG_ASSERT(field->field->table == table);
    if (bitmap_is_set(&full_part_field_set, field->field->field_index))
      return true;
  }
  return false;
}

/**
  Check if all partitioning fields are included.
*/

bool partition_info::is_full_part_expr_in_fields(List<Item> &fields) {
  Field **part_field = full_part_field_array;
  DBUG_ASSERT(*part_field);
  DBUG_TRACE;
  /*
    It is very seldom many fields in full_part_field_array, so it is OK
    to loop over all of them instead of creating a bitmap fields argument
    to compare with.
  */
  do {
    List_iterator<Item> it(fields);
    Item *item;
    Item_field *field;
    bool found = false;

    while ((item = it++)) {
      field = item->field_for_view_update();
      DBUG_ASSERT(field->field->table == table);
      if (*part_field == field->field) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  } while (*(++part_field));
  return true;
}

/**
  Create a new column value in current list with maxvalue.

  @return Operation status
    @retval true   Error
    @retval false  Success

  @note Called from parser.
*/

bool Parser_partition_info::add_max_value() {
  DBUG_TRACE;

  part_column_list_val *col_val;
  if (!(col_val = add_column_value())) {
    return true;
  }
  col_val->max_value = true;
  return false;
}

/**
  Create a new column value in current list.

  @return Pointer to a new part_column_list_val
    @retval  != 0  A part_column_list_val object which have been
                   inserted into its list
    @retval  NULL  Memory allocation failure

  @note Called from parser.
*/

part_column_list_val *Parser_partition_info::add_column_value() {
  uint max_val =
      part_info->num_columns ? part_info->num_columns : MAX_REF_PARTS;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("num_columns = %u, curr_list_object %u, max_val = %u",
                       part_info->num_columns, curr_list_object, max_val));
  if (curr_list_object < max_val) {
    curr_list_val->added_items++;
    return &curr_list_val->col_val_array[curr_list_object++];
  }
  if (!part_info->num_columns && part_info->part_type == partition_type::LIST) {
    /*
      We're trying to add more than MAX_REF_PARTS, this can happen
      in ALTER TABLE using List partitions where the first partition
      uses VALUES IN (1,2,3...,17) where the number of fields in
      the list is more than MAX_REF_PARTS, in this case we know
      that the number of columns must be 1 and we thus reorganize
      into the structure used for 1 column. After this we call
      ourselves recursively which should always succeed.
    */
    if (!reorganize_into_single_field_col_val() && !init_column_part()) {
      return add_column_value();
    }
    return nullptr;
  }
  if (part_info->column_list) {
    my_error(ER_PARTITION_COLUMN_LIST_ERROR, MYF(0));
  } else {
    if (part_info->part_type == partition_type::RANGE)
      my_error(ER_TOO_MANY_VALUES_ERROR, MYF(0), "RANGE");
    else
      my_error(ER_TOO_MANY_VALUES_ERROR, MYF(0), "LIST");
  }
  return nullptr;
}

/**
  Initialise part_elem_value object at setting of a new object.

  @param col_val  Column value object to be initialised
  @param item     Item object representing column value

  @note Helper functions to functions called by parser.
*/

void Parser_partition_info::init_col_val(part_column_list_val *col_val,
                                         Item *item) {
  DBUG_TRACE;

  col_val->item_expression = item;
  col_val->null_value = item->null_value;
  if (item->result_type() == INT_RESULT) {
    /*
      This could be both column_list partitioning and function
      partitioning, but it doesn't hurt to set the function
      partitioning flags about unsignedness.
    */
    curr_list_val->value = item->val_int();
    curr_list_val->unsigned_flag = true;
    if (!item->unsigned_flag && curr_list_val->value < 0)
      curr_list_val->unsigned_flag = false;
    if (!curr_list_val->unsigned_flag) curr_part_elem->signed_flag = true;
  }
  col_val->part_info = nullptr;
}

/**
  Add a column value in VALUES LESS THAN or VALUES IN.

  @param thd   Thread object
  @param item  Item object representing column value

  @return Operation status
    @retval true   Failure
    @retval false  Success

  @note Called from parser.
*/

bool Parser_partition_info::add_column_list_value(THD *thd, Item *item) {
  part_column_list_val *col_val;
  Name_resolution_context *context = &thd->lex->current_select()->context;
  TABLE_LIST *save_list = context->table_list;
  const char *save_where = thd->where;
  DBUG_TRACE;

  if (part_info->part_type == partition_type::LIST &&
      part_info->num_columns == 1U) {
    if (init_column_part()) {
      return true;
    }
  }

  context->table_list = nullptr;
  if (part_info->column_list)
    thd->where = "field list";
  else
    thd->where = "partition function";

  if (item->walk(&Item::check_partition_func_processor, enum_walk::POSTFIX,
                 nullptr)) {
    my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
    return true;
  }
  if (item->fix_fields(thd, (Item **)nullptr) ||
      ((context->table_list = save_list), false) || (!item->const_item())) {
    context->table_list = save_list;
    thd->where = save_where;
    my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
    return true;
  }
  thd->where = save_where;

  if (!(col_val = add_column_value())) {
    return true;
  }
  init_col_val(col_val, item);
  return false;
}

/**
  Initialize a new column for VALUES {LESS THAN|IN}.

  Initialize part_info object for receiving a set of column values
  for a partition, called when parser reaches VALUES LESS THAN or
  VALUES IN.

  @return Operation status
    @retval true   Failure
    @retval false  Success
*/

bool Parser_partition_info::init_column_part() {
  partition_element *p_elem = curr_part_elem;
  part_column_list_val *col_val_array;
  part_elem_value *list_val;
  uint loc_num_columns;
  DBUG_TRACE;

  if (!(list_val = (part_elem_value *)sql_calloc(sizeof(part_elem_value))) ||
      p_elem->list_val_list.push_back(list_val)) {
    mem_alloc_error(sizeof(part_elem_value));
    return true;
  }
  if (part_info->num_columns)
    loc_num_columns = part_info->num_columns;
  else
    loc_num_columns = MAX_REF_PARTS;
  if (!(col_val_array = (part_column_list_val *)sql_calloc(
            loc_num_columns * sizeof(part_column_list_val)))) {
    mem_alloc_error(loc_num_columns * sizeof(part_elem_value));
    return true;
  }
  list_val->col_val_array = col_val_array;
  list_val->added_items = 0;
  curr_list_val = list_val;
  curr_list_object = 0;
  return false;
}

/**
  Reorganize the preallocated buffer into a single field col list.

  @return Operation status
    @retval  true   Failure
    @retval  false  Success

  @note In the case of ALTER TABLE ADD/REORGANIZE PARTITION for LIST
  partitions we can specify list values as:
  VALUES IN (v1, v2,,,, v17) if we're using the first partitioning
  variant with a function or a column list partitioned table with
  one partition field. In this case the parser knows not the
  number of columns start with and allocates MAX_REF_PARTS in the
  array. If we try to allocate something beyond MAX_REF_PARTS we
  will call this function to reorganize into a structure with
  num_columns = 1. Also when the parser knows that we used LIST
  partitioning and we used a VALUES IN like above where number of
  values was smaller than MAX_REF_PARTS or equal, then we will
  reorganize after discovering this in the parser.
*/

bool Parser_partition_info::reorganize_into_single_field_col_val() {
  part_column_list_val *col_val, *new_col_val;
  part_elem_value *val = curr_list_val;
  uint num_values = part_info->num_columns;
  uint i;
  DBUG_TRACE;
  DBUG_ASSERT(part_info->part_type == partition_type::LIST);
  DBUG_ASSERT(!part_info->num_columns ||
              part_info->num_columns == val->added_items);

  if (!num_values) num_values = val->added_items;
  part_info->num_columns = 1;
  val->added_items = 1U;
  col_val = &val->col_val_array[0];
  init_col_val(col_val, col_val->item_expression);
  for (i = 1; i < num_values; i++) {
    col_val = &val->col_val_array[i];
    if (init_column_part()) {
      return true;
    }
    if (!(new_col_val = add_column_value())) {
      return true;
    }
    memcpy(new_col_val, col_val, sizeof(*col_val));
    init_col_val(new_col_val, col_val->item_expression);
  }
  curr_list_val = val;
  return false;
}

/**
  This function handles the case of function-based partitioning.

  It fixes some data structures created in the parser and puts
  them in the format required by the rest of the partitioning
  code.

  @param val        Array of one value
  @param part_elem  The partition instance
  @param part_id    Id of partition instance

  @return Operation status
    @retval true   Failure
    @retval false  Success
*/

bool partition_info::fix_partition_values(part_elem_value *val,
                                          partition_element *part_elem,
                                          uint part_id) {
  part_column_list_val *col_val = val->col_val_array;
  DBUG_TRACE;

  if (col_val->fixed) {
    return false;
  }
  if (val->added_items != 1) {
    my_error(ER_PARTITION_COLUMN_LIST_ERROR, MYF(0));
    return true;
  }
  if (col_val->max_value) {
    /* The parser ensures we're not LIST partitioned here */
    DBUG_ASSERT(part_type == partition_type::RANGE);
    if (defined_max_value) {
      my_error(ER_PARTITION_MAXVALUE_ERROR, MYF(0));
      return true;
    }
    if (part_id == (num_parts - 1)) {
      defined_max_value = true;
      part_elem->max_value = true;
      part_elem->range_value = LLONG_MAX;
    } else {
      my_error(ER_PARTITION_MAXVALUE_ERROR, MYF(0));
      return true;
    }
  } else {
    Item *item_expr = col_val->item_expression;
    if ((val->null_value = item_expr->null_value)) {
      if (part_elem->has_null_value) {
        my_error(ER_MULTIPLE_DEF_CONST_IN_LIST_PART_ERROR, MYF(0));
        return true;
      }
      part_elem->has_null_value = true;
    } else if (item_expr->result_type() != INT_RESULT) {
      my_error(ER_VALUES_IS_NOT_INT_TYPE_ERROR, MYF(0),
               part_elem->partition_name);
      return true;
    }
    if (part_type == partition_type::RANGE) {
      if (part_elem->has_null_value) {
        my_error(ER_NULL_IN_VALUES_LESS_THAN, MYF(0));
        return true;
      }
      part_elem->range_value = val->value;
    }
  }
  col_val->fixed = 2;
  return false;
}

/**
  Get column item with a proper character set according to the field.

  @param item   Item object to start with
  @param field  Field for which the item will be compared to

  @return Column item
    @retval NULL  Error
    @retval item  Returned item
*/

Item *partition_info::get_column_item(Item *item, Field *field) {
  if (field->result_type() == STRING_RESULT &&
      item->collation.collation != field->charset()) {
    if (!(item = convert_charset_partition_constant(item, field->charset()))) {
      my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
      return nullptr;
    }
  }
  return item;
}

/**
  Evaluate VALUES functions for column list values.

  @param thd      Thread object
  @param val      List of column values
  @param part_id  Partition id we are fixing

  @return Operation status
    @retval true   Error
    @retval false  Success

  @note Fix column VALUES and store in memory array adapted to the data type.
*/

bool partition_info::fix_column_value_functions(THD *thd, part_elem_value *val,
                                                uint part_id) {
  uint n_columns = part_field_list.elements;
  bool result = false;
  uint i;
  part_column_list_val *col_val = val->col_val_array;
  DBUG_TRACE;

  if (col_val->fixed > 1) {
    return false;
  }
  for (i = 0; i < n_columns; col_val++, i++) {
    Item *column_item = col_val->item_expression;
    Field *field = part_field_array[i];
    col_val->part_info = this;
    col_val->partition_id = part_id;
    if (col_val->max_value)
      col_val->column_value.field_image = nullptr;
    else {
      col_val->column_value.field_image = nullptr;
      if (!col_val->null_value) {
        uchar *val_ptr;
        uint len = field->pack_length();
        sql_mode_t save_sql_mode;

        if (!(column_item = get_column_item(column_item, field))) {
          result = true;
          goto end;
        }
        save_sql_mode = thd->variables.sql_mode;
        thd->variables.sql_mode = 0;
        uint cond_count = thd->get_stmt_da()->cond_count();
        result = (column_item->save_in_field(field, true) ||
                  (cond_count != thd->get_stmt_da()->cond_count()));
        thd->variables.sql_mode = save_sql_mode;
        if (result) {
          my_error(ER_WRONG_TYPE_COLUMN_VALUE_ERROR, MYF(0));
          goto end;
        }
        if (!(val_ptr = (uchar *)sql_calloc(len))) {
          mem_alloc_error(len);
          result = true;
          goto end;
        }
        col_val->column_value.field_image = val_ptr;
        memcpy(val_ptr, field->ptr, len);
      }
    }
    col_val->fixed = 2;
  }
end:
  return result;
}

/**
  Fix partition data from parser.

  @details The parser generates generic data structures, we need to set them
  up as the rest of the code expects to find them. This is in reality part
  of the syntax check of the parser code.

  It is necessary to call this function in the case of a CREATE TABLE
  statement, in this case we do it early in the check_partition_info
  function.

  It is necessary to call this function for ALTER TABLE where we
  assign a completely new partition structure, in this case we do it
  in prep_alter_part_table after discovering that the partition
  structure is entirely redefined.

  It's necessary to call this method also for ALTER TABLE ADD/REORGANIZE
  of partitions, in this we call it in prep_alter_part_table after
  making some initial checks but before going deep to check the partition
  info, we also assign the column_list variable before calling this function
  here.

  Finally we also call it immediately after returning from parsing the
  partitioning text found in the frm file.

  This function mainly fixes the VALUES parts, these are handled differently
  whether or not we use column list partitioning. Since the parser doesn't
  know which we are using we need to set-up the old data structures after
  the parser is complete when we know if what type of partitioning the
  base table is using.

  For column lists we will handle this in the fix_column_value_function.
  For column lists it is sufficient to verify that the number of columns
  and number of elements are in synch with each other. So only partitioning
  using functions need to be set-up to their data structures.

  @param thd  Thread object

  @return Operation status
    @retval true   Failure
    @retval false  Success
*/

bool partition_info::fix_parser_data(THD *thd) {
  List_iterator<partition_element> it(partitions);
  partition_element *part_elem;
  uint num_elements;
  uint i = 0, j, k;
  DBUG_TRACE;

  if (!(part_type == partition_type::RANGE ||
        part_type == partition_type::LIST)) {
    if (part_type == partition_type::HASH && list_of_part_fields) {
      /* KEY partitioning, check ALGORITHM = N. Should not pass the parser! */
      if (key_algorithm > enum_key_algorithm::KEY_ALGORITHM_55) {
        my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
        return true;
      }
      /* If not set, use DEFAULT = 2 for CREATE and ALTER! */
      if ((thd_sql_command(thd) == SQLCOM_CREATE_TABLE ||
           thd_sql_command(thd) == SQLCOM_ALTER_TABLE) &&
          key_algorithm == enum_key_algorithm::KEY_ALGORITHM_NONE)
        key_algorithm = enum_key_algorithm::KEY_ALGORITHM_55;
    }
    return false;
  }
  if (is_sub_partitioned() && list_of_subpart_fields) {
    /* KEY subpartitioning, check ALGORITHM = N. Should not pass the parser! */
    if (key_algorithm > enum_key_algorithm::KEY_ALGORITHM_55) {
      my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
      return true;
    }
    /* If not set, use DEFAULT = 2 for CREATE and ALTER! */
    if ((thd_sql_command(thd) == SQLCOM_CREATE_TABLE ||
         thd_sql_command(thd) == SQLCOM_ALTER_TABLE) &&
        key_algorithm == enum_key_algorithm::KEY_ALGORITHM_NONE)
      key_algorithm = enum_key_algorithm::KEY_ALGORITHM_55;
  }
  do {
    part_elem = it++;
    List_iterator<part_elem_value> list_val_it(part_elem->list_val_list);
    num_elements = part_elem->list_val_list.elements;
    DBUG_ASSERT(part_type == partition_type::RANGE ? num_elements == 1U : true);
    for (j = 0; j < num_elements; j++) {
      part_elem_value *val = list_val_it++;
      if (column_list) {
        if (val->added_items != num_columns) {
          my_error(ER_PARTITION_COLUMN_LIST_ERROR, MYF(0));
          return true;
        }
        for (k = 0; k < num_columns; k++) {
          part_column_list_val *col_val = &val->col_val_array[k];
          if (col_val->null_value && part_type == partition_type::RANGE) {
            my_error(ER_NULL_IN_VALUES_LESS_THAN, MYF(0));
            return true;
          }
        }
      } else {
        if (fix_partition_values(val, part_elem, i)) {
          return true;
        }
        if (val->null_value) {
          /*
            Null values aren't required in the value part, they are kept per
            partition instance, only LIST partitions have NULL values.
          */
          list_val_it.remove();
        }
      }
    }
  } while (++i < num_parts);
  return false;
}

/**
  helper function to compare strings that can also be
  a NULL pointer.

  @param a  char pointer (can be NULL).
  @param b  char pointer (can be NULL).

  @return false if equal
    @retval true  strings differs
    @retval false strings is equal
*/

static bool strcmp_null(const char *a, const char *b) {
  if (!a && !b) return false;
  if (a && b && !strcmp(a, b)) return false;
  return true;
}

/**
  Check if the new part_info has the same partitioning.

  @param new_part_info  New partition definition to compare with.

  @return True if not considered to have changed the partitioning.
    @retval true  Allowed change (only .frm change, compatible distribution).
    @retval false Different partitioning, will need redistribution of rows.

  @note Currently only used to allow changing from non-set key_algorithm
  to a specified key_algorithm, to avoid rebuild when upgrading from 5.1 of
  such partitioned tables using numeric columns in the partitioning expression.
  For more info see bug#14521864.
  Does not check if columns etc has changed, i.e. only for
  alter_info->flags == ALTER_PARTITION.
*/

bool partition_info::has_same_partitioning(partition_info *new_part_info) {
  DBUG_TRACE;

  DBUG_ASSERT(part_field_array && part_field_array[0]);

  /*
    Only consider pre 5.5.3 .frm's to have same partitioning as
    a new one with KEY ALGORITHM = 1 ().
  */

  if (part_field_array[0]->table->s->mysql_version >= 50503) return false;

  if (!new_part_info || part_type != new_part_info->part_type ||
      num_parts != new_part_info->num_parts ||
      use_default_partitions != new_part_info->use_default_partitions ||
      new_part_info->is_sub_partitioned() != is_sub_partitioned())
    return false;

  if (part_type != partition_type::HASH) {
    /*
      RANGE or LIST partitioning, check if KEY subpartitioned.
      Also COLUMNS partitioning was added in 5.5, so treat that as different.
    */
    if (!is_sub_partitioned() || !new_part_info->is_sub_partitioned() ||
        column_list || new_part_info->column_list || !list_of_subpart_fields ||
        !new_part_info->list_of_subpart_fields ||
        new_part_info->num_subparts != num_subparts ||
        new_part_info->subpart_field_list.elements !=
            subpart_field_list.elements ||
        new_part_info->use_default_subpartitions != use_default_subpartitions)
      return false;
  } else {
    /* Check if KEY partitioned. */
    if (!new_part_info->list_of_part_fields || !list_of_part_fields ||
        new_part_info->part_field_list.elements != part_field_list.elements)
      return false;
  }

  /* Check that it will use the same fields in KEY (fields) list. */
  {
    List_iterator<char> old_field_name_it(part_field_list);
    List_iterator<char> new_field_name_it(new_part_info->part_field_list);
    char *old_name, *new_name;
    while ((old_name = old_field_name_it++)) {
      new_name = new_field_name_it++;
      if (!new_name || my_strcasecmp(system_charset_info, new_name, old_name))
        return false;
    }
  }

  if (is_sub_partitioned()) {
    /* Check that it will use the same fields in KEY subpart fields list. */
    List_iterator<char> old_field_name_it(subpart_field_list);
    List_iterator<char> new_field_name_it(new_part_info->subpart_field_list);
    char *old_name, *new_name;
    while ((old_name = old_field_name_it++)) {
      new_name = new_field_name_it++;
      if (!new_name || my_strcasecmp(system_charset_info, new_name, old_name))
        return false;
    }
  }

  if (!use_default_partitions) {
    /*
      Loop over partitions/subpartition to verify that they are
      the same, including state and name.
    */
    List_iterator<partition_element> part_it(partitions);
    List_iterator<partition_element> new_part_it(new_part_info->partitions);
    uint i = 0;
    do {
      partition_element *part_elem = part_it++;
      partition_element *new_part_elem = new_part_it++;
      /*
        The following must match:
        partition_name, tablespace_name, data_file_name, index_file_name,
        engine_type, part_max_rows, part_min_rows, nodegroup_id.
        (max_value, signed_flag, has_null_value only on partition level,
        RANGE/LIST)
        The following can differ:
          - part_comment
        part_state must be PART_NORMAL!
      */
      if (!part_elem || !new_part_elem ||
          strcmp(part_elem->partition_name, new_part_elem->partition_name) ||
          part_elem->part_state != PART_NORMAL ||
          new_part_elem->part_state != PART_NORMAL ||
          part_elem->max_value != new_part_elem->max_value ||
          part_elem->signed_flag != new_part_elem->signed_flag ||
          part_elem->has_null_value != new_part_elem->has_null_value)
        return false;

      /* new_part_elem may not have engine_type set! */
      if (new_part_elem->engine_type &&
          part_elem->engine_type != new_part_elem->engine_type)
        return false;

      if (is_sub_partitioned()) {
        /*
          Check that both old and new partition has the same definition
          (VALUES IN/VALUES LESS THAN) (No COLUMNS partitioning, see above)
        */
        if (part_type == partition_type::LIST) {
          List_iterator<part_elem_value> list_vals(part_elem->list_val_list);
          List_iterator<part_elem_value> new_list_vals(
              new_part_elem->list_val_list);
          part_elem_value *val;
          part_elem_value *new_val;
          while ((val = list_vals++)) {
            new_val = new_list_vals++;
            if (!new_val) return false;
            if ((!val->null_value && !new_val->null_value) &&
                val->value != new_val->value)
              return false;
          }
          if (new_list_vals++) return false;
        } else {
          DBUG_ASSERT(part_type == partition_type::RANGE);
          if (new_part_elem->range_value != part_elem->range_value)
            return false;
        }

        if (!use_default_subpartitions) {
          List_iterator<partition_element> sub_part_it(
              part_elem->subpartitions);
          List_iterator<partition_element> new_sub_part_it(
              new_part_elem->subpartitions);
          uint j = 0;
          do {
            partition_element *sub_part_elem = sub_part_it++;
            partition_element *new_sub_part_elem = new_sub_part_it++;
            /* new_part_elem may not have engine_type set! */
            if (new_sub_part_elem->engine_type &&
                sub_part_elem->engine_type != new_sub_part_elem->engine_type)
              return false;

            if (strcmp(sub_part_elem->partition_name,
                       new_sub_part_elem->partition_name) ||
                sub_part_elem->part_state != PART_NORMAL ||
                new_sub_part_elem->part_state != PART_NORMAL ||
                sub_part_elem->part_min_rows !=
                    new_sub_part_elem->part_min_rows ||
                sub_part_elem->part_max_rows !=
                    new_sub_part_elem->part_max_rows ||
                sub_part_elem->nodegroup_id != new_sub_part_elem->nodegroup_id)
              return false;

            if (strcmp_null(sub_part_elem->data_file_name,
                            new_sub_part_elem->data_file_name) ||
                strcmp_null(sub_part_elem->index_file_name,
                            new_sub_part_elem->index_file_name) ||
                strcmp_null(sub_part_elem->tablespace_name,
                            new_sub_part_elem->tablespace_name))
              return false;

          } while (++j < num_subparts);
        }
      } else {
        if (part_elem->part_min_rows != new_part_elem->part_min_rows ||
            part_elem->part_max_rows != new_part_elem->part_max_rows ||
            part_elem->nodegroup_id != new_part_elem->nodegroup_id)
          return false;

        if (strcmp_null(part_elem->data_file_name,
                        new_part_elem->data_file_name) ||
            strcmp_null(part_elem->index_file_name,
                        new_part_elem->index_file_name) ||
            strcmp_null(part_elem->tablespace_name,
                        new_part_elem->tablespace_name))
          return false;
      }
    } while (++i < num_parts);
  }

  /*
    Only if key_algorithm was not specified before and it is now set,
    consider this as nothing was changed, and allow change without rebuild!
  */
  if (key_algorithm != enum_key_algorithm::KEY_ALGORITHM_NONE ||
      new_part_info->key_algorithm == enum_key_algorithm::KEY_ALGORITHM_NONE)
    return false;

  return true;
}

static bool has_same_column_order(List<Create_field> *create_list,
                                  Field **field_array) {
  Field **f_ptr;
  List_iterator_fast<Create_field> new_field_it;
  Create_field *new_field = nullptr;
  new_field_it.init(*create_list);

  for (f_ptr = field_array; *f_ptr; f_ptr++) {
    while ((new_field = new_field_it++)) {
      if (new_field->field == *f_ptr) break;
    }
    if (!new_field) break;
  }

  if (!new_field) {
    /* Not same order!*/
    return false;
  }
  return true;
}

/**
  Check if the partitioning columns are in the same order as the given list.

  Used to see if INPLACE alter can be allowed or not. If the order is
  different then the rows must be redistributed for KEY [sub]partitioning.

  @param[in] create_list Column list after ALTER TABLE.
  @return true is same order as before ALTER TABLE, else false.
*/
bool partition_info::same_key_column_order(List<Create_field> *create_list) {
  /* Only need to check for KEY [sub] partitioning. */
  if (list_of_part_fields && !column_list) {
    if (!has_same_column_order(create_list, part_field_array)) return false;
  }
  if (list_of_subpart_fields) {
    if (!has_same_column_order(create_list, subpart_field_array)) return false;
  }
  return true;
}

void partition_info::print_debug(const char *str MY_ATTRIBUTE((unused)),
                                 uint *value) {
  DBUG_TRACE;
  if (value)
    DBUG_PRINT("info", ("parser: %s, val = %u", str, *value));
  else
    DBUG_PRINT("info", ("parser: %s", str));
}

bool has_external_data_or_index_dir(partition_info &pi) {
  List_iterator<partition_element> part_it(pi.partitions);
  for (partition_element *part = part_it++; part; part = part_it++) {
    if (part->data_file_name != nullptr || part->index_file_name != nullptr) {
      return true;
    }
    List_iterator<partition_element> subpart_it(part->subpartitions);
    for (const partition_element *subpart = subpart_it++; subpart;
         subpart = subpart_it++) {
      if (subpart->data_file_name != nullptr ||
          subpart->index_file_name != nullptr) {
        return true;
      }
    }
  }
  return false;
}

/**
  Fill the Tablespace_hash_set with the tablespace names
  used by the partitions on the table.

  @param part_info      - Partition info that could be using tablespaces.
  @param tablespace_set - (OUT) Tablespace_hash_set where tablespace
                          names are collected.

  @return true - On failure.
  @return false - On success.
*/
bool fill_partition_tablespace_names(partition_info *part_info,
                                     Tablespace_hash_set *tablespace_set) {
  // Do nothing if table is not partitioned.
  if (!part_info) return false;

  // Traverse through all partitions.
  List_iterator<partition_element> part_it(part_info->partitions);
  partition_element *part_elem;
  while ((part_elem = part_it++)) {
    // Add tablespace name from partition elements, if used.
    if (part_elem->tablespace_name && strlen(part_elem->tablespace_name)) {
      tablespace_set->insert(part_elem->tablespace_name);
    }

    // Traverse through all subpartitions.
    List_iterator<partition_element> sub_it(part_elem->subpartitions);
    partition_element *sub_elem;
    while ((sub_elem = sub_it++)) {
      // Add tablespace name from sub-partition elements, if used.
      if (sub_elem->tablespace_name && strlen(sub_elem->tablespace_name)) {
        tablespace_set->insert(sub_elem->tablespace_name);
      }
    }
  }

  return false;
}

bool validate_partition_tablespace_name_lengths(partition_info *part_info) {
  // Do nothing if the table is not partitioned.
  if (!part_info) return false;

  // Traverse through all partitions.
  List_iterator<partition_element> part_it(part_info->partitions);
  partition_element *part_elem;
  while ((part_elem = part_it++)) {
    // Check tablespace name length from partition elements, if used.
    if (part_elem->tablespace_name &&
        validate_tablespace_name_length(part_elem->tablespace_name))
      return true;

    // Traverse through all subpartitions.
    List_iterator<partition_element> sub_it(part_elem->subpartitions);
    partition_element *sub_elem;
    while ((sub_elem = sub_it++)) {
      // Check tablespace name length from sub-partition elements, if used.
      if (sub_elem->tablespace_name &&
          validate_tablespace_name_length(sub_elem->tablespace_name))
        return true;
    }
  }

  return false;
}

bool validate_partition_tablespace_names(partition_info *part_info,
                                         const handlerton *default_engine) {
  DBUG_ASSERT(default_engine);

  // Do nothing if the table is not partitioned.
  if (!part_info) return false;

  // Traverse through all partitions.
  List_iterator<partition_element> part_it(part_info->partitions);
  partition_element *part_elem;
  while ((part_elem = part_it++)) {
    // Use default engine if not overridden.
    const handlerton *part_elem_engine = part_elem->engine_type;
    if (part_elem_engine == nullptr) part_elem_engine = default_engine;

    // Check tablespace names from partition elements, if used.
    if (part_elem->tablespace_name &&
        validate_tablespace_name(TS_CMD_NOT_DEFINED, part_elem->tablespace_name,
                                 part_elem_engine))
      return true;

    // Traverse through all subpartitions.
    List_iterator<partition_element> sub_it(part_elem->subpartitions);
    partition_element *sub_elem;
    while ((sub_elem = sub_it++)) {
      // Use default engine if not overridden.
      const handlerton *sub_elem_engine = sub_elem->engine_type;
      if (sub_elem_engine == nullptr) sub_elem_engine = default_engine;

      // Check tablespace name from sub-partition elements, if used.
      if (sub_elem->tablespace_name &&
          validate_tablespace_name(TS_CMD_NOT_DEFINED,
                                   sub_elem->tablespace_name, sub_elem_engine))
        return true;
    }
  }

  return false;
}

bool partition_info::init_partition_bitmap(MY_BITMAP *bitmap,
                                           MEM_ROOT *mem_root) {
  uint32 *bitmap_buf;
  uint bitmap_bits = num_subparts ? (num_subparts * num_parts) : num_parts;
  uint bitmap_bytes = bitmap_buffer_size(bitmap_bits);

  if (!(bitmap_buf = (uint32 *)mem_root->Alloc(bitmap_bytes))) {
    mem_alloc_error(bitmap_bytes);
    return true;
  }
  bitmap_init(bitmap, bitmap_buf, bitmap_bits);
  return false;
}
