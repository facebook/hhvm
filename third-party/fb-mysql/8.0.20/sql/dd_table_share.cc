/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd_table_share.h"

#include "my_config.h"

#include <string.h>
#include <algorithm>
#include <string>
#include <type_traits>

#include "lex_string.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_compare.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin.h"
#include "mysql/psi/psi_base.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "nullable.h"
#include "sql/dd/collection.h"
#include "sql/dd/dd_table.h"       // dd::FIELD_NAME_SEPARATOR_CHAR
#include "sql/dd/dd_tablespace.h"  // dd::get_tablespace_name
// TODO: Avoid exposing dd/impl headers in public files.
#include "sql/dd/impl/utils.h"  // dd::eat_str
#include "sql/dd/properties.h"  // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/types/check_constraint.h"     // dd::Check_constraint
#include "sql/dd/types/column.h"               // dd::enum_column_types
#include "sql/dd/types/column_type_element.h"  // dd::Column_type_element
#include "sql/dd/types/foreign_key.h"
#include "sql/dd/types/foreign_key_element.h"  // dd::Foreign_key_element
#include "sql/dd/types/index.h"                // dd::Index
#include "sql/dd/types/index_element.h"        // dd::Index_element
#include "sql/dd/types/partition.h"            // dd::Partition
#include "sql/dd/types/partition_value.h"      // dd::Partition_value
#include "sql/dd/types/table.h"                // dd::Table
#include "sql/default_values.h"  // prepare_default_value_buffer...
#include "sql/error_handler.h"   // Internal_error_handler
#include "sql/field.h"
#include "sql/gis/srid.h"
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/log.h"
#include "sql/partition_element.h"  // partition_element
#include "sql/partition_info.h"     // partition_info
#include "sql/sql_bitmap.h"
#include "sql/sql_check_constraint.h"  // Sql_check_constraint_share_list
#include "sql/sql_class.h"             // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_list.h"
#include "sql/sql_partition.h"  // generate_partition_syntax
#include "sql/sql_plugin.h"     // plugin_unlock
#include "sql/sql_plugin_ref.h"
#include "sql/sql_table.h"  // primary_key_name
#include "sql/strfunc.h"    // lex_cstring_handle
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "typelib.h"

namespace histograms {
class Histogram;
}  // namespace histograms

enum_field_types dd_get_old_field_type(dd::enum_column_types type) {
  switch (type) {
    case dd::enum_column_types::DECIMAL:
      return MYSQL_TYPE_DECIMAL;

    case dd::enum_column_types::TINY:
      return MYSQL_TYPE_TINY;

    case dd::enum_column_types::SHORT:
      return MYSQL_TYPE_SHORT;

    case dd::enum_column_types::LONG:
      return MYSQL_TYPE_LONG;

    case dd::enum_column_types::FLOAT:
      return MYSQL_TYPE_FLOAT;

    case dd::enum_column_types::DOUBLE:
      return MYSQL_TYPE_DOUBLE;

    case dd::enum_column_types::TYPE_NULL:
      return MYSQL_TYPE_NULL;

    case dd::enum_column_types::TIMESTAMP:
      return MYSQL_TYPE_TIMESTAMP;

    case dd::enum_column_types::LONGLONG:
      return MYSQL_TYPE_LONGLONG;

    case dd::enum_column_types::INT24:
      return MYSQL_TYPE_INT24;

    case dd::enum_column_types::DATE:
      return MYSQL_TYPE_DATE;

    case dd::enum_column_types::TIME:
      return MYSQL_TYPE_TIME;

    case dd::enum_column_types::DATETIME:
      return MYSQL_TYPE_DATETIME;

    case dd::enum_column_types::YEAR:
      return MYSQL_TYPE_YEAR;

    case dd::enum_column_types::NEWDATE:
      return MYSQL_TYPE_NEWDATE;

    case dd::enum_column_types::VARCHAR:
      return MYSQL_TYPE_VARCHAR;

    case dd::enum_column_types::BIT:
      return MYSQL_TYPE_BIT;

    case dd::enum_column_types::TIMESTAMP2:
      return MYSQL_TYPE_TIMESTAMP2;

    case dd::enum_column_types::DATETIME2:
      return MYSQL_TYPE_DATETIME2;

    case dd::enum_column_types::TIME2:
      return MYSQL_TYPE_TIME2;

    case dd::enum_column_types::NEWDECIMAL:
      return MYSQL_TYPE_NEWDECIMAL;

    case dd::enum_column_types::ENUM:
      return MYSQL_TYPE_ENUM;

    case dd::enum_column_types::SET:
      return MYSQL_TYPE_SET;

    case dd::enum_column_types::TINY_BLOB:
      return MYSQL_TYPE_TINY_BLOB;

    case dd::enum_column_types::MEDIUM_BLOB:
      return MYSQL_TYPE_MEDIUM_BLOB;

    case dd::enum_column_types::LONG_BLOB:
      return MYSQL_TYPE_LONG_BLOB;

    case dd::enum_column_types::BLOB:
      return MYSQL_TYPE_BLOB;

    case dd::enum_column_types::VAR_STRING:
      return MYSQL_TYPE_VAR_STRING;

    case dd::enum_column_types::STRING:
      return MYSQL_TYPE_STRING;

    case dd::enum_column_types::GEOMETRY:
      return MYSQL_TYPE_GEOMETRY;

    case dd::enum_column_types::JSON:
      return MYSQL_TYPE_JSON;

    default:
      DBUG_ASSERT(!"Should not hit here"); /* purecov: deadcode */
  }

  return MYSQL_TYPE_LONG;
}

/** For enum in dd::Index */
static enum ha_key_alg dd_get_old_index_algorithm_type(
    dd::Index::enum_index_algorithm type) {
  switch (type) {
    case dd::Index::IA_SE_SPECIFIC:
      return HA_KEY_ALG_SE_SPECIFIC;

    case dd::Index::IA_BTREE:
      return HA_KEY_ALG_BTREE;

    case dd::Index::IA_RTREE:
      return HA_KEY_ALG_RTREE;

    case dd::Index::IA_HASH:
      return HA_KEY_ALG_HASH;

    case dd::Index::IA_FULLTEXT:
      return HA_KEY_ALG_FULLTEXT;

    default:
      DBUG_ASSERT(!"Should not hit here"); /* purecov: deadcode */
  }

  return HA_KEY_ALG_SE_SPECIFIC;
}

/*
  Check if the given key_part is suitable to be promoted as part of
  primary key.
*/
bool is_suitable_for_primary_key(KEY_PART_INFO *key_part, Field *table_field) {
  // Index on virtual generated columns is not allowed to be PK
  // even when the conditions below are true, so this case must be
  // rejected here.
  if (table_field->is_virtual_gcol()) return false;

  /*
    If the key column is of NOT NULL BLOB type, then it
    will definitly have key prefix. And if key part prefix size
    is equal to the BLOB column max size, then we can promote
    it to primary key.
   */
  if (!table_field->is_nullable() && table_field->type() == MYSQL_TYPE_BLOB &&
      table_field->field_length == key_part->length)
    return true;

  if (table_field->is_nullable() ||
      table_field->key_length() != key_part->length)
    return false;

  return true;
}

/**
  Finalize preparation of TABLE_SHARE from dd::Table object by filling
  in remaining info about columns and keys.

  This code similar to code in open_binary_frm(). Can be re-written
  independent to other efforts later.
*/

static bool prepare_share(THD *thd, TABLE_SHARE *share,
                          const dd::Table *table_def) {
  my_bitmap_map *bitmaps;
  bool use_hash;
  handler *handler_file = nullptr;

  // Mark 'system' tables (tables with one row) to help the Optimizer.
  share->system =
      ((share->max_rows == 1) && (share->min_rows == 1) && (share->keys == 0));

  bool use_extended_sk = ha_check_storage_engine_flag(
      share->db_type(), HTON_SUPPORTS_EXTENDED_KEYS);
  // Setup name_hash for quick look-up
  use_hash = share->fields >= MAX_FIELDS_BEFORE_HASH;
  if (use_hash) {
    Field **field_ptr = share->field;
    share->name_hash = new collation_unordered_map<std::string, Field **>(
        system_charset_info, PSI_INSTRUMENT_ME);
    share->name_hash->reserve(share->fields);

    for (uint i = 0; i < share->fields; i++, field_ptr++) {
      share->name_hash->emplace((*field_ptr)->field_name, field_ptr);
    }
  }

  share->m_histograms =
      new malloc_unordered_map<uint, const histograms::Histogram *>(
          PSI_INSTRUMENT_ME);

  // Setup other fields =====================================================
  /* Allocate handler */
  if (!(handler_file = get_new_handler(share, (share->m_part_info != nullptr),
                                       &share->mem_root, share->db_type()))) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), share->path.str,
             "Failed to initialize handler.");
    return true;
  }

  if (handler_file->set_ha_share_ref(&share->ha_share)) {
    my_error(ER_INVALID_DD_OBJECT, MYF(0), share->path.str, "");
    return true;
  }
  share->db_low_byte_first = handler_file->low_byte_first();

  /* Fix key->name and key_part->field */
  if (share->keys) {
    KEY *keyinfo;
    KEY_PART_INFO *key_part;
    uint primary_key = (uint)(
        find_type(primary_key_name, &share->keynames, FIND_TYPE_NO_PREFIX) - 1);
    longlong ha_option = handler_file->ha_table_flags();
    keyinfo = share->key_info;
    key_part = keyinfo->key_part;

    /*
      The following if-else is here for MyRocks:
      set share->primary_key as early as possible, because the return value
      of ha_rocksdb::index_flags(key, ...) (HA_KEYREAD_ONLY bit in particular)
      depends on whether the key is the primary key.
    */
    if (primary_key < MAX_KEY && share->keys_in_use.is_set(primary_key)) {
      share->primary_key = primary_key;
    } else {
      share->primary_key = MAX_KEY;
    }

    dd::Table::Index_collection::const_iterator idx_it(
        table_def->indexes().begin());

    for (uint key = 0; key < share->keys; key++, keyinfo++) {
      /*
        Skip hidden dd::Index objects so idx_it is in sync with key index
        and keyinfo pointer.
      */
      while ((*idx_it)->is_hidden()) {
        ++idx_it;
        continue;
      }

      uint usable_parts = 0;
      keyinfo->name = share->keynames.type_names[key];

      /* Check that fulltext and spatial keys have correct algorithm set. */
      DBUG_ASSERT(!(share->key_info[key].flags & HA_FULLTEXT) ||
                  share->key_info[key].algorithm == HA_KEY_ALG_FULLTEXT);
      DBUG_ASSERT(!(share->key_info[key].flags & HA_SPATIAL) ||
                  share->key_info[key].algorithm == HA_KEY_ALG_RTREE);

      if (primary_key >= MAX_KEY && (keyinfo->flags & HA_NOSAME)) {
        /*
           If the UNIQUE key doesn't have NULL columns and is not a part key
           declare this as a primary key.
        */
        primary_key = key;
        for (uint i = 0; i < keyinfo->user_defined_key_parts; i++) {
          Field *table_field = key_part[i].field;

          if (is_suitable_for_primary_key(&key_part[i], table_field) == false) {
            primary_key = MAX_KEY;
            break;
          }
        }

        /*
          Check that dd::Index::is_candidate_key() used by SEs works in
          the same way as above call to is_suitable_for_primary_key().
        */
        DBUG_ASSERT((primary_key == key) == (*idx_it)->is_candidate_key());

        /*
          The following is here for MyRocks. See the comment above
          about "set share->primary_key as early as possible"
        */
        if (primary_key < MAX_KEY && share->keys_in_use.is_set(primary_key)) {
          share->primary_key = primary_key;
        }
      }

      dd::Index::Index_elements::const_iterator idx_el_it(
          (*idx_it)->elements().begin());

      for (uint i = 0; i < keyinfo->user_defined_key_parts; key_part++, i++) {
        /*
          Skip hidden Index_element objects so idx_el_it is in sync with
          i and key_part pointer.
        */
        while ((*idx_el_it)->is_hidden()) {
          ++idx_el_it;
          continue;
        }

        Field *field = key_part->field;

        key_part->type = field->key_type();
        if (field->is_nullable()) {
          key_part->null_offset = field->null_offset(share->default_values);
          key_part->null_bit = field->null_bit;
          key_part->store_length += HA_KEY_NULL_LENGTH;
          keyinfo->flags |= HA_NULL_PART_KEY;
          keyinfo->key_length += HA_KEY_NULL_LENGTH;
        }
        if (field->type() == MYSQL_TYPE_BLOB ||
            field->real_type() == MYSQL_TYPE_VARCHAR ||
            field->type() == MYSQL_TYPE_GEOMETRY) {
          key_part->store_length += HA_KEY_BLOB_LENGTH;
          if (i + 1 <= keyinfo->user_defined_key_parts)
            keyinfo->key_length += HA_KEY_BLOB_LENGTH;
        }
        key_part->init_flags();

        if (field->is_virtual_gcol()) keyinfo->flags |= HA_VIRTUAL_GEN_KEY;

        setup_key_part_field(share, handler_file, primary_key, keyinfo, key, i,
                             &usable_parts, true);

        field->flags |= PART_KEY_FLAG;
        if (key == primary_key) {
          field->flags |= PRI_KEY_FLAG;
          /*
            "if (ha_option & HA_PRIMARY_KEY_IN_READ_INDEX)" ... was moved below
            for MyRocks
          */
        }
        if (field->key_length() != key_part->length) {
#ifndef TO_BE_DELETED_ON_PRODUCTION
          if (field->type() == MYSQL_TYPE_NEWDECIMAL) {
            /*
               Fix a fatal error in decimal key handling that causes crashes
               on Innodb. We fix it by reducing the key length so that
               InnoDB never gets a too big key when searching.
               This allows the end user to do an ALTER TABLE to fix the
               error.
               */
            keyinfo->key_length -= (key_part->length - field->key_length());
            key_part->store_length -=
                (uint16)(key_part->length - field->key_length());
            key_part->length = (uint16)field->key_length();
            LogErr(ERROR_LEVEL, ER_TABLE_WRONG_KEY_DEFINITION,
                   share->table_name.str, share->table_name.str);
            push_warning_printf(thd, Sql_condition::SL_WARNING,
                                ER_CRASHED_ON_USAGE,
                                "Found wrong key definition in %s; "
                                "Please do \"ALTER TABLE `%s` FORCE\" to fix "
                                "it!",
                                share->table_name.str, share->table_name.str);
            share->crashed = true;  // Marker for CHECK TABLE
            continue;
          }
#endif
          key_part->key_part_flag |= HA_PART_KEY_SEG;
        }

        /*
          Check that dd::Index_element::is_prefix() used by SEs works in
          the same way as code which sets HA_PART_KEY_SEG flag.
        */
        DBUG_ASSERT(
            (*idx_el_it)->is_prefix() ==
            static_cast<bool>(key_part->key_part_flag & HA_PART_KEY_SEG));
        ++idx_el_it;
      }

      /*
        KEY::flags is fully set-up at this point so we can copy it to
        KEY::actual_flags.
      */
      keyinfo->actual_flags = keyinfo->flags;

      if (use_extended_sk && primary_key < MAX_KEY && key &&
          !(keyinfo->flags & HA_NOSAME))
        key_part +=
            add_pk_parts_to_sk(keyinfo, key, share->key_info, primary_key,
                               share, handler_file, &usable_parts);

      /* Skip unused key parts if they exist */
      key_part += keyinfo->unused_key_parts;

      keyinfo->usable_key_parts = usable_parts;  // Filesort

      share->max_key_length =
          std::max(share->max_key_length,
                   keyinfo->key_length + keyinfo->user_defined_key_parts);
      share->total_key_length += keyinfo->key_length;
      /*
         MERGE tables do not have unique indexes. But every key could be
         an unique index on the underlying MyISAM table. (Bug #10400)
         */
      if ((keyinfo->flags & HA_NOSAME) ||
          (ha_option & HA_ANY_INDEX_MAY_BE_UNIQUE))
        share->max_unique_length =
            std::max(share->max_unique_length, keyinfo->key_length);

      ++idx_it;
    }

    /*
      The next call is here for MyRocks:  Now, we have filled in field and key
      definitions, give the storage engine a chance to adjust its properties.
      MyRocks may (and typically does) adjust HA_PRIMARY_KEY_IN_READ_INDEX
      flag in this call.
    */
    if (handler_file->init_with_fields()) return true;

    if (primary_key < MAX_KEY &&
        (handler_file->ha_table_flags() & HA_PRIMARY_KEY_IN_READ_INDEX)) {
      keyinfo = &share->key_info[primary_key];
      key_part = keyinfo->key_part;
      for (uint i = 0; i < keyinfo->user_defined_key_parts; key_part++, i++) {
        Field *field = key_part->field;
        /*
          If this field is part of the primary key and all keys contains
          the primary key, then we can use any key to find this column
        */
        if (field->key_length() == key_part->length &&
            !(field->flags & BLOB_FLAG))
          field->part_of_key = share->keys_in_use;
        if (field->part_of_sortkey.is_set(primary_key))
          field->part_of_sortkey = share->keys_in_use;
      }
    }

    if (share->primary_key != MAX_KEY) {
      /*
         If we are using an integer as the primary key then allow the user to
         refer to it as '_rowid'
         */
      if (share->key_info[primary_key].user_defined_key_parts == 1) {
        Field *field = share->key_info[primary_key].key_part[0].field;
        if (field && field->result_type() == INT_RESULT) {
          /* note that fieldnr here (and rowid_field_offset) starts from 1 */
          share->rowid_field_offset =
              (share->key_info[primary_key].key_part[0].fieldnr);
        }
      }
    }
  } else
    share->primary_key = MAX_KEY;
  destroy(handler_file);

  if (share->found_next_number_field) {
    Field *reg_field = *share->found_next_number_field;
    /* Check that the auto-increment column is the first column of some key. */
    if ((int)(share->next_number_index = (uint)find_ref_key(
                  share->key_info, share->keys, share->default_values,
                  reg_field, &share->next_number_key_offset,
                  &share->next_number_keypart)) < 0) {
      my_error(ER_INVALID_DD_OBJECT, MYF(0), share->path.str,
               "Wrong field definition.");
      return true;
    } else
      reg_field->flags |= AUTO_INCREMENT_FLAG;
  }

  if (share->blob_fields) {
    Field **ptr;
    uint k, *save;

    /* Store offsets to blob fields to find them fast */
    if (!(share->blob_field = save = (uint *)share->mem_root.Alloc(
              (uint)(share->blob_fields * sizeof(uint)))))
      return true;  // OOM error message already reported
    for (k = 0, ptr = share->field; *ptr; ptr++, k++) {
      if ((*ptr)->flags & BLOB_FLAG || (*ptr)->is_array()) (*save++) = k;
    }
  }

  share->column_bitmap_size = bitmap_buffer_size(share->fields);
  if (!(bitmaps = (my_bitmap_map *)share->mem_root.Alloc(
            share->column_bitmap_size))) {
    // OOM error message already reported
    return true; /* purecov: inspected */
  }
  bitmap_init(&share->all_set, bitmaps, share->fields);
  bitmap_set_all(&share->all_set);

  return false;
}

/** Fill tablespace name from dd::Tablespace. */
static bool fill_tablespace_from_dd(THD *thd, TABLE_SHARE *share,
                                    const dd::Table *tab_obj) {
  DBUG_TRACE;

  return dd::get_tablespace_name<dd::Table>(thd, tab_obj, &share->tablespace,
                                            &share->mem_root);
}

/**
  Convert row format value used in DD to corresponding value in old
  row_type enum.
*/

static row_type dd_get_old_row_format(dd::Table::enum_row_format new_format) {
  switch (new_format) {
    case dd::Table::RF_FIXED:
      return ROW_TYPE_FIXED;
    case dd::Table::RF_DYNAMIC:
      return ROW_TYPE_DYNAMIC;
    case dd::Table::RF_COMPRESSED:
      return ROW_TYPE_COMPRESSED;
    case dd::Table::RF_REDUNDANT:
      return ROW_TYPE_REDUNDANT;
    case dd::Table::RF_COMPACT:
      return ROW_TYPE_COMPACT;
    case dd::Table::RF_PAGED:
      return ROW_TYPE_PAGED;
    default:
      DBUG_ASSERT(0);
      break;
  }
  return ROW_TYPE_FIXED;
}

/** Fill TABLE_SHARE from dd::Table object */
static bool fill_share_from_dd(THD *thd, TABLE_SHARE *share,
                               const dd::Table *tab_obj) {
  const dd::Properties &table_options = tab_obj->options();

  // Secondary storage engine.
  if (table_options.exists("secondary_engine")) {
    table_options.get("secondary_engine", &share->secondary_engine,
                      &share->mem_root);
  } else {
    // If no secondary storage engine is set, the share cannot
    // represent a table in a secondary engine.
    DBUG_ASSERT(!share->is_secondary_engine());
  }

  // Read table engine type
  LEX_CSTRING engine_name = lex_cstring_handle(tab_obj->engine());
  if (share->is_secondary_engine())
    engine_name = {share->secondary_engine.str, share->secondary_engine.length};

  plugin_ref tmp_plugin = ha_resolve_by_name_raw(thd, engine_name);
  if (tmp_plugin) {
#ifndef DBUG_OFF
    handlerton *hton = plugin_data<handlerton *>(tmp_plugin);
#endif

    DBUG_ASSERT(hton && ha_storage_engine_is_enabled(hton));
    DBUG_ASSERT(!ha_check_storage_engine_flag(hton, HTON_NOT_USER_SELECTABLE));

    plugin_unlock(nullptr, share->db_plugin);
    share->db_plugin = my_plugin_lock(nullptr, &tmp_plugin);
  } else {
    my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), engine_name.str);
    return true;
  }

  // Set temporarily a good value for db_low_byte_first.
  DBUG_ASSERT(ha_legacy_type(share->db_type()) != DB_TYPE_ISAM);
  share->db_low_byte_first = true;

  // Read other table options
  uint64 option_value = 0;
  bool bool_opt = false;

  // Max rows
  if (table_options.exists("max_rows"))
    table_options.get("max_rows", &share->max_rows);

  // Min rows
  if (table_options.exists("min_rows"))
    table_options.get("min_rows", &share->min_rows);

  // Options from HA_CREATE_INFO::table_options/TABLE_SHARE::db_create_options.
  share->db_create_options = 0;

  table_options.get("pack_record", &bool_opt);
  if (bool_opt) share->db_create_options |= HA_OPTION_PACK_RECORD;

  if (table_options.exists("pack_keys")) {
    table_options.get("pack_keys", &bool_opt);
    share->db_create_options |=
        bool_opt ? HA_OPTION_PACK_KEYS : HA_OPTION_NO_PACK_KEYS;
  }

  if (table_options.exists("checksum")) {
    table_options.get("checksum", &bool_opt);
    if (bool_opt) share->db_create_options |= HA_OPTION_CHECKSUM;
  }

  if (table_options.exists("delay_key_write")) {
    table_options.get("delay_key_write", &bool_opt);
    if (bool_opt) share->db_create_options |= HA_OPTION_DELAY_KEY_WRITE;
  }

  if (table_options.exists("stats_persistent")) {
    table_options.get("stats_persistent", &bool_opt);
    share->db_create_options |=
        bool_opt ? HA_OPTION_STATS_PERSISTENT : HA_OPTION_NO_STATS_PERSISTENT;
  }

  share->db_options_in_use = share->db_create_options;

  // Average row length

  if (table_options.exists("avg_row_length")) {
    table_options.get("avg_row_length", &option_value);
    share->avg_row_length = static_cast<ulong>(option_value);
  }

  // Collation ID
  share->table_charset = dd_get_mysql_charset(tab_obj->collation_id());
  if (!share->table_charset) {
    // Unknown collation
    if (use_mb(default_charset_info)) {
      /* Warn that we may be changing the size of character columns */
      LogErr(WARNING_LEVEL, ER_INVALID_CHARSET_AND_DEFAULT_IS_MB,
             share->path.str);
    }
    share->table_charset = default_charset_info;
  }

  // Row type. First one really used by the storage engine.
  share->real_row_type = dd_get_old_row_format(tab_obj->row_format());

  // Then one which was explicitly specified by user for this table.
  if (table_options.exists("row_type")) {
    table_options.get("row_type", &option_value);
    share->row_type =
        dd_get_old_row_format((dd::Table::enum_row_format)option_value);
  } else
    share->row_type = ROW_TYPE_DEFAULT;

  // Stats_sample_pages
  if (table_options.exists("stats_sample_pages"))
    table_options.get("stats_sample_pages", &share->stats_sample_pages);

  // Stats_auto_recalc
  if (table_options.exists("stats_auto_recalc")) {
    table_options.get("stats_auto_recalc", &option_value);
    share->stats_auto_recalc = (enum_stats_auto_recalc)option_value;
  }

  // mysql version
  share->mysql_version = tab_obj->mysql_version_id();

  // TODO-POST-MERGE-TO-TRUNK: Initialize new field
  // share->last_checked_for_upgrade? Or access tab_obj directly where
  // needed?

  // key block size
  table_options.get("key_block_size", &share->key_block_size);

  // Prepare the default_value buffer.
  if (prepare_default_value_buffer_and_table_share(thd, *tab_obj, share))
    return true;

  // Storage media flags
  if (table_options.exists("storage")) {
    uint32 storage_option_value = 0;
    table_options.get("storage", &storage_option_value);
    share->default_storage_media =
        static_cast<ha_storage_media>(storage_option_value);
  } else
    share->default_storage_media = HA_SM_DEFAULT;

  // Read tablespace name
  if (fill_tablespace_from_dd(thd, share, tab_obj)) return true;

  // Read comment
  dd::String_type comment = tab_obj->comment();
  if (comment.length()) {
    share->comment.str =
        strmake_root(&share->mem_root, comment.c_str(), comment.length() + 1);
    share->comment.length = comment.length();
  }

  // Read Connection strings
  if (table_options.exists("connection_string"))
    table_options.get("connection_string", &share->connect_string,
                      &share->mem_root);

  // Read Compress string
  if (table_options.exists("compress"))
    table_options.get("compress", &share->compress, &share->mem_root);

  // Read Encrypt string
  if (table_options.exists("encrypt_type"))
    table_options.get("encrypt_type", &share->encrypt_type, &share->mem_root);

  return false;
}

/**
  Calculate number of bits used for the column in the record preamble
  (aka null bits number).
*/

static uint column_preamble_bits(const dd::Column *col_obj) {
  uint result = 0;

  if (col_obj->is_nullable()) result++;

  if (col_obj->type() == dd::enum_column_types::BIT) {
    bool treat_bit_as_char = false;
    (void)col_obj->options().get("treat_bit_as_char", &treat_bit_as_char);

    if (!treat_bit_as_char) result += col_obj->char_length() & 7;
  }
  return result;
}

inline void get_auto_flags(const dd::Column &col_obj, uint &auto_flags) {
  /*
    For DEFAULT it is possible to have CURRENT_TIMESTAMP or a
    generation expression.
  */
  if (!col_obj.default_option().empty()) {
    // We're only matching the prefix because there may be parameters
    // e.g. CURRENT_TIMESTAMP(6). Regular strings won't match as they
    // are preceded by the charset and CURRENT_TIMESTAMP as a default
    // expression gets converted to now().
    if (col_obj.default_option().compare(0, 17, "CURRENT_TIMESTAMP") == 0) {
      // The only allowed patterns are "CURRENT_TIMESTAMP" and
      // "CURRENT_TIMESTAP(<integer>)". Stored functions with names
      // starting with "CURRENT_TIMESTAMP" should be filtered out before
      // we get here.
      DBUG_ASSERT(
          col_obj.default_option().size() == 17 ||
          (col_obj.default_option().size() >= 20 &&
           col_obj.default_option()[17] == '(' &&
           col_obj.default_option()[col_obj.default_option().size() - 1] ==
               ')'));

      auto_flags |= Field::DEFAULT_NOW;
    } else {
      auto_flags |= Field::GENERATED_FROM_EXPRESSION;
    }
  }

  /*
    For ON UPDATE the only option which is supported
    at this point is CURRENT_TIMESTAMP.
  */
  if (!col_obj.update_option().empty()) auto_flags |= Field::ON_UPDATE_NOW;

  if (col_obj.is_auto_increment()) auto_flags |= Field::NEXT_NUMBER;

  /*
    Columns can't have AUTO_INCREMENT and DEFAULT/ON UPDATE CURRENT_TIMESTAMP at
    the same time.
  */
  DBUG_ASSERT(!((auto_flags & (Field::DEFAULT_NOW | Field::ON_UPDATE_NOW |
                               Field::GENERATED_FROM_EXPRESSION)) != 0 &&
                (auto_flags & Field::NEXT_NUMBER) != 0));
}

static Field *make_field(const dd::Column &col_obj, const CHARSET_INFO *charset,
                         TABLE_SHARE *share, uchar *ptr, uchar *null_pos,
                         size_t null_bit) {
  auto field_type = dd_get_old_field_type(col_obj.type());
  auto field_length = col_obj.char_length();

  const dd::Properties &column_options = col_obj.options();

  // Reconstruct auto_flags
  auto auto_flags = static_cast<uint>(Field::NONE);
  get_auto_flags(col_obj, auto_flags);

  // Read Interval TYPELIB
  TYPELIB *interval = nullptr;

  if (field_type == MYSQL_TYPE_ENUM || field_type == MYSQL_TYPE_SET) {
    //
    // Allocate space for interval (column elements)
    //
    size_t interval_parts = col_obj.elements_count();

    interval = (TYPELIB *)share->mem_root.Alloc(sizeof(TYPELIB));
    interval->type_names = (const char **)share->mem_root.Alloc(
        sizeof(char *) * (interval_parts + 1));
    interval->type_names[interval_parts] = nullptr;

    interval->type_lengths =
        (uint *)share->mem_root.Alloc(sizeof(uint) * interval_parts);
    interval->count = interval_parts;
    interval->name = nullptr;

    //
    // Iterate through all the column elements
    //
    for (const dd::Column_type_element *ce : col_obj.elements()) {
      // Read the enum/set element name
      dd::String_type element_name = ce->name();

      uint pos = ce->index() - 1;
      interval->type_lengths[pos] = static_cast<uint>(element_name.length());
      interval->type_names[pos] = strmake_root(
          &share->mem_root, element_name.c_str(), element_name.length());
    }
  }

  // Column name
  char *name = nullptr;
  dd::String_type s = col_obj.name();
  DBUG_ASSERT(!s.empty());
  name = strmake_root(&share->mem_root, s.c_str(), s.length());
  name[s.length()] = '\0';

  uint decimals;
  // Decimals
  if (field_type == MYSQL_TYPE_DECIMAL || field_type == MYSQL_TYPE_NEWDECIMAL) {
    DBUG_ASSERT(col_obj.is_numeric_scale_null() == false);
    decimals = col_obj.numeric_scale();
  } else if (field_type == MYSQL_TYPE_FLOAT ||
             field_type == MYSQL_TYPE_DOUBLE) {
    decimals = col_obj.is_numeric_scale_null() ? DECIMAL_NOT_SPECIFIED
                                               : col_obj.numeric_scale();
  } else
    decimals = 0;

  auto geom_type = Field::GEOM_GEOMETRY;
  // Read geometry sub type
  if (field_type == MYSQL_TYPE_GEOMETRY) {
    uint32 sub_type = 0;
    column_options.get("geom_type", &sub_type);
    geom_type = static_cast<Field::geometry_type>(sub_type);
  }

  bool treat_bit_as_char = false;
  if (field_type == MYSQL_TYPE_BIT) {
    column_options.get("treat_bit_as_char", &treat_bit_as_char);
  }

  return make_field(*THR_MALLOC, share, ptr, field_length, null_pos, null_bit,
                    field_type, charset, geom_type, auto_flags, interval, name,
                    col_obj.is_nullable(), col_obj.is_zerofill(),
                    col_obj.is_unsigned(), decimals, treat_bit_as_char, 0,
                    col_obj.srs_id(), col_obj.is_array());
}

/**
  Add Field constructed according to column metadata from dd::Column
  object to TABLE_SHARE.
*/

static bool fill_column_from_dd(THD *thd, TABLE_SHARE *share,
                                const dd::Column *col_obj, uchar *null_pos,
                                uint null_bit_pos, uchar *rec_pos,
                                uint field_nr) {
  char *name = nullptr;
  enum_field_types field_type;
  const CHARSET_INFO *charset = nullptr;
  Field *reg_field;
  ha_storage_media field_storage;
  column_format_type field_column_format;

  //
  // Read column details from dd table
  //

  // Column name
  dd::String_type s = col_obj->name();
  DBUG_ASSERT(!s.empty());
  name = strmake_root(&share->mem_root, s.c_str(), s.length());
  name[s.length()] = '\0';

  const dd::Properties *column_options = &col_obj->options();

  // Type
  field_type = dd_get_old_field_type(col_obj->type());

  // Reconstruct auto_flags
  auto auto_flags = static_cast<uint>(Field::NONE);
  get_auto_flags(*col_obj, auto_flags);

  bool treat_bit_as_char = false;
  if (field_type == MYSQL_TYPE_BIT)
    column_options->get("treat_bit_as_char", &treat_bit_as_char);

  // Collation ID
  charset = dd_get_mysql_charset(col_obj->collation_id());
  if (charset == nullptr) {
    my_printf_error(ER_UNKNOWN_COLLATION,
                    "invalid collation id %llu for table %s, column %s", MYF(0),
                    col_obj->collation_id(), share->table_name.str, name);
    if (thd->is_error()) return true;
    charset = default_charset_info;
  }

  // Decimals
  if (field_type == MYSQL_TYPE_DECIMAL || field_type == MYSQL_TYPE_NEWDECIMAL)
    DBUG_ASSERT(col_obj->is_numeric_scale_null() == false);

  // Read geometry sub type
  if (field_type == MYSQL_TYPE_GEOMETRY) {
    uint32 sub_type;
    column_options->get("geom_type", &sub_type);
  }

  // Read values of storage media and column format options
  if (column_options->exists("storage")) {
    uint32 option_value = 0;
    column_options->get("storage", &option_value);
    field_storage = static_cast<ha_storage_media>(option_value);
  } else
    field_storage = HA_SM_DEFAULT;

  if (column_options->exists("column_format")) {
    uint32 option_value = 0;
    column_options->get("column_format", &option_value);
    field_column_format = static_cast<column_format_type>(option_value);
  } else
    field_column_format = COLUMN_FORMAT_TYPE_DEFAULT;

  // Read Interval TYPELIB
  TYPELIB *interval = nullptr;

  if (field_type == MYSQL_TYPE_ENUM || field_type == MYSQL_TYPE_SET) {
    //
    // Allocate space for interval (column elements)
    //
    size_t interval_parts = col_obj->elements_count();

    interval = (TYPELIB *)share->mem_root.Alloc(sizeof(TYPELIB));
    interval->type_names = (const char **)share->mem_root.Alloc(
        sizeof(char *) * (interval_parts + 1));
    interval->type_names[interval_parts] = nullptr;

    interval->type_lengths =
        (uint *)share->mem_root.Alloc(sizeof(uint) * interval_parts);
    interval->count = interval_parts;
    interval->name = nullptr;

    //
    // Iterate through all the column elements
    //
    for (const dd::Column_type_element *ce : col_obj->elements()) {
      // Read the enum/set element name
      dd::String_type element_name = ce->name();

      uint pos = ce->index() - 1;
      interval->type_lengths[pos] = static_cast<uint>(element_name.length());
      interval->type_names[pos] = strmake_root(
          &share->mem_root, element_name.c_str(), element_name.length());
    }
  }

  //
  // Create FIELD
  //
  reg_field =
      make_field(*col_obj, charset, share, rec_pos, null_pos, null_bit_pos);
  reg_field->field_index = field_nr;
  reg_field->stored_in_db = true;

  // Handle generated columns
  if (!col_obj->is_generation_expression_null()) {
    Value_generator *gcol_info = new (&share->mem_root) Value_generator();

    // Set if GC is virtual or stored
    gcol_info->set_field_stored(!col_obj->is_virtual());

    // Read generation expression.
    dd::String_type gc_expr = col_obj->generation_expression();

    /*
      Place the expression's text into the TABLE_SHARE. Field objects of
      TABLE_SHARE only have that. They don't have a corresponding Item,
      which will be later created for the Field in TABLE, by
      fill_dd_columns_from_create_fields().
    */
    gcol_info->dup_expr_str(&share->mem_root, gc_expr.c_str(),
                            gc_expr.length());
    share->vfields++;
    reg_field->gcol_info = gcol_info;
    reg_field->stored_in_db = gcol_info->get_field_stored();
  }

  // Handle default values generated from expression
  if (auto_flags & Field::GENERATED_FROM_EXPRESSION) {
    Value_generator *default_val_expr =
        new (&share->mem_root) Value_generator();

    // DEFAULT GENERATED is always stored
    default_val_expr->set_field_stored(true);

    // Read generation expression.
    dd::String_type default_val_expr_str = col_obj->default_option();

    // Copy the expression's text into reg_field which is stored on TABLE_SHARE.
    default_val_expr->dup_expr_str(&share->mem_root,
                                   default_val_expr_str.c_str(),
                                   default_val_expr_str.length());
    share->gen_def_field_count++;
    reg_field->m_default_val_expr = default_val_expr;
  }

  if ((auto_flags & Field::NEXT_NUMBER) != 0)
    share->found_next_number_field = &share->field[field_nr];

  // Set field flags
  if (col_obj->has_no_default()) reg_field->flags |= NO_DEFAULT_VALUE_FLAG;

  // Set default value or NULL. Reset required for e.g. CHAR.
  if (col_obj->is_default_value_null()) {
    reg_field->reset();
    reg_field->set_null();
  } else if (field_type == MYSQL_TYPE_BIT && !treat_bit_as_char &&
             (col_obj->char_length() & 7)) {
    // For bit fields with leftover bits, copy leftover bits into the preamble.
    Field_bit *bitfield = dynamic_cast<Field_bit *>(reg_field);
    const uchar leftover_bits =
        static_cast<uchar>(*col_obj->default_value()
                                .substr(reg_field->pack_length() - 1, 1)
                                .data());
    set_rec_bits(leftover_bits, bitfield->bit_ptr, bitfield->bit_ofs,
                 bitfield->bit_len);
    // Copy the main part of the bit field data into the record body.
    memcpy(rec_pos, col_obj->default_value().data(),
           reg_field->pack_length() - 1);
  } else {
    // For any other field with default data, copy the data into the record.
    memcpy(rec_pos, col_obj->default_value().data(), reg_field->pack_length());
  }

  reg_field->set_storage_type(field_storage);
  reg_field->set_column_format(field_column_format);

  // Comments
  dd::String_type comment = col_obj->comment();
  reg_field->comment.length = comment.length();
  if (reg_field->comment.length) {
    reg_field->comment.str =
        strmake_root(&share->mem_root, comment.c_str(), comment.length());
    reg_field->comment.length = comment.length();
  }

  // NOT SECONDARY column option.
  if (column_options->exists("not_secondary"))
    reg_field->flags |= NOT_SECONDARY_FLAG;

  reg_field->set_hidden(col_obj->hidden());

  // Field is prepared. Store it in 'share'
  share->field[field_nr] = reg_field;

  return (false);
}

/**
  Populate TABLE_SHARE::field array according to column metadata
  from dd::Table object.
*/

static bool fill_columns_from_dd(THD *thd, TABLE_SHARE *share,
                                 const dd::Table *tab_obj) {
  // Allocate space for fields in TABLE_SHARE.
  uint fields_size = ((share->fields + 1) * sizeof(Field *));
  share->field = (Field **)share->mem_root.Alloc((uint)fields_size);
  memset(share->field, 0, fields_size);
  share->vfields = 0;
  share->gen_def_field_count = 0;

  // Iterate through all the columns.
  uchar *null_flags MY_ATTRIBUTE((unused));
  uchar *null_pos, *rec_pos;
  null_flags = null_pos = share->default_values;
  rec_pos = share->default_values + share->null_bytes;
  uint null_bit_pos =
      (share->db_create_options & HA_OPTION_PACK_RECORD) ? 0 : 1;
  uint field_nr = 0;
  bool has_vgc = false;
  for (const dd::Column *col_obj : tab_obj->columns()) {
    // Skip hidden columns
    if (col_obj->is_se_hidden()) continue;

    /*
      Fill details of each column.

      Skip virtual generated columns at this point. They reside at the end of
      the record, so we need to do separate pass, to evaluate their offsets
      correctly.
    */
    if (!col_obj->is_virtual()) {
      if (fill_column_from_dd(thd, share, col_obj, null_pos, null_bit_pos,
                              rec_pos, field_nr))
        return true;

      rec_pos += share->field[field_nr]->pack_length_in_rec();
    } else
      has_vgc = true;

    /*
      Virtual generated columns still need to be accounted in null bits and
      field_nr calculations, since they reside at the normal place in record
      preamble and TABLE_SHARE::field array.
    */
    if ((null_bit_pos += column_preamble_bits(col_obj)) > 7) {
      null_pos++;
      null_bit_pos -= 8;
    }
    field_nr++;
  }

  if (has_vgc) {
    /*
      Additional pass to put virtual generated columns at the end of the
      record is required.
    */
    if (share->stored_rec_length >
        static_cast<ulong>(rec_pos - share->default_values))
      share->stored_rec_length = (rec_pos - share->default_values);

    null_pos = share->default_values;
    null_bit_pos = (share->db_create_options & HA_OPTION_PACK_RECORD) ? 0 : 1;
    field_nr = 0;

    for (const dd::Column *col_obj2 : tab_obj->columns()) {
      // Skip hidden columns
      if (col_obj2->is_se_hidden()) continue;

      if (col_obj2->is_virtual()) {
        // Fill details of each column.
        if (fill_column_from_dd(thd, share, col_obj2, null_pos, null_bit_pos,
                                rec_pos, field_nr))
          return true;

        rec_pos += share->field[field_nr]->pack_length_in_rec();
      }

      /*
        Account for all columns while evaluating null_pos/null_bit_pos and
        field_nr.
      */
      if ((null_bit_pos += column_preamble_bits(col_obj2)) > 7) {
        null_pos++;
        null_bit_pos -= 8;
      }
      field_nr++;
    }
  }

  // Make sure the scan of the columns is consistent with other data.
  DBUG_ASSERT(share->null_bytes ==
              (null_pos - null_flags + (null_bit_pos + 7) / 8));
  DBUG_ASSERT(share->last_null_bit_pos == null_bit_pos);
  DBUG_ASSERT(share->fields == field_nr);

  return (false);
}

/** Fill KEY_INFO_PART from dd::Index_element object. */
static void fill_index_element_from_dd(TABLE_SHARE *share,
                                       const dd::Index_element *idx_elem_obj,
                                       KEY_PART_INFO *keypart) {
  //
  // Read index element details
  //

  keypart->length = idx_elem_obj->length();
  keypart->store_length = keypart->length;

  // fieldnr
  keypart->fieldnr = idx_elem_obj->column().ordinal_position();

  // field
  DBUG_ASSERT(keypart->fieldnr > 0);
  Field *field = keypart->field = share->field[keypart->fieldnr - 1];

  // offset
  keypart->offset = field->offset(share->default_values);

  // key type
  keypart->bin_cmp = ((field->real_type() != MYSQL_TYPE_VARCHAR &&
                       field->real_type() != MYSQL_TYPE_STRING) ||
                      (field->charset()->state & MY_CS_BINSORT));
  //
  // Read index order
  //

  // key part order
  if (idx_elem_obj->order() == dd::Index_element::ORDER_DESC)
    keypart->key_part_flag |= HA_REVERSE_SORT;

  // key_part->field=   (Field*) 0; // Will be fixed later
}

/** Fill KEY::key_part array according to metadata from dd::Index object. */
static void fill_index_elements_from_dd(TABLE_SHARE *share,
                                        const dd::Index *idx_obj, int key_nr) {
  //
  // Iterate through all index elements
  //

  uint i = 0;
  KEY *keyinfo = share->key_info + key_nr;
  for (const dd::Index_element *idx_elem_obj : idx_obj->elements()) {
    // Skip hidden index elements
    if (idx_elem_obj->is_hidden()) continue;

    //
    // Read index element details
    //

    fill_index_element_from_dd(share, idx_elem_obj, keyinfo->key_part + i);
    if (keyinfo->key_part[i].field->is_array())
      keyinfo->flags |= HA_MULTI_VALUED_KEY;
    i++;
  }
}

/**
  Add KEY constructed according to index metadata from dd::Index object to
  the TABLE_SHARE.
*/

static bool fill_index_from_dd(THD *thd, TABLE_SHARE *share,
                               const dd::Index *idx_obj, uint key_nr) {
  //
  // Read index details
  //

  // Get the keyinfo that we will prepare now
  KEY *keyinfo = share->key_info + key_nr;

  // Read index name
  const dd::String_type &name = idx_obj->name();
  if (!name.empty()) {
    if (name.length()) {
      keyinfo->name =
          strmake_root(&share->mem_root, name.c_str(), name.length());
      share->keynames.type_names[key_nr] = keyinfo->name;  // Post processing ??
    } else
      share->keynames.type_names[key_nr] = nullptr;
    // share->keynames.count= key_nr+1;
  }

  // Index algorithm
  keyinfo->algorithm = dd_get_old_index_algorithm_type(idx_obj->algorithm());
  keyinfo->is_algorithm_explicit = idx_obj->is_algorithm_explicit();

  // Visibility
  keyinfo->is_visible = idx_obj->is_visible();

  // user defined key parts
  keyinfo->user_defined_key_parts = 0;
  for (const dd::Index_element *idx_ele : idx_obj->elements()) {
    // Skip hidden index elements
    if (!idx_ele->is_hidden()) keyinfo->user_defined_key_parts++;
  }

  // flags
  switch (idx_obj->type()) {
    case dd::Index::IT_MULTIPLE:
      keyinfo->flags = 0;
      break;
    case dd::Index::IT_FULLTEXT:
      keyinfo->flags = HA_FULLTEXT;
      break;
    case dd::Index::IT_SPATIAL:
      keyinfo->flags = HA_SPATIAL;
      break;
    case dd::Index::IT_PRIMARY:
    case dd::Index::IT_UNIQUE:
      keyinfo->flags = HA_NOSAME;
      break;
    default:
      DBUG_ASSERT(0); /* purecov: deadcode */
      keyinfo->flags = 0;
      break;
  }

  if (idx_obj->is_generated()) keyinfo->flags |= HA_GENERATED_KEY;

  /*
    The remaining important SQL-layer flags are set later - either we directly
    store and read them from DD (HA_PACK_KEY, HA_BINARY_PACK_KEY), or calculate
    while handling other key options (HA_USES_COMMENT, HA_USES_PARSER,
    HA_USES_BLOCK_SIZE), or during post-processing step (HA_NULL_PART_KEY).
  */

  // key length
  keyinfo->key_length = 0;
  for (const dd::Index_element *idx_elem : idx_obj->elements()) {
    // Skip hidden index elements
    if (!idx_elem->is_hidden()) keyinfo->key_length += idx_elem->length();
  }

  //
  // Read index options
  //

  const dd::Properties &idx_options = idx_obj->options();

  /*
    Restore flags indicating that key packing optimization was suggested to SE.
    See fill_dd_indexes_for_keyinfo() for explanation why we store these flags
    explicitly.
  */
  uint32 stored_flags = 0;
  idx_options.get("flags", &stored_flags);
  DBUG_ASSERT((stored_flags & ~(HA_PACK_KEY | HA_BINARY_PACK_KEY)) == 0);

  //  Beginning in 8.0.12 HA_PACK_KEY and HA_BINARY_PACK_KEY are no longer set
  //  if the SE does not support it. If the index was created prior to 8.0.12
  //  these bits may be set in the flags option, and when being added to
  //  key_info->flags, the ALTER algorithm analysis will be broken because the
  //  intermediate table is created without these flags, and hence, the
  //  analysis will incorrectly conclude that all indexes have changed.
  //
  //  To workaround this issue, we remove the flags below, depending on the SE
  //  capabilities, when preparing the table share. Thus, if we ALTER the table
  //  at a later stage, indexes not being touched by the ALTER statement will
  //  have the same flags both in the source table and the intermediate table,
  //  and hence, the algorithm analysis will come to the right conclusion.
  if (ha_check_storage_engine_flag(share->db_type(),
                                   HTON_SUPPORTS_PACKED_KEYS) == 0) {
    // Given the assert above, we could just have set stored_flags to 0 here,
    // but keep it like this in case new flags are added.
    stored_flags &= ~(HA_PACK_KEY | HA_BINARY_PACK_KEY);
  }
  keyinfo->flags |= stored_flags;

  // Block size
  if (idx_options.exists("block_size")) {
    idx_options.get("block_size", &keyinfo->block_size);

    DBUG_ASSERT(keyinfo->block_size);

    keyinfo->flags |= HA_USES_BLOCK_SIZE;
  }

  // Read field parser
  if (idx_options.exists("parser_name")) {
    LEX_CSTRING parser_name;
    if (idx_options.get("parser_name", &parser_name, &share->mem_root))
      DBUG_ASSERT(false);

    keyinfo->parser =
        my_plugin_lock_by_name(nullptr, parser_name, MYSQL_FTPARSER_PLUGIN);
    if (!keyinfo->parser) {
      my_error(ER_PLUGIN_IS_NOT_LOADED, MYF(0), parser_name.str);
      if (thd->is_error()) return true;
    }

    keyinfo->flags |= HA_USES_PARSER;
  }

  // Read comment
  dd::String_type comment = idx_obj->comment();
  keyinfo->comment.length = comment.length();

  if (keyinfo->comment.length) {
    keyinfo->comment.str =
        strmake_root(&share->mem_root, comment.c_str(), comment.length());
    keyinfo->comment.length = comment.length();

    keyinfo->flags |= HA_USES_COMMENT;
  }

  return (false);
}

/**
  Check if this is a spatial index that can be used. That is, if there is
  a spatial index on a geometry column without the SRID specified, we will
  hide the index so that the optimizer won't consider the index during
  optimization/execution.

  @param index The index to verify

  @retval true if the index is an usable spatial index, or if it isn't a
          spatial index.
  @retval false if the index is a spatial index on a geometry column without
          an SRID specified.
*/
static bool is_spatial_index_usable(const dd::Index &index) {
  if (index.type() == dd::Index::IT_SPATIAL) {
    /*
      We have already checked for hidden indexes before we get here. But we
      still play safe since the check is very cheap.
    */
    if (index.is_hidden()) return false; /* purecov: deadcode */

    // Check that none of the parts references a column with SRID == NULL
    for (const auto element : index.elements()) {
      if (!element->is_hidden() && !element->column().srs_id().has_value())
        return false;
    }
  }

  return true;
}

/**
  Fill TABLE_SHARE::key_info array according to index metadata
  from dd::Table object.
*/

static bool fill_indexes_from_dd(THD *thd, TABLE_SHARE *share,
                                 const dd::Table *tab_obj) {
  share->keys_for_keyread.init(0);
  share->keys_in_use.init();
  share->visible_indexes.init();

  uint32 primary_key_parts = 0;

  bool use_extended_sk = ha_check_storage_engine_flag(
      share->db_type(), HTON_SUPPORTS_EXTENDED_KEYS);

  // Count number of keys and total number of key parts in the table.

  DBUG_ASSERT(share->keys == 0 && share->key_parts == 0);

  for (const dd::Index *idx_obj : tab_obj->indexes()) {
    // Skip hidden indexes
    if (idx_obj->is_hidden()) continue;

    share->keys++;
    uint key_parts = 0;
    for (const dd::Index_element *idx_ele : idx_obj->elements()) {
      // Skip hidden index elements
      if (!idx_ele->is_hidden()) key_parts++;
    }
    share->key_parts += key_parts;

    // Primary key (or candidate key replacing it) is always first if exists.
    // If such key doesn't exist (e.g. there are no unique keys in the table)
    // we will simply waste some memory.
    if (idx_obj->ordinal_position() == 1) primary_key_parts = key_parts;
  }

  // Allocate and fill KEY objects.
  if (share->keys) {
    KEY_PART_INFO *key_part;
    ulong *rec_per_key;
    rec_per_key_t *rec_per_key_float;
    uint total_key_parts = share->key_parts;

    if (use_extended_sk)
      total_key_parts += (primary_key_parts * (share->keys - 1));

    //
    // Alloc rec_per_key buffer
    //
    if (!(rec_per_key =
              (ulong *)share->mem_root.Alloc(total_key_parts * sizeof(ulong))))
      return true; /* purecov: inspected */

    //
    // Alloc rec_per_key_float buffer
    //
    if (!(rec_per_key_float = (rec_per_key_t *)share->mem_root.Alloc(
              total_key_parts * sizeof(rec_per_key_t))))
      return true; /* purecov: inspected */

    //
    // Alloc buffer to hold keys and key_parts
    //

    if (!(share->key_info = (KEY *)share->mem_root.Alloc(
              share->keys * sizeof(KEY) +
              total_key_parts * sizeof(KEY_PART_INFO))))
      return true; /* purecov: inspected */

    memset(
        share->key_info, 0,
        (share->keys * sizeof(KEY) + total_key_parts * sizeof(KEY_PART_INFO)));
    key_part = (KEY_PART_INFO *)(share->key_info + share->keys);

    //
    // Alloc buffer to hold keynames
    //

    if (!(share->keynames.type_names = (const char **)share->mem_root.Alloc(
              (share->keys + 1) * sizeof(char *))))
      return true; /* purecov: inspected */
    memset(share->keynames.type_names, 0, ((share->keys + 1) * sizeof(char *)));

    share->keynames.type_names[share->keys] = nullptr;
    share->keynames.count = share->keys;

    // In first iteration get all the index_obj, so that we get all
    // user_defined_key_parts for each key. This is required to propertly
    // allocation key_part memory for keys.
    const dd::Index *index_at_pos[MAX_INDEXES];
    uint key_nr = 0;
    for (const dd::Index *idx_obj : tab_obj->indexes()) {
      // Skip hidden indexes
      if (idx_obj->is_hidden()) continue;

      if (fill_index_from_dd(thd, share, idx_obj, key_nr)) return true;

      index_at_pos[key_nr] = idx_obj;

      share->keys_in_use.set_bit(key_nr);

      if (idx_obj->is_visible() && is_spatial_index_usable(*idx_obj))
        share->visible_indexes.set_bit(key_nr);

      key_nr++;
    }

    // Update keyparts now
    key_nr = 0;
    do {
      // Assign the key_part_info buffer
      KEY *keyinfo = &share->key_info[key_nr];
      keyinfo->key_part = key_part;
      keyinfo->set_rec_per_key_array(rec_per_key, rec_per_key_float);
      keyinfo->set_in_memory_estimate(IN_MEMORY_ESTIMATE_UNKNOWN);

      fill_index_elements_from_dd(share, index_at_pos[key_nr], key_nr);

      key_part += keyinfo->user_defined_key_parts;
      rec_per_key += keyinfo->user_defined_key_parts;
      rec_per_key_float += keyinfo->user_defined_key_parts;

      // Post processing code ?
      /*
        Add PK parts if engine supports PK extension for secondary keys.
        Atm it works for Innodb only. Here we add unique first key parts
        to the end of secondary key parts array and increase actual number
        of key parts. Note that primary key is always first if exists.
        Later if there is no PK in the table then number of actual keys parts
        is set to user defined key parts.
        KEY::actual_flags can't be set until we fully set-up KEY::flags.
      */
      keyinfo->actual_key_parts = keyinfo->user_defined_key_parts;
      if (use_extended_sk && key_nr && !(keyinfo->flags & HA_NOSAME)) {
        keyinfo->unused_key_parts = primary_key_parts;
        key_part += primary_key_parts;
        rec_per_key += primary_key_parts;
        rec_per_key_float += primary_key_parts;
        share->key_parts += primary_key_parts;
      }

      // Initialize the rec per key arrays
      for (uint kp = 0; kp < keyinfo->actual_key_parts; ++kp) {
        keyinfo->rec_per_key[kp] = 0;
        keyinfo->set_records_per_key(kp, REC_PER_KEY_UNKNOWN);
      }

      key_nr++;
    } while (key_nr < share->keys);
  }

  return (false);
}

static char *copy_option_string(MEM_ROOT *mem_root,
                                const dd::Properties &options,
                                const dd::String_type &key) {
  dd::String_type tmp_str;
  if (options.exists(key) && !options.get(key, &tmp_str) && tmp_str.length()) {
    return strdup_root(mem_root, tmp_str.c_str());
  }
  return nullptr;
}

static void get_partition_options(MEM_ROOT *mem_root,
                                  partition_element *part_elem,
                                  const dd::Properties &part_options) {
  if (part_options.exists("max_rows"))
    part_options.get("max_rows", &part_elem->part_max_rows);

  if (part_options.exists("min_rows"))
    part_options.get("min_rows", &part_elem->part_min_rows);

  part_elem->data_file_name =
      copy_option_string(mem_root, part_options, "data_file_name");
  part_elem->index_file_name =
      copy_option_string(mem_root, part_options, "index_file_name");

  uint32 nodegroup_id = UNDEF_NODEGROUP;
  if (part_options.exists("nodegroup_id"))
    part_options.get("nodegroup_id", &nodegroup_id);

  DBUG_ASSERT(nodegroup_id <= 0xFFFF);
  part_elem->nodegroup_id = nodegroup_id;
}

static bool get_part_column_values(MEM_ROOT *mem_root,
                                   partition_info *part_info,
                                   partition_element *part_elem,
                                   const dd::Partition *part_obj) {
  part_elem_value *p_elem_values, *p_val;
  part_column_list_val *col_val_array, *col_vals;
  uint list_index = 0, entries = 0;
  uint max_column_id = 0, max_list_index = 0;

  for (const dd::Partition_value *part_value : part_obj->values()) {
    max_column_id = std::max(max_column_id, part_value->column_num());
    max_list_index = std::max(max_list_index, part_value->list_num());
    entries++;
  }
  if (entries != ((max_column_id + 1) * (max_list_index + 1))) {
    DBUG_ASSERT(0); /* purecov: deadcode */
    return true;
  }

  part_info->num_columns = max_column_id + 1;

  if (!multi_alloc_root(mem_root, &p_elem_values,
                        sizeof(*p_elem_values) * (max_list_index + 1),
                        &col_val_array,
                        sizeof(*col_val_array) * part_info->num_columns *
                            (max_list_index + 1),
                        NULL)) {
    return true; /* purecov: inspected */
  }
  memset(p_elem_values, 0, sizeof(*p_elem_values) * (max_list_index + 1));
  memset(
      col_val_array, 0,
      sizeof(*col_val_array) * part_info->num_columns * (max_list_index + 1));
  for (list_index = 0; list_index <= max_list_index; list_index++) {
    p_val = &p_elem_values[list_index];
    p_val->added_items = 1;
    p_val->col_val_array = &col_val_array[list_index * part_info->num_columns];
  }

  for (const dd::Partition_value *part_value : part_obj->values()) {
    p_val = &p_elem_values[part_value->list_num()];
    col_vals = p_val->col_val_array;
    if (part_value->is_value_null()) {
      col_vals[part_value->column_num()].null_value = true;
    } else if (part_value->max_value()) {
      col_vals[part_value->column_num()].max_value = true;
    } else {
      // TODO-PARTITION: Perhaps allocate on the heap instead and when the first
      // table instance is opened, free it and add the field image instead?
      // That way it can be reused for all other table instances.
      col_vals[part_value->column_num()].column_value.value_str =
          strmake_root(mem_root, part_value->value_utf8().c_str(),
                       part_value->value_utf8().length());
    }
  }

  for (list_index = 0; list_index <= max_list_index; list_index++) {
    p_val = &p_elem_values[list_index];
#ifndef DBUG_OFF
    for (uint i = 0; i < part_info->num_columns; i++) {
      DBUG_ASSERT(p_val->col_val_array[i].null_value ||
                  p_val->col_val_array[i].max_value ||
                  p_val->col_val_array[i].column_value.value_str);
    }
#endif
    if (part_elem->list_val_list.push_back(p_val, mem_root)) return true;
  }

  return false;
}

static bool setup_partition_from_dd(THD *thd, MEM_ROOT *mem_root,
                                    partition_info *part_info,
                                    partition_element *part_elem,
                                    const dd::Partition *part_obj,
                                    bool is_subpart) {
  dd::String_type comment = part_obj->comment();
  if (comment.length()) {
    part_elem->part_comment = strdup_root(mem_root, comment.c_str());
    if (!part_elem->part_comment) return true;
  }
  part_elem->partition_name = strdup_root(mem_root, part_obj->name().c_str());
  if (!part_elem->partition_name) return true;

  part_elem->engine_type = part_info->default_engine_type;

  get_partition_options(mem_root, part_elem, part_obj->options());

  // Read tablespace name.
  if (dd::get_tablespace_name<dd::Partition>(
          thd, part_obj, &part_elem->tablespace_name, mem_root))
    return true;

  if (is_subpart) {
    /* Only HASH/KEY subpartitioning allowed, no values allowed, so return! */
    return false;
  }
  // Iterate over all possible values
  if (part_info->part_type == partition_type::RANGE) {
    if (part_info->column_list) {
      if (get_part_column_values(mem_root, part_info, part_elem, part_obj))
        return true;
    } else {
      DBUG_ASSERT(part_obj->values().size() == 1);
      const dd::Partition_value *part_value = *part_obj->values().begin();
      DBUG_ASSERT(part_value->list_num() == 0);
      DBUG_ASSERT(part_value->column_num() == 0);
      if (part_value->max_value()) {
        part_elem->max_value = true;
      } else {
        if (part_value->value_utf8()[0] == '-') {
          part_elem->signed_flag = true;
          if (dd::Properties::from_str(part_value->value_utf8(),
                                       &part_elem->range_value)) {
            return true;
          }
        } else {
          part_elem->signed_flag = false;
          if (dd::Properties::from_str(part_value->value_utf8(),
                                       (ulonglong *)&part_elem->range_value)) {
            return true;
          }
        }
      }
    }
  } else if (part_info->part_type == partition_type::LIST) {
    if (part_info->column_list) {
      if (get_part_column_values(mem_root, part_info, part_elem, part_obj))
        return true;
    } else {
      uint list_index = 0, max_index = 0, entries = 0, null_entry = 0;
      part_elem_value *list_val, *list_val_array = nullptr;
      for (const dd::Partition_value *part_value : part_obj->values()) {
        max_index = std::max(max_index, part_value->list_num());
        entries++;
        if (part_value->value_utf8().empty()) {
          DBUG_ASSERT(!part_elem->has_null_value);
          part_elem->has_null_value = true;
          null_entry = part_value->list_num();
        }
      }
      if (entries != (max_index + 1)) {
        DBUG_ASSERT(0); /* purecov: deadcode */
        return true;
      }
      /* If a list entry is NULL then it is only flagged on the part_elem. */
      if (part_elem->has_null_value) entries--;

      if (entries) {
        list_val_array = (part_elem_value *)mem_root->Alloc(
            sizeof(*list_val_array) * entries);
        if (!list_val_array) return true;
        memset(list_val_array, 0, sizeof(*list_val_array) * entries);
      }

      for (const dd::Partition_value *part_value : part_obj->values()) {
        DBUG_ASSERT(part_value->column_num() == 0);
        if (part_value->value_utf8().empty()) {
          DBUG_ASSERT(part_value->list_num() == null_entry);
          continue;
        }
        list_index = part_value->list_num();
        /*
          If there is a NULL value in the partition values in the DD it is
          marked directly on the partition_element and should not have an own
          list_val. So compact the list_index range by remove the list_index for
          the null_entry.
        */
        if (part_elem->has_null_value && list_index > null_entry) list_index--;
        list_val = &list_val_array[list_index];
        DBUG_ASSERT(!list_val->unsigned_flag && !list_val->value);
        if (part_value->value_utf8()[0] == '-') {
          list_val->unsigned_flag = false;
          if (dd::Properties::from_str(part_value->value_utf8(),
                                       &list_val->value))
            return true;
        } else {
          list_val->unsigned_flag = true;
          if (dd::Properties::from_str(part_value->value_utf8(),
                                       (ulonglong *)&list_val->value))
            return true;
        }
      }
      for (uint i = 0; i < entries; i++) {
        if (part_elem->list_val_list.push_back(&list_val_array[i], mem_root))
          return true;
      }
    }
  } else {
#ifndef DBUG_OFF
    DBUG_ASSERT(part_info->part_type == partition_type::HASH);
    DBUG_ASSERT(part_obj->values().empty());
#endif
  }
  return false;
}

/**
  Set field_list

  To append each field to the field_list it will parse the
  submitted partition_expression string.

  Must be in sync with get_field_list_str!

  @param[in]     mem_root   Where to allocate the memory for the list entries.
  @param[in]     str        String object containing the column names.
  @param[in,out] field_list List to add field names to.

  @return false on success, else true.
*/

static bool set_field_list(MEM_ROOT *mem_root, dd::String_type &str,
                           List<char> *field_list) {
  dd::String_type field_name;
  dd::String_type::const_iterator it(str.begin());
  dd::String_type::const_iterator end(str.end());

  while (it != end) {
    if (dd::eat_str(field_name, it, end, dd::FIELD_NAME_SEPARATOR_CHAR))
      return true;
    size_t len = field_name.length();
    DBUG_ASSERT(len);
    char *name = static_cast<char *>(mem_root->Alloc(len + 1));
    if (!name) return true; /* purecov: inspected */
    memcpy(name, field_name.c_str(), len);
    name[len] = '\0';

    if (field_list->push_back(name, mem_root)) return true;
  }
  return false;
}

/**
  Fill TABLE_SHARE with partitioning details from dd::Partition.

  @details
  Set up as much as possible to ease creating new TABLE instances
  by copying from the TABLE_SHARE.

  Also to prevent future memory duplication partition definitions (names etc)
  are stored on the TABLE_SHARE and can be referenced from each TABLE instance.

  Note that [sub]part_expr still needs to be parsed from
  [sub]part_func_string for each TABLE instance to use the correct
  mem_root etc. To be as compatible with the .frm way to open a table
  as possible we currently generate the full partitioning clause which
  will be parsed for each new TABLE instance.
  TODO-PARTITION:
  - Create a way to handle Item expressions to be shared/copied
    from the TABLE_SHARE.
  - On the open of the first TABLE instance, copy the field images
    to the TABLE_SHARE::partition_info for each partition value.

  @param thd      Thread context.
  @param share    Share to be updated with partitioning details.
  @param tab_obj  dd::Table object to get partition info from.

  @return false if success, else true.
*/

static bool fill_partitioning_from_dd(THD *thd, TABLE_SHARE *share,
                                      const dd::Table *tab_obj) {
  if (tab_obj->partition_type() == dd::Table::PT_NONE) return false;

  // The DD only has information about how the table is partitioned in
  // the primary storage engine, so don't use this information for
  // tables in a secondary storage engine.
  if (share->is_secondary_engine()) return false;

  partition_info *part_info;
  part_info = new (&share->mem_root) partition_info;

  handlerton *hton = plugin_data<handlerton *>(
      ha_resolve_by_name_raw(thd, lex_cstring_handle(tab_obj->engine())));
  DBUG_ASSERT(hton && ha_storage_engine_is_enabled(hton));
  part_info->default_engine_type = hton;
  if (!part_info->default_engine_type) return true;

  // TODO-PARTITION: change partition_info::part_type to same enum as below :)
  switch (tab_obj->partition_type()) {
    case dd::Table::PT_RANGE_COLUMNS:
      part_info->column_list = true;
      part_info->list_of_part_fields = true;
      // Fall through.
    case dd::Table::PT_RANGE:
      part_info->part_type = partition_type::RANGE;
      break;
    case dd::Table::PT_LIST_COLUMNS:
      part_info->column_list = true;
      part_info->list_of_part_fields = true;
      // Fall through.
    case dd::Table::PT_LIST:
      part_info->part_type = partition_type::LIST;
      break;
    case dd::Table::PT_LINEAR_HASH:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::PT_HASH:
      part_info->part_type = partition_type::HASH;
      break;
    case dd::Table::PT_LINEAR_KEY_51:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::PT_KEY_51:
      part_info->key_algorithm = enum_key_algorithm::KEY_ALGORITHM_51;
      part_info->list_of_part_fields = true;
      part_info->part_type = partition_type::HASH;
      break;
    case dd::Table::PT_LINEAR_KEY_55:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::PT_KEY_55:
      part_info->key_algorithm = enum_key_algorithm::KEY_ALGORITHM_55;
      part_info->list_of_part_fields = true;
      part_info->part_type = partition_type::HASH;
      break;
    case dd::Table::PT_AUTO_LINEAR:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::PT_AUTO:
      part_info->key_algorithm = enum_key_algorithm::KEY_ALGORITHM_55;
      part_info->part_type = partition_type::HASH;
      part_info->list_of_part_fields = true;
      part_info->is_auto_partitioned = true;
      share->auto_partitioned = true;
      break;
    default:
      // Unknown partitioning type!
      DBUG_ASSERT(0); /* purecov: deadcode */
      return true;
  }
  switch (tab_obj->subpartition_type()) {
    case dd::Table::ST_NONE:
      part_info->subpart_type = partition_type::NONE;
      break;
    case dd::Table::ST_LINEAR_HASH:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::ST_HASH:
      part_info->subpart_type = partition_type::HASH;
      break;
    case dd::Table::ST_LINEAR_KEY_51:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::ST_KEY_51:
      part_info->key_algorithm = enum_key_algorithm::KEY_ALGORITHM_51;
      part_info->list_of_subpart_fields = true;
      part_info->subpart_type = partition_type::HASH;
      break;
    case dd::Table::ST_LINEAR_KEY_55:
      part_info->linear_hash_ind = true;
      // Fall through.
    case dd::Table::ST_KEY_55:
      part_info->key_algorithm = enum_key_algorithm::KEY_ALGORITHM_55;
      part_info->list_of_subpart_fields = true;
      part_info->subpart_type = partition_type::HASH;
      break;
    default:
      // Unknown sub partitioning type!
      DBUG_ASSERT(0); /* purecov: deadcode */
      return true;
  }

  dd::String_type part_expr = tab_obj->partition_expression();
  if (part_info->list_of_part_fields) {
    if (set_field_list(&share->mem_root, part_expr,
                       &part_info->part_field_list)) {
      return true;
    }
    part_info->part_func_string = nullptr;
    part_info->part_func_len = 0;
  } else {
    part_info->part_func_string =
        strdup_root(&share->mem_root, part_expr.c_str());
    part_info->part_func_len = part_expr.length();
  }
  dd::String_type subpart_expr = tab_obj->subpartition_expression();
  part_info->subpart_func_len = subpart_expr.length();
  if (part_info->subpart_func_len) {
    if (part_info->list_of_subpart_fields) {
      if (set_field_list(&share->mem_root, subpart_expr,
                         &part_info->subpart_field_list)) {
        return true;
      }
      part_info->subpart_func_string = nullptr;
      part_info->subpart_func_len = 0;
    } else {
      part_info->subpart_func_string =
          strdup_root(&share->mem_root, subpart_expr.c_str());
    }
  }

  //
  // Iterate through all the partitions
  //

  partition_element *curr_part_elem;
  List_iterator<partition_element> part_elem_it;

  /* Partitions are sorted first on level and then on number. */

#ifndef DBUG_OFF
  uint number = 0;
#endif
  for (const dd::Partition *part_obj : tab_obj->partitions()) {
#ifndef DBUG_OFF
    /* Must be in sorted order (sorted by level first and then on number). */
    DBUG_ASSERT(part_obj->number() >= number);
    number = part_obj->number();
#endif

    DBUG_ASSERT(part_obj->parent_partition_id() == dd::INVALID_OBJECT_ID);

    curr_part_elem = new (&share->mem_root) partition_element;
    if (!curr_part_elem) {
      return true;
    }

    if (setup_partition_from_dd(thd, &share->mem_root, part_info,
                                curr_part_elem, part_obj, false)) {
      return true;
    }

    if (part_info->partitions.push_back(curr_part_elem, &share->mem_root))
      return true;

    for (const dd::Partition *sub_part_obj : part_obj->subpartitions()) {
      partition_element *curr_sub_part_elem =
          new (&share->mem_root) partition_element;
      if (!curr_sub_part_elem) {
        return true;
      }

      if (setup_partition_from_dd(thd, &share->mem_root, part_info,
                                  curr_sub_part_elem, sub_part_obj, true)) {
        return true;
      }

      if (curr_part_elem->subpartitions.push_back(curr_sub_part_elem,
                                                  &share->mem_root))
        return true;
    }
  }

  // Get partition and sub_partition count.
  part_info->num_parts = part_info->partitions.elements;
  part_info->num_subparts = part_info->partitions[0]->subpartitions.elements;

  switch (tab_obj->default_partitioning()) {
    case dd::Table::DP_NO:
      part_info->use_default_partitions = false;
      part_info->use_default_num_partitions = false;
      break;
    case dd::Table::DP_YES:
      part_info->use_default_partitions = true;
      part_info->use_default_num_partitions = true;
      break;
    case dd::Table::DP_NUMBER:
      part_info->use_default_partitions = true;
      part_info->use_default_num_partitions = false;
      break;
    case dd::Table::DP_NONE:
    default:
      DBUG_ASSERT(0); /* purecov: deadcode */
  }
  switch (tab_obj->default_subpartitioning()) {
    case dd::Table::DP_NO:
      part_info->use_default_subpartitions = false;
      part_info->use_default_num_subpartitions = false;
      break;
    case dd::Table::DP_YES:
      part_info->use_default_subpartitions = true;
      part_info->use_default_num_subpartitions = true;
      break;
    case dd::Table::DP_NUMBER:
      part_info->use_default_subpartitions = true;
      part_info->use_default_num_subpartitions = false;
      break;
    case dd::Table::DP_NONE:
      DBUG_ASSERT(!part_info->is_sub_partitioned());
      break;
    default:
      DBUG_ASSERT(0); /* purecov: deadcode */
  }

  char *buf;
  uint buf_len;

  // Turn off ANSI_QUOTES and other SQL modes which affect printing of
  // generated partitioning clause.
  Sql_mode_parse_guard parse_guard(thd);

  buf = generate_partition_syntax(part_info, &buf_len, true, true, false,
                                  nullptr);

  if (!buf) return true;

  share->partition_info_str = strmake_root(&share->mem_root, buf, buf_len);
  if (!share->partition_info_str) return true;

  share->partition_info_str_len = buf_len;
  share->m_part_info = part_info;
  return (false);
}

/**
  Fill TABLE_SHARE with information about foreign keys from dd::Table.
*/

static bool fill_foreign_keys_from_dd(TABLE_SHARE *share,
                                      const dd::Table *tab_obj) {
  DBUG_ASSERT(share->foreign_keys == 0 && share->foreign_key_parents == 0);

  share->foreign_keys = tab_obj->foreign_keys().size();
  share->foreign_key_parents = tab_obj->foreign_key_parents().size();

  if (share->foreign_keys) {
    if (!(share->foreign_key =
              (TABLE_SHARE_FOREIGN_KEY_INFO *)share->mem_root.Alloc(
                  share->foreign_keys * sizeof(TABLE_SHARE_FOREIGN_KEY_INFO))))
      return true;

    uint i = 0;

    for (const dd::Foreign_key *fk : tab_obj->foreign_keys()) {
      if (lex_string_strmake(&share->mem_root,
                             &share->foreign_key[i].referenced_table_db,
                             fk->referenced_table_schema_name().c_str(),
                             fk->referenced_table_schema_name().length()))
        return true;
      if (lex_string_strmake(&share->mem_root,
                             &share->foreign_key[i].referenced_table_name,
                             fk->referenced_table_name().c_str(),
                             fk->referenced_table_name().length()))
        return true;
      if (lex_string_strmake(&share->mem_root,
                             &share->foreign_key[i].unique_constraint_name,
                             fk->unique_constraint_name().c_str(),
                             fk->unique_constraint_name().length()))
        return true;

      share->foreign_key[i].update_rule = fk->update_rule();
      share->foreign_key[i].delete_rule = fk->delete_rule();

      share->foreign_key[i].columns = fk->elements().size();
      if (!(share->foreign_key[i].column_name =
                (LEX_CSTRING *)share->mem_root.Alloc(
                    share->foreign_key[i].columns * sizeof(LEX_CSTRING))))
        return true;

      uint j = 0;

      for (const dd::Foreign_key_element *fk_el : fk->elements()) {
        if (lex_string_strmake(&share->mem_root,
                               &share->foreign_key[i].column_name[j],
                               fk_el->column().name().c_str(),
                               fk_el->column().name().length()))
          return true;

        ++j;
      }

      ++i;
    }
  }

  if (share->foreign_key_parents) {
    if (!(share->foreign_key_parent =
              (TABLE_SHARE_FOREIGN_KEY_PARENT_INFO *)share->mem_root.Alloc(
                  share->foreign_key_parents *
                  sizeof(TABLE_SHARE_FOREIGN_KEY_PARENT_INFO))))
      return true;

    uint i = 0;

    for (const dd::Foreign_key_parent *fk_p : tab_obj->foreign_key_parents()) {
      if (lex_string_strmake(&share->mem_root,
                             &share->foreign_key_parent[i].referencing_table_db,
                             fk_p->child_schema_name().c_str(),
                             fk_p->child_schema_name().length()))
        return true;
      if (lex_string_strmake(
              &share->mem_root,
              &share->foreign_key_parent[i].referencing_table_name,
              fk_p->child_table_name().c_str(),
              fk_p->child_table_name().length()))
        return true;
      share->foreign_key_parent[i].update_rule = fk_p->update_rule();
      share->foreign_key_parent[i].delete_rule = fk_p->delete_rule();
      ++i;
    }
  }
  return false;
}

/**
  Fill check constraints from dd::Table object to the TABLE_SHARE.

  @param[in,out]      share              TABLE_SHARE instance.
  @param[in]          tab_obj            Table instance.

  @retval   false   On Success.
  @retval   true    On failure.
*/
static bool fill_check_constraints_from_dd(TABLE_SHARE *share,
                                           const dd::Table *tab_obj) {
  DBUG_ASSERT(share->check_constraint_share_list == nullptr);

  if (tab_obj->check_constraints().size() > 0) {
    share->check_constraint_share_list = new (&share->mem_root)
        Sql_check_constraint_share_list(&share->mem_root);
    if (share->check_constraint_share_list == nullptr) return true;  // OOM

    if (share->check_constraint_share_list->reserve(
            tab_obj->check_constraints().size()))
      return true;  // OOM

    for (auto &cc : tab_obj->check_constraints()) {
      // Check constraint name.
      LEX_CSTRING name;
      if (lex_string_strmake(&share->mem_root, &name, cc->name().c_str(),
                             cc->name().length()))
        return true;  // OOM

      // Check constraint expression (clause).
      LEX_CSTRING check_clause;
      if (lex_string_strmake(&share->mem_root, &check_clause,
                             cc->check_clause().c_str(),
                             cc->check_clause().length()))
        return true;  // OOM

      // Check constraint state.
      bool is_cc_enforced =
          (cc->constraint_state() == dd::Check_constraint::CS_ENFORCED);

      share->check_constraint_share_list->push_back(
          Sql_check_constraint_share(name, check_clause, is_cc_enforced));
    }
  }

  return false;
}

bool open_table_def(THD *thd, TABLE_SHARE *share, const dd::Table &table_def) {
  DBUG_TRACE;

  MEM_ROOT *old_root = thd->mem_root;
  thd->mem_root = &share->mem_root;  // Needed for make_field()++
  share->blob_fields = 0;            // HACK

  // Fill the TABLE_SHARE with details.
  bool error = (fill_share_from_dd(thd, share, &table_def) ||
                fill_columns_from_dd(thd, share, &table_def) ||
                fill_indexes_from_dd(thd, share, &table_def) ||
                fill_partitioning_from_dd(thd, share, &table_def) ||
                fill_foreign_keys_from_dd(share, &table_def) ||
                fill_check_constraints_from_dd(share, &table_def));

  thd->mem_root = old_root;

  if (!error) error = prepare_share(thd, share, &table_def);

  if (!error) {
    share->table_category = get_table_category(share->db, share->table_name);
    thd->status_var.opened_shares++;
    return false;
  }
  return true;
}

/*
  Ignore errors related to invalid collation and missing parser during
  open_table_def().
*/
class Open_table_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    return (sql_errno == ER_UNKNOWN_COLLATION ||
            sql_errno == ER_PLUGIN_IS_NOT_LOADED);
  }
};

bool open_table_def_suppress_invalid_meta_data(THD *thd, TABLE_SHARE *share,
                                               const dd::Table &table_def) {
  Open_table_error_handler error_handler;
  thd->push_internal_handler(&error_handler);
  bool error = open_table_def(thd, share, table_def);
  thd->pop_internal_handler();
  return error;
}

//////////////////////////////////////////////////////////////////////////
