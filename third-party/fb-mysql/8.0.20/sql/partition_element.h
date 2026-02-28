#ifndef PARTITION_ELEMENT_INCLUDED
#define PARTITION_ELEMENT_INCLUDED

/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_base.h"     /* ha_rows */
#include "sql/handler.h" /* UNDEF_NODEGROUP */

/**
 * An enum and a struct to handle partitioning and subpartitioning.
 */
enum class partition_type { NONE = 0, RANGE, HASH, LIST };

enum partition_state {
  PART_NORMAL = 0,
  PART_IS_DROPPED = 1,
  PART_TO_BE_DROPPED = 2,
  PART_TO_BE_ADDED = 3,
  PART_TO_BE_REORGED = 4,
  PART_REORGED_DROPPED = 5,
  PART_CHANGED = 6,
  PART_IS_CHANGED = 7,
  PART_IS_ADDED = 8,
  PART_ADMIN = 9
};

/*
  This struct is used to keep track of column expressions as part
  of the COLUMNS concept in conjunction with RANGE and LIST partitioning.
  The value can be either of MINVALUE, MAXVALUE and an expression that
  must be constant and evaluate to the same type as the column it
  represents.

  The data in this fixed in two steps. The parser will only fill in whether
  it is a max_value or provide an expression. Filling in
  column_value, part_info, partition_id, null_value is done by the
  function fix_column_value_function. However the item tree needs
  fixed also before writing it into the frm file (in add_column_list_values).
  To distinguish between those two variants, fixed= 1 after the
  fixing in add_column_list_values and fixed= 2 otherwise. This is
  since the fixing in add_column_list_values isn't a complete fixing.
*/

typedef struct p_column_list_val {
  union column_value_union {
    /**
      When a table is opened this is set to the field image of the value
      from the item_expression below.
    */
    const uchar *field_image;
    /**
      When the values are read from dd.Partition_value it is carried as
      a C-string.
    */
    const char *value_str;
  } column_value;
  /**
    When partition clause is parsed this is set to the item expression
    for the value. Must be NULL if the value was not parsed, but
    read from dd.Partition_value instead.
  */
  Item *item_expression;
  partition_info *part_info;
  uint partition_id;
  /** MAXVALUE is set (only for RANGE COLUMNS) */
  bool max_value;
  /** NULL is set (only for LIST COLUMNS) */
  bool null_value;
  char fixed;
} part_column_list_val;

/*
  This struct is used to contain the value of an element
  in the VALUES IN struct. It needs to keep knowledge of
  whether it is a signed/unsigned value and whether it is
  NULL or not.
*/

typedef struct p_elem_val {
  longlong value;
  uint added_items;
  bool null_value;
  bool unsigned_flag;
  part_column_list_val *col_val_array;
} part_elem_value;

class partition_element {
 public:
  List<partition_element> subpartitions;
  List<part_elem_value> list_val_list;  // list of LIST values/column arrays
  // TODO: Handle options in a more general way, like dd::Properties
  // for max/min rows, tablespace, data/index file, nodegroup etc.
  ha_rows part_max_rows;
  ha_rows part_min_rows;
  longlong range_value;
  const char *partition_name;
  const char *tablespace_name;
  char *part_comment;
  const char *data_file_name;
  const char *index_file_name;
  handlerton *engine_type;
  enum partition_state part_state;
  uint16 nodegroup_id;
  bool has_null_value;
  /* TODO: Move this to partition_info?*/
  bool signed_flag;  // Range value signed
  bool max_value;    // MAXVALUE range

  partition_element()
      : part_max_rows(0),
        part_min_rows(0),
        range_value(0),
        partition_name(nullptr),
        tablespace_name(nullptr),
        part_comment(nullptr),
        data_file_name(nullptr),
        index_file_name(nullptr),
        engine_type(nullptr),
        part_state(PART_NORMAL),
        nodegroup_id(UNDEF_NODEGROUP),
        has_null_value(false),
        signed_flag(false),
        max_value(false) {}
  partition_element(partition_element *part_elem)
      : part_max_rows(part_elem->part_max_rows),
        part_min_rows(part_elem->part_min_rows),
        range_value(0),
        partition_name(nullptr),
        tablespace_name(part_elem->tablespace_name),
        part_comment(part_elem->part_comment),
        data_file_name(part_elem->data_file_name),
        index_file_name(part_elem->index_file_name),
        engine_type(part_elem->engine_type),
        part_state(part_elem->part_state),
        nodegroup_id(part_elem->nodegroup_id),
        has_null_value(false),
        signed_flag(false),
        max_value(false) {}
  inline void set_from_info(const HA_CREATE_INFO *info) {
    data_file_name = info->data_file_name;
    index_file_name = info->index_file_name;
    tablespace_name = info->tablespace;
    part_max_rows = info->max_rows;
    part_min_rows = info->min_rows;
  }
  inline void put_to_info(HA_CREATE_INFO *info) const {
    info->data_file_name = data_file_name;
    info->index_file_name = index_file_name;
    info->tablespace = tablespace_name;
    info->max_rows = part_max_rows;
    info->min_rows = part_min_rows;
  }
};

#endif /* PARTITION_ELEMENT_INCLUDED */
