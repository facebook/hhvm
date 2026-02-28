/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/parse_tree_partitions.h"

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/item.h"
#include "sql/parse_location.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"
#include "sql_string.h"

Partition_parse_context::Partition_parse_context(
    THD *thd_arg, partition_info *part_info_arg,
    partition_element *current_partition_arg,
    partition_element *curr_part_elem_arg, bool is_add_or_reorganize_partition)
    : Parse_context(thd_arg, thd_arg->lex->current_select()),
      Parser_partition_info(part_info_arg, current_partition_arg,
                            curr_part_elem_arg, nullptr, 0),
      is_add_or_reorganize_partition(is_add_or_reorganize_partition) {}

bool PT_subpartition::contextualize(Partition_parse_context *pc) {
  partition_info *const part_info = pc->part_info;

  if (part_info->use_default_subpartitions &&
      part_info->partitions.elements >= 2) {
    /*
      create table t1 (a int)
      partition by list (a) subpartition by hash (a)
      (partition p0 values in (1),
       partition p1 values in (2) subpartition sp11);
      causes us to arrive since we are on the second
      partition, but still use_default_subpartitions
      is set. When we come here we're processing at least
      the second partition (the current partition processed
      have already been put into the partitions list.
    */
    error(pc, pos, ER_THD(pc->thd, ER_PARTITION_WRONG_NO_SUBPART_ERROR));
    return true;
  }

  auto *sub_p_elem =
      new (pc->mem_root) partition_element(pc->current_partition);
  if (sub_p_elem == nullptr) return true;

  if (check_string_char_length(to_lex_cstring(name), "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), name);
    return true;
  }

  sub_p_elem->partition_name = name;

  part_info->use_default_subpartitions = false;
  part_info->use_default_num_subpartitions = false;
  pc->count_curr_subparts++;

  Partition_parse_context subpart_pc(pc->thd, part_info, pc->current_partition,
                                     sub_p_elem,
                                     pc->is_add_or_reorganize_partition);

  if (options != nullptr) {
    for (auto option : *options) {
      if (option->contextualize(&subpart_pc)) return true;
    }
    if (sub_p_elem->engine_type != nullptr)
      part_info->default_engine_type = sub_p_elem->engine_type;
  }
  if (pc->current_partition->subpartitions.push_back(sub_p_elem)) return true;

  return false;
}

bool PT_part_value_item_max::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  if (pc->part_info->part_type == partition_type::LIST) {
    error(pc, pos, ER_THD(pc->thd, ER_MAXVALUE_IN_VALUES_IN));
    return true;
  }
  if (pc->add_max_value()) return true;

  return false;
}

bool PT_part_value_item_expr::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || expr->itemize(pc, &expr)) return true;

  if (!pc->thd->lex->safe_to_cache_query) {
    error(pc, pos, ER_THD(pc->thd, ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR));
    return true;
  }
  if (pc->add_column_list_value(pc->thd, expr)) return true;

  return false;
}

bool PT_part_value_item_list_paren::contextualize(Partition_parse_context *pc) {
  pc->part_info->print_debug("( part_value_item_list_paren", nullptr);
  /* Initialisation code needed for each list of value expressions */
  if (!(pc->part_info->part_type == partition_type::LIST &&
        pc->part_info->num_columns == 1U) &&
      pc->init_column_part()) {
    return true;
  }

  for (auto value : *values) {
    if (value->contextualize(pc)) return true;
  }

  pc->part_info->print_debug(") part_value_item_list_paren", nullptr);
  if (pc->part_info->num_columns == 0)
    pc->part_info->num_columns = pc->curr_list_object;
  if (pc->part_info->num_columns != pc->curr_list_object) {
    /*
      All value items lists must be of equal length, in some cases
      which is covered by the above if-statement we don't know yet
      how many columns is in the partition so the assignment above
      ensures that we only report errors when we know we have an
      error.
    */
    pc->part_info->print_debug("Kilroy I", nullptr);
    error(pc, paren_pos, ER_THD(pc->thd, ER_PARTITION_COLUMN_LIST_ERROR));
    return true;
  }
  pc->curr_list_object = 0;
  return false;
}

bool PT_part_values_in_item::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || item->contextualize(pc)) return true;

  partition_info *const part_info = pc->part_info;
  part_info->print_debug("part_values_in: part_value_item_list_paren", nullptr);

  if (part_info->num_columns != 1U) {
    if (!pc->is_add_or_reorganize_partition || part_info->num_columns == 0 ||
        part_info->num_columns > MAX_REF_PARTS) {
      part_info->print_debug("Kilroy III", nullptr);
      error(pc, pos, ER_THD(pc->thd, ER_PARTITION_COLUMN_LIST_ERROR));
      return true;
    }
    /*
      Reorganize the current large array into a list of small
      arrays with one entry in each array. This can happen
      in the first partition of an ALTER TABLE statement where
      we ADD or REORGANIZE partitions. Also can only happen
      for LIST [COLUMNS] partitions.
    */
    if (pc->reorganize_into_single_field_col_val()) return true;
  }
  return false;
}

bool PT_part_values_in_list::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  for (auto item : *list) {
    if (item->contextualize(pc)) return true;
  }

  if (pc->part_info->num_columns < 2U) {
    error(pc, pos, ER_THD(pc->thd, ER_ROW_SINGLE_PARTITION_FIELD_ERROR));
    return true;
  }
  return false;
}

bool PT_part_definition::contextualize(Partition_parse_context *pc) {
  DBUG_ASSERT(pc->current_partition == nullptr &&
              pc->curr_part_elem == nullptr);

  if (super::contextualize(pc)) return true;

  partition_info *const part_info = pc->part_info;

  auto *const curr_part = new (pc->thd->mem_root) partition_element();
  if (curr_part == nullptr) return true;

  Partition_parse_context ppc(pc->thd, part_info, curr_part, curr_part,
                              pc->is_add_or_reorganize_partition);

  if (!curr_part || part_info->partitions.push_back(curr_part)) return true;
  curr_part->part_state = PART_NORMAL;
  part_info->use_default_partitions = false;
  part_info->use_default_num_partitions = false;

  if (check_string_char_length(to_lex_cstring(name), "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), name.str);
    return true;
  }

  curr_part->partition_name = name.str;

  switch (type) {
    case partition_type::HASH: {
      if (part_info->part_type == partition_type::NONE)
        part_info->part_type = partition_type::HASH;
      else if (part_info->part_type == partition_type::RANGE) {
        errorf(&ppc, pos, ER_THD(pc->thd, ER_PARTITION_REQUIRES_VALUES_ERROR),
               "RANGE", "LESS THAN");
        return true;
      } else if (part_info->part_type == partition_type::LIST) {
        errorf(&ppc, pos, ER_THD(pc->thd, ER_PARTITION_REQUIRES_VALUES_ERROR),
               "LIST", "IN");
        return true;
      }
    } break;
    case partition_type::RANGE: {
      if (part_info->part_type == partition_type::NONE)
        part_info->part_type = partition_type::RANGE;
      else if (part_info->part_type != partition_type::RANGE) {
        my_error(ER_PARTITION_WRONG_VALUES_ERROR, MYF(0), "RANGE", "LESS THAN");
        return true;
      }

      if (opt_part_values == nullptr)  // MAX_VALUE_SYM
      {
        if (part_info->num_columns && part_info->num_columns != 1U) {
          part_info->print_debug("Kilroy II", nullptr);
          error(&ppc, values_pos,
                ER_THD(pc->thd, ER_PARTITION_COLUMN_LIST_ERROR));
          return true;
        } else
          part_info->num_columns = 1U;
        if (ppc.init_column_part() || ppc.add_max_value()) return true;
      } else if (opt_part_values->contextualize(&ppc))
        return true;
    } break;
    case partition_type::LIST: {
      if (part_info->part_type == partition_type::NONE)
        part_info->part_type = partition_type::LIST;
      else if (part_info->part_type != partition_type::LIST) {
        my_error(ER_PARTITION_WRONG_VALUES_ERROR, MYF(0), "LIST", "IN");
        return true;
      }

      if (opt_part_values->contextualize(&ppc)) return true;
    } break;
    default:
      DBUG_ASSERT(false);
      error(&ppc, pos, ER_THD(pc->thd, ER_UNKNOWN_ERROR));
      return true;
  }

  if (opt_part_options != nullptr) {
    for (auto option : *opt_part_options) {
      if (option->contextualize(&ppc)) return true;
    }
    if (curr_part->engine_type != nullptr)
      part_info->default_engine_type = curr_part->engine_type;
  }

  if (opt_sub_partitions == nullptr) {
    if (part_info->num_subparts != 0 && !part_info->use_default_subpartitions) {
      /*
        We come here when we have defined subpartitions on the first
        partition but not on all the subsequent partitions.
      */
      error(&ppc, sub_partitions_pos,
            ER_THD(pc->thd, ER_PARTITION_WRONG_NO_SUBPART_ERROR));
      return true;
    }
  } else {
    for (auto subpartition : *opt_sub_partitions) {
      if (subpartition->contextualize(&ppc)) return true;
    }

    if (part_info->num_subparts != 0) {
      if (part_info->num_subparts != ppc.count_curr_subparts) {
        error(&ppc, sub_partitions_pos,
              ER_THD(pc->thd, ER_PARTITION_WRONG_NO_SUBPART_ERROR));
        return true;
      }
    } else if (ppc.count_curr_subparts > 0) {
      if (part_info->partitions.elements > 1) {
        error(&ppc, sub_partitions_pos,
              ER_THD(pc->thd, ER_PARTITION_WRONG_NO_SUBPART_ERROR));
        return true;
      }
      part_info->num_subparts = ppc.count_curr_subparts;
    }
  }
  return false;
}

bool PT_sub_partition_by_hash::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || hash->itemize(pc, &hash)) return true;

  partition_info *const part_info = pc->part_info;

  part_info->subpart_type = partition_type::HASH;
  part_info->linear_hash_ind = is_linear;

  LEX *const lex = pc->thd->lex;
  if (!lex->safe_to_cache_query) {
    error(pc, hash_pos, ER_THD(pc->thd, ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR));
    return true;
  }
  lex->safe_to_cache_query = true;

  /* TODO: remove const_cast */
  if (part_info->set_part_expr(const_cast<char *>(hash_pos.cpp.start), hash,
                               const_cast<char *>(hash_pos.cpp.end), true))
    return true;

  if (opt_num_subparts > 0) {
    part_info->num_subparts = opt_num_subparts;
    part_info->use_default_num_subpartitions = false;
  }
  return false;
}

bool PT_sub_partition_by_key::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  partition_info *const part_info = pc->part_info;

  part_info->subpart_type = partition_type::HASH;
  part_info->list_of_subpart_fields = true;

  part_info->linear_hash_ind = is_linear;
  part_info->key_algorithm = key_algo;

  if (field_names->elements > MAX_REF_PARTS) {
    my_error(ER_TOO_MANY_PARTITION_FUNC_FIELDS_ERROR, MYF(0),
             "list of subpartition fields");
    return true;
  }
  part_info->subpart_field_list = *field_names;

  if (opt_num_subparts > 0) {
    part_info->num_subparts = opt_num_subparts;
    part_info->use_default_num_subpartitions = false;
  }
  return false;
}

bool PT_part_type_def::set_part_field_list(Partition_parse_context *pc,
                                           List<char> *list) {
  if (list->elements > MAX_REF_PARTS) {
    my_error(ER_TOO_MANY_PARTITION_FUNC_FIELDS_ERROR, MYF(0),
             "list of partition fields");
    return true;
  }
  pc->part_info->num_columns = list->elements;
  pc->part_info->part_field_list = *list;
  return false;
}

bool PT_part_type_def::itemize_part_expr(Partition_parse_context *pc,
                                         const POS &pos, Item **item) {
  if ((*item)->itemize(pc, item)) return true;

  if (!pc->thd->lex->safe_to_cache_query) {
    error(pc, pos, ER_THD(pc->thd, ER_WRONG_EXPR_IN_PARTITION_FUNC_ERROR));
    return true;
  }
  pc->thd->lex->safe_to_cache_query = true;

  if (pc->part_info->set_part_expr(const_cast<char *>(pos.cpp.start), *item,
                                   const_cast<char *>(pos.cpp.end), false))
    return true;

  return false;
}

bool PT_part_type_def_key::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  pc->part_info->part_type = partition_type::HASH;
  pc->part_info->column_list = false;
  pc->part_info->key_algorithm = key_algo;
  pc->part_info->linear_hash_ind = is_linear;
  pc->part_info->list_of_part_fields = true;

  if (opt_columns != nullptr && set_part_field_list(pc, opt_columns))
    return true;

  return false;
}

bool PT_part_type_def_hash::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || itemize_part_expr(pc, expr_pos, &expr))
    return true;

  pc->part_info->part_type = partition_type::HASH;
  pc->part_info->column_list = false;
  pc->part_info->linear_hash_ind = is_linear;
  pc->part_info->num_columns = 1;

  return false;
}

bool PT_part_type_def_range_expr::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || itemize_part_expr(pc, expr_pos, &expr))
    return true;

  pc->part_info->part_type = partition_type::RANGE;
  pc->part_info->column_list = false;
  pc->part_info->num_columns = 1;

  return false;
}

bool PT_part_type_def_range_columns::contextualize(
    Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  pc->part_info->part_type = partition_type::RANGE;
  pc->part_info->column_list = true;
  pc->part_info->list_of_part_fields = true;

  return set_part_field_list(pc, columns);
}

bool PT_part_type_def_list_expr::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc) || itemize_part_expr(pc, expr_pos, &expr))
    return true;

  pc->part_info->part_type = partition_type::LIST;
  pc->part_info->column_list = false;
  pc->part_info->num_columns = 1;

  return false;
}

bool PT_part_type_def_list_columns::contextualize(Partition_parse_context *pc) {
  if (super::contextualize(pc)) return true;

  pc->part_info->part_type = partition_type::LIST;
  pc->part_info->column_list = true;
  pc->part_info->list_of_part_fields = true;

  return set_part_field_list(pc, columns);
}

bool PT_partition::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  Partition_parse_context part_pc(pc->thd, &part_info, false);
  if (part_type_def->contextualize(&part_pc)) return true;

  if (opt_num_parts != 0) {
    part_info.num_parts = opt_num_parts;
    part_info.use_default_num_partitions = false;
  }

  if (opt_sub_part != nullptr && opt_sub_part->contextualize(&part_pc))
    return true;

  if (part_defs == nullptr) {
    if (part_info.part_type == partition_type::RANGE) {
      my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), "RANGE");
      return true;
    } else if (part_info.part_type == partition_type::LIST) {
      my_error(ER_PARTITIONS_MUST_BE_DEFINED_ERROR, MYF(0), "LIST");
      return true;
    }
  } else {
    for (auto part_def : *part_defs) {
      if (part_def->contextualize(&part_pc)) return true;
    }
    uint count_curr_parts = part_info.partitions.elements;

    if (part_info.num_parts != 0) {
      if (part_info.num_parts != count_curr_parts) {
        error(&part_pc, part_defs_pos,
              ER_THD(pc->thd, ER_PARTITION_WRONG_NO_PART_ERROR));
        return true;
      }
    } else if (count_curr_parts > 0)
      part_info.num_parts = count_curr_parts;
  }
  return false;
}
