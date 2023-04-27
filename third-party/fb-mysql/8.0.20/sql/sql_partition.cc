/* Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  This file is a container for general functionality related
  to partitioning. It contains functionality used by all handlers that
  support partitioning, such as the partitioning handler itself and the NDB
  handler. (Much of the code in this file has been split into partition_info.cc
  and the header files partition_info.h + partition_element.h + sql_partition.h)

  The first version supports RANGE partitioning, LIST partitioning, HASH
  partitioning and composite partitioning (hereafter called subpartitioning)
  where each RANGE/LIST partitioning is HASH partitioned. The hash function
  can either be supplied by the user or by only a list of fields (also
  called KEY partitioning), where the MySQL server will use an internal
  hash function.
  There are quite a few defaults that can be used as well.

  The second version introduces a new variant of RANGE and LIST partitioning
  which is often referred to as column lists in the code variables. This
  enables a user to specify a set of columns and their concatenated value
  as the partition value. By comparing the concatenation of these values
  the proper partition can be choosen.
*/

#include "sql/sql_partition.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <algorithm>

#include "field_types.h"  // enum_field_types
#include "m_string.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "sql/create_field.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/derror.h"      // ER_THD
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"       // enum_monotoncity_info
#include "sql/item_func.h"  // Item_func
#include "sql/key.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"     // mysql_tmpdir
#include "sql/opt_range.h"  // store_key_image_to_rec
#include "sql/parse_tree_node_base.h"
#include "sql/partition_info.h"                  // partition_info
#include "sql/partitioning/partition_handler.h"  // Partition_handler
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/sql_alter.h"
#include "sql/sql_base.h"   // wait_while_table_is_used
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // parse_sql
#include "sql/sql_show.h"
#include "sql/sql_table.h"  // build_table_filename
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql_string.h"

struct MEM_ROOT;

using std::max;
using std::min;

/*
  Partition related functions declarations and some static constants;
*/
const LEX_CSTRING partition_keywords[] = {{STRING_WITH_LEN("HASH")},
                                          {STRING_WITH_LEN("RANGE")},
                                          {STRING_WITH_LEN("LIST")},
                                          {STRING_WITH_LEN("KEY")},
                                          {STRING_WITH_LEN("MAXVALUE")},
                                          {STRING_WITH_LEN("LINEAR ")},
                                          {STRING_WITH_LEN(" COLUMNS")},
                                          {STRING_WITH_LEN("ALGORITHM")}

};
static const char *part_str = "PARTITION";
static const char *sub_str = "SUB";
static const char *by_str = "BY";
static const char *space_str = " ";
static const char *equal_str = "=";
static const char *end_paren_str = ")";
static const char *begin_paren_str = "(";
static const char *comma_str = ",";

static int get_partition_id_list_col(partition_info *part_info, uint32 *part_id,
                                     longlong *func_value);
static int get_partition_id_list(partition_info *part_info, uint32 *part_id,
                                 longlong *func_value);
static int get_partition_id_range_col(partition_info *part_info,
                                      uint32 *part_id, longlong *func_value);
static int get_partition_id_range(partition_info *part_info, uint32 *part_id,
                                  longlong *func_value);
static int get_part_id_charset_func_part(partition_info *part_info,
                                         uint32 *part_id, longlong *func_value);
static int get_part_id_charset_func_subpart(partition_info *part_info,
                                            uint32 *part_id);
static int get_partition_id_hash_nosub(partition_info *part_info,
                                       uint32 *part_id, longlong *func_value);
static int get_partition_id_key_nosub(partition_info *part_info,
                                      uint32 *part_id, longlong *func_value);
static int get_partition_id_linear_hash_nosub(partition_info *part_info,
                                              uint32 *part_id,
                                              longlong *func_value);
static int get_partition_id_linear_key_nosub(partition_info *part_info,
                                             uint32 *part_id,
                                             longlong *func_value);
static int get_partition_id_with_sub(partition_info *part_info, uint32 *part_id,
                                     longlong *func_value);
static int get_partition_id_hash_sub(partition_info *part_info,
                                     uint32 *part_id);
static int get_partition_id_key_sub(partition_info *part_info, uint32 *part_id);
static int get_partition_id_linear_hash_sub(partition_info *part_info,
                                            uint32 *part_id);
static int get_partition_id_linear_key_sub(partition_info *part_info,
                                           uint32 *part_id);
static uint32 get_next_partition_via_walking(PARTITION_ITERATOR *);
static void set_up_range_analysis_info(partition_info *part_info);
static uint32 get_next_subpartition_via_walking(PARTITION_ITERATOR *);

static uint32 get_partition_id_range_for_endpoint(partition_info *part_info,
                                                  bool left_endpoint,
                                                  bool include_endpoint);
static uint32 get_next_partition_id_list(PARTITION_ITERATOR *part_iter);
static int get_part_iter_for_interval_via_mapping(
    partition_info *part_info, bool is_subpart, uint32 *store_length_array,
    uchar *min_value, uchar *max_value, uint min_len, uint max_len, uint flags,
    PARTITION_ITERATOR *part_iter);
static int get_part_iter_for_interval_cols_via_map(
    partition_info *part_info, bool is_subpart, uint32 *store_length_array,
    uchar *min_value, uchar *max_value, uint min_len, uint max_len, uint flags,
    PARTITION_ITERATOR *part_iter);
static int get_part_iter_for_interval_via_walking(
    partition_info *part_info, bool is_subpart, uint32 *store_length_array,
    uchar *min_value, uchar *max_value, uint min_len, uint max_len, uint flags,
    PARTITION_ITERATOR *part_iter);

static int cmp_rec_and_tuple(part_column_list_val *val, uint32 nvals_in_rec);
static int cmp_rec_and_tuple_prune(part_column_list_val *val,
                                   uint32 n_vals_in_rec, bool is_left_endpoint,
                                   bool include_endpoint);

static void set_field_ptr(Field **ptr, const uchar *new_buf,
                          const uchar *old_buf);

static uint32 get_list_array_idx_for_endpoint(partition_info *part_info,
                                              bool left_endpoint,
                                              bool include_endpoint);

/*
  Convert constants in VALUES definition to the character set the
  corresponding field uses.

  SYNOPSIS
    convert_charset_partition_constant()
    item                                Item to convert
    cs                                  Character set to convert to

  RETURN VALUE
    NULL                                Error
    item                                New converted item
*/

Item *convert_charset_partition_constant(Item *item, const CHARSET_INFO *cs) {
  THD *thd = current_thd;
  Name_resolution_context *context = &thd->lex->current_select()->context;
  TABLE_LIST *save_list = context->table_list;
  const char *save_where = thd->where;

  item = item->safe_charset_converter(thd, cs);
  context->table_list = nullptr;
  thd->where = "convert character set partition constant";
  if (!item || item->fix_fields(thd, (Item **)nullptr)) item = nullptr;
  thd->where = save_where;
  context->table_list = save_list;
  return item;
}

/**
  A support function to check if a name is in a list of strings.

  @param name        String searched for
  @param list_names  A list of names searched in

  @return True if if the name is in the list.
    @retval true   String found
    @retval false  String not found
*/

static bool is_name_in_list(const char *name, List<String> list_names) {
  List_iterator<String> names_it(list_names);
  uint num_names = list_names.elements;
  uint i = 0;

  do {
    String *list_name = names_it++;
    if (!(my_strcasecmp(system_charset_info, name, list_name->c_ptr())))
      return true;
  } while (++i < num_names);
  return false;
}

/*
  Set-up defaults for partitions.

  SYNOPSIS
    partition_default_handling()
    table                         Table object
    part_info                     Partition info to set up
    is_create_table_ind           Is this part of a table creation
    normalized_path               Normalized path name of table and database

  RETURN VALUES
    true                          Error
    false                         Success
*/

static bool partition_default_handling(TABLE *table, partition_info *part_info,
                                       bool is_create_table_ind,
                                       const char *normalized_path) {
  Partition_handler *part_handler = table->file->get_partition_handler();
  DBUG_TRACE;

  if (!part_handler) {
    DBUG_ASSERT(0);
    my_error(ER_PARTITION_CLAUSE_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  if (!is_create_table_ind) {
    if (part_info->use_default_num_partitions) {
      if (part_handler->get_num_parts(normalized_path, &part_info->num_parts)) {
        return true;
      }
    } else if (part_info->is_sub_partitioned() &&
               part_info->use_default_num_subpartitions) {
      uint num_parts;
      if (part_handler->get_num_parts(normalized_path, &num_parts)) {
        return true;
      }
      DBUG_ASSERT(part_info->num_parts > 0);
      DBUG_ASSERT((num_parts % part_info->num_parts) == 0);
      part_info->num_subparts = num_parts / part_info->num_parts;
    }
  }
  part_info->set_up_defaults_for_partitioning(part_handler, nullptr, 0U);
  return false;
}

/*
  A useful routine used by update_row for partition handlers to calculate
  the partition ids of the old and the new record.

  SYNOPSIS
    get_parts_for_update()
    old_data                Buffer of old record
    new_data                Buffer of new record
    rec0                    Reference to table->record[0]
    part_info               Reference to partition information
    out:old_part_id         The returned partition id of old record
    out:new_part_id         The returned partition id of new record

  RETURN VALUE
    0                       Success
    > 0                     Error code
*/

int get_parts_for_update(const uchar *old_data,
                         const uchar *new_data MY_ATTRIBUTE((unused)),
                         const uchar *rec0, partition_info *part_info,
                         uint32 *old_part_id, uint32 *new_part_id,
                         longlong *new_func_value) {
  Field **part_field_array = part_info->full_part_field_array;
  int error;
  longlong old_func_value;
  DBUG_TRACE;

  DBUG_ASSERT(new_data == rec0);  // table->record[0]
  set_field_ptr(part_field_array, old_data, rec0);
  error = part_info->get_partition_id(part_info, old_part_id, &old_func_value);
  set_field_ptr(part_field_array, rec0, old_data);
  if (unlikely(error)) {
    part_info->err_value = old_func_value;
    return error;
  }
  if (unlikely((error = part_info->get_partition_id(part_info, new_part_id,
                                                    new_func_value)))) {
    part_info->err_value = *new_func_value;
    return error;
  }
  return 0;
}

/*
  A useful routine used by delete_row for partition handlers to calculate
  the partition id.

  SYNOPSIS
    get_part_for_delete()
    buf                     Buffer of old record
    rec0                    Reference to table->record[0]
    part_info               Reference to partition information
    out:part_id             The returned partition id to delete from

  RETURN VALUE
    0                       Success
    > 0                     Error code

  DESCRIPTION
    Dependent on whether buf is not record[0] we need to prepare the
    fields. Then we call the function pointer get_partition_id to
    calculate the partition id.
*/

int get_part_for_delete(const uchar *buf, const uchar *rec0,
                        partition_info *part_info, uint32 *part_id) {
  int error;
  longlong func_value;
  DBUG_TRACE;

  if (likely(buf == rec0)) {
    if (unlikely((error = part_info->get_partition_id(part_info, part_id,
                                                      &func_value)))) {
      part_info->err_value = func_value;
      return error;
    }
    DBUG_PRINT("info", ("Delete from partition %d", *part_id));
  } else {
    Field **part_field_array = part_info->full_part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    error = part_info->get_partition_id(part_info, part_id, &func_value);
    set_field_ptr(part_field_array, rec0, buf);
    if (unlikely(error)) {
      part_info->err_value = func_value;
      return error;
    }
    DBUG_PRINT("info", ("Delete from partition %d (path2)", *part_id));
  }
  return 0;
}

/*
  This method is used to set-up both partition and subpartitioning
  field array and used for all types of partitioning.
  It is part of the logic around fix_partition_func.

  SYNOPSIS
    set_up_field_array()
    table                TABLE object for which partition fields are set-up
    sub_part             Is the table subpartitioned as well

  RETURN VALUE
    true                 Error, some field didn't meet requirements
    false                Ok, partition field array set-up

  DESCRIPTION

    A great number of functions below here is part of the fix_partition_func
    method. It is used to set up the partition structures for execution from
    openfrm. It is called at the end of the openfrm when the table struct has
    been set-up apart from the partition information.
    It involves:
    1) Setting arrays of fields for the partition functions.
    2) Setting up binary search array for LIST partitioning
    3) Setting up array for binary search for RANGE partitioning
    4) Setting up key_map's to assist in quick evaluation whether one
       can deduce anything from a given index of what partition to use
    5) Checking whether a set of partitions can be derived from a range on
       a field in the partition function.
    As part of doing this there is also a great number of error controls.
    This is actually the place where most of the things are checked for
    partition information when creating a table.
    Things that are checked includes
    1) All fields of partition function in Primary keys and unique indexes
       (if not supported)


    Create an array of partition fields (NULL terminated). Before this method
    is called fix_fields or find_table_in_sef has been called to set
    GET_FIXED_FIELDS_FLAG on all fields that are part of the partition
    function.
*/

static bool set_up_field_array(TABLE *table, bool is_sub_part) {
  Field **ptr, *field, **field_array;
  uint num_fields = 0;
  uint size_field_array;
  uint i = 0;
  uint inx;
  partition_info *part_info = table->part_info;
  int result = false;
  DBUG_TRACE;

  ptr = table->field;
  while ((field = *(ptr++))) {
    if (field->flags & GET_FIXED_FIELDS_FLAG) num_fields++;
  }
  if (num_fields > MAX_REF_PARTS) {
    const char *err_str;
    if (is_sub_part)
      err_str = "subpartition function";
    else
      err_str = "partition function";
    my_error(ER_TOO_MANY_PARTITION_FUNC_FIELDS_ERROR, MYF(0), err_str);
    return true;
  }
  if (num_fields == 0) {
    /*
      We are using hidden key as partitioning field
    */
    DBUG_ASSERT(!is_sub_part);
    return result;
  }
  size_field_array = (num_fields + 1) * sizeof(Field *);
  field_array = (Field **)sql_calloc(size_field_array);
  if (unlikely(!field_array)) {
    mem_alloc_error(size_field_array);
    result = true;
  }
  ptr = table->field;
  while ((field = *(ptr++))) {
    if (field->flags & GET_FIXED_FIELDS_FLAG) {
      field->flags &= ~GET_FIXED_FIELDS_FLAG;
      field->flags |= FIELD_IN_PART_FUNC_FLAG;
      if (likely(!result)) {
        if (!is_sub_part && part_info->column_list) {
          List_iterator<char> it(part_info->part_field_list);
          char *field_name;

          DBUG_ASSERT(num_fields == part_info->part_field_list.elements);
          inx = 0;
          do {
            field_name = it++;
            if (!my_strcasecmp(system_charset_info, field_name,
                               field->field_name))
              break;
          } while (++inx < num_fields);
          if (inx == num_fields) {
            /*
              Should not occur since it should already been checked in either
              add_column_list_values, handle_list_of_fields,
              check_partition_info etc.
            */
            DBUG_ASSERT(0);
            my_error(ER_FIELD_NOT_FOUND_PART_ERROR, MYF(0));
            result = true;
            continue;
          }
        } else
          inx = i;
        field_array[inx] = field;
        i++;

        /*
          We check that the fields are proper. It is required for each
          field in a partition function to:
          1) Not be a BLOB of any type
            A BLOB takes too long time to evaluate so we don't want it for
            performance reasons.
        */

        if (unlikely(field->flags & BLOB_FLAG)) {
          my_error(ER_BLOB_FIELD_IN_PART_FUNC_ERROR, MYF(0));
          result = true;
        }
      }
    }
  }
  field_array[num_fields] = nullptr;
  if (!is_sub_part) {
    part_info->part_field_array = field_array;
    part_info->num_part_fields = num_fields;
  } else {
    part_info->subpart_field_array = field_array;
    part_info->num_subpart_fields = num_fields;
  }
  return result;
}

/*
  Create a field array including all fields of both the partitioning and the
  subpartitioning functions.

  SYNOPSIS
    create_full_part_field_array()
    thd                  Thread handle
    table                TABLE object for which partition fields are set-up
    part_info            Reference to partitioning data structure

  RETURN VALUE
    true                 Memory allocation of field array failed
    false                Ok

  DESCRIPTION
    If there is no subpartitioning then the same array is used as for the
    partitioning. Otherwise a new array is built up using the flag
    FIELD_IN_PART_FUNC in the field object.
    This function is called from fix_partition_func
*/

static bool create_full_part_field_array(THD *thd, TABLE *table,
                                         partition_info *part_info) {
  bool result = false;
  Field **ptr;
  my_bitmap_map *bitmap_buf;
  DBUG_TRACE;

  if (!part_info->is_sub_partitioned()) {
    part_info->full_part_field_array = part_info->part_field_array;
    part_info->num_full_part_fields = part_info->num_part_fields;
  } else {
    Field *field, **field_array;
    uint num_part_fields = 0, size_field_array;
    ptr = table->field;
    while ((field = *(ptr++))) {
      if (field->flags & FIELD_IN_PART_FUNC_FLAG) num_part_fields++;
    }
    size_field_array = (num_part_fields + 1) * sizeof(Field *);
    field_array = (Field **)sql_calloc(size_field_array);
    if (unlikely(!field_array)) {
      mem_alloc_error(size_field_array);
      result = true;
      goto end;
    }
    num_part_fields = 0;
    ptr = table->field;
    while ((field = *(ptr++))) {
      if (field->flags & FIELD_IN_PART_FUNC_FLAG)
        field_array[num_part_fields++] = field;
    }
    field_array[num_part_fields] = nullptr;
    part_info->full_part_field_array = field_array;
    part_info->num_full_part_fields = num_part_fields;
  }

  /*
    Initialize the set of all fields used in partition and subpartition
    expression. Required for testing of partition fields in write_set
    when updating. We need to set all bits in read_set because the row
    may need to be inserted in a different [sub]partition.
  */
  if (!(bitmap_buf = (my_bitmap_map *)thd->alloc(
            bitmap_buffer_size(table->s->fields)))) {
    mem_alloc_error(bitmap_buffer_size(table->s->fields));
    result = true;
    goto end;
  }
  if (bitmap_init(&part_info->full_part_field_set, bitmap_buf,
                  table->s->fields)) {
    mem_alloc_error(table->s->fields);
    result = true;
    goto end;
  }
  /*
    full_part_field_array may be NULL if storage engine supports native
    partitioning.
  */
  if ((ptr = part_info->full_part_field_array))
    for (; *ptr; ptr++)
      bitmap_set_bit(&part_info->full_part_field_set, (*ptr)->field_index);

end:
  return result;
}

/*

  Clear flag GET_FIXED_FIELDS_FLAG in all fields of a key previously set by
  set_indicator_in_key_fields (always used in pairs).

  SYNOPSIS
    clear_indicator_in_key_fields()
    key_info                  Reference to find the key fields

  RETURN VALUE
    NONE

  DESCRIPTION
    These support routines is used to set/reset an indicator of all fields
    in a certain key. It is used in conjunction with another support routine
    that traverse all fields in the PF to find if all or some fields in the
    PF is part of the key. This is used to check primary keys and unique
    keys involve all fields in PF (unless supported) and to derive the
    key_map's used to quickly decide whether the index can be used to
    derive which partitions are needed to scan.
*/

static void clear_indicator_in_key_fields(KEY *key_info) {
  KEY_PART_INFO *key_part;
  uint key_parts = key_info->user_defined_key_parts, i;
  for (i = 0, key_part = key_info->key_part; i < key_parts; i++, key_part++)
    key_part->field->flags &= (~GET_FIXED_FIELDS_FLAG);
}

/*
  Set flag GET_FIXED_FIELDS_FLAG in all fields of a key.

  SYNOPSIS
    set_indicator_in_key_fields
    key_info                  Reference to find the key fields

  RETURN VALUE
    NONE
*/

static void set_indicator_in_key_fields(KEY *key_info) {
  KEY_PART_INFO *key_part;
  uint key_parts = key_info->user_defined_key_parts, i;
  for (i = 0, key_part = key_info->key_part; i < key_parts; i++, key_part++)
    key_part->field->flags |= GET_FIXED_FIELDS_FLAG;
}

/*
  Check if all or some fields in partition field array is part of a key
  previously used to tag key fields.

  SYNOPSIS
    check_fields_in_PF()
    ptr                  Partition field array
    out:all_fields       Is all fields of partition field array used in key
    out:some_fields      Is some fields of partition field array used in key

  RETURN VALUE
    all_fields, some_fields
*/

static void check_fields_in_PF(Field **ptr, bool *all_fields,
                               bool *some_fields) {
  DBUG_TRACE;

  *all_fields = true;
  *some_fields = false;
  if ((!ptr) || !(*ptr)) {
    *all_fields = false;
    return;
  }
  do {
    /* Check if the field of the PF is part of the current key investigated */
    if ((*ptr)->flags & GET_FIXED_FIELDS_FLAG)
      *some_fields = true;
    else
      *all_fields = false;
  } while (*(++ptr));
}

/*
  Clear flag GET_FIXED_FIELDS_FLAG in all fields of the table.
  This routine is used for error handling purposes.

  SYNOPSIS
    clear_field_flag()
    table                TABLE object for which partition fields are set-up

  RETURN VALUE
    NONE
*/

static void clear_field_flag(TABLE *table) {
  Field **ptr;
  DBUG_TRACE;

  for (ptr = table->field; *ptr; ptr++)
    (*ptr)->flags &= (~GET_FIXED_FIELDS_FLAG);
}

/*
  find_field_in_table_sef finds the field given its name. All fields get
  GET_FIXED_FIELDS_FLAG set.

  SYNOPSIS
    handle_list_of_fields()
    it                   A list of field names for the partition function
    table                TABLE object for which partition fields are set-up
    part_info            Reference to partitioning data structure
    sub_part             Is the table subpartitioned as well

  RETURN VALUE
    true                 Fields in list of fields not part of table
    false                All fields ok and array created

  DESCRIPTION
    This routine sets-up the partition field array for KEY partitioning, it
    also verifies that all fields in the list of fields is actually a part of
    the table.

*/

static bool handle_list_of_fields(List_iterator<char> it, TABLE *table,
                                  partition_info *part_info, bool is_sub_part) {
  bool result;
  char *field_name;
  bool is_list_empty = true;
  DBUG_TRACE;

  while ((field_name = it++)) {
    is_list_empty = false;
    Field *field = find_field_in_table_sef(table, field_name);
    if (likely(field != nullptr))
      field->flags |= GET_FIXED_FIELDS_FLAG;
    else {
      my_error(ER_FIELD_NOT_FOUND_PART_ERROR, MYF(0));
      clear_field_flag(table);
      result = true;
      goto end;
    }
  }
  if (is_list_empty && part_info->part_type == partition_type::HASH) {
    uint primary_key = table->s->primary_key;
    if (primary_key != MAX_KEY) {
      uint num_key_parts = table->key_info[primary_key].user_defined_key_parts,
           i;
      /*
        In the case of an empty list we use primary key as partition key.
      */
      for (i = 0; i < num_key_parts; i++) {
        Field *field = table->key_info[primary_key].key_part[i].field;
        field->flags |= GET_FIXED_FIELDS_FLAG;
      }
    } else {
      if (table->s->db_type()->partition_flags &&
          (table->s->db_type()->partition_flags() & HA_USE_AUTO_PARTITION)) {
        /*
          This engine can handle automatic partitioning and there is no
          primary key. In this case we rely on that the engine handles
          partitioning based on a hidden key. Thus we allocate no
          array for partitioning fields.
        */
        return false;
      } else {
        my_error(ER_FIELD_NOT_FOUND_PART_ERROR, MYF(0));
        return true;
      }
    }
  }
  result = set_up_field_array(table, is_sub_part);
end:
  return result;
}

/*
  Support function to check if all VALUES * (expression) is of the
  right sign (no signed constants when unsigned partition function)

  SYNOPSIS
    check_signed_flag()
    part_info                Partition info object

  RETURN VALUES
    0                        No errors due to sign errors
    >0                       Sign error
*/

static int check_signed_flag(partition_info *part_info) {
  int error = 0;
  uint i = 0;
  if (part_info->part_type != partition_type::HASH &&
      part_info->part_expr->unsigned_flag) {
    List_iterator<partition_element> part_it(part_info->partitions);
    do {
      partition_element *part_elem = part_it++;

      if (part_elem->signed_flag) {
        my_error(ER_PARTITION_CONST_DOMAIN_ERROR, MYF(0));
        error = ER_PARTITION_CONST_DOMAIN_ERROR;
        break;
      }
    } while (++i < part_info->num_parts);
  }
  return error;
}

/**
  Initialize lex object for use in fix_fields and parsing.

  @param thd      The thread object
  @param table    The table object
  @param lex      The LEX object, must be initialized and contain select_lex.

  @returns  false if success, true if error

  @details
    This function is used to set up a lex object on the
    stack for resolving of fields from a single table.
*/

static bool init_lex_with_single_table(THD *thd, TABLE *table, LEX *lex) {
  SELECT_LEX *select_lex = lex->select_lex;
  Name_resolution_context *context = &select_lex->context;
  /*
    We will call the parser to create a part_info struct based on the
    partition string stored in the frm file.
    We will use a local lex object for this purpose. However we also
    need to set the Name_resolution_object for this lex object. We
    do this by using add_table_to_list where we add the table that
    we're working with to the Name_resolution_context.
  */
  thd->lex = lex;
  auto table_ident = new (thd->mem_root) Table_ident(
      thd->get_protocol(), table->s->db, table->s->table_name, true);
  if (table_ident == nullptr) return true;

  TABLE_LIST *table_list =
      select_lex->add_table_to_list(thd, table_ident, nullptr, 0);
  if (table_list == nullptr) return true;

  context->resolve_in_table_list_only(table_list);
  lex->use_only_table_context = true;
  table->get_fields_in_item_tree = true;
  table_list->table = table;
  table_list->cacheable_table = false;
  return false;
}

/**
  End use of local lex with single table

  SYNOPSIS
    end_lex_with_single_table()
    @param thd               The thread object
    @param table             The table object
    @param old_lex           The real lex object connected to THD

  DESCRIPTION
    This function restores the real lex object after calling
    init_lex_with_single_table and also restores some table
    variables temporarily set.
*/

static void end_lex_with_single_table(THD *thd, TABLE *table, LEX *old_lex) {
  LEX *lex = thd->lex;
  table->get_fields_in_item_tree = false;
  lex_end(lex);
  thd->lex = old_lex;
}

/*
  The function uses a new feature in fix_fields where the flag
  GET_FIXED_FIELDS_FLAG is set for all fields in the item tree.
  This field must always be reset before returning from the function
  since it is used for other purposes as well.

  SYNOPSIS
    fix_fields_part_func()
    thd                  The thread object
    func_expr            The item tree reference of the partition function
    table                The table object
    part_info            Reference to partitioning data structure
    is_sub_part          Is the table subpartitioned as well
    is_create_table_ind  Indicator of whether openfrm was called as part of
                         CREATE or ALTER TABLE

  RETURN VALUE
    true                 An error occurred, something was wrong with the
                         partition function.
    false                Ok, a partition field array was created

  DESCRIPTION
    This function is used to build an array of partition fields for the
    partitioning function and subpartitioning function. The partitioning
    function is an item tree that must reference at least one field in the
    table. This is checked first in the parser that the function doesn't
    contain non-cacheable parts (like a random function) and by checking
    here that the function isn't a constant function.

    Calculate the number of fields in the partition function.
    Use it allocate memory for array of Field pointers.
    Initialise array of field pointers. Use information set when
    calling fix_fields and reset it immediately after.
    The get_fields_in_item_tree activates setting of bit in flags
    on the field object.
*/

static bool fix_fields_part_func(THD *thd, Item *func_expr, TABLE *table,
                                 bool is_sub_part, bool is_create_table_ind) {
  partition_info *part_info = table->part_info;
  bool result = true;
  int error;
  LEX *old_lex = thd->lex;
  LEX lex;
  SELECT_LEX_UNIT unit(CTX_NONE);
  SELECT_LEX select(thd->mem_root, nullptr, nullptr);
  lex.new_static_query(&unit, &select);

  DBUG_TRACE;

  if (init_lex_with_single_table(thd, table, &lex)) goto end;

  {
    Item_ident::Change_context ctx(&lex.select_lex->context);
    func_expr->walk(&Item::change_context_processor, enum_walk::POSTFIX,
                    (uchar *)&ctx);
  }
  thd->where = "partition function";
  /*
    In execution we must avoid the use of thd->change_item_tree since
    we might release memory before statement is completed. We do this
    by temporarily setting the stmt_arena->mem_root to be the mem_root
    of the table object, this also ensures that any memory allocated
    during fix_fields will not be released at end of execution of this
    statement. Thus the item tree will remain valid also in subsequent
    executions of this table object. We do however not at the moment
    support allocations during execution of val_int so any item class
    that does this during val_int must be disallowed as partition
    function.
    SEE Bug #21658

    This is a tricky call to prepare for since it can have a large number
    of interesting side effects, both desirable and undesirable.
  */
  {
    const bool save_agg_func = thd->lex->current_select()->agg_func_used();

    error = func_expr->fix_fields(thd, &func_expr);

    /*
      Restore agg_func.
      fix_fields should not affect the optimizer later, see Bug#46923.
    */
    thd->lex->current_select()->set_agg_func_used(save_agg_func);
  }
  if (unlikely(error)) {
    DBUG_PRINT("info", ("Field in partition function not part of table"));
    clear_field_flag(table);
    goto end;
  }
  if (unlikely(func_expr->const_item())) {
    my_error(ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR, MYF(0));
    clear_field_flag(table);
    goto end;
  }

  /*
    We don't allow creating partitions with expressions with non matching
    arguments as a (sub)partitioning function,
    but we want to allow such expressions when opening existing tables for
    easier maintenance. This exception should be deprecated at some point
    in future so that we always throw an error.
  */
  if (func_expr->walk(&Item::check_valid_arguments_processor,
                      enum_walk::POSTFIX, nullptr)) {
    if (is_create_table_ind) {
      my_error(ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR, MYF(0));
      goto end;
    } else
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR,
                   ER_THD(thd, ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR));
  }

  if ((!is_sub_part) && (error = check_signed_flag(part_info))) goto end;
  result = set_up_field_array(table, is_sub_part);
end:
  end_lex_with_single_table(thd, table, old_lex);
#if !defined(DBUG_OFF)
  Item_ident::Change_context nul_ctx(nullptr);
  func_expr->walk(&Item::change_context_processor, enum_walk::POSTFIX,
                  (uchar *)&nul_ctx);
#endif
  return result;
}

/*
  Check that the primary key contains all partition fields if defined

  SYNOPSIS
    check_primary_key()
    table                TABLE object for which partition fields are set-up

  RETURN VALUES
    true                 Not all fields in partitioning function was part
                         of primary key
    false                Ok, all fields of partitioning function were part
                         of primary key

  DESCRIPTION
    This function verifies that if there is a primary key that it contains
    all the fields of the partition function.
    This is a temporary limitation that will hopefully be removed after a
    while.
*/

static bool check_primary_key(TABLE *table) {
  uint primary_key = table->s->primary_key;
  bool all_fields, some_fields;
  bool result = false;
  DBUG_TRACE;

  if (primary_key < MAX_KEY) {
    set_indicator_in_key_fields(table->key_info + primary_key);
    check_fields_in_PF(table->part_info->full_part_field_array, &all_fields,
                       &some_fields);
    clear_indicator_in_key_fields(table->key_info + primary_key);
    if (unlikely(!all_fields)) {
      my_error(ER_UNIQUE_KEY_NEED_ALL_FIELDS_IN_PF, MYF(0), "PRIMARY KEY");
      result = true;
    }
  }
  return result;
}

/*
  Check that unique keys contains all partition fields

  SYNOPSIS
    check_unique_keys()
    table                TABLE object for which partition fields are set-up

  RETURN VALUES
    true                 Not all fields in partitioning function was part
                         of all unique keys
    false                Ok, all fields of partitioning function were part
                         of unique keys

  DESCRIPTION
    This function verifies that if there is a unique index that it contains
    all the fields of the partition function.
    This is a temporary limitation that will hopefully be removed after a
    while.
*/

static bool check_unique_keys(TABLE *table) {
  bool all_fields, some_fields;
  bool result = false;
  uint keys = table->s->keys;
  uint i;
  DBUG_TRACE;

  for (i = 0; i < keys; i++) {
    if (table->key_info[i].flags & HA_NOSAME)  // Unique index
    {
      set_indicator_in_key_fields(table->key_info + i);
      check_fields_in_PF(table->part_info->full_part_field_array, &all_fields,
                         &some_fields);
      clear_indicator_in_key_fields(table->key_info + i);
      if (unlikely(!all_fields)) {
        my_error(ER_UNIQUE_KEY_NEED_ALL_FIELDS_IN_PF, MYF(0), "UNIQUE INDEX");
        result = true;
        break;
      }
    }
  }
  return result;
}

/*
  An important optimisation is whether a range on a field can select a subset
  of the partitions.
  A prerequisite for this to happen is that the PF is a growing function OR
  a shrinking function.
  This can never happen for a multi-dimensional PF. Thus this can only happen
  with PF with at most one field involved in the PF.
  The idea is that if the function is a growing function and you know that
  the field of the PF is 4 <= A <= 6 then we can convert this to a range
  in the PF instead by setting the range to PF(4) <= PF(A) <= PF(6). In the
  case of RANGE PARTITIONING and LIST PARTITIONING this can be used to
  calculate a set of partitions rather than scanning all of them.
  Thus the following prerequisites are there to check if sets of partitions
  can be found.
  1) Only possible for RANGE and LIST partitioning (not for subpartitioning)
  2) Only possible if PF only contains 1 field
  3) Possible if PF is a growing function of the field
  4) Possible if PF is a shrinking function of the field
  OBSERVATION:
  1) IF f1(A) is a growing function AND f2(A) is a growing function THEN
     f1(A) + f2(A) is a growing function
     f1(A) * f2(A) is a growing function if f1(A) >= 0 and f2(A) >= 0
  2) IF f1(A) is a growing function and f2(A) is a shrinking function THEN
     f1(A) / f2(A) is a growing function if f1(A) >= 0 and f2(A) > 0
  3) IF A is a growing function then a function f(A) that removes the
     least significant portion of A is a growing function
     E.g. DATE(datetime) is a growing function
     MONTH(datetime) is not a growing/shrinking function
  4) IF f1(A) is a growing function and f2(A) is a growing function THEN
     f1(f2(A)) and f2(f1(A)) are also growing functions
  5) IF f1(A) is a shrinking function and f2(A) is a growing function THEN
     f1(f2(A)) is a shrinking function and f2(f1(A)) is a shrinking function
  6) f1(A) = A is a growing function
  7) f1(A) = A*a + b (where a and b are constants) is a growing function

  By analysing the item tree of the PF we can use these deducements and
  derive whether the PF is a growing function or a shrinking function or
  neither of it.

  If the PF is range capable then a flag is set on the table object
  indicating this to notify that we can use also ranges on the field
  of the PF to deduce a set of partitions if the fields of the PF were
  not all fully bound.

  SYNOPSIS
    check_range_capable_PF()

  DESCRIPTION
    Support for this is not implemented yet.
*/

static void check_range_capable_PF(TABLE *) { DBUG_TRACE; }

/**
  Set up partition bitmaps

    @param part_info     Reference to partitioning data structure

  @return Operation status
    @retval true         Memory allocation failure
    @retval false        Success

    Allocate memory for bitmaps of the partitioned table
    and initialise it.
*/

static bool set_up_partition_bitmaps(partition_info *part_info) {
  DBUG_TRACE;

  DBUG_ASSERT(!part_info->bitmaps_are_initialized);

  if (part_info->init_partition_bitmap(&part_info->read_partitions,
                                       &part_info->table->mem_root))
    return true;
  if (part_info->init_partition_bitmap(&part_info->lock_partitions,
                                       &part_info->table->mem_root))
    return true;

  part_info->bitmaps_are_initialized = true;
  part_info->set_partition_bitmaps(nullptr);
  return false;
}

/*
  Set up partition key maps

  SYNOPSIS
    set_up_partition_key_maps()
    table                TABLE object for which partition fields are set-up
    part_info            Reference to partitioning data structure

  RETURN VALUES
    None

  DESCRIPTION
    This function sets up a couple of key maps to be able to quickly check
    if an index ever can be used to deduce the partition fields or even
    a part of the fields of the  partition function.
    We set up the following key_map's.
    PF = Partition Function
    1) All fields of the PF is set even by equal on the first fields in the
       key
    2) All fields of the PF is set if all fields of the key is set
    3) At least one field in the PF is set if all fields is set
    4) At least one field in the PF is part of the key
*/

static void set_up_partition_key_maps(TABLE *table, partition_info *part_info) {
  uint keys = table->s->keys;
  uint i;
  bool all_fields, some_fields;
  DBUG_TRACE;

  part_info->all_fields_in_PF.clear_all();
  part_info->all_fields_in_PPF.clear_all();
  part_info->all_fields_in_SPF.clear_all();
  part_info->some_fields_in_PF.clear_all();
  for (i = 0; i < keys; i++) {
    set_indicator_in_key_fields(table->key_info + i);
    check_fields_in_PF(part_info->full_part_field_array, &all_fields,
                       &some_fields);
    if (all_fields) part_info->all_fields_in_PF.set_bit(i);
    if (some_fields) part_info->some_fields_in_PF.set_bit(i);
    if (part_info->is_sub_partitioned()) {
      check_fields_in_PF(part_info->part_field_array, &all_fields,
                         &some_fields);
      if (all_fields) part_info->all_fields_in_PPF.set_bit(i);
      check_fields_in_PF(part_info->subpart_field_array, &all_fields,
                         &some_fields);
      if (all_fields) part_info->all_fields_in_SPF.set_bit(i);
    }
    clear_indicator_in_key_fields(table->key_info + i);
  }
}

/*
  Set up function pointers for partition function

  SYNOPSIS
    set_up_partition_func_pointers()
    part_info            Reference to partitioning data structure

  RETURN VALUE
    NONE

  DESCRIPTION
    Set-up all function pointers for calculation of partition id,
    subpartition id and the upper part in subpartitioning. This is to speed up
    execution of get_partition_id which is executed once every record to be
    written and deleted and twice for updates.
*/

static void set_up_partition_func_pointers(partition_info *part_info) {
  DBUG_TRACE;

  if (part_info->is_sub_partitioned()) {
    part_info->get_partition_id = get_partition_id_with_sub;
    if (part_info->part_type == partition_type::RANGE) {
      if (part_info->column_list)
        part_info->get_part_partition_id = get_partition_id_range_col;
      else
        part_info->get_part_partition_id = get_partition_id_range;
      if (part_info->list_of_subpart_fields) {
        if (part_info->linear_hash_ind)
          part_info->get_subpartition_id = get_partition_id_linear_key_sub;
        else
          part_info->get_subpartition_id = get_partition_id_key_sub;
      } else {
        if (part_info->linear_hash_ind)
          part_info->get_subpartition_id = get_partition_id_linear_hash_sub;
        else
          part_info->get_subpartition_id = get_partition_id_hash_sub;
      }
    } else /* LIST Partitioning */
    {
      if (part_info->column_list)
        part_info->get_part_partition_id = get_partition_id_list_col;
      else
        part_info->get_part_partition_id = get_partition_id_list;
      if (part_info->list_of_subpart_fields) {
        if (part_info->linear_hash_ind)
          part_info->get_subpartition_id = get_partition_id_linear_key_sub;
        else
          part_info->get_subpartition_id = get_partition_id_key_sub;
      } else {
        if (part_info->linear_hash_ind)
          part_info->get_subpartition_id = get_partition_id_linear_hash_sub;
        else
          part_info->get_subpartition_id = get_partition_id_hash_sub;
      }
    }
  } else /* No subpartitioning */
  {
    part_info->get_part_partition_id = nullptr;
    part_info->get_subpartition_id = nullptr;
    if (part_info->part_type == partition_type::RANGE) {
      if (part_info->column_list)
        part_info->get_partition_id = get_partition_id_range_col;
      else
        part_info->get_partition_id = get_partition_id_range;
    } else if (part_info->part_type == partition_type::LIST) {
      if (part_info->column_list)
        part_info->get_partition_id = get_partition_id_list_col;
      else
        part_info->get_partition_id = get_partition_id_list;
    } else /* HASH partitioning */
    {
      if (part_info->list_of_part_fields) {
        if (part_info->linear_hash_ind)
          part_info->get_partition_id = get_partition_id_linear_key_nosub;
        else
          part_info->get_partition_id = get_partition_id_key_nosub;
      } else {
        if (part_info->linear_hash_ind)
          part_info->get_partition_id = get_partition_id_linear_hash_nosub;
        else
          part_info->get_partition_id = get_partition_id_hash_nosub;
      }
    }
  }
  /*
    We need special functions to handle character sets since they require copy
    of field pointers and restore afterwards. For subpartitioned tables we do
    the copy and restore individually on the part and subpart parts. For non-
    subpartitioned tables we use the same functions as used for the parts part
    of subpartioning.
    Thus for subpartitioned tables the get_partition_id is always
    get_partition_id_with_sub, even when character sets exists.
  */
  if (part_info->part_charset_field_array) {
    if (part_info->is_sub_partitioned()) {
      DBUG_ASSERT(part_info->get_part_partition_id);
      if (!part_info->column_list) {
        part_info->get_part_partition_id_charset =
            part_info->get_part_partition_id;
        part_info->get_part_partition_id = get_part_id_charset_func_part;
      }
    } else {
      DBUG_ASSERT(part_info->get_partition_id);
      if (!part_info->column_list) {
        part_info->get_part_partition_id_charset = part_info->get_partition_id;
        part_info->get_part_partition_id = get_part_id_charset_func_part;
      }
    }
  }
  if (part_info->subpart_charset_field_array) {
    DBUG_ASSERT(part_info->get_subpartition_id);
    part_info->get_subpartition_id_charset = part_info->get_subpartition_id;
    part_info->get_subpartition_id = get_part_id_charset_func_subpart;
  }
}

/*
  For linear hashing we need a mask which is on the form 2**n - 1 where
  2**n >= num_parts. Thus if num_parts is 6 then mask is 2**3 - 1 = 8 - 1 = 7.

  SYNOPSIS
    set_linear_hash_mask()
    part_info            Reference to partitioning data structure
    num_parts            Number of parts in linear hash partitioning

  RETURN VALUE
    NONE
*/

void set_linear_hash_mask(partition_info *part_info, uint num_parts) {
  uint mask;

  for (mask = 1; mask < num_parts; mask <<= 1)
    ;
  part_info->linear_hash_mask = mask - 1;
}

/*
  This function calculates the partition id provided the result of the hash
  function using linear hashing parameters, mask and number of partitions.

  SYNOPSIS
    get_part_id_from_linear_hash()
    hash_value          Hash value calculated by HASH function or KEY function
    mask                Mask calculated previously by set_linear_hash_mask
    num_parts           Number of partitions in HASH partitioned part

  RETURN VALUE
    part_id             The calculated partition identity (starting at 0)

  DESCRIPTION
    The partition is calculated according to the theory of linear hashing.
    See e.g. Linear hashing: a new tool for file and table addressing,
    Reprinted from VLDB-80 in Readings Database Systems, 2nd ed, M. Stonebraker
    (ed.), Morgan Kaufmann 1994.
*/

static uint32 get_part_id_from_linear_hash(longlong hash_value, uint mask,
                                           uint num_parts) {
  uint32 part_id = (uint32)(hash_value & mask);

  if (part_id >= num_parts) {
    uint new_mask = ((mask + 1) >> 1) - 1;
    part_id = (uint32)(hash_value & new_mask);
  }
  return part_id;
}

/*
  Check if a particular field is in need of character set
  handling for partition functions.

  SYNOPSIS
    field_is_partition_charset()
    field                         The field to check

  RETURN VALUES
    false                        Not in need of character set handling
    true                         In need of character set handling
*/

bool field_is_partition_charset(Field *field) {
  if (!(field->type() == MYSQL_TYPE_STRING) &&
      !(field->type() == MYSQL_TYPE_VARCHAR))
    return false;
  {
    const CHARSET_INFO *cs = field->charset();
    if (!(field->type() == MYSQL_TYPE_STRING) || !(cs->state & MY_CS_BINSORT))
      return true;
    return false;
  }
}

/*
  Check that partition function doesn't contain any forbidden
  character sets and collations.

  SYNOPSIS
    check_part_func_fields()
    ptr                                 Array of Field pointers
    ok_with_charsets                    Will we report allowed charset
                                        fields as ok
  RETURN VALUES
    false                               Success
    true                                Error

  DESCRIPTION
    We will check in this routine that the fields of the partition functions
    do not contain unallowed parts. It can also be used to check if there
    are fields that require special care by calling my_strnxfrm before
    calling the functions to calculate partition id.
*/

bool check_part_func_fields(Field **ptr, bool ok_with_charsets) {
  Field *field;
  DBUG_TRACE;

  while ((field = *(ptr++))) {
    /*
      For CHAR/VARCHAR fields we need to take special precautions.
      Binary collation with CHAR is automatically supported. Other
      types need some kind of standardisation function handling
    */
    if (field_is_partition_charset(field)) {
      const CHARSET_INFO *cs = field->charset();
      if (!ok_with_charsets || cs->mbmaxlen > 1 || cs->strxfrm_multiply > 1) {
        return true;
      }
    }
  }
  return false;
}

/*
  fix partition functions

  SYNOPSIS
    fix_partition_func()
    thd                  The thread object
    table                TABLE object for which partition fields are set-up
    is_create_table_ind  Indicator of whether openfrm was called as part of
                         CREATE or ALTER TABLE

  RETURN VALUE
    true                 Error
    false                Success

  DESCRIPTION
    The name parameter contains the full table name and is used to get the
    database name of the table which is used to set-up a correct
    TABLE_LIST object for use in fix_fields.

NOTES
    This function is called as part of opening the table by opening the .frm
    file. It is a part of CREATE TABLE to do this so it is quite permissible
    that errors due to erroneus syntax isn't found until we come here.
    If the user has used a non-existing field in the table is one such example
    of an error that is not discovered until here.
*/

bool fix_partition_func(THD *thd, TABLE *table, bool is_create_table_ind) {
  bool result = true;
  partition_info *part_info = table->part_info;
  enum_mark_columns save_mark_used_columns = thd->mark_used_columns;
  Partition_handler *part_handler;
  const ulong save_want_privilege = thd->want_privilege;
  DBUG_TRACE;

  if (part_info->fixed) {
    return false;
  }
  thd->mark_used_columns = MARK_COLUMNS_NONE;
  thd->want_privilege = 0;

  if (!is_create_table_ind || thd->lex->sql_command != SQLCOM_CREATE_TABLE) {
    if (partition_default_handling(table, part_info, is_create_table_ind,
                                   table->s->normalized_path.str)) {
      return true;
    }
  }
  if (part_info->is_sub_partitioned()) {
    DBUG_ASSERT(part_info->subpart_type == partition_type::HASH);
    /*
      Subpartition is defined. We need to verify that subpartitioning
      function is correct.
    */
    if (part_info->linear_hash_ind)
      set_linear_hash_mask(part_info, part_info->num_subparts);
    if (part_info->list_of_subpart_fields) {
      List_iterator<char> it(part_info->subpart_field_list);
      if (unlikely(handle_list_of_fields(it, table, part_info, true))) goto end;
    } else {
      if (unlikely(fix_fields_part_func(thd, part_info->subpart_expr, table,
                                        true, is_create_table_ind)))
        goto end;
      if (unlikely(part_info->subpart_expr->result_type() != INT_RESULT)) {
        part_info->report_part_expr_error(true);
        goto end;
      }
    }
  }
  DBUG_ASSERT(part_info->part_type != partition_type::NONE);
  /*
    Partition is defined. We need to verify that partitioning
    function is correct.
  */
  if (part_info->part_type == partition_type::HASH) {
    if (part_info->linear_hash_ind)
      set_linear_hash_mask(part_info, part_info->num_parts);
    if (part_info->list_of_part_fields) {
      List_iterator<char> it(part_info->part_field_list);
      if (unlikely(handle_list_of_fields(it, table, part_info, false)))
        goto end;
    } else {
      if (unlikely(fix_fields_part_func(thd, part_info->part_expr, table, false,
                                        is_create_table_ind)))
        goto end;
      if (unlikely(part_info->part_expr->result_type() != INT_RESULT)) {
        part_info->report_part_expr_error(false);
        goto end;
      }
    }
    part_info->fixed = true;
  } else {
    const char *error_str;
    if (part_info->column_list) {
      List_iterator<char> it(part_info->part_field_list);
      if (unlikely(handle_list_of_fields(it, table, part_info, false)))
        goto end;
    } else {
      if (unlikely(fix_fields_part_func(thd, part_info->part_expr, table, false,
                                        is_create_table_ind)))
        goto end;
    }
    part_info->fixed = true;
    if (part_info->part_type == partition_type::RANGE) {
      error_str = partition_keywords[PKW_RANGE].str;
      if (unlikely(part_info->check_range_constants(thd))) goto end;
    } else if (part_info->part_type == partition_type::LIST) {
      error_str = partition_keywords[PKW_LIST].str;
      if (unlikely(part_info->check_list_constants(thd))) goto end;
    } else {
      DBUG_ASSERT(0);
      my_error(ER_INCONSISTENT_PARTITION_INFO_ERROR, MYF(0));
      goto end;
    }
    if (unlikely(part_info->num_parts < 1)) {
      my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), error_str);
      goto end;
    }
    if (unlikely(!part_info->column_list &&
                 part_info->part_expr->result_type() != INT_RESULT)) {
      part_info->report_part_expr_error(false);
      goto end;
    }
  }
  if (((part_info->part_type != partition_type::HASH ||
        part_info->list_of_part_fields == false) &&
       !part_info->column_list &&
       check_part_func_fields(part_info->part_field_array, true)) ||
      (part_info->list_of_subpart_fields == false &&
       part_info->is_sub_partitioned() &&
       check_part_func_fields(part_info->subpart_field_array, true))) {
    /*
      Range/List/HASH (but not KEY) and not COLUMNS or HASH subpartitioning
      with columns in the partitioning expression using unallowed charset.
    */
    my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
    goto end;
  }
  if (unlikely(create_full_part_field_array(thd, table, part_info))) goto end;
  if (unlikely(check_primary_key(table))) goto end;
  if (unlikely((!(table->s->db_type()->partition_flags &&
                  (table->s->db_type()->partition_flags() &
                   HA_CAN_PARTITION_UNIQUE))) &&
               check_unique_keys(table)))
    goto end;
  if (unlikely(set_up_partition_bitmaps(part_info))) goto end;
  if (unlikely(part_info->set_up_charset_field_preps())) {
    my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
    goto end;
  }
  if (unlikely(part_info->check_partition_field_length())) {
    my_error(ER_PARTITION_FIELDS_TOO_LONG, MYF(0));
    goto end;
  }
  check_range_capable_PF(table);
  set_up_partition_key_maps(table, part_info);
  set_up_partition_func_pointers(part_info);
  set_up_range_analysis_info(part_info);
  part_handler = table->file->get_partition_handler();
  if (part_handler) {
    part_handler->set_part_info(part_info, false);
    result = false;
  } else {
    DBUG_ASSERT(0);
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
  }
end:
  thd->mark_used_columns = save_mark_used_columns;
  thd->want_privilege = save_want_privilege;
  DBUG_PRINT("info", ("thd->mark_used_columns: %d", thd->mark_used_columns));
  return result;
}

// TODO: Change this to use streams instead, to make it possible to skip
//       temporary files etc. and write directly to a string if wanted.
/*
  The code below is support routines for the reverse parsing of the
  partitioning syntax. This feature is very useful to generate syntax for
  all default values to avoid all default checking when opening the frm
  file. It is also used when altering the partitioning by use of various
  ALTER TABLE commands. Finally it is used for SHOW CREATE TABLES.
*/

static int add_write(File fptr, const char *buf, size_t len) {
  size_t ret_code =
      mysql_file_write(fptr, (const uchar *)buf, len, MYF(MY_FNABP));

  if (likely(ret_code == 0))
    return 0;
  else
    return 1;
}

static int add_string_object(File fptr, String *string) {
  return add_write(fptr, string->ptr(), string->length());
}

static int add_string(File fptr, const char *string) {
  return add_write(fptr, string, strlen(string));
}

static int add_string_len(File fptr, const char *string, size_t len) {
  return add_write(fptr, string, len);
}

static int add_space(File fptr) { return add_string(fptr, space_str); }

static int add_comma(File fptr) { return add_string(fptr, comma_str); }

static int add_equal(File fptr) { return add_string(fptr, equal_str); }

static int add_end_parenthesis(File fptr) {
  return add_string(fptr, end_paren_str);
}

static int add_begin_parenthesis(File fptr) {
  return add_string(fptr, begin_paren_str);
}

static int add_part_key_word(File fptr, const char *key_string) {
  int err = add_string(fptr, key_string);
  err += add_space(fptr);
  return err;
}

static int add_partition(File fptr) {
  char buff[22];
  strxmov(buff, part_str, space_str, NullS);
  return add_string(fptr, buff);
}

static int add_subpartition(File fptr) {
  int err = add_string(fptr, sub_str);

  return err + add_partition(fptr);
}

static int add_partition_by(File fptr) {
  char buff[22];
  strxmov(buff, part_str, space_str, by_str, space_str, NullS);
  return add_string(fptr, buff);
}

static int add_subpartition_by(File fptr) {
  int err = add_string(fptr, sub_str);

  return err + add_partition_by(fptr);
}

/**
  Append field list to string.

  Used by KEY and COLUMNS partitioning.

  @param[in]     thd        Thread handle.
  @param[in,out] str        String to append.
  @param[in]     field_list List of field names to append.

  @return false if success, else true.
*/

static bool append_field_list(THD *thd, String *str, List<char> field_list) {
  uint i, num_fields;

  List_iterator<char> part_it(field_list);
  num_fields = field_list.elements;
  i = 0;
  ulonglong save_options = thd->variables.option_bits;
  thd->variables.option_bits &= ~OPTION_QUOTE_SHOW_CREATE;
  while (i < num_fields) {
    const char *field_str = part_it++;
    append_identifier(thd, str, field_str, strlen(field_str));
    if (i != (num_fields - 1)) {
      if (str->append(',')) {
        thd->variables.option_bits = save_options;
        return true;
      }
    }
    i++;
  }
  thd->variables.option_bits = save_options;
  return false;
}

static int add_part_field_list(File fptr, List<char> field_list) {
  int err = 0;
  THD *thd = current_thd;
  String str("", 0, system_charset_info);

  err += add_begin_parenthesis(fptr);
  if (append_field_list(thd, &str, field_list)) {
    err++;
  }
  err += add_string_object(fptr, &str);
  err += add_end_parenthesis(fptr);
  return err;
}

static int add_ident_string(File fptr, const char *name) {
  String name_string("", 0, system_charset_info);
  append_identifier(current_thd, &name_string, name, strlen(name));
  return add_string_object(fptr, &name_string);
}

static int add_name_string(File fptr, const char *name) {
  int err;
  THD *thd = current_thd;
  ulonglong save_options = thd->variables.option_bits;
  thd->variables.option_bits &= ~OPTION_QUOTE_SHOW_CREATE;
  err = add_ident_string(fptr, name);
  thd->variables.option_bits = save_options;
  return err;
}

static int add_int(File fptr, longlong number) {
  char buff[32];
  llstr(number, buff);
  return add_string(fptr, buff);
}

static int add_uint(File fptr, ulonglong number) {
  char buff[32];
  ullstr(number, buff);
  return add_string(fptr, buff);
}

/*
   Must escape strings in partitioned tables frm-files,
   parsing it later with mysql_unpack_partition will fail otherwise.
*/
static int add_quoted_string(File fptr, const char *quotestr) {
  String orgstr(quotestr, system_charset_info);
  String escapedstr;
  int err = add_string(fptr, "'");
  err += append_escaped(&escapedstr, &orgstr);
  err += add_string(fptr, escapedstr.c_ptr_safe());
  return err + add_string(fptr, "'");
}

/** Truncate the partition file name from a path if it exists.

A partition file name will contain one or more '#' characters.
One of the occurrences of '#' will be either "#P#" or "#p#" depending
on whether the storage engine has converted the filename to lower case.
If we need to truncate the name, we will allocate a new string and replace
with, in case the original string was owned by something else.

  @param[in]      root   MEM_ROOT to allocate from. If NULL alter the string
                         directly.
  @param[in,out]  path   Pointer to string to check and truncate.
*/
void truncate_partition_filename(MEM_ROOT *root, const char **path) {
  if (*path) {
    const char *last_slash = strrchr(*path, FN_LIBCHAR);

#ifdef _WIN32
    if (!last_slash) last_slash = strrchr(*path, FN_LIBCHAR2);
#endif

    if (last_slash) {
      /* Look for a partition-type filename */
      for (const char *pound = strchr(last_slash, '#'); pound;
           pound = strchr(pound + 1, '#')) {
        if ((pound[1] == 'P' || pound[1] == 'p') && pound[2] == '#') {
          if (root == nullptr) {
            char *p = const_cast<char *>(last_slash);
            *p = '\0';
          } else {
            *path = strmake_root(root, *path, last_slash - *path);
          }
          break;
        }
      }
    }
  }
}

/**
  @brief  Output a filepath.  Similar to add_keyword_string except it
also converts \ to / on Windows and skips the partition file name at
the end if found.

  @note  When Mysql sends a DATA DIRECTORY from SQL for partitions it does
not use a file name, but it does for DATA DIRECTORY on a non-partitioned
table.  So when the storage engine is asked for the DATA DIRECTORY string
after a restart through Handler::update_create_options(), the storage
engine may include the filename.
*/
static int add_keyword_path(File fptr, const char *keyword, const char *path) {
  if (strlen(path) >= FN_REFLEN) {
    my_error(ER_PATH_LENGTH, MYF(0), keyword);
    return 1;
  }

  int err = add_string(fptr, keyword);

  err += add_space(fptr);
  err += add_equal(fptr);
  err += add_space(fptr);

  char temp_path[FN_REFLEN];
  const char *temp_path_p[1];
  temp_path_p[0] = temp_path;
  strncpy(temp_path, path, FN_REFLEN - 1);
  temp_path[FN_REFLEN - 1] = '\0';
#ifdef _WIN32
  /* Convert \ to / to be able to create table on unix */
  char *pos, *end;
  size_t length = strlen(temp_path);
  for (pos = temp_path, end = pos + length; pos < end; pos++) {
    if (*pos == '\\') *pos = '/';
  }
#endif

  /*
  If the partition file name with its "#P#" identifier
  is found after the last slash, truncate that filename.
  */
  truncate_partition_filename(nullptr, temp_path_p);

  err += add_quoted_string(fptr, temp_path);

  return err + add_space(fptr);
}

static int add_keyword_string(File fptr, const char *keyword,
                              bool should_use_quotes, const char *keystr) {
  int err = add_string(fptr, keyword);

  err += add_space(fptr);
  err += add_equal(fptr);
  err += add_space(fptr);
  if (should_use_quotes)
    err += add_quoted_string(fptr, keystr);
  else
    err += add_string(fptr, keystr);
  return err + add_space(fptr);
}

static int add_keyword_int(File fptr, const char *keyword, longlong num) {
  int err = add_string(fptr, keyword);

  err += add_space(fptr);
  err += add_equal(fptr);
  err += add_space(fptr);
  err += add_int(fptr, num);
  return err + add_space(fptr);
}

static int add_engine(File fptr, handlerton *engine_type) {
  const char *engine_str = ha_resolve_storage_engine_name(engine_type);
  DBUG_ASSERT(engine_type != nullptr);
  DBUG_PRINT("info", ("ENGINE: %s", engine_str));
  int err = add_string(fptr, "ENGINE = ");
  return err + add_string(fptr, engine_str);
}

static int add_partition_options(File fptr, partition_element *p_elem) {
  int err = 0;

  err += add_space(fptr);
  if (p_elem->tablespace_name) {
    err += add_string(fptr, "TABLESPACE = ");
    err += add_ident_string(fptr, p_elem->tablespace_name);
    err += add_space(fptr);
  }
  if (p_elem->nodegroup_id != UNDEF_NODEGROUP)
    err += add_keyword_int(fptr, "NODEGROUP", (longlong)p_elem->nodegroup_id);
  if (p_elem->part_max_rows)
    err += add_keyword_int(fptr, "MAX_ROWS", (longlong)p_elem->part_max_rows);
  if (p_elem->part_min_rows)
    err += add_keyword_int(fptr, "MIN_ROWS", (longlong)p_elem->part_min_rows);
  if (!(current_thd->variables.sql_mode & MODE_NO_DIR_IN_CREATE)) {
    if (p_elem->data_file_name)
      err += add_keyword_path(fptr, "DATA DIRECTORY", p_elem->data_file_name);
    if (p_elem->index_file_name)
      err += add_keyword_path(fptr, "INDEX DIRECTORY", p_elem->index_file_name);
  }
  if (p_elem->part_comment)
    err += add_keyword_string(fptr, "COMMENT", true, p_elem->part_comment);
  return err + add_engine(fptr, p_elem->engine_type);
}

/*
  Check partition fields for result type and if they need
  to check the character set.

  SYNOPSIS
    check_part_field()
    sql_type              Type provided by user
    field_name            Name of field, used for error handling
    result_type           Out value: Result type of field
    need_cs_check         Out value: Do we need character set check

  RETURN VALUES
    true                  Error
    false                 Ok
*/

static int check_part_field(enum_field_types sql_type, const char *field_name,
                            Item_result *result_type, bool *need_cs_check) {
  if (sql_type >= MYSQL_TYPE_TINY_BLOB && sql_type <= MYSQL_TYPE_BLOB) {
    my_error(ER_BLOB_FIELD_IN_PART_FUNC_ERROR, MYF(0));
    return true;
  }
  switch (sql_type) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
      *result_type = INT_RESULT;
      *need_cs_check = false;
      return false;
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_DATETIME2:
      *result_type = STRING_RESULT;
      *need_cs_check = true;
      return false;
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
      *result_type = STRING_RESULT;
      *need_cs_check = true;
      return false;
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_GEOMETRY:
      goto error;
    default:
      goto error;
  }
error:
  my_error(ER_FIELD_TYPE_NOT_ALLOWED_AS_PARTITION_FIELD, MYF(0), field_name);
  return true;
}

/*
  Find the given field's Create_field object using name of field

  SYNOPSIS
    get_sql_field()
    field_name                   Field name
    create_fields                Info from ALTER TABLE/CREATE TABLE

  RETURN VALUE
    sql_field                    Object filled in by parser about field
    NULL                         No field found
*/

static Create_field *get_sql_field(const char *field_name,
                                   List<Create_field> *create_fields) {
  List_iterator<Create_field> it(*create_fields);
  Create_field *sql_field;
  DBUG_TRACE;

  while ((sql_field = it++)) {
    if (!(my_strcasecmp(system_charset_info, sql_field->field_name,
                        field_name))) {
      return sql_field;
    }
  }
  return nullptr;
}

int expr_to_string(String *val_conv, Item *item_expr, Field *field,
                   const char *field_name, const HA_CREATE_INFO *create_info,
                   List<Create_field> *create_fields) {
  char buffer[MAX_KEY_LENGTH];
  String str(buffer, sizeof(buffer), &my_charset_bin);
  String *res;
  const CHARSET_INFO *field_cs;
  bool need_cs_check = false;
  Item_result result_type = STRING_RESULT;

  /*
    This function is called at a very early stage, even before
    we have prepared the sql_field objects. Thus we have to
    find the proper sql_field object and get the character set
    from that object.
  */
  if (create_info) {
    Create_field *sql_field;

    if (!(sql_field = get_sql_field(field_name, create_fields))) {
      my_error(ER_FIELD_NOT_FOUND_PART_ERROR, MYF(0));
      return 1;
    }
    if (check_part_field(sql_field->sql_type, sql_field->field_name,
                         &result_type, &need_cs_check))
      return 1;
    if (need_cs_check)
      field_cs = get_sql_field_charset(sql_field, create_info);
    else
      field_cs = nullptr;
  } else {
    result_type = field->result_type();
    if (check_part_field(field->real_type(), field->field_name, &result_type,
                         &need_cs_check))
      return 1;
    DBUG_ASSERT(result_type == field->result_type());
    if (need_cs_check)
      field_cs = field->charset();
    else
      field_cs = nullptr;
  }
  if (result_type != item_expr->result_type()) {
    my_error(ER_WRONG_TYPE_COLUMN_VALUE_ERROR, MYF(0));
    return 1;
  }
  if (field_cs && field_cs != item_expr->collation.collation) {
    if (!(item_expr =
              convert_charset_partition_constant(item_expr, field_cs))) {
      my_error(ER_PARTITION_FUNCTION_IS_NOT_ALLOWED, MYF(0));
      return 1;
    }
  }
  val_conv->set_charset(system_charset_info);
  res = item_expr->val_str(&str);
  if (get_cs_converted_part_value_from_string(current_thd, item_expr, res,
                                              val_conv, field_cs, false)) {
    return 1;
  }
  return 0;
}

static int add_column_list_values(File fptr, partition_info *part_info,
                                  part_elem_value *list_value) {
  int err = 0;
  uint i;
  uint num_elements = part_info->part_field_list.elements;
  bool use_parenthesis = (part_info->part_type == partition_type::LIST &&
                          part_info->num_columns > 1U);

  if (use_parenthesis) err += add_begin_parenthesis(fptr);
  for (i = 0; i < num_elements; i++) {
    part_column_list_val *col_val = &list_value->col_val_array[i];
    if (col_val->max_value)
      err += add_string(fptr, partition_keywords[PKW_MAXVALUE].str);
    else if (col_val->null_value)
      err += add_string(fptr, "NULL");
    else {
      char buffer[MAX_KEY_LENGTH];
      String str(buffer, sizeof(buffer), &my_charset_bin);
      Item *item_expr = col_val->item_expression;
      if (!item_expr) {
        /*
          The values are not from the parser, but from the
          dd::Partition_values table. See fill_partitioning_from_dd().
        */
        DBUG_ASSERT(col_val->column_value.value_str);
        err += add_string(fptr, col_val->column_value.value_str);
      } else if (item_expr->null_value) {
        err += add_string(fptr, "NULL");
      } else {
        String val_conv;
        err +=
            expr_to_string(&val_conv, item_expr, part_info->part_field_array[i],
                           nullptr, nullptr, nullptr);
        err += add_string_object(fptr, &val_conv);
      }
    }
    if (i != (num_elements - 1)) err += add_string(fptr, comma_str);
  }
  if (use_parenthesis) err += add_end_parenthesis(fptr);
  return err;
}

static int add_partition_values(File fptr, partition_info *part_info,
                                partition_element *p_elem) {
  int err = 0;

  if (part_info->part_type == partition_type::RANGE) {
    err += add_string(fptr, " VALUES LESS THAN ");
    if (part_info->column_list) {
      List_iterator<part_elem_value> list_val_it(p_elem->list_val_list);
      part_elem_value *list_value = list_val_it++;
      err += add_begin_parenthesis(fptr);
      err += add_column_list_values(fptr, part_info, list_value);
      err += add_end_parenthesis(fptr);
    } else {
      if (!p_elem->max_value) {
        err += add_begin_parenthesis(fptr);
        if (p_elem->signed_flag)
          err += add_int(fptr, p_elem->range_value);
        else
          err += add_uint(fptr, p_elem->range_value);
        err += add_end_parenthesis(fptr);
      } else
        err += add_string(fptr, partition_keywords[PKW_MAXVALUE].str);
    }
  } else if (part_info->part_type == partition_type::LIST) {
    uint i;
    List_iterator<part_elem_value> list_val_it(p_elem->list_val_list);
    err += add_string(fptr, " VALUES IN ");
    uint num_items = p_elem->list_val_list.elements;

    err += add_begin_parenthesis(fptr);
    if (p_elem->has_null_value) {
      err += add_string(fptr, "NULL");
      if (num_items == 0) {
        err += add_end_parenthesis(fptr);
        goto end;
      }
      err += add_comma(fptr);
    }
    i = 0;
    do {
      part_elem_value *list_value = list_val_it++;

      if (part_info->column_list)
        err += add_column_list_values(fptr, part_info, list_value);
      else {
        if (!list_value->unsigned_flag)
          err += add_int(fptr, list_value->value);
        else
          err += add_uint(fptr, list_value->value);
      }
      if (i != (num_items - 1)) err += add_comma(fptr);
    } while (++i < num_items);
    err += add_end_parenthesis(fptr);
  }
end:
  return err;
}

/**
  Add 'KEY' word, with optional 'ALGORTIHM = N'.

  @param fptr                   File to write to.
  @param part_info              partition_info holding the used key_algorithm
  @param current_comment_start  NULL, or comment string encapsulating the
                                PARTITION BY clause.

  @return Operation status.
    @retval 0    Success
    @retval != 0 Failure
*/

static int add_key_with_algorithm(File fptr, partition_info *part_info,
                                  const char *current_comment_start) {
  int err = 0;
  err += add_part_key_word(fptr, partition_keywords[PKW_KEY].str);

  /*
    current_comment_start is given when called from SHOW CREATE TABLE,
    Then only add ALGORITHM = 1, not the default 2 or non-set 0!
    For .frm current_comment_start is NULL, then add ALGORITHM if != 0.
  */
  if (part_info->key_algorithm ==
          enum_key_algorithm::KEY_ALGORITHM_51 ||  // SHOW
      (!current_comment_start &&                   // .frm
       (part_info->key_algorithm != enum_key_algorithm::KEY_ALGORITHM_NONE))) {
    /* If we already are within a comment, end that comment first. */
    if (current_comment_start) err += add_string(fptr, "*/ ");
    err += add_string(fptr, "/*!50611 ");
    err += add_part_key_word(fptr, partition_keywords[PKW_ALGORITHM].str);
    err += add_equal(fptr);
    err += add_space(fptr);
    err += add_int(fptr, static_cast<longlong>(part_info->key_algorithm));
    err += add_space(fptr);
    err += add_string(fptr, "*/ ");
    if (current_comment_start) {
      /* Skip new line. */
      if (current_comment_start[0] == '\n') current_comment_start++;
      err += add_string(fptr, current_comment_start);
      err += add_space(fptr);
    }
  }
  return err;
}

static char *get_file_content(File fptr, uint *buf_length, bool use_sql_alloc) {
  my_off_t buffer_length;
  char *buf;
  buffer_length = mysql_file_seek(fptr, 0L, MY_SEEK_END, MYF(0));
  if (unlikely(buffer_length == MY_FILEPOS_ERROR)) return nullptr;
  if (unlikely(mysql_file_seek(fptr, 0L, MY_SEEK_SET, MYF(0)) ==
               MY_FILEPOS_ERROR))
    return nullptr;
  *buf_length = (uint)buffer_length;
  if (use_sql_alloc)
    buf = (char *)(*THR_MALLOC)->Alloc(*buf_length + 1);
  else
    buf = (char *)my_malloc(key_memory_partition_syntax_buffer, *buf_length + 1,
                            MYF(MY_WME));
  if (!buf) return nullptr;

  if (unlikely(
          mysql_file_read(fptr, (uchar *)buf, *buf_length, MYF(MY_FNABP)))) {
    if (!use_sql_alloc) my_free(buf);
    buf = nullptr;
  } else
    buf[*buf_length] = 0;
  return buf;
}

/**
  Generate the partition syntax from the partition data structure.
  Useful for support of generating defaults, SHOW CREATE TABLES
  and easy partition management.

  @param  part_info               The partitioning data structure
  @param  buf_length              A pointer to the returned buffer length
  @param  use_sql_alloc           Allocate buffer from sql_alloc if true
                                  otherwise use my_malloc
  @param  show_partition_options  Should we display partition options
  @param  print_expr              Indicates whether partitioning expressions
                                  should be re-printed to get quoting according
                                  to current sql_mode.
  @param  current_comment_start   NULL, or comment string encapsulating the
                                  PARTITION BY clause.

  @retval NULL - error

  @note
  Here we will generate the full syntax for the given command where all
  defaults have been expanded. By so doing the it is also possible to
  make lots of checks of correctness while at it.
  This could will also be reused for SHOW CREATE TABLES and also for all
  type ALTER TABLE commands focusing on changing the PARTITION structure
  in any fashion.

  The implementation writes the syntax to a temporary file (essentially
  an abstraction of a dynamic array) and if all writes goes well it
  allocates a buffer and writes the syntax into this one and returns it.

  As a security precaution the file is deleted before writing into it. This
  means that no other processes on the machine can open and read the file
  while this processing is ongoing.

  The code is optimised for minimal code size since it is not used in any
  common queries.
*/

char *generate_partition_syntax(partition_info *part_info, uint *buf_length,
                                bool use_sql_alloc, bool show_partition_options,
                                bool print_expr,
                                const char *current_comment_start) {
  uint i, j, tot_num_parts, num_subparts;
  partition_element *part_elem;
  int err = 0;
  List_iterator<partition_element> part_it(part_info->partitions);
  File fptr;
  char *buf = nullptr;  // Return buffer
  DBUG_TRACE;

  if (!(fptr = mysql_tmpfile("psy"))) {
    return nullptr;
  }
  err += add_space(fptr);
  err += add_partition_by(fptr);
  switch (part_info->part_type) {
    case partition_type::RANGE:
      err += add_part_key_word(fptr, partition_keywords[PKW_RANGE].str);
      break;
    case partition_type::LIST:
      err += add_part_key_word(fptr, partition_keywords[PKW_LIST].str);
      break;
    case partition_type::HASH:
      if (part_info->linear_hash_ind)
        err += add_string(fptr, partition_keywords[PKW_LINEAR].str);
      if (part_info->list_of_part_fields) {
        err += add_key_with_algorithm(fptr, part_info, current_comment_start);
        err += add_part_field_list(fptr, part_info->part_field_list);
      } else
        err += add_part_key_word(fptr, partition_keywords[PKW_HASH].str);
      break;
    default:
      DBUG_ASSERT(0);
      /* We really shouldn't get here, no use in continuing from here */
      my_error(ER_OUT_OF_RESOURCES, MYF(ME_FATALERROR));
      return nullptr;
  }
  if (part_info->part_func_len) {
    err += add_begin_parenthesis(fptr);
    if (print_expr) {
      // Default on-stack buffer which allows to avoid malloc() in most cases.
      char expr_buff[256];
      String tmp(expr_buff, sizeof(expr_buff), system_charset_info);
      tmp.length(0);

      // No point in including schema and table name for identifiers
      // since any columns must be in this table.
      part_info->part_expr->print(
          current_thd, &tmp,
          enum_query_type(QT_TO_SYSTEM_CHARSET | QT_NO_DB | QT_NO_TABLE));
      err += add_string_len(fptr, tmp.ptr(), tmp.length());
    } else {
      err += add_string_len(fptr, part_info->part_func_string,
                            part_info->part_func_len);
    }
    err += add_end_parenthesis(fptr);
  } else if (part_info->column_list) {
    err += add_string(fptr, partition_keywords[PKW_COLUMNS].str);
    err += add_part_field_list(fptr, part_info->part_field_list);
  }
  if ((!part_info->use_default_num_partitions) &&
      part_info->use_default_partitions) {
    err += add_string(fptr, "\n");
    err += add_string(fptr, "PARTITIONS ");
    err += add_int(fptr, part_info->num_parts);
  }
  if (part_info->is_sub_partitioned()) {
    err += add_string(fptr, "\n");
    err += add_subpartition_by(fptr);
    /* Must be hash partitioning for subpartitioning */
    if (part_info->linear_hash_ind)
      err += add_string(fptr, partition_keywords[PKW_LINEAR].str);
    if (part_info->list_of_subpart_fields) {
      err += add_key_with_algorithm(fptr, part_info, current_comment_start);
      err += add_part_field_list(fptr, part_info->subpart_field_list);
    } else
      err += add_part_key_word(fptr, partition_keywords[PKW_HASH].str);
    if (part_info->subpart_func_len) {
      err += add_begin_parenthesis(fptr);
      if (print_expr) {
        // Default on-stack buffer which allows to avoid malloc() in most cases.
        char expr_buff[256];
        String tmp(expr_buff, sizeof(expr_buff), system_charset_info);
        tmp.length(0);

        // No point in including schema and table name for identifiers
        // since any columns must be in this table.
        part_info->subpart_expr->print(
            current_thd, &tmp,
            enum_query_type(QT_TO_SYSTEM_CHARSET | QT_NO_DB | QT_NO_TABLE));
        err += add_string_len(fptr, tmp.ptr(), tmp.length());
      } else {
        err += add_string_len(fptr, part_info->subpart_func_string,
                              part_info->subpart_func_len);
      }
      err += add_end_parenthesis(fptr);
    }
    if ((!part_info->use_default_num_subpartitions) &&
        part_info->use_default_subpartitions) {
      err += add_string(fptr, "\n");
      err += add_string(fptr, "SUBPARTITIONS ");
      err += add_int(fptr, part_info->num_subparts);
    }
  }
  tot_num_parts = part_info->partitions.elements;
  num_subparts = part_info->num_subparts;

  if (!part_info->use_default_partitions) {
    bool first = true;
    err += add_string(fptr, "\n");
    err += add_begin_parenthesis(fptr);
    i = 0;
    do {
      part_elem = part_it++;
      if (part_elem->part_state != PART_TO_BE_DROPPED &&
          part_elem->part_state != PART_REORGED_DROPPED) {
        if (!first) {
          err += add_comma(fptr);
          err += add_string(fptr, "\n");
          err += add_space(fptr);
        }
        first = false;
        err += add_partition(fptr);
        err += add_name_string(fptr, part_elem->partition_name);
        err += add_partition_values(fptr, part_info, part_elem);
        if (!part_info->is_sub_partitioned() ||
            part_info->use_default_subpartitions) {
          if (show_partition_options)
            err += add_partition_options(fptr, part_elem);
        } else {
          err += add_string(fptr, "\n");
          err += add_space(fptr);
          err += add_begin_parenthesis(fptr);
          List_iterator<partition_element> sub_it(part_elem->subpartitions);
          partition_element *sub_elem;
          j = 0;
          do {
            sub_elem = sub_it++;
            err += add_subpartition(fptr);
            err += add_name_string(fptr, sub_elem->partition_name);
            if (show_partition_options)
              err += add_partition_options(fptr, sub_elem);
            if (j != (num_subparts - 1)) {
              err += add_comma(fptr);
              err += add_string(fptr, "\n");
              err += add_space(fptr);
              err += add_space(fptr);
            } else
              err += add_end_parenthesis(fptr);
          } while (++j < num_subparts);
        }
      }
      if (i == (tot_num_parts - 1)) err += add_end_parenthesis(fptr);
    } while (++i < tot_num_parts);
  }
  if (err) goto close_file;
  buf = get_file_content(fptr, buf_length, use_sql_alloc);
close_file:
  if (buf == nullptr) {
    my_error(ER_INTERNAL_ERROR, MYF(0), "Failed to generate partition syntax");
  }
  mysql_file_close(fptr, MYF(0));
  return buf;
}

/*
  Check if partition key fields are modified and if it can be handled by the
  underlying storage engine.

  SYNOPSIS
    partition_key_modified
    table                TABLE object for which partition fields are set-up
    fields               Bitmap representing fields to be modified

  RETURN VALUES
    true                 Need special handling of UPDATE
    false                Normal UPDATE handling is ok
*/

bool partition_key_modified(TABLE *table, const MY_BITMAP *fields) {
  Field **fld;
  partition_info *part_info = table->part_info;
  DBUG_TRACE;

  if (!part_info) return false;
  if (table->s->db_type()->partition_flags &&
      (table->s->db_type()->partition_flags() & HA_CAN_UPDATE_PARTITION_KEY))
    return false;
  for (fld = part_info->full_part_field_array; *fld; fld++)
    if (bitmap_is_set(fields, (*fld)->field_index)) return true;
  return false;
}

/*
  A function to handle correct handling of NULL values in partition
  functions.
  SYNOPSIS
    part_val_int()
    item_expr                 The item expression to evaluate
    out:result                The value of the partition function,
                                LLONG_MIN if any null value in function
  RETURN VALUES
    true      Error in val_int()
    false     ok
*/

static inline int part_val_int(Item *item_expr, longlong *result) {
  *result = item_expr->val_int();
  if (item_expr->null_value) {
    if (current_thd->is_error())
      return true;
    else
      *result = LLONG_MIN;
  }
  return false;
}

/*
  The next set of functions are used to calculate the partition identity.
  A handler sets up a variable that corresponds to one of these functions
  to be able to quickly call it whenever the partition id needs to calculated
  based on the record in table->record[0] (or set up to fake that).
  There are 4 functions for hash partitioning and 2 for RANGE/LIST partitions.
  In addition there are 4 variants for RANGE subpartitioning and 4 variants
  for LIST subpartitioning thus in total there are 14 variants of this
  function.

  We have a set of support functions for these 14 variants. There are 4
  variants of hash functions and there is a function for each. The KEY
  partitioning uses the function calculate_key_hash_value to calculate the hash
  value based on an array of fields. The linear hash variants uses the
  method get_part_id_from_linear_hash to get the partition id using the
  hash value and some parameters calculated from the number of partitions.
*/

/*
  A simple support function to calculate part_id given local part and
  sub part.

  SYNOPSIS
    get_part_id_for_sub()
    loc_part_id             Local partition id
    sub_part_id             Subpartition id
    num_subparts            Number of subparts
*/

inline static uint32 get_part_id_for_sub(uint32 loc_part_id, uint32 sub_part_id,
                                         uint num_subparts) {
  return (uint32)((loc_part_id * num_subparts) + sub_part_id);
}

/*
  Calculate part_id for (SUB)PARTITION BY HASH

  SYNOPSIS
    get_part_id_hash()
    num_parts                Number of hash partitions
    part_expr                Item tree of hash function
    out:part_id              The returned partition id
    out:func_value           Value of hash function

  RETURN VALUE
    != 0                          Error code
    false                         Success
*/

static int get_part_id_hash(uint num_parts, Item *part_expr, uint32 *part_id,
                            longlong *func_value) {
  longlong int_hash_id;
  DBUG_TRACE;

  if (part_val_int(part_expr, func_value)) return HA_ERR_NO_PARTITION_FOUND;

  int_hash_id = *func_value % num_parts;

  *part_id = int_hash_id < 0 ? (uint32)-int_hash_id : (uint32)int_hash_id;
  return false;
}

/*
  Calculate part_id for (SUB)PARTITION BY LINEAR HASH

  SYNOPSIS
    get_part_id_linear_hash()
    part_info           A reference to the partition_info struct where all the
                        desired information is given
    num_parts           Number of hash partitions
    part_expr           Item tree of hash function
    out:part_id         The returned partition id
    out:func_value      Value of hash function

  RETURN VALUE
    != 0     Error code
    0        OK
*/

static int get_part_id_linear_hash(partition_info *part_info, uint num_parts,
                                   Item *part_expr, uint32 *part_id,
                                   longlong *func_value) {
  DBUG_TRACE;

  if (part_val_int(part_expr, func_value)) return HA_ERR_NO_PARTITION_FOUND;

  *part_id = get_part_id_from_linear_hash(
      *func_value, part_info->linear_hash_mask, num_parts);
  return false;
}

/**
  Calculate part_id for (SUB)PARTITION BY KEY

  @param file                Handler to storage engine
  @param field_array         Array of fields for PARTTION KEY
  @param num_parts           Number of KEY partitions
  @param [out] func_value    Returns calculated hash value

  @return Calculated partition id
*/

inline static uint32 get_part_id_key(handler *file, Field **field_array,
                                     uint num_parts, longlong *func_value) {
  DBUG_TRACE;
  *func_value = file->calculate_key_hash_value(field_array);
  return (uint32)(*func_value % num_parts);
}

/*
  Calculate part_id for (SUB)PARTITION BY LINEAR KEY

  SYNOPSIS
    get_part_id_linear_key()
    part_info           A reference to the partition_info struct where all the
                        desired information is given
    field_array         Array of fields for PARTTION KEY
    num_parts            Number of KEY partitions

  RETURN VALUE
    Calculated partition id
*/

inline static uint32 get_part_id_linear_key(partition_info *part_info,
                                            Field **field_array, uint num_parts,
                                            longlong *func_value) {
  DBUG_TRACE;

  *func_value = part_info->table->file->calculate_key_hash_value(field_array);
  return get_part_id_from_linear_hash(*func_value, part_info->linear_hash_mask,
                                      num_parts);
}

/*
  Copy to field buffers and set up field pointers

  SYNOPSIS
    copy_to_part_field_buffers()
    ptr                          Array of fields to copy
    field_bufs                   Array of field buffers to copy to
    restore_ptr                  Array of pointers to restore to

  RETURN VALUES
    NONE
  DESCRIPTION
    This routine is used to take the data from field pointer, convert
    it to a standard format and store this format in a field buffer
    allocated for this purpose. Next the field pointers are moved to
    point to the field buffers. There is a separate to restore the
    field pointers after this call.
*/

static void copy_to_part_field_buffers(Field **ptr, uchar **field_bufs,
                                       uchar **restore_ptr) {
  Field *field;
  while ((field = *(ptr++))) {
    *restore_ptr = field->ptr;
    restore_ptr++;
    if (!field->is_null()) {
      const CHARSET_INFO *cs = field->charset();
      uint max_len = field->pack_length();
      uint data_len = field->data_length();
      uchar *field_buf = *field_bufs;
      /*
         We only use the field buffer for VARCHAR and CHAR strings
         which isn't of a binary collation. We also only use the
         field buffer for fields which are not currently NULL.
         The field buffer will store a normalised string. We use
         the strnxfrm method to normalise the string.
       */
      if (field->type() == MYSQL_TYPE_VARCHAR) {
        uint len_bytes = ((Field_varstring *)field)->length_bytes;
        my_strnxfrm(cs, field_buf + len_bytes, max_len, field->ptr + len_bytes,
                    data_len);
        if (len_bytes == 1)
          *field_buf = (uchar)data_len;
        else
          int2store(field_buf, data_len);
      } else {
        my_strnxfrm(cs, field_buf, max_len, field->ptr, max_len);
      }
      field->ptr = field_buf;
    }
    field_bufs++;
  }
  return;
}

/*
  Restore field pointers
  SYNOPSIS
    restore_part_field_pointers()
    ptr                            Array of fields to restore
    restore_ptr                    Array of field pointers to restore to

  RETURN VALUES
*/

static void restore_part_field_pointers(Field **ptr, uchar **restore_ptr) {
  Field *field;
  while ((field = *(ptr++))) {
    field->ptr = *restore_ptr;
    restore_ptr++;
  }
  return;
}

/*
  This function is used to calculate the partition id where all partition
  fields have been prepared to point to a record where the partition field
  values are bound.

  SYNOPSIS
    get_partition_id()
    part_info           A reference to the partition_info struct where all the
                        desired information is given
    out:part_id         The partition id is returned through this pointer
    out:func_value      Value of partition function (longlong)

  RETURN VALUE
    part_id                     Partition id of partition that would contain
                                row with given values of PF-fields
    HA_ERR_NO_PARTITION_FOUND   The fields of the partition function didn't
                                fit into any partition and thus the values of
                                the PF-fields are not allowed.

  DESCRIPTION
    A routine used from write_row, update_row and delete_row from any
    handler supporting partitioning. It is also a support routine for
    get_partition_set used to find the set of partitions needed to scan
    for a certain index scan or full table scan.

    It is actually 9 different variants of this function which are called
    through a function pointer.

    get_partition_id_list
    get_partition_id_list_col
    get_partition_id_range
    get_partition_id_range_col
    get_partition_id_hash_nosub
    get_partition_id_key_nosub
    get_partition_id_linear_hash_nosub
    get_partition_id_linear_key_nosub
    get_partition_id_with_sub
*/

/*
  This function is used to calculate the main partition to use in the case of
  subpartitioning and we don't know enough to get the partition identity in
  total.

  SYNOPSIS
    get_part_partition_id()
    part_info           A reference to the partition_info struct where all the
                        desired information is given
    out:part_id         The partition id is returned through this pointer
    out:func_value      The value calculated by partition function

  RETURN VALUE
    HA_ERR_NO_PARTITION_FOUND   The fields of the partition function didn't
                                fit into any partition and thus the values of
                                the PF-fields are not allowed.
    0                           OK

  DESCRIPTION

    It is actually 8 different variants of this function which are called
    through a function pointer.

    get_partition_id_list
    get_partition_id_list_col
    get_partition_id_range
    get_partition_id_range_col
    get_partition_id_hash_nosub
    get_partition_id_key_nosub
    get_partition_id_linear_hash_nosub
    get_partition_id_linear_key_nosub
*/

static int get_part_id_charset_func_part(partition_info *part_info,
                                         uint32 *part_id,
                                         longlong *func_value) {
  int res;
  DBUG_TRACE;

  copy_to_part_field_buffers(part_info->part_charset_field_array,
                             part_info->part_field_buffers,
                             part_info->restore_part_field_ptrs);
  res =
      part_info->get_part_partition_id_charset(part_info, part_id, func_value);
  restore_part_field_pointers(part_info->part_charset_field_array,
                              part_info->restore_part_field_ptrs);
  return res;
}

static int get_part_id_charset_func_subpart(partition_info *part_info,
                                            uint32 *part_id) {
  int res;
  DBUG_TRACE;

  copy_to_part_field_buffers(part_info->subpart_charset_field_array,
                             part_info->subpart_field_buffers,
                             part_info->restore_subpart_field_ptrs);
  res = part_info->get_subpartition_id_charset(part_info, part_id);
  restore_part_field_pointers(part_info->subpart_charset_field_array,
                              part_info->restore_subpart_field_ptrs);
  return res;
}

static int get_partition_id_list_col(partition_info *part_info, uint32 *part_id,
                                     longlong *) {
  part_column_list_val *list_col_array = part_info->list_col_array;
  uint num_columns = part_info->part_field_list.elements;
  int list_index, cmp;
  int min_list_index = 0;
  int max_list_index = part_info->num_list_values - 1;
  DBUG_TRACE;

  while (max_list_index >= min_list_index) {
    list_index = (max_list_index + min_list_index) >> 1;
    cmp = cmp_rec_and_tuple(list_col_array + list_index * num_columns,
                            num_columns);
    if (cmp > 0)
      min_list_index = list_index + 1;
    else if (cmp < 0) {
      if (!list_index) goto notfound;
      max_list_index = list_index - 1;
    } else {
      *part_id = (uint32)list_col_array[list_index * num_columns].partition_id;
      return 0;
    }
  }
notfound:
  *part_id = 0;
  return HA_ERR_NO_PARTITION_FOUND;
}

static int get_partition_id_list(partition_info *part_info, uint32 *part_id,
                                 longlong *func_value) {
  LIST_PART_ENTRY *list_array = part_info->list_array;
  int list_index;
  int min_list_index = 0;
  int max_list_index = part_info->num_list_values - 1;
  longlong part_func_value;
  int error = part_val_int(part_info->part_expr, &part_func_value);
  longlong list_value;
  bool unsigned_flag = part_info->part_expr->unsigned_flag;
  DBUG_TRACE;

  if (error) goto notfound;

  if (part_info->part_expr->null_value) {
    if (part_info->has_null_value) {
      *part_id = part_info->has_null_part_id;
      return 0;
    }
    goto notfound;
  }
  *func_value = part_func_value;
  if (unsigned_flag) part_func_value -= 0x8000000000000000ULL;
  while (max_list_index >= min_list_index) {
    list_index = (max_list_index + min_list_index) >> 1;
    list_value = list_array[list_index].list_value;
    if (list_value < part_func_value)
      min_list_index = list_index + 1;
    else if (list_value > part_func_value) {
      if (!list_index) goto notfound;
      max_list_index = list_index - 1;
    } else {
      *part_id = (uint32)list_array[list_index].partition_id;
      return 0;
    }
  }
notfound:
  *part_id = 0;
  return HA_ERR_NO_PARTITION_FOUND;
}

static uint32 get_partition_id_cols_list_for_endpoint(partition_info *part_info,
                                                      bool left_endpoint,
                                                      bool include_endpoint,
                                                      uint32 nparts) {
  part_column_list_val *list_col_array = part_info->list_col_array;
  uint num_columns = part_info->part_field_list.elements;
  uint list_index;
  uint min_list_index = 0;
  int cmp;
  /* Notice that max_list_index = last_index + 1 here! */
  uint max_list_index = part_info->num_list_values;
  DBUG_TRACE;

  /* Find the matching partition (including taking endpoint into account). */
  do {
    /* Midpoint, adjusted down, so it can never be >= max_list_index. */
    list_index = (max_list_index + min_list_index) >> 1;
    cmp = cmp_rec_and_tuple_prune(list_col_array + list_index * num_columns,
                                  nparts, left_endpoint, include_endpoint);
    if (cmp > 0) {
      min_list_index = list_index + 1;
    } else {
      max_list_index = list_index;
      if (cmp == 0) break;
    }
  } while (max_list_index > min_list_index);
  list_index = max_list_index;

  /* Given value must be LESS THAN or EQUAL to the found partition. */
  DBUG_ASSERT(
      list_index == part_info->num_list_values ||
      (0 >= cmp_rec_and_tuple_prune(list_col_array + list_index * num_columns,
                                    nparts, left_endpoint, include_endpoint)));
  /* Given value must be GREATER THAN the previous partition. */
  DBUG_ASSERT(list_index == 0 ||
              (0 < cmp_rec_and_tuple_prune(
                       list_col_array + (list_index - 1) * num_columns, nparts,
                       left_endpoint, include_endpoint)));

  /* Include the right endpoint if not already passed end of array. */
  if (!left_endpoint && include_endpoint && cmp == 0 &&
      list_index < part_info->num_list_values)
    list_index++;

  return list_index;
}

/**
  Find the sub-array part_info->list_array that corresponds to given interval.

  @param part_info         Partitioning info (partitioning type must be LIST)
  @param left_endpoint     true  - the interval is [a; +inf) or (a; +inf)
                           false - the interval is (-inf; a] or (-inf; a)
  @param include_endpoint  true iff the interval includes the endpoint

  This function finds the sub-array of part_info->list_array where values of
  list_array[idx].list_value are contained within the specifed interval.
  list_array is ordered by list_value, so
  1. For [a; +inf) or (a; +inf)-type intervals (left_endpoint==true), the
     sought sub-array starts at some index idx and continues till array end.
     The function returns first number idx, such that
     list_array[idx].list_value is contained within the passed interval.

  2. For (-inf; a] or (-inf; a)-type intervals (left_endpoint==false), the
     sought sub-array starts at array start and continues till some last
     index idx.
     The function returns first number idx, such that
     list_array[idx].list_value is NOT contained within the passed interval.
     If all array elements are contained, part_info->num_list_values is
     returned.

  @note The caller will call this function and then will run along the
  sub-array of list_array to collect partition ids. If the number of list
  values is significantly higher then number of partitions, this could be slow
  and we could invent some other approach. The "run over list array" part is
  already wrapped in a get_next()-like function.

  @return The index of corresponding sub-array of part_info->list_array.
*/

static uint32 get_list_array_idx_for_endpoint_charset(partition_info *part_info,
                                                      bool left_endpoint,
                                                      bool include_endpoint) {
  uint32 res;
  copy_to_part_field_buffers(part_info->part_field_array,
                             part_info->part_field_buffers,
                             part_info->restore_part_field_ptrs);
  res = get_list_array_idx_for_endpoint(part_info, left_endpoint,
                                        include_endpoint);
  restore_part_field_pointers(part_info->part_field_array,
                              part_info->restore_part_field_ptrs);
  return res;
}

static uint32 get_list_array_idx_for_endpoint(partition_info *part_info,
                                              bool left_endpoint,
                                              bool include_endpoint) {
  LIST_PART_ENTRY *list_array = part_info->list_array;
  uint list_index;
  uint min_list_index = 0, max_list_index = part_info->num_list_values - 1;
  longlong list_value;
  /* Get the partitioning function value for the endpoint */
  longlong part_func_value =
      part_info->part_expr->val_int_endpoint(left_endpoint, &include_endpoint);
  bool unsigned_flag = part_info->part_expr->unsigned_flag;
  DBUG_TRACE;

  if (part_info->part_expr->null_value) {
    /*
      Special handling for MONOTONIC functions that can return NULL for
      values that are comparable. I.e.
      '2000-00-00' can be compared to '2000-01-01' but TO_DAYS('2000-00-00')
      returns NULL which cannot be compared used <, >, <=, >= etc.

      Otherwise, just return the the first index (lowest value).
    */
    enum_monotonicity_info monotonic;
    monotonic = part_info->part_expr->get_monotonicity_info();
    if (monotonic != MONOTONIC_INCREASING_NOT_NULL &&
        monotonic != MONOTONIC_STRICT_INCREASING_NOT_NULL) {
      /* F(col) can not return NULL, return index with lowest value */
      return 0;
    }
  }

  if (unsigned_flag) part_func_value -= 0x8000000000000000ULL;
  DBUG_ASSERT(part_info->num_list_values);
  do {
    list_index = (max_list_index + min_list_index) >> 1;
    list_value = list_array[list_index].list_value;
    if (list_value < part_func_value)
      min_list_index = list_index + 1;
    else if (list_value > part_func_value) {
      if (!list_index) goto notfound;
      max_list_index = list_index - 1;
    } else {
      return list_index + ((left_endpoint ^ include_endpoint) ? 1 : 0);
    }
  } while (max_list_index >= min_list_index);
notfound:
  if (list_value < part_func_value) list_index++;
  return list_index;
}

static int get_partition_id_range_col(partition_info *part_info,
                                      uint32 *part_id, longlong *) {
  part_column_list_val *range_col_array = part_info->range_col_array;
  uint num_columns = part_info->part_field_list.elements;
  uint max_partition = part_info->num_parts - 1;
  uint min_part_id = 0;
  uint max_part_id = max_partition;
  uint loc_part_id;
  DBUG_TRACE;

  while (max_part_id > min_part_id) {
    loc_part_id = (max_part_id + min_part_id + 1) >> 1;
    if (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                          num_columns) >= 0)
      min_part_id = loc_part_id + 1;
    else
      max_part_id = loc_part_id - 1;
  }
  loc_part_id = max_part_id;
  if (loc_part_id != max_partition)
    if (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                          num_columns) >= 0)
      loc_part_id++;
  *part_id = (uint32)loc_part_id;
  if (loc_part_id == max_partition &&
      (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                         num_columns) >= 0))
    return HA_ERR_NO_PARTITION_FOUND;

  DBUG_PRINT("exit", ("partition: %d", *part_id));
  return 0;
}

int get_partition_id_range(partition_info *part_info, uint32 *part_id,
                           longlong *func_value) {
  longlong *range_array = part_info->range_int_array;
  uint max_partition = part_info->num_parts - 1;
  uint min_part_id = 0;
  uint max_part_id = max_partition;
  uint loc_part_id;
  longlong part_func_value;
  int error = part_val_int(part_info->part_expr, &part_func_value);
  bool unsigned_flag = part_info->part_expr->unsigned_flag;
  DBUG_TRACE;

  if (error) return HA_ERR_NO_PARTITION_FOUND;

  if (part_info->part_expr->null_value) {
    *part_id = 0;
    return 0;
  }
  *func_value = part_func_value;
  if (unsigned_flag) part_func_value -= 0x8000000000000000ULL;
  /* Search for the partition containing part_func_value */
  while (max_part_id > min_part_id) {
    loc_part_id = (max_part_id + min_part_id) / 2;
    if (range_array[loc_part_id] <= part_func_value)
      min_part_id = loc_part_id + 1;
    else
      max_part_id = loc_part_id;
  }
  loc_part_id = max_part_id;
  *part_id = (uint32)loc_part_id;
  if (loc_part_id == max_partition &&
      part_func_value >= range_array[loc_part_id] &&
      !part_info->defined_max_value)
    return HA_ERR_NO_PARTITION_FOUND;

  DBUG_PRINT("exit", ("partition: %d", *part_id));
  return 0;
}

/*
  Find the sub-array of part_info->range_int_array that covers given interval

  SYNOPSIS
    get_partition_id_range_for_endpoint()
      part_info         Partitioning info (partitioning type must be RANGE)
      left_endpoint     true  - the interval is [a; +inf) or (a; +inf)
                        false - the interval is (-inf; a] or (-inf; a).
      include_endpoint  true <=> the endpoint itself is included in the
                        interval

  DESCRIPTION
    This function finds the sub-array of part_info->range_int_array where the
    elements have non-empty intersections with the given interval.

    A range_int_array element at index idx represents the interval

      [range_int_array[idx-1], range_int_array[idx]),

    intervals are disjoint and ordered by their right bound, so

    1. For [a; +inf) or (a; +inf)-type intervals (left_endpoint==true), the
       sought sub-array starts at some index idx and continues till array end.
       The function returns first number idx, such that the interval
       represented by range_int_array[idx] has non empty intersection with
       the passed interval.

    2. For (-inf; a] or (-inf; a)-type intervals (left_endpoint==false), the
       sought sub-array starts at array start and continues till some last
       index idx.
       The function returns first number idx, such that the interval
       represented by range_int_array[idx] has EMPTY intersection with the
       passed interval.
       If the interval represented by the last array element has non-empty
       intersection with the passed interval, part_info->num_parts is
       returned.

  RETURN
    The edge of corresponding part_info->range_int_array sub-array.
*/

static uint32 get_partition_id_range_for_endpoint_charset(
    partition_info *part_info, bool left_endpoint, bool include_endpoint) {
  uint32 res;
  copy_to_part_field_buffers(part_info->part_field_array,
                             part_info->part_field_buffers,
                             part_info->restore_part_field_ptrs);
  res = get_partition_id_range_for_endpoint(part_info, left_endpoint,
                                            include_endpoint);
  restore_part_field_pointers(part_info->part_field_array,
                              part_info->restore_part_field_ptrs);
  return res;
}

static uint32 get_partition_id_range_for_endpoint(partition_info *part_info,
                                                  bool left_endpoint,
                                                  bool include_endpoint) {
  longlong *range_array = part_info->range_int_array;
  longlong part_end_val;
  uint max_partition = part_info->num_parts - 1;
  uint min_part_id = 0, max_part_id = max_partition, loc_part_id;
  /* Get the partitioning function value for the endpoint */
  longlong part_func_value =
      part_info->part_expr->val_int_endpoint(left_endpoint, &include_endpoint);

  bool unsigned_flag = part_info->part_expr->unsigned_flag;
  DBUG_TRACE;

  if (part_info->part_expr->null_value) {
    /*
      Special handling for MONOTONIC functions that can return NULL for
      values that are comparable. I.e.
      '2000-00-00' can be compared to '2000-01-01' but TO_DAYS('2000-00-00')
      returns NULL which cannot be compared used <, >, <=, >= etc.

      Otherwise, just return the first partition
      (may be included if not left endpoint)
    */
    enum_monotonicity_info monotonic;
    monotonic = part_info->part_expr->get_monotonicity_info();
    if (monotonic != MONOTONIC_INCREASING_NOT_NULL &&
        monotonic != MONOTONIC_STRICT_INCREASING_NOT_NULL) {
      /* F(col) can not return NULL, return partition with lowest value */
      if (!left_endpoint && include_endpoint) return 1;
      return 0;
    }
  }

  if (unsigned_flag) part_func_value -= 0x8000000000000000ULL;
  if (left_endpoint && !include_endpoint) part_func_value++;

  /*
    Search for the partition containing part_func_value
    (including the right endpoint).
  */
  while (max_part_id > min_part_id) {
    loc_part_id = (max_part_id + min_part_id) / 2;
    if (range_array[loc_part_id] < part_func_value)
      min_part_id = loc_part_id + 1;
    else
      max_part_id = loc_part_id;
  }
  loc_part_id = max_part_id;

  /* Adjust for endpoints */
  part_end_val = range_array[loc_part_id];
  if (left_endpoint) {
    DBUG_ASSERT(
        part_func_value > part_end_val
            ? (loc_part_id == max_partition && !part_info->defined_max_value)
            : 1);
    /*
      In case of PARTITION p VALUES LESS THAN MAXVALUE
      the maximum value is in the current (last) partition.
      If value is equal or greater than the endpoint,
      the range starts from the next partition.
    */
    if (part_func_value >= part_end_val &&
        (loc_part_id < max_partition || !part_info->defined_max_value))
      loc_part_id++;
  } else {
    /* if 'WHERE <= X' and partition is LESS THAN (X) include next partition */
    if (include_endpoint && loc_part_id < max_partition &&
        part_func_value == part_end_val)
      loc_part_id++;

    /* Right endpoint, set end after correct partition */
    loc_part_id++;
  }
  return loc_part_id;
}

static int get_partition_id_hash_nosub(partition_info *part_info,
                                       uint32 *part_id, longlong *func_value) {
  return get_part_id_hash(part_info->num_parts, part_info->part_expr, part_id,
                          func_value);
}

static int get_partition_id_linear_hash_nosub(partition_info *part_info,
                                              uint32 *part_id,
                                              longlong *func_value) {
  return get_part_id_linear_hash(part_info, part_info->num_parts,
                                 part_info->part_expr, part_id, func_value);
}

static int get_partition_id_key_nosub(partition_info *part_info,
                                      uint32 *part_id, longlong *func_value) {
  *part_id =
      get_part_id_key(part_info->table->file, part_info->part_field_array,
                      part_info->num_parts, func_value);
  return 0;
}

static int get_partition_id_linear_key_nosub(partition_info *part_info,
                                             uint32 *part_id,
                                             longlong *func_value) {
  *part_id = get_part_id_linear_key(part_info, part_info->part_field_array,
                                    part_info->num_parts, func_value);
  return 0;
}

static int get_partition_id_with_sub(partition_info *part_info, uint32 *part_id,
                                     longlong *func_value) {
  uint32 loc_part_id, sub_part_id;
  uint num_subparts;
  int error;
  DBUG_TRACE;

  if (unlikely((error = part_info->get_part_partition_id(
                    part_info, &loc_part_id, func_value)))) {
    return error;
  }
  num_subparts = part_info->num_subparts;
  if (unlikely(
          (error = part_info->get_subpartition_id(part_info, &sub_part_id)))) {
    return error;
  }
  *part_id = get_part_id_for_sub(loc_part_id, sub_part_id, num_subparts);
  return 0;
}

/*
  This function is used to calculate the subpartition id

  SYNOPSIS
    get_subpartition_id()
    part_info           A reference to the partition_info struct where all the
                        desired information is given

  RETURN VALUE
    part_id             The subpartition identity

  DESCRIPTION
    A routine used in some SELECT's when only partial knowledge of the
    partitions is known.

    It is actually 4 different variants of this function which are called
    through a function pointer.

    get_partition_id_hash_sub
    get_partition_id_key_sub
    get_partition_id_linear_hash_sub
    get_partition_id_linear_key_sub
*/

static int get_partition_id_hash_sub(partition_info *part_info,
                                     uint32 *part_id) {
  longlong func_value;
  return get_part_id_hash(part_info->num_subparts, part_info->subpart_expr,
                          part_id, &func_value);
}

static int get_partition_id_linear_hash_sub(partition_info *part_info,
                                            uint32 *part_id) {
  longlong func_value;
  return get_part_id_linear_hash(part_info, part_info->num_subparts,
                                 part_info->subpart_expr, part_id, &func_value);
}

static int get_partition_id_key_sub(partition_info *part_info,
                                    uint32 *part_id) {
  longlong func_value;
  *part_id =
      get_part_id_key(part_info->table->file, part_info->subpart_field_array,
                      part_info->num_subparts, &func_value);
  return false;
}

static int get_partition_id_linear_key_sub(partition_info *part_info,
                                           uint32 *part_id) {
  longlong func_value;
  *part_id = get_part_id_linear_key(part_info, part_info->subpart_field_array,
                                    part_info->num_subparts, &func_value);
  return false;
}

/*
  Set an indicator on all partition fields that are set by the key

  SYNOPSIS
    set_PF_fields_in_key()
    key_info                   Information about the index
    key_length                 Length of key

  RETURN VALUE
    true                       Found partition field set by key
    false                      No partition field set by key
*/

static bool set_PF_fields_in_key(KEY *key_info, uint key_length) {
  KEY_PART_INFO *key_part;
  bool found_part_field = false;
  DBUG_TRACE;

  for (key_part = key_info->key_part; (int)key_length > 0; key_part++) {
    if (key_part->null_bit) key_length--;
    if (key_part->type == HA_KEYTYPE_BIT) {
      if (((Field_bit *)key_part->field)->bit_len) key_length--;
    }
    if (key_part->key_part_flag & (HA_BLOB_PART + HA_VAR_LENGTH_PART)) {
      key_length -= HA_KEY_BLOB_LENGTH;
    }
    if (key_length < key_part->length) break;
    key_length -= key_part->length;
    if (key_part->field->flags & FIELD_IN_PART_FUNC_FLAG) {
      found_part_field = true;
      key_part->field->flags |= GET_FIXED_FIELDS_FLAG;
    }
  }
  return found_part_field;
}

/*
  We have found that at least one partition field was set by a key, now
  check if a partition function has all its fields bound or not.

  SYNOPSIS
    check_part_func_bound()
    ptr                     Array of fields NULL terminated (partition fields)

  RETURN VALUE
    true                    All fields in partition function are set
    false                   Not all fields in partition function are set
*/

static bool check_part_func_bound(Field **ptr) {
  bool result = true;
  DBUG_TRACE;

  for (; *ptr; ptr++) {
    if (!((*ptr)->flags & GET_FIXED_FIELDS_FLAG)) {
      result = false;
      break;
    }
  }
  return result;
}

/*
  Get the id of the subpartitioning part by using the key buffer of the
  index scan.

  SYNOPSIS
    get_sub_part_id_from_key()
    table         The table object
    buf           A buffer that can be used to evaluate the partition function
    key_info      The index object
    key_spec      A key_range containing key and key length
    out:part_id   The returned partition id

  RETURN VALUES
    true                    All fields in partition function are set
    false                   Not all fields in partition function are set

  DESCRIPTION
    Use key buffer to set-up record in buf, move field pointers and
    get the partition identity and restore field pointers afterwards.
*/

static int get_sub_part_id_from_key(const TABLE *table, uchar *buf,
                                    KEY *key_info, const key_range *key_spec,
                                    uint32 *part_id) {
  uchar *rec0 = table->record[0];
  partition_info *part_info = table->part_info;
  int res;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    res = part_info->get_subpartition_id(part_info, part_id);
  } else {
    Field **part_field_array = part_info->subpart_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    res = part_info->get_subpartition_id(part_info, part_id);
    set_field_ptr(part_field_array, rec0, buf);
  }
  return res;
}

/*
  Get the id of the partitioning part by using the key buffer of the
  index scan.

  SYNOPSIS
    get_part_id_from_key()
    table         The table object
    buf           A buffer that can be used to evaluate the partition function
    key_info      The index object
    key_spec      A key_range containing key and key length
    out:part_id   Partition to use

  RETURN VALUES
    true          Partition to use not found
    false         Ok, part_id indicates partition to use

  DESCRIPTION
    Use key buffer to set-up record in buf, move field pointers and
    get the partition identity and restore field pointers afterwards.
*/

static bool get_part_id_from_key(const TABLE *table, uchar *buf, KEY *key_info,
                                 const key_range *key_spec, uint32 *part_id) {
  bool result;
  uchar *rec0 = table->record[0];
  partition_info *part_info = table->part_info;
  longlong func_value;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    result = part_info->get_part_partition_id(part_info, part_id, &func_value);
  } else {
    Field **part_field_array = part_info->part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    result = part_info->get_part_partition_id(part_info, part_id, &func_value);
    set_field_ptr(part_field_array, rec0, buf);
  }
  return result;
}

/*
  Get the partitioning id of the full PF by using the key buffer of the
  index scan.

  SYNOPSIS
    get_full_part_id_from_key()
    table         The table object
    buf           A buffer that is used to evaluate the partition function
    key_info      The index object
    key_spec      A key_range containing key and key length
    out:part_spec A partition id containing start part and end part

  RETURN VALUES
    part_spec
    No partitions to scan is indicated by end_part > start_part when returning

  DESCRIPTION
    Use key buffer to set-up record in buf, move field pointers if needed and
    get the partition identity and restore field pointers afterwards.
*/

void get_full_part_id_from_key(const TABLE *table, uchar *buf, KEY *key_info,
                               const key_range *key_spec,
                               part_id_range *part_spec) {
  bool result;
  partition_info *part_info = table->part_info;
  uchar *rec0 = table->record[0];
  longlong func_value;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    result = part_info->get_partition_id(part_info, &part_spec->start_part,
                                         &func_value);
  } else {
    Field **part_field_array = part_info->full_part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    result = part_info->get_partition_id(part_info, &part_spec->start_part,
                                         &func_value);
    set_field_ptr(part_field_array, rec0, buf);
  }
  part_spec->end_part = part_spec->start_part;
  if (unlikely(result)) part_spec->start_part++;
}

/**
  @brief Verify that all rows in a table is in the given partition

  @param table      Table which contains the data that will be checked if
                    it is matching the partition definition.
  @param part_table Partitioned table containing the partition to check.
  @param part_id    Which partition to match with.

  @return Operation status
    @retval true                Not all rows match the given partition
    @retval false               OK
*/
bool verify_data_with_partition(TABLE *table, TABLE *part_table,
                                uint32 part_id) {
  uint32 found_part_id;
  longlong func_value; /* Unused */
  handler *file;
  int error;
  uchar *old_rec;
  partition_info *part_info;
  DBUG_TRACE;
  DBUG_ASSERT(table && table->file && part_table && part_table->part_info &&
              part_table->file);

  /*
    Verify all table rows.
    First implementation uses full scan + evaluates partition functions for
    every row. TODO: add optimization to use index if possible, see WL#5397.

    1) Open both tables (already done) and set the row buffers to use
       the same buffer (to avoid copy).
    2) Init rnd on table.
    3) loop over all rows.
      3.1) verify that partition_id on the row is correct. Break if error.
  */
  file = table->file;
  part_info = part_table->part_info;
  bitmap_union(table->read_set, &part_info->full_part_field_set);
  old_rec = part_table->record[0];
  part_table->record[0] = table->record[0];
  set_field_ptr(part_info->full_part_field_array, table->record[0], old_rec);
  if ((error = file->ha_rnd_init(true))) {
    file->print_error(error, MYF(0));
    goto err;
  }

  do {
    if ((error = file->ha_rnd_next(table->record[0]))) {
      if (error == HA_ERR_RECORD_DELETED) continue;
      if (error == HA_ERR_END_OF_FILE)
        error = 0;
      else
        file->print_error(error, MYF(0));
      break;
    }
    if ((error = part_info->get_partition_id(part_info, &found_part_id,
                                             &func_value))) {
      part_info->err_value = func_value;
      part_table->file->print_error(error, MYF(0));
      break;
    }
    DEBUG_SYNC(current_thd, "swap_partition_first_row_read");
    if (found_part_id != part_id) {
      my_error(ER_ROW_DOES_NOT_MATCH_PARTITION, MYF(0));
      error = 1;
      break;
    }
  } while (true);
  (void)file->ha_rnd_end();
err:
  set_field_ptr(part_info->full_part_field_array, old_rec, table->record[0]);
  part_table->record[0] = old_rec;
  if (error) return true;
  return false;
}

/*
  Prune the set of partitions to use in query

  SYNOPSIS
    prune_partition_set()
    table         The table object
    out:part_spec Contains start part, end part

  DESCRIPTION
    This function is called to prune the range of partitions to scan by
    checking the read_partitions bitmap.
    If start_part > end_part at return it means no partition needs to be
    scanned. If start_part == end_part it always means a single partition
    needs to be scanned.

  RETURN VALUE
    part_spec
*/
void prune_partition_set(const TABLE *table, part_id_range *part_spec) {
  int last_partition = -1;
  uint i = part_spec->start_part;
  partition_info *part_info = table->part_info;
  DBUG_TRACE;

  if (i)
    i = bitmap_get_next_set(&part_info->read_partitions, i - 1);
  else
    i = bitmap_get_first_set(&part_info->read_partitions);

  part_spec->start_part = i;

  /* TODO: Only check next bit, no need to prune end if >= 2 partitions. */
  for (; i <= part_spec->end_part;
       i = bitmap_get_next_set(&part_info->read_partitions, i)) {
    DBUG_PRINT("info", ("Partition %d is set", i));
    if (last_partition == -1)
      /* First partition found in set and pruned bitmap */
      part_spec->start_part = i;
    last_partition = i;
  }
  if (last_partition == -1) /* No partition found in pruned bitmap */
    part_spec->start_part = part_spec->end_part + 1;
  else  // if (last_partition != -1)
    part_spec->end_part = last_partition;
}

/*
  Get the set of partitions to use in query.

  SYNOPSIS
    get_partition_set()
    table         The table object
    buf           A buffer that can be used to evaluate the partition function
    index         The index of the key used, if MAX_KEY no index used
    key_spec      A key_range containing key and key length
    out:part_spec Contains start part, end part and indicator if bitmap is
                  used for which partitions to scan

  DESCRIPTION
    This function is called to discover which partitions to use in an index
    scan or a full table scan.
    It returns a range of partitions to scan. If there are holes in this
    range with partitions that are not needed to scan a bit array is used
    to signal which partitions to use and which not to use.
    If start_part > end_part at return it means no partition needs to be
    scanned. If start_part == end_part it always means a single partition
    needs to be scanned.

  RETURN VALUE
    part_spec
*/
void get_partition_set(const TABLE *table, uchar *buf, const uint index,
                       const key_range *key_spec, part_id_range *part_spec) {
  partition_info *part_info = table->part_info;
  uint num_parts = part_info->get_tot_partitions();
  uint i, part_id;
  uint sub_part = num_parts;
  uint32 part_part = num_parts;
  KEY *key_info = nullptr;
  bool found_part_field = false;
  DBUG_TRACE;

  part_spec->start_part = 0;
  part_spec->end_part = num_parts - 1;
  if ((index < MAX_KEY) && key_spec &&
      key_spec->flag == (uint)HA_READ_KEY_EXACT &&
      part_info->some_fields_in_PF.is_set(index)) {
    key_info = table->key_info + index;
    /*
      The index can potentially provide at least one PF-field (field in the
      partition function). Thus it is interesting to continue our probe.
    */
    if (key_spec->length == key_info->key_length) {
      /*
        The entire key is set so we can check whether we can immediately
        derive either the complete PF or if we can derive either
        the top PF or the subpartitioning PF. This can be established by
        checking precalculated bits on each index.
      */
      if (part_info->all_fields_in_PF.is_set(index)) {
        /*
          We can derive the exact partition to use, no more than this one
          is needed.
        */
        get_full_part_id_from_key(table, buf, key_info, key_spec, part_spec);
        /*
          Check if range can be adjusted by looking in read_partitions
        */
        prune_partition_set(table, part_spec);
        return;
      } else if (part_info->is_sub_partitioned()) {
        if (part_info->all_fields_in_SPF.is_set(index)) {
          if (get_sub_part_id_from_key(table, buf, key_info, key_spec,
                                       &sub_part)) {
            part_spec->start_part = num_parts;
            return;
          }
        } else if (part_info->all_fields_in_PPF.is_set(index)) {
          if (get_part_id_from_key(table, buf, key_info, key_spec,
                                   &part_part)) {
            /*
              The value of the RANGE or LIST partitioning was outside of
              allowed values. Thus it is certain that the result of this
              scan will be empty.
            */
            part_spec->start_part = num_parts;
            return;
          }
        }
      }
    } else {
      /*
        Set an indicator on all partition fields that are bound.
        If at least one PF-field was bound it pays off to check whether
        the PF or PPF or SPF has been bound.
        (PF = Partition Function, SPF = Subpartition Function and
         PPF = Partition Function part of subpartitioning)
      */
      if ((found_part_field =
               set_PF_fields_in_key(key_info, key_spec->length))) {
        if (check_part_func_bound(part_info->full_part_field_array)) {
          /*
            We were able to bind all fields in the partition function even
            by using only a part of the key. Calculate the partition to use.
          */
          get_full_part_id_from_key(table, buf, key_info, key_spec, part_spec);
          clear_indicator_in_key_fields(key_info);
          /*
            Check if range can be adjusted by looking in read_partitions
          */
          prune_partition_set(table, part_spec);
          return;
        } else if (part_info->is_sub_partitioned()) {
          if (check_part_func_bound(part_info->subpart_field_array)) {
            if (get_sub_part_id_from_key(table, buf, key_info, key_spec,
                                         &sub_part)) {
              part_spec->start_part = num_parts;
              clear_indicator_in_key_fields(key_info);
              return;
            }
          } else if (check_part_func_bound(part_info->part_field_array)) {
            if (get_part_id_from_key(table, buf, key_info, key_spec,
                                     &part_part)) {
              part_spec->start_part = num_parts;
              clear_indicator_in_key_fields(key_info);
              return;
            }
          }
        }
      }
    }
  }
  {
    /*
      The next step is to analyse the table condition to see whether any
      information about which partitions to scan can be derived from there.
      Currently not implemented.
    */
  }
  /*
    If we come here we have found a range of sorts we have either discovered
    nothing or we have discovered a range of partitions with possible holes
    in it. We need a bitvector to further the work here.
  */
  if (!(part_part == num_parts && sub_part == num_parts)) {
    /*
      We can only arrive here if we are using subpartitioning.
    */
    if (part_part != num_parts) {
      /*
        We know the top partition and need to scan all underlying
        subpartitions. This is a range without holes.
      */
      DBUG_ASSERT(sub_part == num_parts);
      part_spec->start_part = part_part * part_info->num_subparts;
      part_spec->end_part = part_spec->start_part + part_info->num_subparts - 1;
    } else {
      DBUG_ASSERT(sub_part != num_parts);
      part_spec->start_part = sub_part;
      part_spec->end_part =
          sub_part + (part_info->num_subparts * (part_info->num_parts - 1));
      for (i = 0, part_id = sub_part; i < part_info->num_parts;
           i++, part_id += part_info->num_subparts)
        ;  // Set bit part_id in bit array
    }
  }
  if (found_part_field) clear_indicator_in_key_fields(key_info);
  /*
    Check if range can be adjusted by looking in read_partitions
  */
  prune_partition_set(table, part_spec);
}

/*
   If the table is partitioned we will read the partition info into the
   .frm file here.
   -------------------------------
   |  Fileinfo     64 bytes      |
   -------------------------------
   | Formnames     7 bytes       |
   -------------------------------
   | Not used    4021 bytes      |
   -------------------------------
   | Keyinfo + record            |
   -------------------------------
   | Padded to next multiple     |
   | of IO_SIZE                  |
   -------------------------------
   | Forminfo     288 bytes      |
   -------------------------------
   | Screen buffer, to make      |
   |field names readable        |
   -------------------------------
   | Packed field info           |
   |17 + 1 + strlen(field_name) |
   | + 1 end of file character   |
   -------------------------------
   | Partition info              |
   -------------------------------
   We provide the length of partition length in Fileinfo[55-58].

   Read the partition syntax from the frm file and parse it to get the
   data structures of the partitioning.

   SYNOPSIS
     mysql_unpack_partition()
     thd                           Thread object
     part_buf                      Partition info from frm file
     part_info_len                 Length of partition syntax
     table                         Table object of partitioned table
     create_table_ind              Is it called from CREATE TABLE
     default_db_type               What is the default engine of the table
     work_part_info_used           Flag is raised if we don't create new
                                   part_info, but used thd->work_part_info

   RETURN VALUE
     true                          Error
     false                         Sucess

   DESCRIPTION
     Read the partition syntax from the current position in the frm file.
     Initiate a LEX object, save the list of item tree objects to free after
     the query is done. Set-up partition info object such that parser knows
     it is called from internally. Call parser to create data structures
     (best possible recreation of item trees and so forth since there is no
     serialisation of these objects other than in parseable text format).
     We need to save the text of the partition functions since it is not
     possible to retrace this given an item tree.
*/

bool mysql_unpack_partition(THD *thd, char *part_buf, uint part_info_len,
                            TABLE *table, bool is_create_table_ind,
                            handlerton *default_db_type,
                            bool *work_part_info_used) {
  bool result = true;
  partition_info *part_info;
  const CHARSET_INFO *old_character_set_client =
      thd->variables.character_set_client;
  LEX *old_lex = thd->lex;
  LEX lex;
  SELECT_LEX_UNIT unit(CTX_NONE);
  SELECT_LEX select(thd->mem_root, nullptr, nullptr);
  lex.new_static_query(&unit, &select);

  sql_digest_state *parent_digest = thd->m_digest;
  PSI_statement_locker *parent_locker = thd->m_statement_psi;
  Partition_handler *part_handler;
  DBUG_TRACE;

  thd->variables.character_set_client = system_charset_info;
  // This isn't strictly needed, but here for consistency.
  Sql_mode_parse_guard parse_guard(thd);

  Partition_expr_parser_state parser_state;
  if (parser_state.init(thd, part_buf, part_info_len)) goto end;

  if (init_lex_with_single_table(thd, table, &lex)) goto end;

  /*
    All Items created is put into a free list on the THD object. This list
    is used to free all Item objects after completing a query. We don't
    want that to happen with the Item tree created as part of the partition
    info. This should be attached to the table object and remain so until
    the table object is released.
    Thus we move away the current list temporarily and start a new list that
    we then save in the partition info structure.
  */
  *work_part_info_used = false;
  DBUG_PRINT("info", ("Parse: %s", part_buf));

  thd->m_digest = nullptr;
  thd->m_statement_psi = nullptr;
  if (parse_sql(thd, &parser_state, nullptr) ||
      parser_state.result->fix_parser_data(thd)) {
    thd->free_items();
    thd->m_digest = parent_digest;
    thd->m_statement_psi = parent_locker;
    goto end;
  }
  part_info = parser_state.result;
  thd->m_digest = parent_digest;
  thd->m_statement_psi = parent_locker;
  /*
    The parsed syntax residing in the frm file can still contain defaults.
    The reason is that the frm file is sometimes saved outside of this
    MySQL Server and used in backup and restore of clusters or partitioned
    tables. It is not certain that the restore will restore exactly the
    same default partitioning.

    The easiest manner of handling this is to simply continue using the
    part_info we already built up during mysql_create_table if we are
    in the process of creating a table. If the table already exists we
    need to discover the number of partitions for the default parts. Since
    the handler object hasn't been created here yet we need to postpone this
    to the fix_partition_func method.
  */

  DBUG_PRINT("info", ("Successful parse"));
  DBUG_PRINT("info",
             ("default engine = %s, default_db_type = %s",
              ha_resolve_storage_engine_name(part_info->default_engine_type),
              ha_resolve_storage_engine_name(default_db_type)));
  if (is_create_table_ind && old_lex->sql_command == SQLCOM_CREATE_TABLE) {
    /*
      When we come here we are doing a create table. In this case we
      have already done some preparatory work on the old part_info
      object. We don't really need this new partition_info object.
      Thus we go back to the old partition info object.
      We need to free any memory objects allocated on item_free_list
      by the parser since we are keeping the old info from the first
      parser call in CREATE TABLE.

      This table object can not be used any more. However, since
      this is CREATE TABLE, we know that it will be destroyed by the
      caller, and rely on that.
    */
    thd->free_items();
    part_info = thd->work_part_info;
    *work_part_info_used = true;
  }
  table->part_info = part_info;
  part_info->table = table;
  part_handler = table->file->get_partition_handler();
  DBUG_ASSERT(part_handler != nullptr);
  part_handler->set_part_info(part_info, true);
  if (!part_info->default_engine_type)
    part_info->default_engine_type = default_db_type;
  DBUG_ASSERT(part_info->default_engine_type == default_db_type);
  DBUG_ASSERT(part_info->default_engine_type->db_type != DB_TYPE_UNKNOWN);

  {
    /*
      This code part allocates memory for the serialised item information for
      the partition functions. In most cases this is not needed but if the
      table is used for SHOW CREATE TABLES or ALTER TABLE that modifies
      partition information it is needed and the info is lost if we don't
      save it here so unfortunately we have to do it here even if in most
      cases it is not needed. This is a consequence of that item trees are
      not serialisable.
    */
    size_t part_func_len = part_info->part_func_len;
    size_t subpart_func_len = part_info->subpart_func_len;
    char *part_func_string = nullptr;
    char *subpart_func_string = nullptr;
    /*
      TODO: Verify that it really should be allocated on the thd?
      Or simply remove it and use part_expr->print() instead?
    */
    if ((part_func_len &&
         !((part_func_string = (char *)thd->alloc(part_func_len)))) ||
        (subpart_func_len &&
         !((subpart_func_string = (char *)thd->alloc(subpart_func_len))))) {
      mem_alloc_error(part_func_len);
      thd->free_items();
      goto end;
    }
    if (part_func_len)
      memcpy(part_func_string, part_info->part_func_string, part_func_len);
    if (subpart_func_len)
      memcpy(subpart_func_string, part_info->subpart_func_string,
             subpart_func_len);
    part_info->part_func_string = part_func_string;
    part_info->subpart_func_string = subpart_func_string;
  }

  result = false;
end:
  end_lex_with_single_table(thd, table, old_lex);
  thd->variables.character_set_client = old_character_set_client;
  return result;
}

/*
  Set engine type on all partition element objects
  SYNOPSIS
    set_engine_all_partitions()
    part_info                  Partition info
    engine_type                Handlerton reference of engine
  RETURN VALUES
    NONE
*/

static void set_engine_all_partitions(partition_info *part_info,
                                      handlerton *engine_type) {
  uint i = 0;
  List_iterator<partition_element> part_it(part_info->partitions);
  do {
    partition_element *part_elem = part_it++;

    part_elem->engine_type = engine_type;
    if (part_info->is_sub_partitioned()) {
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      uint j = 0;

      do {
        partition_element *sub_elem = sub_it++;

        sub_elem->engine_type = engine_type;
      } while (++j < part_info->num_subparts);
    }
  } while (++i < part_info->num_parts);
}

/*
  We need to check if engine used by all partitions can handle
  partitioning natively.

  SYNOPSIS
    check_native_partitioned()
    create_info            Create info in CREATE TABLE
    out:ret_val            Return value
    part_info              Partition info
    thd                    Thread object

  RETURN VALUES
  Value returned in bool ret_value
    true                   Native partitioning supported by engine
    false                  Need to use partition handler

  Return value from function
    true                   Error
    false                  Success
*/

static bool check_native_partitioned(HA_CREATE_INFO *create_info, bool *ret_val,
                                     partition_info *part_info, THD *thd) {
  bool table_engine_set;
  handlerton *engine_type = part_info->default_engine_type;
  handlerton *old_engine_type = engine_type;
  DBUG_TRACE;

  if (create_info->used_fields & HA_CREATE_USED_ENGINE) {
    table_engine_set = true;
    engine_type = create_info->db_type;
  } else {
    table_engine_set = false;
    if (thd->lex->sql_command != SQLCOM_CREATE_TABLE) {
      table_engine_set = true;
    }
  }
  DBUG_PRINT("info",
             ("engine_type = %s, table_engine_set = %u",
              ha_resolve_storage_engine_name(engine_type), table_engine_set));
  if (part_info->check_engine_mix(engine_type, table_engine_set)) goto error;

  /*
    All engines are of the same type. Check if this engine supports
    native partitioning.
  */

  if (!engine_type) engine_type = old_engine_type;
  DBUG_PRINT("info",
             ("engine_type = %s", ha_resolve_storage_engine_name(engine_type)));
  if (engine_type->partition_flags) {
    create_info->db_type = engine_type;
    DBUG_PRINT("info", ("Changed to native partitioning"));
    *ret_val = true;
  }
  return false;
error:
  /*
    Mixed engines not yet supported but when supported it will need
    the partition handler
  */
  my_error(ER_MIX_HANDLER_ERROR, MYF(0));
  *ret_val = false;
  return true;
}

/**
  Set part_state for all partitions to given state.

  @param tab_part_info  partition_info holding all partitions.
  @param part_state     Which state to set for the named partitions.
*/

void set_all_part_state(partition_info *tab_part_info,
                        enum partition_state part_state) {
  uint part_count = 0;
  List_iterator<partition_element> part_it(tab_part_info->partitions);

  do {
    partition_element *part_elem = part_it++;
    part_elem->part_state = part_state;
    if (tab_part_info->is_sub_partitioned()) {
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      partition_element *sub_elem;
      while ((sub_elem = sub_it++)) {
        sub_elem->part_state = part_state;
      }
    }
  } while (++part_count < tab_part_info->num_parts);
}

/**
  Sets which partitions to be used in the command.

  @param alter_info     Alter_info pointer holding partition names and flags.
  @param tab_part_info  partition_info holding all partitions.
  @param part_state     Which state to set for the named partitions.
  @param include_subpartitions Also include subpartitions in the search.

  @return Operation status
    @retval false  Success
    @retval true   Failure
*/

bool set_part_state(Alter_info *alter_info, partition_info *tab_part_info,
                    enum partition_state part_state,
                    bool include_subpartitions) {
  uint part_count = 0;
  uint num_parts_found = 0;
  List_iterator<partition_element> part_it(tab_part_info->partitions);

  do {
    partition_element *part_elem = part_it++;
    if ((alter_info->flags & Alter_info::ALTER_ALL_PARTITION) ||
        (is_name_in_list(part_elem->partition_name,
                         alter_info->partition_names))) {
      /*
        Mark the partition.
        I.e mark the partition as a partition to be "changed" by
        analyzing/optimizing/rebuilding/checking/repairing/...
      */
      num_parts_found++;
      part_elem->part_state = part_state;
      DBUG_PRINT("info", ("Setting part_state to %u for partition %s",
                          part_state, part_elem->partition_name));
    } else if (include_subpartitions && tab_part_info->is_sub_partitioned()) {
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      partition_element *sub_elem;
      while ((sub_elem = sub_it++)) {
        if (is_name_in_list(sub_elem->partition_name,
                            alter_info->partition_names)) {
          num_parts_found++;
          sub_elem->part_state = part_state;
          DBUG_PRINT("info", ("Setting part_state to %u for subpartition %s",
                              part_state, sub_elem->partition_name));
        } else
          sub_elem->part_state = PART_NORMAL;
      }
      part_elem->part_state = PART_NORMAL;
    } else
      part_elem->part_state = PART_NORMAL;
  } while (++part_count < tab_part_info->num_parts);

  if (num_parts_found != alter_info->partition_names.elements &&
      !(alter_info->flags & Alter_info::ALTER_ALL_PARTITION)) {
    /* Not all given partitions found, revert and return failure */
    set_all_part_state(tab_part_info, PART_NORMAL);
    return true;
  }
  return false;
}

/**
  @brief Check if partition is exchangable with table by checking table options

  @param table_create_info Table options from table.
  @param part_elem         All the info of the partition.

  @retval false if they are equal, otherwise true.

  @note Any differens that would cause a change in the frm file is prohibited.
  Such options as data_file_name, index_file_name, min_rows, max_rows etc. are
  not allowed to differ. But comment is allowed to differ.
*/
bool compare_partition_options(HA_CREATE_INFO *table_create_info,
                               partition_element *part_elem) {
#define MAX_COMPARE_PARTITION_OPTION_ERRORS 5
  const char *option_diffs[MAX_COMPARE_PARTITION_OPTION_ERRORS + 1];
  int i, errors = 0;
  DBUG_TRACE;
  // TODO: Add test for EXCHANGE PARTITION with TABLESPACES!
  // Then if all works, simply remove the check for TABLESPACE (and eventually
  // DATA/INDEX DIRECTORY too).

  /*
    Note that there are not yet any engine supporting tablespace together
    with partitioning. TODO: when there are, add compare.
  */
  if (part_elem->tablespace_name || table_create_info->tablespace)
    option_diffs[errors++] = "TABLESPACE";
  if (part_elem->part_max_rows != table_create_info->max_rows)
    option_diffs[errors++] = "MAX_ROWS";
  if (part_elem->part_min_rows != table_create_info->min_rows)
    option_diffs[errors++] = "MIN_ROWS";
  if (part_elem->index_file_name || table_create_info->index_file_name)
    option_diffs[errors++] = "INDEX DIRECTORY";

  for (i = 0; i < errors; i++)
    my_error(ER_PARTITION_EXCHANGE_DIFFERENT_OPTION, MYF(0), option_diffs[i]);
  return errors != 0;
}

/*
  Prepare for ALTER TABLE of partition structure

  @param[in] thd                 Thread object
  @param[in] table               Table object
  @param[in,out] alter_info      Alter information
  @param[in,out] create_info     Create info for CREATE TABLE
  @param[in]  alter_ctx          ALTER TABLE runtime context
  @param[out] partition_changed  Boolean indicating whether partition changed
  @param[out] new_part_info      New partition_info object if in-place alter
                                 which requires mark-up in partition_info is
                                 possible.

  @return Operation status
    @retval true                 Error
    @retval false                Success

  @note
    This method handles all preparations for ALTER TABLE for partitioned
    tables.
    We need to handle both partition management command such as Add Partition
    and others here as well as an ALTER TABLE that completely changes the
    partitioning and yet others that don't change anything at all. We start
    by checking the partition management variants and then check the general
    change patterns.
*/

uint prep_alter_part_table(THD *thd, TABLE *table, Alter_info *alter_info,
                           HA_CREATE_INFO *create_info,
                           Alter_table_ctx *alter_ctx MY_ATTRIBUTE((unused)),
                           bool *partition_changed,
                           partition_info **new_part_info) {
  DBUG_TRACE;
  DBUG_ASSERT(new_part_info);

  /* Remove partitioning on a not partitioned table is not possible */
  if (!table->part_info &&
      (alter_info->flags & Alter_info::ALTER_REMOVE_PARTITIONING)) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  if (thd->work_part_info &&
      !(thd->work_part_info = thd->lex->part_info->get_clone(thd, true)))
    return true;

  /* ALTER_ADMIN_PARTITION is handled in mysql_admin_table */
  DBUG_ASSERT(!(alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION));

  if (alter_info->flags &
      (Alter_info::ALTER_ADD_PARTITION | Alter_info::ALTER_DROP_PARTITION |
       Alter_info::ALTER_COALESCE_PARTITION |
       Alter_info::ALTER_REORGANIZE_PARTITION | Alter_info::ALTER_TABLE_REORG |
       Alter_info::ALTER_REBUILD_PARTITION)) {
    partition_info *tab_part_info;
    partition_info *alt_part_info = thd->work_part_info;
    uint flags = 0;
    bool is_last_partition_reorged = false;
    part_elem_value *tab_max_elem_val = nullptr;
    part_elem_value *alt_max_elem_val = nullptr;
    longlong tab_max_range = 0, alt_max_range = 0;
    Partition_handler *part_handler = table->file->get_partition_handler();

    if (!table->part_info) {
      my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
      return true;
    }
    if (!part_handler) {
      DBUG_ASSERT(0);
      my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
      return true;
    }

    /*
      Open our intermediate table, we will operate on a temporary instance
      of the original table, to be able to skip copying all partitions.
      Open it as a copy of the original table, and modify its partition_info
      object to allow in-place ALTER implementation to perform the changes.
    */
    DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::TABLE, alter_ctx->db, alter_ctx->table_name,
        MDL_INTENTION_EXCLUSIVE));

    /*
      We will operate on a cached instance of the original table,
      to be able to skip copying all non-changed partitions
      while allowing concurrent access.

      We create a new partition_info object which will carry
      the new state of the partitions. It will only be temporary
      attached to the handler when needed and then detached afterwards
      (through handler::set_part_info()). That way it will not get reused
      by next statement, even if the table object is reused due to LOCK TABLE.
    */
    tab_part_info = table->part_info->get_full_clone(thd);
    if (!tab_part_info) {
      mem_alloc_error(sizeof(partition_info));
      return true;
    }

    if (alter_info->flags & Alter_info::ALTER_TABLE_REORG) {
      uint new_part_no, curr_part_no;
      /*
        'ALTER TABLE t REORG PARTITION' only allowed with auto partition
         if default partitioning is used.
      */

      if (tab_part_info->part_type != partition_type::HASH ||
          ((table->s->db_type()->partition_flags() & HA_USE_AUTO_PARTITION) &&
           !tab_part_info->use_default_num_partitions) ||
          ((!(table->s->db_type()->partition_flags() &
              HA_USE_AUTO_PARTITION)) &&
           tab_part_info->use_default_num_partitions)) {
        my_error(ER_REORG_NO_PARAM_ERROR, MYF(0));
        goto err;
      }
      new_part_no = part_handler->get_default_num_partitions(create_info);
      curr_part_no = tab_part_info->num_parts;
      if (new_part_no == curr_part_no) {
        /*
          No change is needed, we will have the same number of partitions
          after the change as before. Thus we can reply ok immediately
          without any changes at all.
        */
        flags = part_handler->alter_flags(alter_info->flags);
        if (flags & HA_INPLACE_CHANGE_PARTITION) {
          *new_part_info = tab_part_info;
          /* Force table re-open for consistency with the main case. */
          table->m_needs_reopen = true;
        }

        thd->work_part_info = tab_part_info;
        return false;
      } else if (new_part_no > curr_part_no) {
        /*
          We will add more partitions, we use the ADD PARTITION without
          setting the flag for no default number of partitions
        */
        alter_info->flags |= Alter_info::ALTER_ADD_PARTITION;
        thd->work_part_info->num_parts = new_part_no - curr_part_no;
      } else {
        /*
          We will remove hash partitions, we use the COALESCE PARTITION
          without setting the flag for no default number of partitions
        */
        alter_info->flags |= Alter_info::ALTER_COALESCE_PARTITION;
        alter_info->num_parts = curr_part_no - new_part_no;
      }
    }
    if (!(flags = part_handler->alter_flags(alter_info->flags))) {
      my_error(ER_PARTITION_FUNCTION_FAILURE, MYF(0));
      goto err;
    }
    if (flags & HA_INPLACE_CHANGE_PARTITION) {
      /*
        "Inplace" change of partitioning is supported in this
        case. We will change TABLE::part_info (as this is how we pass
        information to storage engine in this case), so the table
        must be reopened.
      */
      *new_part_info = tab_part_info;
      table->m_needs_reopen = true;
    }
    DBUG_PRINT("info", ("*fast_alter_table flags: 0x%x", flags));
    if ((alter_info->flags & Alter_info::ALTER_ADD_PARTITION) ||
        (alter_info->flags & Alter_info::ALTER_REORGANIZE_PARTITION)) {
      if (thd->work_part_info->part_type != tab_part_info->part_type) {
        if (thd->work_part_info->part_type == partition_type::NONE) {
          if (tab_part_info->part_type == partition_type::RANGE) {
            my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), "RANGE");
            goto err;
          } else if (tab_part_info->part_type == partition_type::LIST) {
            my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), "LIST");
            goto err;
          }
          /*
            Hash partitions can be altered without parser finds out about
            that it is HASH partitioned. So no error here.
          */
        } else {
          if (thd->work_part_info->part_type == partition_type::RANGE) {
            my_error(ER_PARTITION_WRONG_VALUES_ERROR, MYF(0), "RANGE",
                     "LESS THAN");
          } else if (thd->work_part_info->part_type == partition_type::LIST) {
            DBUG_ASSERT(thd->work_part_info->part_type == partition_type::LIST);
            my_error(ER_PARTITION_WRONG_VALUES_ERROR, MYF(0), "LIST", "IN");
          } else if (tab_part_info->part_type == partition_type::RANGE) {
            my_error(ER_PARTITION_REQUIRES_VALUES_ERROR, MYF(0), "RANGE",
                     "LESS THAN");
          } else {
            DBUG_ASSERT(tab_part_info->part_type == partition_type::LIST);
            my_error(ER_PARTITION_REQUIRES_VALUES_ERROR, MYF(0), "LIST", "IN");
          }
          goto err;
        }
      }
      if ((tab_part_info->column_list &&
           alt_part_info->num_columns != tab_part_info->num_columns) ||
          (!tab_part_info->column_list &&
           (tab_part_info->part_type == partition_type::RANGE ||
            tab_part_info->part_type == partition_type::LIST) &&
           alt_part_info->num_columns != 1U) ||
          (!tab_part_info->column_list &&
           tab_part_info->part_type == partition_type::HASH &&
           alt_part_info->num_columns != 0)) {
        my_error(ER_PARTITION_COLUMN_LIST_ERROR, MYF(0));
        goto err;
      }
      alt_part_info->column_list = tab_part_info->column_list;
      if (alt_part_info->fix_parser_data(thd)) {
        goto err;
      }
    }
    if (alter_info->flags & Alter_info::ALTER_ADD_PARTITION) {
      /*
        We start by moving the new partitions to the list of temporary
        partitions. We will then check that the new partitions fit in the
        partitioning scheme as currently set-up.
        Partitions are always added at the end in ADD PARTITION.
      */
      uint num_new_partitions = alt_part_info->num_parts;
      uint num_orig_partitions = tab_part_info->num_parts;
      uint check_total_partitions = num_new_partitions + num_orig_partitions;
      uint new_total_partitions = check_total_partitions;
      /*
        We allow quite a lot of values to be supplied by defaults, however we
        must know the number of new partitions in this case.
      */
      if (thd->lex->no_write_to_binlog &&
          tab_part_info->part_type != partition_type::HASH) {
        my_error(ER_NO_BINLOG_ERROR, MYF(0));
        goto err;
      }
      if (tab_part_info->defined_max_value) {
        my_error(ER_PARTITION_MAXVALUE_ERROR, MYF(0));
        goto err;
      }
      if (num_new_partitions == 0) {
        my_error(ER_ADD_PARTITION_NO_NEW_PARTITION, MYF(0));
        goto err;
      }
      if (tab_part_info->is_sub_partitioned()) {
        if (alt_part_info->num_subparts == 0)
          alt_part_info->num_subparts = tab_part_info->num_subparts;
        else if (alt_part_info->num_subparts != tab_part_info->num_subparts) {
          my_error(ER_ADD_PARTITION_SUBPART_ERROR, MYF(0));
          goto err;
        }
        check_total_partitions =
            new_total_partitions * alt_part_info->num_subparts;
      }
      if (check_total_partitions > MAX_PARTITIONS) {
        my_error(ER_TOO_MANY_PARTITIONS_ERROR, MYF(0));
        goto err;
      }
      alt_part_info->part_type = tab_part_info->part_type;
      alt_part_info->subpart_type = tab_part_info->subpart_type;
      if (alt_part_info->set_up_defaults_for_partitioning(
              part_handler, nullptr, tab_part_info->num_parts)) {
        goto err;
      }
      /*
      Handling of on-line cases:

      ADD PARTITION for RANGE/LIST PARTITIONING:
      ------------------------------------------
      For range and list partitions add partition is simply adding a
      new empty partition to the table. If the handler support this we
      will use the simple method of doing this. The figure below shows
      an example of this and the states involved in making this change.

      Existing partitions                                     New added
      partitions
      ------       ------        ------        ------      |  ------    ------
      |    |       |    |        |    |        |    |      |  |    |    |    |
      | p0 |       | p1 |        | p2 |        | p3 |      |  | p4 |    | p5 |
      ------       ------        ------        ------      |  ------    ------
      PART_NORMAL  PART_NORMAL   PART_NORMAL   PART_NORMAL    PART_TO_BE_ADDED*2
      PART_NORMAL  PART_NORMAL   PART_NORMAL   PART_NORMAL    PART_IS_ADDED*2

      The first line is the states before adding the new partitions and the
      second line is after the new partitions are added. All the partitions are
      in the partitions list, no partitions are placed in the temp_partitions
      list.

      ADD PARTITION for HASH PARTITIONING
      -----------------------------------
      This little figure tries to show the various partitions involved when
      adding two new partitions to a linear hash based partitioned table with
      four partitions to start with, which lists are used and the states they
      pass through. Adding partitions to a normal hash based is similar except
      that it is always all the existing partitions that are reorganised not
      only a subset of them.

      Existing partitions                                     New added
      partitions
      ------       ------        ------        ------      |  ------    ------
      |    |       |    |        |    |        |    |      |  |    |    |    |
      | p0 |       | p1 |        | p2 |        | p3 |      |  | p4 |    | p5 |
      ------       ------        ------        ------      |  ------    ------
      PART_CHANGED PART_CHANGED  PART_NORMAL   PART_NORMAL    PART_TO_BE_ADDED
      PART_IS_CHANGED*2          PART_NORMAL   PART_NORMAL    PART_IS_ADDED
      PART_NORMAL  PART_NORMAL   PART_NORMAL   PART_NORMAL    PART_IS_ADDED

      Reorganised existing partitions
      ------      ------
      |    |      |    |
      | p0'|      | p1'|
      ------      ------

      p0 - p5 will be in the partitions list of partitions.
      p0' and p1' will actually not exist as separate objects, there presence
      can be deduced from the state of the partition and also the names of those
      partitions can be deduced this way.

      After adding the partitions and copying the partition data to p0', p1',
      p4 and p5 from p0 and p1 the states change to adapt for the new situation
      where p0 and p1 is dropped and replaced by p0' and p1' and the new p4 and
      p5 are in the table again.

      The first line above shows the states of the partitions before we start
      adding and copying partitions, the second after completing the adding
      and copying and finally the third line after also dropping the partitions
      that are reorganised.
      */
      if (*new_part_info && tab_part_info->part_type == partition_type::HASH) {
        uint part_no = 0, start_part = 1, start_sec_part = 1;
        uint end_part = 0, end_sec_part = 0;
        uint upper_2n = tab_part_info->linear_hash_mask + 1;
        uint lower_2n = upper_2n >> 1;
        bool all_parts = true;
        if (tab_part_info->linear_hash_ind && num_new_partitions < upper_2n) {
          /*
            An analysis of which parts needs reorganisation shows that it is
            divided into two intervals. The first interval is those parts
            that are reorganised up until upper_2n - 1. From upper_2n and
            onwards it starts again from partition 0 and goes on until
            it reaches p(upper_2n - 1). If the last new partition reaches
            beyond upper_2n - 1 then the first interval will end with
            p(lower_2n - 1) and start with p(num_orig_partitions - lower_2n).
            If lower_2n partitions are added then p0 to p(lower_2n - 1) will
            be reorganised which means that the two interval becomes one
            interval at this point. Thus only when adding less than
            lower_2n partitions and going beyond a total of upper_2n we
            actually get two intervals.

            To exemplify this assume we have 6 partitions to start with and
            add 1, 2, 3, 5, 6, 7, 8, 9 partitions.
            The first to add after p5 is p6 = 110 in bit numbers. Thus we
            can see that 10 = p2 will be partition to reorganise if only one
            partition.
            If 2 partitions are added we reorganise [p2, p3]. Those two
            cases are covered by the second if part below.
            If 3 partitions are added we reorganise [p2, p3] U [p0,p0]. This
            part is covered by the else part below.
            If 5 partitions are added we get [p2,p3] U [p0, p2] = [p0, p3].
            This is covered by the first if part where we need the max check
            to here use lower_2n - 1.
            If 7 partitions are added we get [p2,p3] U [p0, p4] = [p0, p4].
            This is covered by the first if part but here we use the first
            calculated end_part.
            Finally with 9 new partitions we would also reorganise p6 if we
            used the method below but we cannot reorganise more partitions
            than what we had from the start and thus we simply set all_parts
            to true. In this case we don't get into this if-part at all.
          */
          all_parts = false;
          if (num_new_partitions >= lower_2n) {
            /*
              In this case there is only one interval since the two intervals
              overlap and this starts from zero to last_part_no - upper_2n
            */
            start_part = 0;
            end_part = new_total_partitions - (upper_2n + 1);
            end_part = max(lower_2n - 1, end_part);
          } else if (new_total_partitions <= upper_2n) {
            /*
              Also in this case there is only one interval since we are not
              going over a 2**n boundary
            */
            start_part = num_orig_partitions - lower_2n;
            end_part = start_part + (num_new_partitions - 1);
          } else {
            /* We have two non-overlapping intervals since we are not
               passing a 2**n border and we have not at least lower_2n
               new parts that would ensure that the intervals become
               overlapping.
            */
            start_part = num_orig_partitions - lower_2n;
            end_part = upper_2n - 1;
            start_sec_part = 0;
            end_sec_part = new_total_partitions - (upper_2n + 1);
          }
        }
        List_iterator<partition_element> tab_it(tab_part_info->partitions);
        part_no = 0;
        do {
          partition_element *p_elem = tab_it++;
          if (all_parts || (part_no >= start_part && part_no <= end_part) ||
              (part_no >= start_sec_part && part_no <= end_sec_part)) {
            p_elem->part_state = PART_CHANGED;
          }
        } while (++part_no < num_orig_partitions);
      }
      /*
        Need to concatenate the lists here to make it possible to check the
        partition info for correctness using check_partition_info.
        For on-line add partition we set the state of this partition to
        PART_TO_BE_ADDED to ensure that it is known that it is not yet
        usable (becomes usable when partition is created and the switch of
        partition configuration is made.
      */
      {
        List_iterator<partition_element> alt_it(alt_part_info->partitions);
        uint part_count = 0;
        do {
          partition_element *part_elem = alt_it++;
          if (*new_part_info) part_elem->part_state = PART_TO_BE_ADDED;
          if (tab_part_info->partitions.push_back(part_elem)) {
            mem_alloc_error(1);
            goto err;
          }
        } while (++part_count < num_new_partitions);
        tab_part_info->num_parts += num_new_partitions;
      }
      /*
        If we specify partitions explicitly we don't use defaults anymore.
        Using ADD PARTITION also means that we don't have the default number
        of partitions anymore. We use this code also for Table reorganisations
        and here we don't set any default flags to false.
      */
      if (!(alter_info->flags & Alter_info::ALTER_TABLE_REORG)) {
        if (!alt_part_info->use_default_partitions) {
          DBUG_PRINT("info", ("part_info: %p", tab_part_info));
          tab_part_info->use_default_partitions = false;
        }
        tab_part_info->use_default_num_partitions = false;
        tab_part_info->is_auto_partitioned = false;
      }
    } else if (alter_info->flags & Alter_info::ALTER_DROP_PARTITION) {
      /*
        Drop a partition from a range partition and list partitioning is
        always safe and can be made more or less immediate. It is necessary
        however to ensure that the partition to be removed is safely removed
        and that REPAIR TABLE can remove the partition if for some reason the
        command to drop the partition failed in the middle.
      */
      uint part_count = 0;
      uint num_parts_dropped = alter_info->partition_names.elements;
      uint num_parts_found = 0;
      List_iterator<partition_element> part_it(tab_part_info->partitions);

      tab_part_info->is_auto_partitioned = false;
      if (!(tab_part_info->part_type == partition_type::RANGE ||
            tab_part_info->part_type == partition_type::LIST)) {
        my_error(ER_ONLY_ON_RANGE_LIST_PARTITION, MYF(0), "DROP");
        goto err;
      }
      do {
        partition_element *part_elem = part_it++;
        if (is_name_in_list(part_elem->partition_name,
                            alter_info->partition_names)) {
          /*
            Set state to indicate that the partition is to be dropped.
          */
          num_parts_found++;
          part_elem->part_state = PART_TO_BE_DROPPED;
        }
      } while (++part_count < tab_part_info->num_parts);
      if (num_parts_found != num_parts_dropped) {
        my_error(ER_DROP_PARTITION_NON_EXISTENT, MYF(0), "DROP");
        goto err;
      }
      if (num_parts_found == tab_part_info->num_parts) {
        my_error(ER_DROP_LAST_PARTITION, MYF(0));
        goto err;
      }
      tab_part_info->num_parts -= num_parts_dropped;
    } else if (alter_info->flags & Alter_info::ALTER_REBUILD_PARTITION) {
      set_engine_all_partitions(tab_part_info,
                                tab_part_info->default_engine_type);
      if (set_part_state(alter_info, tab_part_info, PART_CHANGED, false)) {
        my_error(ER_DROP_PARTITION_NON_EXISTENT, MYF(0), "REBUILD");
        goto err;
      }
      if (!(*new_part_info)) {
        table->file->print_error(HA_ERR_WRONG_COMMAND, MYF(0));
        goto err;
      }
    } else if (alter_info->flags & Alter_info::ALTER_COALESCE_PARTITION) {
      uint num_parts_coalesced = alter_info->num_parts;
      uint num_parts_remain = tab_part_info->num_parts - num_parts_coalesced;
      List_iterator<partition_element> part_it(tab_part_info->partitions);
      if (tab_part_info->part_type != partition_type::HASH) {
        my_error(ER_COALESCE_ONLY_ON_HASH_PARTITION, MYF(0));
        goto err;
      }
      if (num_parts_coalesced == 0) {
        my_error(ER_COALESCE_PARTITION_NO_PARTITION, MYF(0));
        goto err;
      }
      if (num_parts_coalesced >= tab_part_info->num_parts) {
        my_error(ER_DROP_LAST_PARTITION, MYF(0));
        goto err;
      }
      /*
      Online handling:
      COALESCE PARTITION:
      -------------------
      The figure below shows the manner in which partitions are handled when
      performing an on-line coalesce partition and which states they go through
      at start, after adding and copying partitions and finally after dropping
      the partitions to drop. The figure shows an example using four partitions
      to start with, using linear hash and coalescing one partition (always the
      last partition).

      Using linear hash then all remaining partitions will have a new
      reorganised part.

      Existing partitions                     Coalesced partition
      ------       ------              ------   |      ------
      |    |       |    |              |    |   |      |    |
      | p0 |       | p1 |              | p2 |   |      | p3 |
      ------       ------              ------   |      ------
      PART_NORMAL  PART_CHANGED        PART_NORMAL     PART_REORGED_DROPPED
      PART_NORMAL  PART_IS_CHANGED     PART_NORMAL     PART_TO_BE_DROPPED
      PART_NORMAL  PART_NORMAL         PART_NORMAL     PART_IS_DROPPED

      Reorganised existing partitions
                  ------
                  |    |
                  | p1'|
                  ------

      p0 - p3 is in the partitions list.
      The p1' partition will actually not be in any list it is deduced from the
      state of p1.
      */
      {
        uint part_count = 0, start_part = 1, start_sec_part = 1;
        uint end_part = 0, end_sec_part = 0;
        bool all_parts = true;
        if (*new_part_info && tab_part_info->linear_hash_ind) {
          uint upper_2n = tab_part_info->linear_hash_mask + 1;
          uint lower_2n = upper_2n >> 1;
          all_parts = false;
          if (num_parts_coalesced >= lower_2n) {
            all_parts = true;
          } else if (num_parts_remain >= lower_2n) {
            end_part = tab_part_info->num_parts - (lower_2n + 1);
            start_part = num_parts_remain - lower_2n;
          } else {
            start_part = 0;
            end_part = tab_part_info->num_parts - (lower_2n + 1);
            end_sec_part = (lower_2n >> 1) - 1;
            start_sec_part = end_sec_part - (lower_2n - (num_parts_remain + 1));
          }
        }
        do {
          partition_element *p_elem = part_it++;
          if (*new_part_info &&
              (all_parts ||
               (part_count >= start_part && part_count <= end_part) ||
               (part_count >= start_sec_part && part_count <= end_sec_part)))
            p_elem->part_state = PART_CHANGED;
          if (++part_count > num_parts_remain) {
            if (*new_part_info)
              p_elem->part_state = PART_REORGED_DROPPED;
            else
              part_it.remove();
          }
        } while (part_count < tab_part_info->num_parts);
        tab_part_info->num_parts = num_parts_remain;
      }
      if (!(alter_info->flags & Alter_info::ALTER_TABLE_REORG)) {
        tab_part_info->use_default_num_partitions = false;
        tab_part_info->is_auto_partitioned = false;
      }
    } else if (alter_info->flags & Alter_info::ALTER_REORGANIZE_PARTITION) {
      /*
        Reorganise partitions takes a number of partitions that are next
        to each other (at least for RANGE PARTITIONS) and then uses those
        to create a set of new partitions. So data is copied from those
        partitions into the new set of partitions. Those new partitions
        can have more values in the LIST value specifications or less both
        are allowed. The ranges can be different but since they are
        changing a set of consecutive partitions they must cover the same
        range as those changed from.
        This command can be used on RANGE and LIST partitions.
      */
      uint num_parts_reorged = alter_info->partition_names.elements;
      uint num_parts_new = thd->work_part_info->partitions.elements;
      uint check_total_partitions;

      tab_part_info->is_auto_partitioned = false;
      if (num_parts_reorged > tab_part_info->num_parts) {
        my_error(ER_REORG_PARTITION_NOT_EXIST, MYF(0));
        goto err;
      }
      if (!(tab_part_info->part_type == partition_type::RANGE ||
            tab_part_info->part_type == partition_type::LIST) &&
          (num_parts_new != num_parts_reorged)) {
        my_error(ER_REORG_HASH_ONLY_ON_SAME_NO, MYF(0));
        goto err;
      }
      if (tab_part_info->is_sub_partitioned() && alt_part_info->num_subparts &&
          alt_part_info->num_subparts != tab_part_info->num_subparts) {
        my_error(ER_PARTITION_WRONG_NO_SUBPART_ERROR, MYF(0));
        goto err;
      }
      check_total_partitions = tab_part_info->num_parts + num_parts_new;
      check_total_partitions -= num_parts_reorged;
      if (check_total_partitions > MAX_PARTITIONS) {
        my_error(ER_TOO_MANY_PARTITIONS_ERROR, MYF(0));
        goto err;
      }
      alt_part_info->part_type = tab_part_info->part_type;
      alt_part_info->subpart_type = tab_part_info->subpart_type;
      alt_part_info->num_subparts = tab_part_info->num_subparts;
      DBUG_ASSERT(!alt_part_info->use_default_partitions);
      /* We specified partitions explicitly so don't use defaults anymore. */
      tab_part_info->use_default_partitions = false;
      if (alt_part_info->set_up_defaults_for_partitioning(part_handler, nullptr,
                                                          0)) {
        goto err;
      }
      /*
      Online handling:
      REORGANIZE PARTITION:
      ---------------------
      The figure exemplifies the handling of partitions, their state changes and
      how they are organised. It exemplifies four partitions where two of the
      partitions are reorganised (p1 and p2) into two new partitions (p4 and
      p5). The reason of this change could be to change range limits, change
      list values or for hash partitions simply reorganise the partition which
      could also involve moving them to new disks or new node groups (MySQL
      Cluster).

      Existing partitions
      ------       ------        ------        ------
      |    |       |    |        |    |        |    |
      | p0 |       | p1 |        | p2 |        | p3 |
      ------       ------        ------        ------
      PART_NORMAL  PART_TO_BE_REORGED          PART_NORMAL
      PART_NORMAL  PART_TO_BE_DROPPED          PART_NORMAL
      PART_NORMAL  PART_IS_DROPPED             PART_NORMAL

      Reorganised new partitions (replacing p1 and p2)
      ------      ------
      |    |      |    |
      | p4 |      | p5 |
      ------      ------
      PART_TO_BE_ADDED
      PART_IS_ADDED
      PART_IS_ADDED

      All unchanged partitions and the new partitions are in the partitions list
      in the order they will have when the change is completed. The reorganised
      partitions are placed in the temp_partitions list. PART_IS_ADDED is only a
      temporary state not written in the frm file. It is used to ensure we write
      the generated partition syntax in a correct manner.
      */
      {
        List_iterator<partition_element> tab_it(tab_part_info->partitions);
        uint part_count = 0;
        bool found_first = false;
        bool found_last = false;
        uint drop_count = 0;
        do {
          partition_element *part_elem = tab_it++;
          is_last_partition_reorged = false;
          if (is_name_in_list(part_elem->partition_name,
                              alter_info->partition_names)) {
            is_last_partition_reorged = true;
            drop_count++;
            if (tab_part_info->column_list) {
              List_iterator<part_elem_value> p(part_elem->list_val_list);
              tab_max_elem_val = p++;
            } else
              tab_max_range = part_elem->range_value;
            if (*new_part_info &&
                tab_part_info->temp_partitions.push_back(part_elem)) {
              mem_alloc_error(1);
              goto err;
            }
            if (*new_part_info) part_elem->part_state = PART_TO_BE_REORGED;
            if (!found_first) {
              uint alt_part_count = 0;
              partition_element *alt_part_elem;
              List_iterator<partition_element> alt_it(
                  alt_part_info->partitions);
              found_first = true;
              do {
                alt_part_elem = alt_it++;
                if (tab_part_info->column_list) {
                  List_iterator<part_elem_value> p(
                      alt_part_elem->list_val_list);
                  alt_max_elem_val = p++;
                } else
                  alt_max_range = alt_part_elem->range_value;

                if (*new_part_info)
                  alt_part_elem->part_state = PART_TO_BE_ADDED;
                if (alt_part_count == 0)
                  tab_it.replace(alt_part_elem);
                else
                  tab_it.after(alt_part_elem);
              } while (++alt_part_count < num_parts_new);
            } else if (found_last) {
              my_error(ER_CONSECUTIVE_REORG_PARTITIONS, MYF(0));
              goto err;
            } else
              tab_it.remove();
          } else {
            if (found_first) found_last = true;
          }
        } while (++part_count < tab_part_info->num_parts);
        if (drop_count != num_parts_reorged) {
          my_error(ER_DROP_PARTITION_NON_EXISTENT, MYF(0), "REORGANIZE");
          goto err;
        }
        tab_part_info->num_parts = check_total_partitions;
      }
    } else {
      DBUG_ASSERT(false);
    }
    *partition_changed = true;
    thd->work_part_info = tab_part_info;
    if (alter_info->flags & Alter_info::ALTER_ADD_PARTITION ||
        alter_info->flags & Alter_info::ALTER_REORGANIZE_PARTITION) {
      if (tab_part_info->use_default_subpartitions &&
          !alt_part_info->use_default_subpartitions) {
        tab_part_info->use_default_subpartitions = false;
        tab_part_info->use_default_num_subpartitions = false;
      }
      if (tab_part_info->check_partition_info(thd, (handlerton **)nullptr,
                                              table->file, nullptr, true)) {
        goto err;
      }
      /*
        The check below needs to be performed after check_partition_info
        since this function "fixes" the item trees of the new partitions
        to reorganize into
      */
      if (alter_info->flags == Alter_info::ALTER_REORGANIZE_PARTITION &&
          tab_part_info->part_type == partition_type::RANGE) {
        bool is_error;
        if (is_last_partition_reorged) {
          if (tab_part_info->column_list) {
            is_error = tab_part_info->compare_column_values(
                alt_max_elem_val->col_val_array,
                tab_max_elem_val->col_val_array);  // a < b.
          } else {
            is_error = alt_max_range < tab_max_range;
          }
        } else {
          if (tab_part_info->column_list) {
            is_error = tab_part_info->compare_column_values(
                           alt_max_elem_val->col_val_array,
                           tab_max_elem_val->col_val_array) ||
                       tab_part_info->compare_column_values(
                           tab_max_elem_val->col_val_array,
                           alt_max_elem_val->col_val_array);  // a != b.
          } else {
            is_error = alt_max_range != tab_max_range;
          }
        }
        if (is_error) {
          /*
            For range partitioning the total resulting range before and
            after the change must be the same except in one case. This is
            when the last partition is reorganised, in this case it is
            acceptable to increase the total range.
            The reason is that it is not allowed to have "holes" in the
            middle of the ranges and thus we should not allow to reorganise
            to create "holes".
          */
          my_error(ER_REORG_OUTSIDE_RANGE, MYF(0));
          goto err;
        }
      }
    }
  } else {
    /*
     When thd->lex->part_info has a reference to a partition_info the
     ALTER TABLE contained a definition of a partitioning.

     Case I:
       If there was a partition before and there is a new one defined.
       We use the new partitioning. The new partitioning is already
       defined in the correct variable so no work is needed to
       accomplish this.
       We do however need to update partition_changed to ensure that not
       only the frm file is changed in the ALTER TABLE command.

     Case IIa:
       There was a partitioning before and there is no new one defined.
       Also the user has not specified to remove partitioning explicitly.

       We use the old partitioning also for the new table. We do this
       by assigning the partition_info from the table loaded in
       open_table to the partition_info struct used by mysql_create_table
       later in this method.

     Case IIb:
       There was a partitioning before and there is no new one defined.
       The user has specified explicitly to remove partitioning

       Since the user has specified explicitly to remove partitioning
       we override the old partitioning info and create a new table using
       the specified engine.
       In this case the partition also is changed.

     Case III:
       There was no partitioning before altering the table, there is
       partitioning defined in the altered table. Use the new partitioning.
       No work needed since the partitioning info is already in the
       correct variable.

       In this case we discover one case where the new partitioning is using
       the same partition function as the default (PARTITION BY KEY or
       PARTITION BY LINEAR KEY with the list of fields equal to the primary
       key fields OR PARTITION BY [LINEAR] KEY() for tables without primary
       key)
       Also here partition has changed and thus a new table must be
       created.

     Case IV:
       There was no partitioning before and no partitioning defined.
       Obviously no work needed.
    */
    partition_info *tab_part_info = table->part_info;

    if (tab_part_info) {
      /*
        The table must be reopened, this is necessary to avoid situations
        where a failing ALTER leaves behind a TABLE object which has its
        partitioning information updated by the SE, as InnoDB is doing in
        update_create_info().
      */
      table->m_needs_reopen = true;
      if (alter_info->flags & Alter_info::ALTER_REMOVE_PARTITIONING) {
        DBUG_PRINT("info", ("Remove partitioning"));
        if (!(create_info->used_fields & HA_CREATE_USED_ENGINE)) {
          DBUG_PRINT("info", ("No explicit engine used"));
          create_info->db_type = tab_part_info->default_engine_type;
        }
        DBUG_PRINT("info",
                   ("New engine type: %s",
                    ha_resolve_storage_engine_name(create_info->db_type)));
        thd->work_part_info = nullptr;
        *partition_changed = true;
      } else if (!thd->work_part_info) {
        /*
          Retain partitioning but possibly with a new storage engine
          beneath.

          Create a copy of TABLE::part_info to be able to modify it freely.
        */
        if (!(tab_part_info = tab_part_info->get_clone(thd))) return true;
        thd->work_part_info = tab_part_info;
        if (create_info->used_fields & HA_CREATE_USED_ENGINE &&
            create_info->db_type != tab_part_info->default_engine_type) {
          /*
            Make sure change of engine happens to all partitions.
          */
          DBUG_PRINT("info", ("partition changed"));
          if (tab_part_info->is_auto_partitioned) {
            /*
              If the user originally didn't specify partitioning to be
              used we can remove it now.
            */
            thd->work_part_info = nullptr;
          } else {
            /*
              Ensure that all partitions have the proper engine set-up
            */
            set_engine_all_partitions(thd->work_part_info,
                                      create_info->db_type);
          }
          *partition_changed = true;
        }
      }
    }
    if (thd->work_part_info) {
      partition_info *part_info = thd->work_part_info;
      bool is_native_partitioned = false;
      /*
        Need to cater for engine types that can handle partition without
        using the partition handler.
      */
      if (part_info != tab_part_info) {
        if (part_info->fix_parser_data(thd)) {
          goto err;
        }
        /*
          Compare the old and new part_info. If only key_algorithm
          change is done, don't consider it as changed partitioning (to avoid
          rebuild). This is to handle KEY (numeric_cols) partitioned tables
          created in 5.1. For more info, see bug#14521864.
        */
        if (alter_info->flags != Alter_info::ALTER_PARTITION ||
            !table->part_info ||
            alter_info->requested_algorithm !=
                Alter_info::ALTER_TABLE_ALGORITHM_INPLACE ||
            !table->part_info->has_same_partitioning(part_info)) {
          DBUG_PRINT("info", ("partition changed"));
          *partition_changed = true;
        }
      }
      /*
        Set up partition default_engine_type either from the create_info
        or from the previus table
      */
      if (create_info->used_fields & HA_CREATE_USED_ENGINE)
        part_info->default_engine_type = create_info->db_type;
      else {
        if (tab_part_info)
          part_info->default_engine_type = tab_part_info->default_engine_type;
        else
          part_info->default_engine_type = create_info->db_type;
      }
      if (check_native_partitioned(create_info, &is_native_partitioned,
                                   part_info, thd)) {
        goto err;
      }
      if (!is_native_partitioned) {
        my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0), "native partitioning");
        goto err;
      }
    }
  }
  return false;
err:
  *new_part_info = nullptr;
  return true;
}

/*
  Prepare for calling val_int on partition function by setting fields to
  point to the record where the values of the PF-fields are stored.

  SYNOPSIS
    set_field_ptr()
    ptr                 Array of fields to change ptr
    new_buf             New record pointer
    old_buf             Old record pointer

  DESCRIPTION
    Set ptr in field objects of field array to refer to new_buf record
    instead of previously old_buf. Used before calling val_int and after
    it is used to restore pointers to table->record[0].
    This routine is placed outside of partition code since it can be useful
    also for other programs.
*/

static void set_field_ptr(Field **ptr, const uchar *new_buf,
                          const uchar *old_buf) {
  ptrdiff_t diff = (new_buf - old_buf);
  DBUG_TRACE;

  do {
    (*ptr)->move_field_offset(diff);
  } while (*(++ptr));
}

/**
  Append all fields in read_set to string

  @param[in,out] str   String to append to.
  @param[in]     row   Row to append.
  @param[in]     table Table containing read_set and fields for the row.
*/
void append_row_to_str(String &str, const uchar *row, TABLE *table) {
  Field **fields, **field_ptr;
  const uchar *rec;
  uint num_fields = bitmap_bits_set(table->read_set);
  uint curr_field_index = 0;
  bool is_rec0 = !row || row == table->record[0];
  if (!row)
    rec = table->record[0];
  else
    rec = row;

  /* Create a new array of all read fields. */
  fields = (Field **)my_malloc(key_memory_handler_errmsgs,
                               sizeof(void *) * (num_fields + 1), MYF(0));
  if (!fields) return;
  fields[num_fields] = nullptr;
  for (field_ptr = table->field; *field_ptr; field_ptr++) {
    if (!bitmap_is_set(table->read_set, (*field_ptr)->field_index)) continue;
    fields[curr_field_index++] = *field_ptr;
  }

  if (!is_rec0) set_field_ptr(fields, rec, table->record[0]);

  for (field_ptr = fields; *field_ptr; field_ptr++) {
    Field *field = *field_ptr;
    str.append(" ");
    str.append(field->field_name);
    str.append(":");
    field_unpack(&str, field, 0, false);
  }

  if (!is_rec0) set_field_ptr(fields, table->record[0], rec);
  my_free(fields);
}

/*
  SYNOPSIS
    mem_alloc_error()
    size                Size of memory attempted to allocate
    None

  RETURN VALUES
    None

  DESCRIPTION
    A routine to use for all the many places in the code where memory
    allocation error can happen, a tremendous amount of them, needs
    simple routine that signals this error.
*/

void mem_alloc_error(size_t size) {
  my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), static_cast<int>(size));
}

/**
  Return comma-separated list of used partitions in the provided given string.

    @param      part_info  Partitioning info
    @param[out] parts      The resulting list of string to fill

    Generate a list of used partitions (from bits in part_info->read_partitions
    bitmap), and store it into the provided String object.

    @note
    The produced string must not be longer then MAX_PARTITIONS * (1 + FN_LEN).
    In case of UPDATE, only the partitions read is given, not the partitions
    that was written or locked.
*/

bool make_used_partitions_str(partition_info *part_info,
                              List<const char> *parts) {
  parts->empty();
  partition_element *pe;
  uint partition_id = 0;
  List_iterator<partition_element> it(part_info->partitions);
  StringBuffer<FN_LEN> part_str(system_charset_info);

  if (part_info->is_sub_partitioned()) {
    partition_element *head_pe;
    while ((head_pe = it++)) {
      List_iterator<partition_element> it2(head_pe->subpartitions);
      while ((pe = it2++)) {
        if (bitmap_is_set(&part_info->read_partitions, partition_id)) {
          part_str.length(0);
          if ((part_str.append(head_pe->partition_name,
                               strlen(head_pe->partition_name),
                               system_charset_info)) ||
              part_str.append('_') ||
              part_str.append(pe->partition_name, strlen(pe->partition_name),
                              system_charset_info) ||
              parts->push_back(part_str.dup(current_thd->mem_root)))
            return true;
        }
        partition_id++;
      }
    }
  } else {
    while ((pe = it++)) {
      if (bitmap_is_set(&part_info->read_partitions, partition_id)) {
        part_str.length(0);
        if (part_str.append(pe->partition_name, strlen(pe->partition_name),
                            system_charset_info) ||
            parts->push_back(part_str.dup(current_thd->mem_root)))
          return true;
      }
      partition_id++;
    }
  }
  return false;
}

/****************************************************************************
 * Partition interval analysis support
 ***************************************************************************/

/*
  Setup partition_info::* members related to partitioning range analysis

  SYNOPSIS
    set_up_partition_func_pointers()
      part_info  Partitioning info structure

  DESCRIPTION
    Assuming that passed partition_info structure already has correct values
    for members that specify [sub]partitioning type, table fields, and
    functions, set up partition_info::* members that are related to
    Partitioning Interval Analysis (see get_partitions_in_range_iter for its
    definition)

  IMPLEMENTATION
    There are three available interval analyzer functions:
    (1) get_part_iter_for_interval_via_mapping
    (2) get_part_iter_for_interval_cols_via_map
    (3) get_part_iter_for_interval_via_walking

    They all have limited applicability:
    (1) is applicable for "PARTITION BY <RANGE|LIST>(func(t.field))", where
    func is a monotonic function.

    (2) is applicable for "PARTITION BY <RANGE|LIST> COLUMNS (field_list)

    (3) is applicable for
      "[SUB]PARTITION BY <any-partitioning-type>(any_func(t.integer_field))"

    If both (1) and (3) are applicable, (1) is preferred over (3).

    This function sets part_info::get_part_iter_for_interval according to
    this criteria, and also sets some auxilary fields that the function
    uses.
*/
static void set_up_range_analysis_info(partition_info *part_info) {
  /* Set the catch-all default */
  part_info->get_part_iter_for_interval = nullptr;
  part_info->get_subpart_iter_for_interval = nullptr;

  /*
    Check if get_part_iter_for_interval_via_mapping() can be used for
    partitioning
  */
  switch (part_info->part_type) {
    case partition_type::RANGE:
    case partition_type::LIST:
      if (!part_info->column_list) {
        if (part_info->part_expr->get_monotonicity_info() != NON_MONOTONIC) {
          part_info->get_part_iter_for_interval =
              get_part_iter_for_interval_via_mapping;
          goto setup_subparts;
        }
      } else {
        part_info->get_part_iter_for_interval =
            get_part_iter_for_interval_cols_via_map;
        goto setup_subparts;
      }
    default:;
  }

  /*
    Check if get_part_iter_for_interval_via_walking() can be used for
    partitioning
  */
  if (part_info->num_part_fields == 1) {
    Field *field = part_info->part_field_array[0];
    switch (field->type()) {
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_LONGLONG:
        part_info->get_part_iter_for_interval =
            get_part_iter_for_interval_via_walking;
        break;
      default:;
    }
  }

setup_subparts:
  /*
    Check if get_part_iter_for_interval_via_walking() can be used for
    subpartitioning
  */
  if (part_info->num_subpart_fields == 1) {
    Field *field = part_info->subpart_field_array[0];
    switch (field->type()) {
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_LONGLONG:
        part_info->get_subpart_iter_for_interval =
            get_part_iter_for_interval_via_walking;
        break;
      default:;
    }
  }
}

/*
  This function takes a memory of packed fields in opt-range format
  and stores it in record format. To avoid having to worry about how
  the length of fields are calculated in opt-range format we send
  an array of lengths used for each field in store_length_array.

  SYNOPSIS
  store_tuple_to_record()
  pfield                         Field array
  store_length_array             Array of field lengths
  value                          Memory where fields are stored
  value_end                      End of memory

  RETURN VALUE
  nparts                         Number of fields assigned
*/
static uint32 store_tuple_to_record(Field **pfield, uint32 *store_length_array,
                                    uchar *value, uchar *value_end) {
  /* This function is inspired by store_key_image_rec. */
  uint32 nparts = 0;
  uchar *loc_value;
  while (value < value_end) {
    loc_value = value;
    if ((*pfield)->is_nullable()) {
      if (*loc_value)
        (*pfield)->set_null();
      else
        (*pfield)->set_notnull();
      loc_value++;
    }
    uint len = (*pfield)->pack_length();
    (*pfield)->set_key_image(loc_value, len);
    value += *store_length_array;
    store_length_array++;
    nparts++;
    pfield++;
  }
  return nparts;
}

/**
  RANGE(columns) partitioning: compare partition value bound and probe tuple.

  @param val           Partition column values.
  @param nvals_in_rec  Number of (prefix) fields to compare.

  @return Less than/Equal to/Greater than 0 if the record is L/E/G than val.

  @note The partition value bound is always a full tuple (but may include the
  MAXVALUE special value). The probe tuple may be a prefix of partitioning
  tuple.
*/

static int cmp_rec_and_tuple(part_column_list_val *val, uint32 nvals_in_rec) {
  partition_info *part_info = val->part_info;
  Field **field = part_info->part_field_array;
  Field **fields_end = field + nvals_in_rec;
  int res;

  for (; field != fields_end; field++, val++) {
    if (val->max_value) return -1;
    if ((*field)->is_null()) {
      if (val->null_value) continue;
      return -1;
    }
    if (val->null_value) return +1;
    res = (*field)->cmp(val->column_value.field_image);
    if (res) return res;
  }
  return 0;
}

/**
  Compare record and columns partition tuple including endpoint handling.

  @param  val               Columns partition tuple
  @param  n_vals_in_rec     Number of columns to compare
  @param  is_left_endpoint  True if left endpoint (part_tuple < rec or
                            part_tuple <= rec)
  @param  include_endpoint  If endpoint is included (part_tuple <= rec or
                            rec <= part_tuple)

  @return Less than/Equal to/Greater than 0 if the record is L/E/G than
  the partition tuple.

  @see get_list_array_idx_for_endpoint() and
  get_partition_id_range_for_endpoint().
*/

static int cmp_rec_and_tuple_prune(part_column_list_val *val,
                                   uint32 n_vals_in_rec, bool is_left_endpoint,
                                   bool include_endpoint) {
  int cmp;
  Field **field;
  if ((cmp = cmp_rec_and_tuple(val, n_vals_in_rec))) return cmp;
  field = val->part_info->part_field_array + n_vals_in_rec;
  if (!(*field)) {
    /* Full match. Only equal if including endpoint. */
    if (include_endpoint) return 0;

    if (is_left_endpoint)
      return +4; /* Start of range, part_tuple < rec, return higher. */
    return -4;   /* End of range, rec < part_tupe, return lesser. */
  }
  /*
    The prefix is equal and there are more partition columns to compare.

    If including left endpoint or not including right endpoint
    then the record is considered lesser compared to the partition.

    i.e:
    part(10, x) <= rec(10, unknown) and rec(10, unknown) < part(10, x)
    part <= rec -> lesser (i.e. this or previous partitions)
    rec < part -> lesser (i.e. this or previous partitions)
  */
  if (is_left_endpoint == include_endpoint) return -2;

  /*
    If right endpoint and the first additional partition value
    is MAXVALUE, then the record is lesser.
  */
  if (!is_left_endpoint && (val + n_vals_in_rec)->max_value) return -3;

  /*
    Otherwise the record is considered greater.

    rec <= part -> greater (i.e. does not match this partition, seek higher).
    part < rec -> greater (i.e. does not match this partition, seek higher).
  */
  return 2;
}

typedef uint32 (*get_endpoint_func)(partition_info *, bool left_endpoint,
                                    bool include_endpoint);

typedef uint32 (*get_col_endpoint_func)(partition_info *, bool left_endpoint,
                                        bool include_endpoint,
                                        uint32 num_parts);

/**
  Get partition for RANGE COLUMNS endpoint.

  @param part_info         Partitioning metadata.
  @param is_left_endpoint     True if left endpoint (const <=/< cols)
  @param include_endpoint  True if range includes the endpoint (<=/>=)
  @param nparts            Total number of partitions

  @return Partition id of matching partition.

  @see get_partition_id_cols_list_for_endpoint and
  get_partition_id_range_for_endpoint.
*/

static uint32 get_partition_id_cols_range_for_endpoint(
    partition_info *part_info, bool is_left_endpoint, bool include_endpoint,
    uint32 nparts) {
  uint min_part_id = 0, max_part_id = part_info->num_parts, loc_part_id;
  part_column_list_val *range_col_array = part_info->range_col_array;
  uint num_columns = part_info->part_field_list.elements;
  DBUG_TRACE;

  /* Find the matching partition (including taking endpoint into account). */
  do {
    /* Midpoint, adjusted down, so it can never be > last partition. */
    loc_part_id = (max_part_id + min_part_id) >> 1;
    if (0 <=
        cmp_rec_and_tuple_prune(range_col_array + loc_part_id * num_columns,
                                nparts, is_left_endpoint, include_endpoint))
      min_part_id = loc_part_id + 1;
    else
      max_part_id = loc_part_id;
  } while (max_part_id > min_part_id);
  loc_part_id = max_part_id;

  /* Given value must be LESS THAN the found partition. */
  DBUG_ASSERT(loc_part_id == part_info->num_parts ||
              (0 > cmp_rec_and_tuple_prune(
                       range_col_array + loc_part_id * num_columns, nparts,
                       is_left_endpoint, include_endpoint)));
  /* Given value must be GREATER THAN or EQUAL to the previous partition. */
  DBUG_ASSERT(loc_part_id == 0 ||
              (0 <= cmp_rec_and_tuple_prune(
                        range_col_array + (loc_part_id - 1) * num_columns,
                        nparts, is_left_endpoint, include_endpoint)));

  if (!is_left_endpoint) {
    /* Set the end after this partition if not already after the last. */
    if (loc_part_id < part_info->num_parts) loc_part_id++;
  }
  return loc_part_id;
}

static int get_part_iter_for_interval_cols_via_map(
    partition_info *part_info, bool, uint32 *store_length_array,
    uchar *min_value, uchar *max_value, uint min_len, uint max_len, uint flags,
    PARTITION_ITERATOR *part_iter) {
  uint32 nparts;
  get_col_endpoint_func get_col_endpoint;
  DBUG_TRACE;

  if (part_info->part_type == partition_type::RANGE) {
    get_col_endpoint = get_partition_id_cols_range_for_endpoint;
    part_iter->get_next = get_next_partition_id_range;
  } else if (part_info->part_type == partition_type::LIST) {
    get_col_endpoint = get_partition_id_cols_list_for_endpoint;
    part_iter->get_next = get_next_partition_id_list;
    part_iter->part_info = part_info;
    DBUG_ASSERT(part_info->num_list_values);
  } else {
    assert(0);
    get_col_endpoint = nullptr;
  }

  if (flags & NO_MIN_RANGE)
    part_iter->part_nums.start = part_iter->part_nums.cur = 0;
  else {
    // Copy from min_value to record
    nparts =
        store_tuple_to_record(part_info->part_field_array, store_length_array,
                              min_value, min_value + min_len);
    part_iter->part_nums.start = part_iter->part_nums.cur =
        get_col_endpoint(part_info, true, !(flags & NEAR_MIN), nparts);
  }
  if (flags & NO_MAX_RANGE) {
    if (part_info->part_type == partition_type::RANGE)
      part_iter->part_nums.end = part_info->num_parts;
    else /* LIST_PARTITION */
    {
      DBUG_ASSERT(part_info->part_type == partition_type::LIST);
      part_iter->part_nums.end = part_info->num_list_values;
    }
  } else {
    // Copy from max_value to record
    nparts =
        store_tuple_to_record(part_info->part_field_array, store_length_array,
                              max_value, max_value + max_len);
    part_iter->part_nums.end =
        get_col_endpoint(part_info, false, !(flags & NEAR_MAX), nparts);
  }
  if (part_iter->part_nums.start == part_iter->part_nums.end) return 0;
  return 1;
}

/**
  Partitioning Interval Analysis: Initialize the iterator for "mapping" case

  @param part_info   Partition info
  @param is_subpart  true  - act for subpartitioning
                     false - act for partitioning
  @param store_length_array  Ignored.
  @param min_value   minimum field value, in opt_range key format.
  @param max_value   minimum field value, in opt_range key format.
  @param min_len     Ignored.
  @param max_len     Ignored.
  @param flags       Some combination of NEAR_MIN, NEAR_MAX, NO_MIN_RANGE,
                     NO_MAX_RANGE.
  @param part_iter   Iterator structure to be initialized

  @details Initialize partition set iterator to walk over the interval in
  ordered-array-of-partitions (for RANGE partitioning) or
  ordered-array-of-list-constants (for LIST partitioning) space.

  This function is used when partitioning is done by
  <RANGE|LIST>(ascending_func(t.field)), and we can map an interval in
  t.field space into a sub-array of partition_info::range_int_array or
  partition_info::list_array (see get_partition_id_range_for_endpoint,
  get_list_array_idx_for_endpoint for details).

  The function performs this interval mapping, and sets the iterator to
  traverse the sub-array and return appropriate partitions.

  @return Status of iterator
    @retval 0   No matching partitions (iterator not initialized)
    @retval 1   Ok, iterator intialized for traversal of matching partitions.
    @retval -1  All partitions would match (iterator not initialized)
*/

static int get_part_iter_for_interval_via_mapping(
    partition_info *part_info, bool is_subpart MY_ATTRIBUTE((unused)),
    uint32 *store_length_array, /* ignored */
    uchar *min_value, uchar *max_value, uint min_len,
    uint max_len, /* ignored */
    uint flags, PARTITION_ITERATOR *part_iter) {
  Field *field = part_info->part_field_array[0];
  uint32 max_endpoint_val = 0;
  get_endpoint_func get_endpoint = nullptr;
  bool can_match_multiple_values; /* is not '=' */
  uint field_len = field->pack_length_in_rec();
  MYSQL_TIME start_date;
  bool check_zero_dates = false;
  bool zero_in_start_date = true;
  DBUG_TRACE;
  DBUG_ASSERT(!is_subpart);
  (void)store_length_array;
  (void)min_len;
  (void)max_len;
  part_iter->ret_null_part = part_iter->ret_null_part_orig = false;

  if (part_info->part_type == partition_type::RANGE) {
    if (part_info->part_charset_field_array)
      get_endpoint = get_partition_id_range_for_endpoint_charset;
    else
      get_endpoint = get_partition_id_range_for_endpoint;
    max_endpoint_val = part_info->num_parts;
    part_iter->get_next = get_next_partition_id_range;
  } else if (part_info->part_type == partition_type::LIST) {
    if (part_info->part_charset_field_array)
      get_endpoint = get_list_array_idx_for_endpoint_charset;
    else
      get_endpoint = get_list_array_idx_for_endpoint;
    max_endpoint_val = part_info->num_list_values;
    part_iter->get_next = get_next_partition_id_list;
    part_iter->part_info = part_info;
    if (max_endpoint_val == 0) {
      /*
        We handle this special case without optimisations since it is
        of little practical value but causes a great number of complex
        checks later in the code.
      */
      part_iter->part_nums.start = part_iter->part_nums.end = 0;
      part_iter->part_nums.cur = 0;
      part_iter->ret_null_part = part_iter->ret_null_part_orig = true;
      return -1;
    }
  } else
    MY_ASSERT_UNREACHABLE();

  can_match_multiple_values = (flags || !min_value || !max_value ||
                               memcmp(min_value, max_value, field_len));
  if (can_match_multiple_values &&
      (part_info->part_type == partition_type::RANGE ||
       part_info->has_null_value)) {
    /* Range scan on RANGE or LIST partitioned table */
    enum_monotonicity_info monotonic;
    monotonic = part_info->part_expr->get_monotonicity_info();
    if (monotonic == MONOTONIC_INCREASING_NOT_NULL ||
        monotonic == MONOTONIC_STRICT_INCREASING_NOT_NULL) {
      /* col is NOT NULL, but F(col) can return NULL, add NULL partition */
      part_iter->ret_null_part = part_iter->ret_null_part_orig = true;
      check_zero_dates = true;
    }
  }

  /*
    Find minimum: Do special handling if the interval has left bound in form
     " NULL <= X ":
  */
  if (field->is_nullable() && part_info->has_null_value &&
      !(flags & (NO_MIN_RANGE | NEAR_MIN)) && *min_value) {
    part_iter->ret_null_part = part_iter->ret_null_part_orig = true;
    part_iter->part_nums.start = part_iter->part_nums.cur = 0;
    if (!(flags & NO_MAX_RANGE) && *max_value) {
      /* The right bound is X <= NULL, i.e. it is a "X IS NULL" interval */
      part_iter->part_nums.end = 0;
      return 1;
    }
  } else {
    if (flags & NO_MIN_RANGE)
      part_iter->part_nums.start = part_iter->part_nums.cur = 0;
    else {
      /*
        Store the interval edge in the record buffer, and call the
        function that maps the edge in table-field space to an edge
        in ordered-set-of-partitions (for RANGE partitioning) or
        index-in-ordered-array-of-list-constants (for LIST) space.
      */
      store_key_image_to_rec(field, min_value, field_len);
      bool include_endp = !(flags & NEAR_MIN);
      part_iter->part_nums.start = get_endpoint(part_info, true, include_endp);
      if (!can_match_multiple_values && part_info->part_expr->null_value) {
        /* col = x and F(x) = NULL -> only search NULL partition */
        part_iter->part_nums.cur = part_iter->part_nums.start = 0;
        part_iter->part_nums.end = 0;
        part_iter->ret_null_part = part_iter->ret_null_part_orig = true;
        return 1;
      }
      part_iter->part_nums.cur = part_iter->part_nums.start;
      if (check_zero_dates && !part_info->part_expr->null_value) {
        if (!(flags & NO_MAX_RANGE) && (field->type() == MYSQL_TYPE_DATE ||
                                        field->type() == MYSQL_TYPE_DATETIME)) {
          /* Monotonic, but return NULL for dates with zeros in month/day. */
          zero_in_start_date = field->get_date(&start_date, 0);
          DBUG_PRINT("info",
                     ("zero start %u %04d-%02d-%02d", zero_in_start_date,
                      start_date.year, start_date.month, start_date.day));
        }
      }
      if (part_iter->part_nums.start == max_endpoint_val)
        return 0; /* No partitions */
    }
  }

  /* Find maximum, do the same as above but for right interval bound */
  if (flags & NO_MAX_RANGE)
    part_iter->part_nums.end = max_endpoint_val;
  else {
    store_key_image_to_rec(field, max_value, field_len);
    bool include_endp = !(flags & NEAR_MAX);
    part_iter->part_nums.end = get_endpoint(part_info, false, include_endp);
    if (check_zero_dates && !zero_in_start_date &&
        !part_info->part_expr->null_value) {
      MYSQL_TIME end_date;
      bool zero_in_end_date = field->get_date(&end_date, 0);
      /*
        This is an optimization for TO_DAYS()/TO_SECONDS() to avoid scanning
        the NULL partition for ranges that cannot include a date with 0 as
        month/day.
      */
      DBUG_PRINT("info", ("zero end %u %04d-%02d-%02d", zero_in_end_date,
                          end_date.year, end_date.month, end_date.day));
      DBUG_ASSERT(!memcmp(((Item_func *)part_info->part_expr)->func_name(),
                          "to_days", 7) ||
                  !memcmp(((Item_func *)part_info->part_expr)->func_name(),
                          "to_seconds", 10));
      if (!zero_in_end_date && start_date.month == end_date.month &&
          start_date.year == end_date.year)
        part_iter->ret_null_part = part_iter->ret_null_part_orig = false;
    }
    if (part_iter->part_nums.start >= part_iter->part_nums.end &&
        !part_iter->ret_null_part)
      return 0; /* No partitions */
  }
  return 1; /* Ok, iterator initialized */
}

/* See get_part_iter_for_interval_via_walking for definition of what this is */
#define MAX_RANGE_TO_WALK 32

/*
  Partitioning Interval Analysis: Initialize iterator to walk field interval

  SYNOPSIS
    get_part_iter_for_interval_via_walking()
      part_info   Partition info
      is_subpart  true  - act for subpartitioning
                  false - act for partitioning
      min_value   minimum field value, in opt_range key format.
      max_value   minimum field value, in opt_range key format.
      flags       Some combination of NEAR_MIN, NEAR_MAX, NO_MIN_RANGE,
                  NO_MAX_RANGE.
      part_iter   Iterator structure to be initialized

  DESCRIPTION
    Initialize partition set iterator to walk over interval in integer field
    space. That is, for "const1 <=? t.field <=? const2" interval, initialize
    the iterator to return a set of [sub]partitions obtained with the
    following procedure:
      get partition id for t.field = const1,   return it
      get partition id for t.field = const1+1, return it
       ...                 t.field = const1+2, ...
       ...                           ...       ...
       ...                 t.field = const2    ...

  IMPLEMENTATION
    See get_partitions_in_range_iter for general description of interval
    analysis. We support walking over the following intervals:
      "t.field IS NULL"
      "c1 <=? t.field <=? c2", where c1 and c2 are finite.
    Intervals with +inf/-inf, and [NULL, c1] interval can be processed but
    that is more tricky and I don't have time to do it right now.

  RETURN
    0 - No matching partitions, iterator not initialized
    1 - Some partitions would match, iterator intialized for traversing them
   -1 - All partitions would match, iterator not initialized
*/

static int get_part_iter_for_interval_via_walking(
    partition_info *part_info, bool is_subpart,
    uint32 *store_length_array, /* ignored */
    uchar *min_value, uchar *max_value, uint min_len,
    uint max_len, /* ignored */
    uint flags, PARTITION_ITERATOR *part_iter) {
  Field *field;
  uint total_parts;
  partition_iter_func get_next_func;
  DBUG_TRACE;
  (void)store_length_array;
  (void)min_len;
  (void)max_len;

  part_iter->ret_null_part = part_iter->ret_null_part_orig = false;
  if (is_subpart) {
    field = part_info->subpart_field_array[0];
    total_parts = part_info->num_subparts;
    get_next_func = get_next_subpartition_via_walking;
  } else {
    field = part_info->part_field_array[0];
    total_parts = part_info->num_parts;
    get_next_func = get_next_partition_via_walking;
  }

  /* Handle the "t.field IS NULL" interval, it is a special case */
  if (field->is_nullable() && !(flags & (NO_MIN_RANGE | NO_MAX_RANGE)) &&
      *min_value && *max_value) {
    /*
      We don't have a part_iter->get_next() function that would find which
      partition "t.field IS NULL" belongs to, so find partition that contains
      NULL right here, and return an iterator over singleton set.
    */
    uint32 part_id;
    field->set_null();
    if (is_subpart) {
      if (!part_info->get_subpartition_id(part_info, &part_id)) {
        init_single_partition_iterator(part_id, part_iter);
        return 1; /* Ok, iterator initialized */
      }
    } else {
      longlong dummy;
      int res =
          part_info->is_sub_partitioned()
              ? part_info->get_part_partition_id(part_info, &part_id, &dummy)
              : part_info->get_partition_id(part_info, &part_id, &dummy);
      if (!res) {
        init_single_partition_iterator(part_id, part_iter);
        return 1; /* Ok, iterator initialized */
      }
    }
    return 0; /* No partitions match */
  }

  if ((field->is_nullable() &&
       ((!(flags & NO_MIN_RANGE) && *min_value) ||    // NULL <? X
        (!(flags & NO_MAX_RANGE) && *max_value))) ||  // X <? NULL
      (flags & (NO_MIN_RANGE | NO_MAX_RANGE)))        // -inf at any bound
  {
    return -1; /* Can't handle this interval, have to use all partitions */
  }

  /* Get integers for left and right interval bound */
  ulonglong a, b;
  uint len = field->pack_length_in_rec();
  store_key_image_to_rec(field, min_value, len);
  a = field->val_int();

  store_key_image_to_rec(field, max_value, len);
  b = field->val_int();

  /*
    Handle a special case where the distance between interval bounds is
    exactly 4G-1. This interval is too big for range walking, and if it is an
    (x,y]-type interval then the following "b +=..." code will convert it to
    an empty interval by "wrapping around" a + 4G-1 + 1 = a.
  */
  if (b - a == ~0ULL) return -1;

  if (flags & NEAR_MIN) ++a;
  if (!(flags & NEAR_MAX)) ++b;
  ulonglong n_values = b - a;

  /*
    Will it pay off to enumerate all values in the [a..b] range and evaluate
    the partitioning function for every value? It depends on
     1. whether we'll be able to infer that some partitions are not used
     2. if time savings from not scanning these partitions will be greater
        than time spent in enumeration.
    We will assume that the cost of accessing one extra partition is greater
    than the cost of evaluating the partitioning function O(#partitions).
    This means we should jump at any chance to eliminate a partition, which
    gives us this logic:

    Do the enumeration if
     - the number of values to enumerate is comparable to the number of
       partitions, or
     - there are not many values to enumerate.
  */
  if ((n_values > 2 * total_parts) && n_values > MAX_RANGE_TO_WALK) return -1;

  part_iter->field_vals.start = part_iter->field_vals.cur = a;
  part_iter->field_vals.end = b;
  part_iter->part_info = part_info;
  part_iter->get_next = get_next_func;
  return 1;
}

/*
  PARTITION_ITERATOR::get_next implementation: enumerate partitions in range

  SYNOPSIS
    get_next_partition_id_range()
      part_iter  Partition set iterator structure

  DESCRIPTION
    This is implementation of PARTITION_ITERATOR::get_next() that returns
    [sub]partition ids in [min_partition_id, max_partition_id] range.
    The function conforms to partition_iter_func type.

  RETURN
    partition id
    NOT_A_PARTITION_ID if there are no more partitions
*/

uint32 get_next_partition_id_range(PARTITION_ITERATOR *part_iter) {
  if (part_iter->part_nums.cur >= part_iter->part_nums.end) {
    if (part_iter->ret_null_part) {
      part_iter->ret_null_part = false;
      return 0; /* NULL always in first range partition */
    }
    part_iter->part_nums.cur = part_iter->part_nums.start;
    part_iter->ret_null_part = part_iter->ret_null_part_orig;
    return NOT_A_PARTITION_ID;
  } else
    return part_iter->part_nums.cur++;
}

/*
  PARTITION_ITERATOR::get_next implementation for LIST partitioning

  SYNOPSIS
    get_next_partition_id_list()
      part_iter  Partition set iterator structure

  DESCRIPTION
    This implementation of PARTITION_ITERATOR::get_next() is special for
    LIST partitioning: it enumerates partition ids in
    part_info->list_array[i] (list_col_array[i*cols] for COLUMNS LIST
    partitioning) where i runs over [min_idx, max_idx] interval.
    The function conforms to partition_iter_func type.

  RETURN
    partition id
    NOT_A_PARTITION_ID if there are no more partitions
*/

static uint32 get_next_partition_id_list(PARTITION_ITERATOR *part_iter) {
  if (part_iter->part_nums.cur >= part_iter->part_nums.end) {
    if (part_iter->ret_null_part) {
      part_iter->ret_null_part = false;
      return part_iter->part_info->has_null_part_id;
    }
    part_iter->part_nums.cur = part_iter->part_nums.start;
    part_iter->ret_null_part = part_iter->ret_null_part_orig;
    return NOT_A_PARTITION_ID;
  } else {
    partition_info *part_info = part_iter->part_info;
    uint32 num_part = part_iter->part_nums.cur++;
    if (part_info->column_list) {
      uint num_columns = part_info->part_field_list.elements;
      return part_info->list_col_array[num_part * num_columns].partition_id;
    }
    return part_info->list_array[num_part].partition_id;
  }
}

/*
  PARTITION_ITERATOR::get_next implementation: walk over field-space interval

  SYNOPSIS
    get_next_partition_via_walking()
      part_iter  Partitioning iterator

  DESCRIPTION
    This implementation of PARTITION_ITERATOR::get_next() returns ids of
    partitions that contain records with partitioning field value within
    [start_val, end_val] interval.
    The function conforms to partition_iter_func type.

  RETURN
    partition id
    NOT_A_PARTITION_ID if there are no more partitioning.
*/

static uint32 get_next_partition_via_walking(PARTITION_ITERATOR *part_iter) {
  uint32 part_id;
  Field *field = part_iter->part_info->part_field_array[0];
  while (part_iter->field_vals.cur != part_iter->field_vals.end) {
    longlong dummy;
    field->store(part_iter->field_vals.cur++, field->flags & UNSIGNED_FLAG);
    if ((part_iter->part_info->is_sub_partitioned() &&
         !part_iter->part_info->get_part_partition_id(part_iter->part_info,
                                                      &part_id, &dummy)) ||
        !part_iter->part_info->get_partition_id(part_iter->part_info, &part_id,
                                                &dummy))
      return part_id;
  }
  part_iter->field_vals.cur = part_iter->field_vals.start;
  return NOT_A_PARTITION_ID;
}

/* Same as get_next_partition_via_walking, but for subpartitions */

static uint32 get_next_subpartition_via_walking(PARTITION_ITERATOR *part_iter) {
  Field *field = part_iter->part_info->subpart_field_array[0];
  uint32 res;
  if (part_iter->field_vals.cur == part_iter->field_vals.end) {
    part_iter->field_vals.cur = part_iter->field_vals.start;
    return NOT_A_PARTITION_ID;
  }
  field->store(part_iter->field_vals.cur++, field->flags & UNSIGNED_FLAG);
  if (part_iter->part_info->get_subpartition_id(part_iter->part_info, &res))
    return NOT_A_PARTITION_ID;
  return res;
}

uint get_partition_field_store_length(Field *field) {
  uint store_length;

  store_length = field->key_length();
  if (field->is_nullable()) store_length += HA_KEY_NULL_LENGTH;
  if (field->real_type() == MYSQL_TYPE_VARCHAR)
    store_length += HA_KEY_BLOB_LENGTH;
  return store_length;
}
