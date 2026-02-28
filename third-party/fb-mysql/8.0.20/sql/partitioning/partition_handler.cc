/*
   Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/partitioning/partition_handler.h"

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <new>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "myisam.h"  // MI_MAX_MSG_BUF
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/discrete_interval.h"
#include "sql/field.h"
#include "sql/key.h"  // key_rec_cmp
#include "sql/log.h"
#include "sql/partition_element.h"
#include "sql/partition_info.h"  // NOT_A_PARTITION_ID
#include "sql/protocol.h"
#include "sql/protocol_classic.h"
#include "sql/set_var.h"
#include "sql/sql_alter.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_partition.h"  // LIST_PART_ENTRY, part_id_range
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE_SHARE
#include "sql/thr_malloc.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_mutex.h"

namespace dd {
class Table;
}  // namespace dd

// In sql_class.cc:
int thd_binlog_format(const MYSQL_THD thd);

/** operation names for the enum_part_operation. */
static const char *opt_op_name[] = {
    "optimize",           "analyze",     "check", "repair",
    "assign_to_keycache", "preload_keys"};

static PSI_memory_key key_memory_Partition_share;
static PSI_memory_key key_memory_partition_sort_buffer;
static PSI_memory_key key_memory_Partition_admin;
#ifdef HAVE_PSI_INTERFACE
PSI_mutex_key key_partition_auto_inc_mutex;
static PSI_memory_info all_partitioning_memory[] = {
    {&key_memory_Partition_share, "Partition_share", 0, 0, PSI_DOCUMENT_ME},
    {&key_memory_partition_sort_buffer, "partition_sort_buffer", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_memory_Partition_admin, "Partition_admin", 0, 0, PSI_DOCUMENT_ME}};
static PSI_mutex_info all_partitioning_mutex[] = {
    {&key_partition_auto_inc_mutex, "Partition_share::auto_inc_mutex", 0, 0,
     PSI_DOCUMENT_ME}};
#endif

void partitioning_init() {
#ifdef HAVE_PSI_INTERFACE
  int count;
  count = static_cast<int>(array_elements(all_partitioning_memory));
  mysql_memory_register("sql", all_partitioning_memory, count);
  count = static_cast<int>(array_elements(all_partitioning_mutex));
  mysql_mutex_register("sql", all_partitioning_mutex, count);
#endif
}

/*
  Implementation of Partition_share class.
*/

Partition_share::Partition_share()
    : auto_inc_initialized(false),
      auto_inc_mutex(nullptr),
      next_auto_inc_val(0),
      partition_names(nullptr) {}

Partition_share::~Partition_share() {
  if (auto_inc_mutex) {
    mysql_mutex_destroy(auto_inc_mutex);
    my_free(auto_inc_mutex);
  }
  if (partition_names) {
    my_free(partition_names);
  }
}

/**
  Initialize auto increment mutex in share.

  @return Operation status.
    @retval true  Failure (out of memory).
    @retval false Success.
*/

bool Partition_share::init_auto_inc_mutex(
    TABLE_SHARE *table_share MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  DBUG_ASSERT(!auto_inc_mutex);
#ifndef DBUG_OFF
  if (table_share->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_assert_owner(&table_share->LOCK_ha_data);
  }
#endif /* DBUG_OFF */
  auto_inc_mutex = static_cast<mysql_mutex_t *>(my_malloc(
      key_memory_Partition_share, sizeof(*auto_inc_mutex), MYF(MY_WME)));
  if (!auto_inc_mutex) {
    return true;
  }
  mysql_mutex_init(key_partition_auto_inc_mutex, auto_inc_mutex,
                   MY_MUTEX_INIT_FAST);
  return false;
}

/**
  Release reserved auto increment values not used.
  @param thd             Thread.
  @param table_share     Table Share
  @param next_insert_id  Next insert id (first non used auto inc value).
  @param max_reserved    End of reserved auto inc range.
*/
void Partition_share::release_auto_inc_if_possible(
    THD *thd, TABLE_SHARE *table_share MY_ATTRIBUTE((unused)),
    const ulonglong next_insert_id, const ulonglong max_reserved) {
  DBUG_ASSERT(auto_inc_mutex);

#ifndef DBUG_OFF
  if (table_share->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_assert_owner(auto_inc_mutex);
  }
#endif /* DBUG_OFF */

  /*
    If the current auto_increment values is lower than the reserved value (1)
    and the reserved value was reserved by this thread (2), then we can
    lower the reserved value.
    However, we cannot lower the value if there are forced/non generated
    values from 'SET INSERT_ID = forced_val' (3). */
  if (next_insert_id < next_auto_inc_val &&                       // (1)
      max_reserved >= next_auto_inc_val &&                        // (2)
      thd->auto_inc_intervals_forced.maximum() < next_insert_id)  // (3)
  {
    next_auto_inc_val = next_insert_id;
  }
}

/**
  Populate the partition_name_hash in part_share.
*/

bool Partition_share::populate_partition_name_hash(partition_info *part_info) {
  uint tot_names;
  uint num_subparts = part_info->num_subparts;
  DBUG_TRACE;
  DBUG_ASSERT(!part_info->is_sub_partitioned() || num_subparts);

  if (num_subparts == 0) {
    num_subparts = 1;
  }

  /*
    TABLE_SHARE::LOCK_ha_data must been locked before calling this function.
    This ensures only one thread/table instance will execute this.
  */

#ifndef DBUG_OFF
  if (part_info->table->s->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_assert_owner(&part_info->table->s->LOCK_ha_data);
  }
#endif
  if (partition_name_hash != nullptr) {
    return false;
  }
  tot_names = part_info->num_parts;
  if (part_info->is_sub_partitioned()) {
    tot_names += part_info->num_parts * num_subparts;
  }
  partition_names = static_cast<const uchar **>(my_malloc(
      key_memory_Partition_share,
      part_info->get_tot_partitions() * sizeof(*partition_names), MYF(MY_WME)));
  if (!partition_names) {
    return true;
  }
  partition_name_hash.reset(
      new collation_unordered_map<std::string,
                                  unique_ptr_my_free<PART_NAME_DEF>>(
          system_charset_info, key_memory_Partition_share));

  List_iterator<partition_element> part_it(part_info->partitions);
  uint i = 0;
  do {
    partition_element *part_elem = part_it++;
    DBUG_ASSERT(part_elem->part_state == PART_NORMAL);
    if (part_elem->part_state == PART_NORMAL) {
      if (insert_partition_name_in_hash(part_elem->partition_name,
                                        i * num_subparts, false))
        goto err;
      if (part_info->is_sub_partitioned()) {
        List_iterator<partition_element> subpart_it(part_elem->subpartitions);
        partition_element *sub_elem;
        uint j = 0;
        do {
          sub_elem = subpart_it++;
          if (insert_partition_name_in_hash(sub_elem->partition_name,
                                            i * num_subparts + j, true))
            goto err;

        } while (++j < num_subparts);
      }
    }
  } while (++i < part_info->num_parts);

  for (const auto &key_and_value : *partition_name_hash) {
    PART_NAME_DEF *part_def = key_and_value.second.get();
    if (part_def->is_subpart == part_info->is_sub_partitioned()) {
      partition_names[part_def->part_id] = part_def->partition_name;
    }
  }

  return false;
err:
  partition_name_hash.reset();
  my_free(partition_names);
  partition_names = nullptr;

  return true;
}

/**
  Insert a partition name in the partition_name_hash.

  @param name        Name of partition
  @param part_id     Partition id (number)
  @param is_subpart  Set if the name belongs to a subpartition

  @return Operation status
    @retval true   Failure
    @retval false  Success
*/

bool Partition_share::insert_partition_name_in_hash(const char *name,
                                                    uint part_id,
                                                    bool is_subpart) {
  PART_NAME_DEF *part_def;
  char *part_name;
  uint part_name_length;
  DBUG_TRACE;
  /*
    Calculate and store the length here, to avoid doing it when
    searching the hash.
  */
  part_name_length = static_cast<uint>(strlen(name));
  /*
    Must use memory that lives as long as table_share.
    Freed in the Partition_share destructor.
    Since we use my_multi_malloc, then my_free(part_def) will also free
    part_name, as a part of my_hash_free.
  */
  if (!my_multi_malloc(key_memory_Partition_share, MY_WME, &part_def,
                       sizeof(PART_NAME_DEF), &part_name, part_name_length + 1,
                       NULL)) {
    return true;
  }
  memcpy(part_name, name, part_name_length + 1);
  part_def->partition_name = pointer_cast<uchar *>(part_name);
  part_def->length = part_name_length;
  part_def->part_id = part_id;
  part_def->is_subpart = is_subpart;
  return !partition_name_hash
              ->emplace(part_name, unique_ptr_my_free<PART_NAME_DEF>(part_def))
              .second;
}

const char *Partition_share::get_partition_name(size_t part_id) const {
  if (partition_names == nullptr) {
    return nullptr;
  }
  return reinterpret_cast<const char *>(partition_names[part_id]);
}

int Partition_handler::truncate_partition(dd::Table *table_def) {
  handler *file = get_handler();
  if (!file) {
    return HA_ERR_WRONG_COMMAND;
  }
  DBUG_ASSERT(file->table_share->tmp_table != NO_TMP_TABLE ||
              file->m_lock_type == F_WRLCK);
  file->mark_trx_read_write();
  return truncate_partition_low(table_def);
}

int Partition_handler::exchange_partition(uint part_id,
                                          dd::Table *part_table_def,
                                          dd::Table *swap_table_def) {
  handler *file = get_handler();
  if (!file) {
    return HA_ERR_WRONG_COMMAND;
  }
  DBUG_ASSERT(file->table_share->tmp_table != NO_TMP_TABLE ||
              file->m_lock_type != F_UNLCK);
  file->mark_trx_read_write();
  return exchange_partition_low(part_id, part_table_def, swap_table_def);
}

/*
  Implementation of Partition_helper class.
*/
Partition_helper::Partition_helper(handler *main_handler)
    : m_handler(main_handler),
      m_part_info(),
      m_tot_parts(),
      m_last_part(),
      m_err_rec(),
      m_ordered(),
      m_ordered_scan_ongoing(),
      m_ordered_rec_buffer(),
      m_queue() {}

Partition_helper::~Partition_helper() {
  DBUG_ASSERT(m_ordered_rec_buffer == nullptr);
  DBUG_ASSERT(m_key_not_found_partitions.bitmap == nullptr);
}

/**
  Set partition info.

  To be called from Partition_handler.

  @param  part_info  Partition info to use.
  @param  early      True if called when part_info only created and parsed,
                     but not setup, checked or fixed.
  */
void Partition_helper::set_part_info_low(partition_info *part_info,
                                         bool early) {
  /*
    ha_partition will set m_tot_parts from the .par file during creating
    the new handler.
    And this call can be earlier than the partition_default_handling(),
    so get_tot_partitions() may return zero.
  */
  if (m_tot_parts == 0 && (m_part_info == nullptr || !early)) {
    m_tot_parts = part_info->get_tot_partitions();
  }
  m_part_info = part_info;
  m_is_sub_partitioned = m_part_info->is_sub_partitioned();
}

/**
  Initialize the partitioning helper for use after the table is opened.

  @param part_share  Partitioning share (used for auto increment).

  @return Operation status.
    @retval false for success otherwise true.
*/

bool Partition_helper::open_partitioning(Partition_share *part_share) {
  m_table = get_table();
  DBUG_ASSERT(m_part_info == m_table->part_info);
  m_part_share = part_share;
  m_tot_parts = m_part_info->get_tot_partitions();
  if (bitmap_init(&m_key_not_found_partitions, nullptr, m_tot_parts)) {
    return true;
  }
  bitmap_clear_all(&m_key_not_found_partitions);
  m_key_not_found = false;
  m_is_sub_partitioned = m_part_info->is_sub_partitioned();
  m_auto_increment_lock = false;
  m_auto_increment_safe_stmt_log_lock = false;
  m_pkey_is_clustered = m_handler->primary_key_is_clustered();
  m_part_spec.start_part = NOT_A_PARTITION_ID;
  m_part_spec.end_part = NOT_A_PARTITION_ID;
  m_index_scan_type = PARTITION_NO_INDEX_SCAN;
  m_start_key.key = nullptr;
  m_start_key.length = 0;
  m_scan_value = 3;
  m_reverse_order = false;
  m_curr_key_info[0] = nullptr;
  m_curr_key_info[1] = nullptr;
  m_curr_key_info[2] = nullptr;
  m_top_entry = NO_CURRENT_PART_ID;
  m_ref_usage = REF_NOT_USED;
  m_rec_length = m_table->s->reclength;
  return false;
}

void Partition_helper::close_partitioning() {
  bitmap_free(&m_key_not_found_partitions);
  DBUG_ASSERT(!m_ordered_rec_buffer);
  destroy_record_priority_queue();
}

void Partition_helper::lock_auto_increment() {
  /* lock already taken */
  if (m_auto_increment_safe_stmt_log_lock) return;
  DBUG_ASSERT(!m_auto_increment_lock);
  if (m_table->s->tmp_table == NO_TMP_TABLE) {
    m_auto_increment_lock = true;
    m_part_share->lock_auto_inc();
  }
}

/****************************************************************************
                MODULE change record
****************************************************************************/

/**
  Insert a row to the partitioned table.

  @param buf The row in MySQL Row Format.

  @return Operation status.
    @retval    0 Success
    @retval != 0 Error code
*/

int Partition_helper::ph_write_row(uchar *buf) {
  uint32 part_id;
  int error;
  longlong func_value;
  bool have_auto_increment =
      m_table->next_number_field && buf == m_table->record[0];
  THD *thd = get_thd();
  sql_mode_t saved_sql_mode = thd->variables.sql_mode;
  bool saved_autoinc_field_has_expl_non_null_value =
      m_table->autoinc_field_has_explicit_non_null_value;
#ifndef DBUG_OFF
  my_bitmap_map *old_map;
#endif /* DBUG_OFF */
  DBUG_TRACE;
  DBUG_ASSERT(buf == m_table->record[0]);

  /*
    If we have an auto_increment column and we are writing a changed row
    or a new row, then update the auto_increment value in the record.
  */
  if (have_auto_increment) {
    error = m_handler->update_auto_increment();

    /*
      If we have failed to set the auto-increment value for this row,
      it is highly likely that we will not be able to insert it into
      the correct partition. We must check and fail if neccessary.
    */
    if (error) return error;

    /*
      Don't allow generation of auto_increment value the partitions handler.
      If a partitions handler would change the value, then it might not
      match the partition any longer.
      This can occur if 'SET INSERT_ID = 0; INSERT (NULL)',
      So allow this by adding 'MODE_NO_AUTO_VALUE_ON_ZERO' to sql_mode.
      The partitions handler::next_insert_id must always be 0. Otherwise
      we need to forward release_auto_increment, or reset it for all
      partitions.
    */
    if (m_table->next_number_field->val_int() == 0) {
      m_table->autoinc_field_has_explicit_non_null_value = true;
      thd->variables.sql_mode |= MODE_NO_AUTO_VALUE_ON_ZERO;
    }
  }

#ifndef DBUG_OFF
  /* Temporary mark the partitioning fields as readable. */
  old_map = dbug_tmp_use_all_columns(m_table, m_table->read_set);
#endif /* DBUG_OFF */

  error = m_part_info->get_partition_id(m_part_info, &part_id, &func_value);

#ifndef DBUG_OFF
  dbug_tmp_restore_column_map(m_table->read_set, old_map);
#endif /* DBUG_OFF */

  if (unlikely(error)) {
    m_part_info->err_value = func_value;
    goto exit;
  }
  if (!m_part_info->is_partition_locked(part_id)) {
    DBUG_PRINT("info", ("Write to non-locked partition %u (func_value: %ld)",
                        part_id, (long)func_value));
    error = HA_ERR_NOT_IN_LOCK_PARTITIONS;
    goto exit;
  }
  m_last_part = part_id;
  DBUG_PRINT("info", ("Insert in partition %d", part_id));

  error = write_row_in_part(part_id, buf);

  if (have_auto_increment && !m_table->s->next_number_keypart) {
    set_auto_increment_if_higher();
  }
exit:
  thd->variables.sql_mode = saved_sql_mode;
  m_table->autoinc_field_has_explicit_non_null_value =
      saved_autoinc_field_has_expl_non_null_value;
  return error;
}

/**
  Update an existing row in the partitioned table.

  Yes, update_row() does what you expect, it updates a row. old_data will
  have the previous row record in it, while new_data will have the newest
  data in it.
  Keep in mind that the server can do updates based on ordering if an
  ORDER BY clause was used. Consecutive ordering is not guaranteed.

  If the new record belongs to a different partition than the old record
  then it will be inserted into the new partition and deleted from the old.

  new_data is always record[0]
  old_data is always record[1]

  @param old_data  The old record in MySQL Row Format.
  @param new_data  The new record in MySQL Row Format.

  @return Operation status.
    @retval    0 Success
    @retval != 0 Error code
*/

int Partition_helper::ph_update_row(const uchar *old_data, uchar *new_data) {
  uint32 new_part_id, old_part_id;
  int error = 0;
  longlong func_value;
  DBUG_TRACE;
  m_err_rec = nullptr;

  // Need to read partition-related columns, to locate the row's partition:
  DBUG_ASSERT(
      bitmap_is_subset(&m_part_info->full_part_field_set, m_table->read_set));
  if ((error = get_parts_for_update(old_data, new_data, m_table->record[0],
                                    m_part_info, &old_part_id, &new_part_id,
                                    &func_value))) {
    return error;
  }
  if (!bitmap_is_set(&(m_part_info->lock_partitions), new_part_id)) {
    error = HA_ERR_NOT_IN_LOCK_PARTITIONS;
    return error;
  }

  /*
    The protocol for updating a row is:
    1) position the handler (cursor) on the row to be updated,
       either through the last read row (rnd or index) or by rnd_pos.
    2) call update_row with both old and new full records as arguments.

    This means that m_last_part should already be set to actual partition
    where the row was read from. And if that is not the same as the
    calculated part_id we found a misplaced row, we return an error to
    notify the user that something is broken in the row distribution
    between partitions! Since we don't check all rows on read, we return an
    error instead of correcting m_last_part, to make the user aware of the
    problem!

    Notice that HA_READ_BEFORE_WRITE_REMOVAL does not require this protocol,
    so this is not supported for this engine.
  */
  if (old_part_id != m_last_part) {
    m_err_rec = old_data;
    return HA_ERR_ROW_IN_WRONG_PARTITION;
  }

  m_last_part = new_part_id;
  if (new_part_id == old_part_id) {
    DBUG_PRINT("info", ("Update in partition %d", new_part_id));
    error = update_row_in_part(new_part_id, old_data, new_data);
  } else {
    Field *saved_next_number_field = m_table->next_number_field;
    /*
      Don't allow generation of auto_increment value for update.
      table->next_number_field is never set on UPDATE.
      But is set for INSERT ... ON DUPLICATE KEY UPDATE,
      and since update_row() does not generate or update an auto_inc value,
      we cannot have next_number_field set when moving a row
      to another partition with write_row(), since that could
      generate/update the auto_inc value.
      This gives the same behavior for partitioned vs non partitioned tables.
    */
    m_table->next_number_field = nullptr;
    DBUG_PRINT("info", ("Update from partition %d to partition %d", old_part_id,
                        new_part_id));
    error = write_row_in_part(new_part_id, new_data);
    m_table->next_number_field = saved_next_number_field;
    if (!error) {
      error = delete_row_in_part(old_part_id, old_data);
    }
  }

  /*
    if updating an auto_increment column, update
    m_part_share->next_auto_inc_val if needed.
    (not to be used if auto_increment on secondary field in a multi-column
    index)
    mysql_update does not set table->next_number_field, so we use
    table->found_next_number_field instead.
    Also checking that the field is marked in the write set.
  */
  if (m_table->found_next_number_field && new_data == m_table->record[0] &&
      !m_table->s->next_number_keypart &&
      bitmap_is_set(m_table->write_set,
                    m_table->found_next_number_field->field_index)) {
    set_auto_increment_if_higher();
  }
  return error;
}

/**
  Delete an existing row in the partitioned table.

  This will delete a row. buf will contain a copy of the row to be deleted.
  The server will call this right after the current row has been read
  (from either a previous rnd_xxx() or index_xxx() call).
  If you keep a pointer to the last row or can access a primary key it will
  make doing the deletion quite a bit easier.
  Keep in mind that the server does no guarentee consecutive deletions.
  ORDER BY clauses can be used.

  buf is either record[0] or record[1]

  @param buf  The record in MySQL Row Format.

  @return Operation status.
    @retval    0 Success
    @retval != 0 Error code
*/

int Partition_helper::ph_delete_row(const uchar *buf) {
  int error;
  uint part_id;
  DBUG_TRACE;
  m_err_rec = nullptr;

  DBUG_ASSERT(
      bitmap_is_subset(&m_part_info->full_part_field_set, m_table->read_set));
  if ((error = get_part_for_delete(buf, m_table->record[0], m_part_info,
                                   &part_id))) {
    return error;
  }
  if (!m_part_info->is_partition_locked(part_id)) {
    return HA_ERR_NOT_IN_LOCK_PARTITIONS;
  }

  /*
    The protocol for deleting a row is:
    1) position the handler (cursor) on the row to be deleted,
       either through the last read row (rnd or index) or by rnd_pos.
    2) call delete_row with the full record as argument.

    This means that m_last_part should already be set to actual partition
    where the row was read from. And if that is not the same as the
    calculated part_id we found a misplaced row, we return an error to
    notify the user that something is broken in the row distribution
    between partitions! Since we don't check all rows on read, we return an
    error instead of forwarding the delete to the correct (m_last_part)
    partition!

    Notice that HA_READ_BEFORE_WRITE_REMOVAL does not require this protocol,
    so this is not supported for this engine.

    TODO: change the assert in InnoDB into an error instead and make this one
    an assert instead and remove the get_part_for_delete()!
  */
  if (part_id != m_last_part) {
    m_err_rec = buf;
    return HA_ERR_ROW_IN_WRONG_PARTITION;
  }
  /* Should never call delete_row on a partition which is not read */
  DBUG_ASSERT(m_part_info->is_partition_used(part_id));

  m_last_part = part_id;
  error = delete_row_in_part(part_id, buf);
  return error;
}

/**
  Get a range of auto increment values.

  Can only be used if the auto increment field is the first field in an index.

  This method is called by update_auto_increment which in turn is called
  by the individual handlers as part of write_row. We use the
  part_share->next_auto_inc_val, or search all
  partitions for the highest auto_increment_value if not initialized or
  if auto_increment field is a secondary part of a key, we must search
  every partition when holding a mutex to be sure of correctness.

  @param[in]   increment           Increment value.
  @param[in]   nb_desired_values   Number of desired values.
  @param[out]  first_value         First auto inc value reserved
                                      or MAX if failure.
  @param[out]  nb_reserved_values  Number of values reserved.
*/

void Partition_helper ::get_auto_increment_first_field(
    ulonglong increment, ulonglong nb_desired_values, ulonglong *first_value,
    ulonglong *nb_reserved_values) {
  THD *thd = get_thd();
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("inc: %lu desired_values: %lu first_value: %lu", (ulong)increment,
              (ulong)nb_desired_values, (ulong)*first_value));
  DBUG_ASSERT(increment && nb_desired_values);
  /*
    next_number_keypart is != 0 if the auto_increment column is a secondary
    column in the index (it is allowed in MyISAM)
  */
  DBUG_ASSERT(m_table->s->next_number_keypart == 0);
  *first_value = 0;

  /*
    Get a lock for handling the auto_increment in part_share
    for avoiding two concurrent statements getting the same number.
  */
  lock_auto_increment();

  /* Initialize if not already done. */
  if (!m_part_share->auto_inc_initialized) {
    initialize_auto_increment(false);
  }

  /*
    In a multi-row insert statement like INSERT SELECT and LOAD DATA
    where the number of candidate rows to insert is not known in advance
    we must hold a lock/mutex for the whole statement if we have statement
    based replication. Because the statement-based binary log contains
    only the first generated value used by the statement, and slaves assumes
    all other generated values used by this statement were consecutive to
    this first one, we must exclusively lock the generator until the statement
    is done.
  */
  int binlog_format = thd_binlog_format(thd);
  if (!m_auto_increment_safe_stmt_log_lock &&
      thd->lex->sql_command != SQLCOM_INSERT &&
      binlog_format != BINLOG_FORMAT_UNSPEC &&
      binlog_format != BINLOG_FORMAT_ROW) {
    DBUG_PRINT("info", ("locking auto_increment_safe_stmt_log_lock"));
    m_auto_increment_safe_stmt_log_lock = true;
  }

  /* this gets corrected (for offset/increment) in update_auto_increment */
  *first_value = m_part_share->next_auto_inc_val;
  m_part_share->next_auto_inc_val += nb_desired_values * increment;
  if (m_part_share->next_auto_inc_val < *first_value) {
    /* Overflow, set to max. */
    m_part_share->next_auto_inc_val = ULLONG_MAX;
  }

  unlock_auto_increment();
  DBUG_PRINT("info", ("*first_value: %lu", (ulong)*first_value));
  *nb_reserved_values = nb_desired_values;
}

inline void Partition_helper::set_auto_increment_if_higher() {
  Field_num *field = static_cast<Field_num *>(m_table->found_next_number_field);
  ulonglong nr =
      (field->unsigned_flag || field->val_int() > 0) ? field->val_int() : 0;
  lock_auto_increment();
  if (!m_part_share->auto_inc_initialized) {
    initialize_auto_increment(false);
  }
  /* must hold the mutex when looking/changing m_part_share. */
  if (nr >= m_part_share->next_auto_inc_val) {
    m_part_share->next_auto_inc_val = nr + 1;
  }
  unlock_auto_increment();
  save_auto_increment(nr);
}

void Partition_helper::ph_release_auto_increment() {
  DBUG_TRACE;

  if (m_table->s->next_number_keypart) {
    release_auto_increment_all_parts();
  } else if (m_handler->next_insert_id) {
    ulonglong max_reserved = m_handler->auto_inc_interval_for_cur_row.maximum();
    lock_auto_increment();
    m_part_share->release_auto_inc_if_possible(
        get_thd(), m_table->s, m_handler->next_insert_id, max_reserved);
    DBUG_PRINT("info", ("part_share->next_auto_inc_val: %lu",
                        (ulong)m_part_share->next_auto_inc_val));

    /* Unlock the multi row statement lock taken in get_auto_increment */
    if (m_auto_increment_safe_stmt_log_lock) {
      m_auto_increment_safe_stmt_log_lock = false;
      DBUG_PRINT("info", ("unlocking auto_increment_safe_stmt_log_lock"));
    }

    unlock_auto_increment();
  }
}

/**
  Calculate key hash value from an null terminated array of fields.
  Support function for KEY partitioning.

  @param field_array   An array of the fields in KEY partitioning

  @return hash_value calculated

  @note Uses the hash function on the character set of the field.
  Integer and floating point fields use the binary character set by default.
*/

uint32 Partition_helper::ph_calculate_key_hash_value(Field **field_array) {
  ulong nr1 = 1;
  ulong nr2 = 4;
  bool use_51_hash = (*field_array)->table->part_info->key_algorithm ==
                     enum_key_algorithm::KEY_ALGORITHM_51;

  do {
    Field *field = *field_array;
    if (use_51_hash) {
      switch (field->real_type()) {
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_NEWDATE: {
          if (field->is_null()) {
            nr1 ^= (nr1 << 1) | 1;
            continue;
          }
          /* Force this to my_hash_sort_bin, which was used in 5.1! */
          uint len = field->pack_length();
          uint64 tmp1 = nr1;
          uint64 tmp2 = nr2;

          my_charset_bin.coll->hash_sort(&my_charset_bin, field->ptr, len,
                                         &tmp1, &tmp2);

          // NOTE: This truncates to 32-bit on Windows, to keep on-disk
          // stability.
          nr1 = static_cast<ulong>(tmp1);
          nr2 = static_cast<ulong>(tmp2);

          /* Done with this field, continue with next one. */
          continue;
        }
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_BIT:
          /* Not affected, same in 5.1 and 5.5 */
          break;
        /*
          ENUM/SET uses my_hash_sort_simple in 5.1 (i.e. my_charset_latin1)
          and my_hash_sort_bin in 5.5!
        */
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_SET: {
          if (field->is_null()) {
            nr1 ^= (nr1 << 1) | 1;
            continue;
          }
          /* Force this to my_hash_sort_bin, which was used in 5.1! */
          uint len = field->pack_length();
          uint64 tmp1 = nr1;
          uint64 tmp2 = nr2;

          my_charset_latin1.coll->hash_sort(&my_charset_latin1, field->ptr, len,
                                            &tmp1, &tmp2);

          // NOTE: This truncates to 32-bit on Windows, to keep on-disk
          // stability.
          nr1 = static_cast<ulong>(tmp1);
          nr2 = static_cast<ulong>(tmp2);
          continue;
        }
        /* New types in mysql-5.6. */
        case MYSQL_TYPE_DATETIME2:
        case MYSQL_TYPE_TIME2:
        case MYSQL_TYPE_TIMESTAMP2:
          /* Not affected, 5.6+ only! */
          break;

        /* These types should not be allowed for partitioning! */
        case MYSQL_TYPE_NULL:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_GEOMETRY:
          /* fall through. */
        default:
          DBUG_ASSERT(0);  // New type?
                           /* Fall through for default hashing (5.5). */
      }
      /* fall through, use collation based hashing. */
    }
    field->hash(&nr1, &nr2);
  } while (*(++field_array));
  return (uint32)nr1;
}

bool Partition_helper::print_partition_error(int error) {
  THD *thd = get_thd();
  DBUG_TRACE;

  /* Should probably look for my own errors first */
  DBUG_PRINT("enter", ("error: %d", error));

  if ((error == HA_ERR_NO_PARTITION_FOUND) &&
      (thd->lex->alter_info == nullptr ||
       !(thd->lex->alter_info->flags & Alter_info::ALTER_TRUNCATE_PARTITION))) {
    m_part_info->print_no_partition_found(thd, m_table);
    // print_no_partition_found() reports an error, so we can just return here.
    return false;
  } else if (error == HA_ERR_ROW_IN_WRONG_PARTITION) {
    /*
      Should only happen on DELETE or UPDATE!
      Or in ALTER TABLE REBUILD/REORGANIZE where there are a misplaced
      row that needed to move to an old partition (not in the given set).
    */
    DBUG_ASSERT(thd_sql_command(thd) == SQLCOM_DELETE ||
                thd_sql_command(thd) == SQLCOM_DELETE_MULTI ||
                thd_sql_command(thd) == SQLCOM_UPDATE ||
                thd_sql_command(thd) == SQLCOM_UPDATE_MULTI ||
                thd_sql_command(thd) == SQLCOM_ALTER_TABLE);
    DBUG_ASSERT(m_err_rec);
    if (m_err_rec) {
      size_t max_length;
      char buf[MAX_KEY_LENGTH];
      String str(buf, sizeof(buf), system_charset_info);
      uint32 part_id;
      DBUG_ASSERT(m_last_part < m_tot_parts);
      str.length(0);
      if (thd_sql_command(thd) == SQLCOM_ALTER_TABLE) {
        str.append("from REBUILD/REORGANIZED partition: ");
        str.append_ulonglong(m_last_part);
        str.append(" to non included partition (new definition): ");
      } else {
        str.append_ulonglong(m_last_part);
        str.append(". Correct is ");
      }
      if (get_part_for_delete(m_err_rec, m_table->record[0], m_part_info,
                              &part_id)) {
        str.append("?");
      } else {
        str.append_ulonglong(part_id);
      }
      append_row_to_str(str, m_err_rec, m_table);

      /* Log this error, so the DBA can notice it and fix it! */
      LogErr(ERROR_LEVEL, ER_ROW_IN_WRONG_PARTITION_PLEASE_REPAIR,
             m_table->s->table_name.str, str.c_ptr_safe());

      max_length =
          (MYSQL_ERRMSG_SIZE - strlen(ER_THD(thd, ER_ROW_IN_WRONG_PARTITION)));
      if (str.length() >= max_length) {
        str.length(max_length - 4);
        str.append(STRING_WITH_LEN("..."));
      }
      my_error(ER_ROW_IN_WRONG_PARTITION, MYF(0), str.c_ptr_safe());
      m_err_rec = nullptr;
      return false;
    }
  }

  return true;
}

void Partition_helper::prepare_change_partitions() {
  List_iterator<partition_element> part_it(m_part_info->partitions);
  uint num_subparts =
      m_part_info->is_sub_partitioned() ? m_part_info->num_subparts : 1;
  uint temp_partitions = m_part_info->temp_partitions.elements;
  bool first = true;
  uint i = 0;
  partition_element *part_elem;

  /*
    Use the read_partitions bitmap for reorganized partitions,
    i.e. what to copy.
  */
  bitmap_clear_all(&m_part_info->read_partitions);

  while ((part_elem = part_it++) != nullptr) {
    if (part_elem->part_state == PART_CHANGED ||
        part_elem->part_state == PART_REORGED_DROPPED) {
      for (uint sp = 0; sp < num_subparts; sp++) {
        bitmap_set_bit(&m_part_info->read_partitions, i * num_subparts + sp);
      }
      DBUG_ASSERT(first);
    } else if (first && temp_partitions &&
               part_elem->part_state == PART_TO_BE_ADDED) {
      /*
        When doing an ALTER TABLE REORGANIZE PARTITION a number of
        partitions is to be reorganized into a set of new partitions.
        The reorganized partitions are in this case in the temp_partitions
        list. We mark all of them in one batch and thus we only do this
        until we find the first partition with state PART_TO_BE_ADDED
        since this is where the new partitions go in and where the old
        ones used to be.
      */
      first = false;
      DBUG_ASSERT(((i * num_subparts) + temp_partitions * num_subparts) <=
                  m_tot_parts);
      for (uint sp = 0; sp < temp_partitions * num_subparts; sp++) {
        bitmap_set_bit(&m_part_info->read_partitions, i * num_subparts + sp);
      }
    }

    ++i;
  }
}

/**
  Copy partitions as part of ALTER TABLE of partitions.

  SE and prepare_change_partitions has done all the preparations,
  now it is time to actually copy the data from the reorganized
  partitions to the new partitions.

  @param[out] deleted  Number of records deleted.

  @return Operation status
    @retval  0  Success
    @retval >0  Error code
*/

int Partition_helper::copy_partitions(ulonglong *const deleted) {
  uint new_part = 0;
  int result = 0;
  longlong func_value;
  DBUG_TRACE;

  if (m_part_info->linear_hash_ind) {
    if (m_part_info->part_type == partition_type::HASH)
      set_linear_hash_mask(m_part_info, m_part_info->num_parts);
    else
      set_linear_hash_mask(m_part_info, m_part_info->num_subparts);
  }

  /*
    m_part_info->read_partitions bitmap is setup for all the reorganized
    partitions to be copied. So we can use the normal handler rnd interface
    for reading.
  */
  if ((result = m_handler->ha_rnd_init(true))) {
    return result;
  }
  while (true) {
    if ((result = m_handler->ha_rnd_next(m_table->record[0]))) {
      if (result == HA_ERR_RECORD_DELETED) continue;  // Probably MyISAM
      if (result != HA_ERR_END_OF_FILE) goto error;
      /*
        End-of-file reached, break out to end the copy process.
      */
      break;
    }
    /* Found record to insert into new handler */
    if (m_part_info->get_partition_id(m_part_info, &new_part, &func_value)) {
      /*
        This record is in the original table but will not be in the new
        table since it doesn't fit into any partition any longer due to
        changed partitioning ranges or list values.
      */
      (*deleted)++;
    } else {
      if ((result = write_row_in_new_part(new_part))) {
        goto error;
      }
    }
  }
  m_handler->ha_rnd_end();
  return false;
error:
  m_handler->ha_rnd_end();
  return result;
}

/**
  Check/fix misplaced rows.

  @param read_part_id  Partition to check/fix.
  @param repair   If true, move misplaced rows to correct partition.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error
*/

int Partition_helper::check_misplaced_rows(uint read_part_id, bool repair) {
  int result = 0;
  THD *thd = get_thd();
  bool ignore = thd->lex->is_ignore();
  uint32 correct_part_id;
  longlong func_value;
  ha_rows num_misplaced_rows = 0;
  ha_rows num_deleted_rows = 0;

  DBUG_TRACE;

  if (repair) {
    /* We must read the full row, if we need to move it! */
    bitmap_set_all(m_table->read_set);
    bitmap_set_all(m_table->write_set);
  } else {
    /* Only need to read the partitioning fields. */
    bitmap_union(m_table->read_set, &m_part_info->full_part_field_set);
    /* Fill the base columns of virtual generated columns if necessary */
    for (Field **ptr = m_part_info->full_part_field_array; *ptr; ptr++) {
      if ((*ptr)->is_virtual_gcol()) m_table->mark_gcol_in_maps(*ptr);
    }
  }

  if ((result = rnd_init_in_part(read_part_id, true))) return result;

  while (true) {
    if ((result = ph_rnd_next_in_part(read_part_id, m_table->record[0]))) {
      if (result == HA_ERR_RECORD_DELETED) continue;
      if (result != HA_ERR_END_OF_FILE) break;

      if (num_misplaced_rows > 0) {
        if (repair) {
          if (num_deleted_rows > 0) {
            print_admin_msg(thd, MI_MAX_MSG_BUF, "warning", m_table->s->db.str,
                            m_table->alias, opt_op_name[REPAIR_PARTS],
                            "Moved %lld misplaced rows, deleted %lld rows",
                            num_misplaced_rows - num_deleted_rows,
                            num_deleted_rows);
          } else {
            print_admin_msg(thd, MI_MAX_MSG_BUF, "warning", m_table->s->db.str,
                            m_table->alias, opt_op_name[REPAIR_PARTS],
                            "Moved %lld misplaced rows", num_misplaced_rows);
          }
        } else {
          print_admin_msg(thd, MI_MAX_MSG_BUF, "error", m_table->s->db.str,
                          m_table->alias, opt_op_name[CHECK_PARTS],
                          "Found %lld misplaced rows in partition %u",
                          num_misplaced_rows, read_part_id);
        }
      }
      /* End-of-file reached, all rows are now OK, reset result and break. */
      result = 0;
      break;
    }

    result = m_part_info->get_partition_id(m_part_info, &correct_part_id,
                                           &func_value);
    // TODO: Add code to delete rows not matching any partition.
    if (result) break;

    if (correct_part_id != read_part_id) {
      num_misplaced_rows++;
      m_err_rec = nullptr;
      if (!repair) {
        /* Check. */
        result = HA_ADMIN_NEEDS_UPGRADE;
        char buf[MAX_KEY_LENGTH];
        String str(buf, sizeof(buf), system_charset_info);
        str.length(0);
        append_row_to_str(str, m_err_rec, m_table);
        print_admin_msg(thd, MI_MAX_MSG_BUF, "error", m_table->s->db.str,
                        m_table->alias, opt_op_name[CHECK_PARTS],
                        "Found a misplaced row"
                        " in part %d should be in part %d:\n%s",
                        read_part_id, correct_part_id, str.c_ptr_safe());
        /* Break on first misplaced row, unless ignore is given! */
        if (!ignore) break;
      } else {
        DBUG_PRINT("info", ("Moving row from partition %d to %d", read_part_id,
                            correct_part_id));

        /*
          Insert row into correct partition. Notice that there are no commit
          for every N row, so the repair will be one large transaction!
        */
        if ((result = write_row_in_part(correct_part_id, m_table->record[0]))) {
          /*
            We have failed to insert a row, it might have been a duplicate!
          */
          char buf[MAX_KEY_LENGTH];
          String str(buf, sizeof(buf), system_charset_info);
          str.length(0);
          if (result == HA_ERR_FOUND_DUPP_KEY) {
            if (ignore) {
              str.append("Duplicate key found, deleting the record:\n");
              num_deleted_rows++;
            } else {
              str.append(
                  "Duplicate key found, "
                  "please update or delete the record:\n");
              result = HA_ADMIN_CORRUPT;
            }
          }
          append_row_to_str(str, m_err_rec, m_table);

          /*
            If the engine supports transactions, the failure will be
            rollbacked.
          */
          if (!m_handler->has_transactions() || ignore ||
              result == HA_ADMIN_CORRUPT) {
            /* Log this error, so the DBA can notice it and fix it! */
            LogErr(ERROR_LEVEL, ER_WRITE_ROW_TO_PARTITION_FAILED,
                   m_table->s->table_name.str, read_part_id, correct_part_id,
                   str.c_ptr_safe());
          }
          print_admin_msg(thd, MI_MAX_MSG_BUF, "error", m_table->s->db.str,
                          m_table->alias, opt_op_name[REPAIR_PARTS],
                          "Failed to move/insert a row"
                          " from part %d into part %d:\n%s",
                          read_part_id, correct_part_id, str.c_ptr_safe());
          if (!ignore || result != HA_ERR_FOUND_DUPP_KEY) break;
        }

        /* Delete row from wrong partition. */
        if ((result = delete_row_in_part(read_part_id, m_table->record[0]))) {
          result = HA_ADMIN_CORRUPT;
          if (m_handler->has_transactions()) break;
          /*
            We have introduced a duplicate, since we failed to remove it
            from the wrong partition.
          */
          char buf[MAX_KEY_LENGTH];
          String str(buf, sizeof(buf), system_charset_info);
          str.length(0);
          append_row_to_str(str, m_err_rec, m_table);

          /* Log this error, so the DBA can notice it and fix it! */
          LogErr(ERROR_LEVEL,
                 ER_PARTITION_MOVE_CREATED_DUPLICATE_ROW_PLEASE_FIX,
                 m_table->s->table_name.str, read_part_id, result,
                 correct_part_id, str.c_ptr_safe());
          break;
        }
      }
    }
  }

  int tmp_result = rnd_end_in_part(read_part_id, true);
  return result ? result : tmp_result;
}

/**
  Read next row during full partition scan (scan in random row order).

  This function can evaluate the virtual generated columns. If virtual
  generated columns are involved, you should not call rnd_next_in_part
  directly but this one.

  @param         part_id  Partition to read from.
  @param[in,out] buf      buffer that should be filled with data.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_rnd_next_in_part(uint part_id, uchar *buf) {
  int result = rnd_next_in_part(part_id, buf);

  if (!result && m_table->has_gcol())
    result = update_generated_read_fields(buf, m_table);

  return result;
}

/** Set used partitions bitmap from Alter_info.

  @return false if success else true.
*/

bool Partition_helper::set_altered_partitions() {
  Alter_info *const alter_info = get_thd()->lex->alter_info;

  DBUG_ASSERT(alter_info != nullptr);

  if ((alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION) == 0 ||
      (alter_info->flags & Alter_info::ALTER_ALL_PARTITION)) {
    /*
      Full table command, not ALTER TABLE t <cmd> PARTITION <partition list>.
      All partitions are already set, so do nothing.
    */
    return false;
  }
  return m_part_info->set_read_partitions(&alter_info->partition_names);
}

/**
  Print a message row formatted for ANALYZE/CHECK/OPTIMIZE/REPAIR TABLE.

  Modeled after mi_check_print_msg.

  @param thd         Thread context.
  @param len         Needed length for message buffer.
  @param msg_type    Message type.
  @param db_name     Database name.
  @param table_name  Table name.
  @param op_name     Operation name.
  @param fmt         Message (in printf format with additional arguments).

  @return Operation status.
    @retval false for success else true.
*/

bool Partition_helper::print_admin_msg(THD *thd, uint len, const char *msg_type,
                                       const char *db_name,
                                       const char *table_name,
                                       const char *op_name, const char *fmt,
                                       ...) {
  va_list args;
  Protocol *protocol = thd->get_protocol();
  uint length;
  size_t msg_length;
  char name[NAME_LEN * 2 + 2];
  char *msgbuf;
  bool error = true;

  if (!(msgbuf = (char *)my_malloc(key_memory_Partition_admin, len, MYF(0))))
    return true;
  va_start(args, fmt);
  msg_length = vsnprintf(msgbuf, len, fmt, args);
  va_end(args);
  if (msg_length >= (len - 1)) goto err;
  msgbuf[len - 1] = 0;  // healthy paranoia

  if (!thd->get_protocol()->connection_alive()) {
    LogErr(ERROR_LEVEL, ER_PARTITION_HANDLER_ADMIN_MSG, msgbuf);
    goto err;
  }

  length = (uint)(strxmov(name, db_name, ".", table_name, NullS) - name);
  /*
     TODO: switch from protocol to push_warning here. The main reason we didn't
     it yet is parallel repair. Due to following trace:
     mi_check_print_msg/push_warning/sql_alloc/my_pthread_getspecific_ptr.

     Also we likely need to lock mutex here (in both cases with protocol and
     push_warning).
  */
  DBUG_PRINT("info", ("print_admin_msg:  %s, %s, %s, %s", name, op_name,
                      msg_type, msgbuf));
  protocol->start_row();
  protocol->store_string(name, length, system_charset_info);
  protocol->store(op_name, system_charset_info);
  protocol->store(msg_type, system_charset_info);
  protocol->store_string(msgbuf, msg_length, system_charset_info);
  if (protocol->end_row()) {
    LogErr(ERROR_LEVEL, ER_MY_NET_WRITE_FAILED_FALLING_BACK_ON_STDERR, msgbuf);
    goto err;
  }
  error = false;
err:
  my_free(msgbuf);
  return error;
}

/**
  Set table->read_set taking partitioning expressions into account.
*/

inline void Partition_helper::set_partition_read_set() {
  /*
    For operations that may need to change data, we may need to extend
    read_set.
  */
  if (m_handler->get_lock_type() == F_WRLCK) {
    /*
      If write_set contains any of the fields used in partition and
      subpartition expression, we need to set all bits in read_set because
      the row may need to be inserted in a different [sub]partition. In
      other words update_row() can be converted into write_row(), which
      requires a complete record.
    */
    if (bitmap_is_overlapping(&m_part_info->full_part_field_set,
                              m_table->write_set)) {
      bitmap_set_all(m_table->read_set);
    } else {
      /*
        Some handlers only read fields as specified by the bitmap for the
        read set. For partitioned handlers we always require that the
        fields of the partition functions are read such that we can
        calculate the partition id to place updated and deleted records.
      */
      bitmap_union(m_table->read_set, &m_part_info->full_part_field_set);
      /* Fill the base columns of virtual generated columns if necessary */
      for (Field **ptr = m_part_info->full_part_field_array; *ptr; ptr++) {
        if ((*ptr)->is_virtual_gcol()) m_table->mark_gcol_in_maps(*ptr);
      }
    }
    // Mark virtual generated columns writable. This test should be consistent
    // with the one in update_generated_read_fields().
    for (Field **vf = m_table->vfield; vf && *vf; vf++) {
      if ((*vf)->is_virtual_gcol() &&
          bitmap_is_set(m_table->read_set, (*vf)->field_index))
        bitmap_set_bit(m_table->write_set, (*vf)->field_index);
    }
  }
}

/****************************************************************************
                MODULE full table scan
****************************************************************************/

/**
  Initialize engine for random reads.

  rnd_init() is called when the server wants the storage engine to do a
  table scan or when the server wants to access data through rnd_pos.

  When scan is used we will scan one handler partition at a time.
  When preparing for rnd_pos we will initialize all handler partitions.
  No extra cache handling is needed when scanning is not performed.

  Before initializing we will call rnd_end to ensure that we clean up from
  any previous incarnation of a table scan.

  @param scan  false for initialize for random reads through rnd_pos()
               true for initialize for random scan through rnd_next().

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_rnd_init(bool scan) {
  int error;
  uint i = 0;
  uint part_id;
  DBUG_TRACE;

  set_partition_read_set();

  /* Now we see what the index of our first important partition is */
  DBUG_PRINT("info", ("m_part_info->read_partitions: %p",
                      m_part_info->read_partitions.bitmap));
  part_id = m_part_info->get_first_used_partition();
  DBUG_PRINT("info", ("m_part_spec.start_part %d", part_id));

  if (MY_BIT_NONE == part_id) {
    error = 0;
    goto err1;
  }

  DBUG_PRINT("info", ("rnd_init on partition %d", part_id));
  if (scan) {
    /* A scan can be restarted without rnd_end() in between! */
    if (m_scan_value == 1 && m_part_spec.start_part != NOT_A_PARTITION_ID) {
      /* End previous scan on partition before restart. */
      if ((error = rnd_end_in_part(m_part_spec.start_part, scan))) {
        return error;
      }
    }
    m_scan_value = 1;
    if ((error = rnd_init_in_part(part_id, scan))) goto err;
  } else {
    m_scan_value = 0;
    for (i = part_id; i < MY_BIT_NONE;
         i = m_part_info->get_next_used_partition(i)) {
      if ((error = rnd_init_in_part(i, scan))) goto err;
    }
  }
  m_part_spec.start_part = part_id;
  m_part_spec.end_part = m_tot_parts - 1;
  DBUG_PRINT("info", ("m_scan_value=%d", m_scan_value));
  return 0;

err:
  /* Call rnd_end for all previously initialized partitions. */
  for (; part_id < i; part_id = m_part_info->get_next_used_partition(part_id)) {
    rnd_end_in_part(part_id, scan);
  }
err1:
  m_scan_value = 2;
  m_part_spec.start_part = NO_CURRENT_PART_ID;
  return error;
}

/**
  End of a table scan.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_rnd_end() {
  int error = 0;
  DBUG_TRACE;
  switch (m_scan_value) {
    case 3:  // Error
      DBUG_ASSERT(0);
      /* fall through. */
    case 2:  // Error
      break;
    case 1:
      if (NO_CURRENT_PART_ID != m_part_spec.start_part)  // Table scan
      {
        error = rnd_end_in_part(m_part_spec.start_part, true);
      }
      break;
    case 0:
      uint i;
      for (i = m_part_info->get_first_used_partition(); i < MY_BIT_NONE;
           i = m_part_info->get_next_used_partition(i)) {
        int part_error;
        part_error = rnd_end_in_part(i, false);
        if (part_error && !error) {
          error = part_error;
        }
      }
      break;
  }
  m_scan_value = 3;
  m_part_spec.start_part = NO_CURRENT_PART_ID;
  return error;
}

/**
  Read next row during full table scan (scan in random row order).

  This is called for each row of the table scan. When you run out of records
  you should return HA_ERR_END_OF_FILE.
  The Field structure for the table is the key to getting data into buf
  in a manner that will allow the server to understand it.

  @param[out] buf  buffer that should be filled with data.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_rnd_next(uchar *buf) {
  int result = HA_ERR_END_OF_FILE;
  uint part_id = m_part_spec.start_part;
  DBUG_TRACE;

  if (NO_CURRENT_PART_ID == part_id) {
    /*
      The original set of partitions to scan was empty and thus we report
      the result here.
    */
    goto end;
  }

  DBUG_ASSERT(m_scan_value == 1);

  while (true) {
    result = rnd_next_in_part(part_id, buf);
    if (!result) {
      m_last_part = part_id;
      m_part_spec.start_part = part_id;
      return 0;
    }

    /*
      if we get here, then the current partition ha_rnd_next returned failure
    */
    if (result == HA_ERR_RECORD_DELETED) continue;  // Probably MyISAM

    if (result != HA_ERR_END_OF_FILE)
      goto end_dont_reset_start_part;  // Return error

    /* End current partition */
    DBUG_PRINT("info", ("rnd_end on partition %d", part_id));
    if ((result = rnd_end_in_part(part_id, true))) break;

    /* Shift to next partition */
    part_id = m_part_info->get_next_used_partition(part_id);
    if (part_id >= m_tot_parts) {
      result = HA_ERR_END_OF_FILE;
      break;
    }
    m_last_part = part_id;
    m_part_spec.start_part = part_id;
    DBUG_PRINT("info", ("rnd_init on partition %d", part_id));
    if ((result = rnd_init_in_part(part_id, true))) break;
  }

end:
  m_part_spec.start_part = NO_CURRENT_PART_ID;
end_dont_reset_start_part:
  return result;
}

/**
  Save position of current row.

  position() is called after each call to rnd_next() if the data needs
  to be ordered or accessed later.

  The server uses ref to store data. ref_length in the above case is
  the size needed to store current_position. ref is just a byte array
  that the server will maintain. If you are using offsets to mark rows, then
  current_position should be the offset. If it is a primary key like in
  InnoDB, then it needs to be a primary key.

  @param record  Current record in MySQL Row Format.
*/

void Partition_helper::ph_position(const uchar *record) {
  DBUG_ASSERT(m_part_info->is_partition_used(m_last_part));
  DBUG_TRACE;
  DBUG_PRINT("info", ("record: %p", record));
  DBUG_DUMP("record", record, m_rec_length);

  /*
    If m_ref_usage is set, then the ref is already stored in the
    priority queue (m_queue) when doing ordered scans.
  */
  if (m_ref_usage != REF_NOT_USED && m_ordered_scan_ongoing) {
    DBUG_ASSERT(!m_queue->empty());
    DBUG_ASSERT(m_ordered_rec_buffer);
    DBUG_ASSERT(!m_curr_key_info[1]);
    DBUG_ASSERT(uint2korr(m_queue->top()) == m_last_part);
    /* We already have the ref and part id. */
    memcpy(m_handler->ref, m_queue->top(), m_handler->ref_length);
  } else {
    DBUG_PRINT("info", ("m_last_part: %u", m_last_part));
    int2store(m_handler->ref, m_last_part);
    position_in_last_part(m_handler->ref + PARTITION_BYTES_IN_POS, record);
  }
  DBUG_DUMP("ref_out", m_handler->ref, m_handler->ref_length);
}

/****************************************************************************
                MODULE index scan
****************************************************************************/
/*
  Positions an index cursor to the index specified in the handle. Fetches the
  row if available. If the key value is null, begin at the first key of the
  index.

  There are loads of optimizations possible here for the partition handler.
  The same optimizations can also be checked for full table scan although
  only through conditions and not from index ranges.
  Phase one optimizations:
    Check if the fields of the partition function are bound. If so only use
    the single partition it becomes bound to.
  Phase two optimizations:
    If it can be deducted through range or list partitioning that only a
    subset of the partitions are used, then only use those partitions.
*/

/**
  Setup the ordered record buffer and the priority queue.

  Call destroy_record_priority_queue() to deallocate or clean-up
  from failure.

  @return false on success, else true.
*/

int Partition_helper::init_record_priority_queue() {
  uint used_parts = m_part_info->num_partitions_used();
  DBUG_TRACE;
  DBUG_ASSERT(!m_ordered_rec_buffer);
  DBUG_ASSERT(!m_queue);
  /* Initialize the priority queue. */
  // TODO: Create test to see the cost of allocating when needed vs
  // allocate once and keep between statements. Also test on NUMA
  // machines to see the difference (I guess that allocating when needed
  // will allocate on 'correct' NUMA node and be faster.)
  if (!m_queue) {
    m_queue = new (std::nothrow) Prio_queue(Key_rec_less(m_curr_key_info));
    if (!m_queue) {
      return HA_ERR_OUT_OF_MEM;
    }
  }
  /* Initialize the ordered record buffer.  */
  if (!m_ordered_rec_buffer) {
    uint alloc_len;
    /*
      Allocate record buffer for each used partition.
      If PK is clustered index, it is either the primary sort key or is
      added as secondary sort. So we only need to allocate for part id
      and a full record per partition.
      Otherwise if the clustered index was generated, we might need to
      do a secondary sort by rowid (handler::ref) and must allocate for
      ref (includes part id) and full record per partition. We don't
      know yet if we need to do secondary sort by rowid, so we must
      allocate space for it.
      TODO: enhance ha_index_init() for HA_EXTRA_SECONDARY_SORT_ROWID to
      avoid allocating space for handler::ref when not needed.
      When enhancing ha_index_init() care must be taken on ph_position(),
      so InnoDB's row_id is correctly handled (taken from m_last_part).
    */
    if (m_pkey_is_clustered && m_table->s->primary_key != MAX_KEY) {
      m_rec_offset = PARTITION_BYTES_IN_POS;
      m_ref_usage = REF_NOT_USED;
    } else {
      m_rec_offset = m_handler->ref_length;
      m_ref_usage = REF_STORED_IN_PQ;
    }
    alloc_len = used_parts * (m_rec_offset + m_rec_length);
    /* Allocate a key for temporary use when setting up the scan. */
    alloc_len += m_table->s->max_key_length;

    m_ordered_rec_buffer = static_cast<uchar *>(
        my_malloc(key_memory_partition_sort_buffer, alloc_len, MYF(MY_WME)));
    if (!m_ordered_rec_buffer) {
      return HA_ERR_OUT_OF_MEM;
    }

    /*
      We set-up one record per partition and each record has 2 bytes in
      front where the partition id is written. This is used by ordered
      index_read.
      If we need to also sort by rowid (handler::ref), then m_curr_key_info[1]
      is NULL and we add the rowid before the record.
      We also set-up a reference to the first record for temporary use in
      setting up the scan.
    */
    char *ptr = (char *)m_ordered_rec_buffer;
    uint i;
    for (i = m_part_info->get_first_used_partition(); i < MY_BIT_NONE;
         i = m_part_info->get_next_used_partition(i)) {
      DBUG_PRINT("info", ("init rec-buf for part %u", i));
      int2store(ptr, i);
      ptr += m_rec_offset + m_rec_length;
    }
    m_start_key.key = (const uchar *)ptr;
    /*
      Initialize priority queue, initialized to reading forward.
      Start by only sort by KEY, HA_EXTRA_SECONDARY_SORT_ROWID
      will be given if we should sort by handler::ref too.
    */
    m_queue->m_rec_offset = m_rec_offset;
    if (m_queue->reserve(used_parts)) {
      return HA_ERR_OUT_OF_MEM;
    }
  }
  return init_record_priority_queue_for_parts(used_parts);
}

/**
  Destroy the ordered record buffer and the priority queue.
*/

void Partition_helper::destroy_record_priority_queue() {
  DBUG_TRACE;
  destroy_record_priority_queue_for_parts();
  if (m_ordered_rec_buffer) {
    my_free(m_ordered_rec_buffer);
    m_ordered_rec_buffer = nullptr;
  }
  if (m_queue) {
    m_queue->clear();
    delete m_queue;
    m_queue = nullptr;
  }
  m_ref_usage = REF_NOT_USED;
  m_ordered_scan_ongoing = false;
}

/**
  Common setup for index_init.

  Set up variables and initialize the record priority queue.

  @param inx     Index to be used.
  @param sorted  True if the rows must be returned in index order.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_init_setup(uint inx, bool sorted) {
  DBUG_TRACE;

  DBUG_ASSERT(inx != MAX_KEY);
  DBUG_PRINT("info", ("inx %u sorted %u", inx, sorted));

  set_partition_read_set();

  m_part_spec.start_part = NO_CURRENT_PART_ID;
  m_start_key.length = 0;
  m_ordered = sorted;
  m_ref_usage = REF_NOT_USED;
  m_curr_key_info[0] = m_table->key_info + inx;
  m_curr_key_info[1] = nullptr;
  /*
    There are two cases where it is not enough to only sort on the key:
    1) For clustered indexes, the optimizer assumes that all keys
       have the rest of the PK columns appended to the KEY, so it will
       sort by PK as secondary sort key.
    2) Rowid-Order-Retrieval access methods, like index_merge_intersect
       and index_merge_union. These methods requires the index to be sorted
       on rowid (handler::ref) as secondary sort key.
  */
  if (m_pkey_is_clustered && m_table->s->primary_key != MAX_KEY &&
      inx != m_table->s->primary_key) {
    /*
      if PK is clustered, then the key cmp must use the pk to
      differentiate between equal key in given index.
    */
    DBUG_PRINT("info", ("Clustered pk, using pk as secondary cmp"));
    m_curr_key_info[1] = m_table->key_info + m_table->s->primary_key;
  }

  return 0;
}

/**
  Read one record in an index scan and start an index scan.

  index_read_map starts a new index scan using a start key. The MySQL Server
  will check the end key on its own. Thus to function properly the
  partitioned handler need to ensure that it delivers records in the sort
  order of the MySQL Server.
  index_read_map can be restarted without calling index_end on the previous
  index scan and without calling index_init. In this case the index_read_map
  is on the same index as the previous index_scan. This is particularly
  used in conjunction with multi read ranges.

  @param[out] buf          Read row in MySQL Row Format
  @param[in]  key          Key parts in consecutive order
  @param[in]  keypart_map  Which part of key is used
  @param[in]  find_flag    What type of key condition is used

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_read_map(uchar *buf, const uchar *key,
                                        key_part_map keypart_map,
                                        enum ha_rkey_function find_flag) {
  DBUG_TRACE;
  m_index_scan_type = PARTITION_INDEX_READ;
  m_start_key.key = key;
  m_start_key.keypart_map = keypart_map;
  m_start_key.flag = find_flag;
  return common_index_read(buf, true);
}

/**
  Common routine for a number of index_read variants.

  @param[out] buf             Buffer where the record should be returned.
  @param[in]  have_start_key  true <=> the left endpoint is available, i.e.
                              we're in index_read call or in read_range_first
                              call and the range has left endpoint.
                              false <=> there is no left endpoint (we're in
                              read_range_first() call and the range has no left
                              endpoint).

  @return Operation status
    @retval 0                    OK
    @retval HA_ERR_END_OF_FILE   Whole index scanned, without finding the
  record.
    @retval HA_ERR_KEY_NOT_FOUND Record not found, but index cursor positioned.
    @retval other                Error code.

  @details
    Start scanning the range (when invoked from read_range_first()) or doing
    an index lookup (when invoked from index_read_XXX):
     - If possible, perform partition selection
     - Find the set of partitions we're going to use
     - Depending on whether we need ordering:
        NO:  Get the first record from first used partition (see
             handle_unordered_scan_next_partition)
        YES: Fill the priority queue and get the record that is the first in
             the ordering
*/

int Partition_helper::common_index_read(uchar *buf, bool have_start_key) {
  int error;
  m_reverse_order = false;
  DBUG_TRACE;

  DBUG_PRINT("info", ("m_ordered %u m_ordered_scan_ong %u", m_ordered,
                      m_ordered_scan_ongoing));

  if (have_start_key) {
    m_start_key.length = calculate_key_len(m_table, m_handler->active_index,
                                           m_start_key.keypart_map);
    DBUG_PRINT("info",
               ("have_start_key map %lu find_flag %u len %u",
                m_start_key.keypart_map, m_start_key.flag, m_start_key.length));
    DBUG_ASSERT(m_start_key.length);
  }
  if ((error = partition_scan_set_up(buf, have_start_key))) {
    return error;
  }

  if (have_start_key && (m_start_key.flag == HA_READ_KEY_OR_PREV ||
                         m_start_key.flag == HA_READ_PREFIX_LAST ||
                         m_start_key.flag == HA_READ_PREFIX_LAST_OR_PREV ||
                         m_start_key.flag == HA_READ_BEFORE_KEY)) {
    m_reverse_order = true;
    m_ordered_scan_ongoing = true;
  }
  DBUG_PRINT("info", ("m_ordered %u m_o_scan_ong %u have_start_key %u",
                      m_ordered, m_ordered_scan_ongoing, have_start_key));
  if (!m_ordered_scan_ongoing) {
    /*
      We use unordered index scan when read_range is used and flag
      is set to not use ordered.
      We also use an unordered index scan when the number of partitions to
      scan is only one.
      The unordered index scan will use the partition set created.
    */
    DBUG_PRINT("info", ("doing unordered scan"));
    error = handle_unordered_scan_next_partition(buf);
  } else {
    /*
      In all other cases we will use the ordered index scan. This will use
      the partition set created by the get_partition_set method.
    */
    error = handle_ordered_index_scan(buf);
  }
  return error;
}

/**
  Start an index scan from leftmost record and return first record.

  index_first() asks for the first key in the index.
  This is similar to index_read except that there is no start key since
  the scan starts from the leftmost entry and proceeds forward with
  index_next.

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_first(uchar *buf) {
  DBUG_TRACE;

  m_index_scan_type = PARTITION_INDEX_FIRST;
  m_reverse_order = false;
  return common_first_last(buf);
}

/**
  Start an index scan from rightmost record and return first record.

  index_last() asks for the last key in the index.
  This is similar to index_read except that there is no start key since
  the scan starts from the rightmost entry and proceeds forward with
  index_prev.

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_last(uchar *buf) {
  DBUG_TRACE;

  int error = HA_ERR_END_OF_FILE;
  uint part_id = m_part_info->get_first_used_partition();
  if (part_id == MY_BIT_NONE) {
    /* No partition to scan. */
    return error;
  }
  m_index_scan_type = PARTITION_INDEX_LAST;
  m_reverse_order = true;
  return common_first_last(buf);
}

/**
  Common routine for index_first/index_last.

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::common_first_last(uchar *buf) {
  int error;
  DBUG_TRACE;

  if ((error = partition_scan_set_up(buf, false))) {
    return error;
  }
  if (!m_ordered_scan_ongoing && m_index_scan_type != PARTITION_INDEX_LAST) {
    return handle_unordered_scan_next_partition(buf);
  }
  return handle_ordered_index_scan(buf);
}

/**
  Read last using key.

  This is used in join_read_last_key to optimize away an ORDER BY.
  Can only be used on indexes supporting HA_READ_ORDER.

  @param[out] buf          Read row in MySQL Row Format
  @param[in]  key          Key
  @param[in]  keypart_map  Which part of key is used

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_read_last_map(uchar *buf, const uchar *key,
                                             key_part_map keypart_map) {
  DBUG_TRACE;

  m_ordered = true;  // Safety measure
  m_index_scan_type = PARTITION_INDEX_READ_LAST;
  m_start_key.key = key;
  m_start_key.keypart_map = keypart_map;
  m_start_key.flag = HA_READ_PREFIX_LAST;
  return common_index_read(buf, true);
}

/**
  Read index by key and keymap.

  Positions an index cursor to the index specified.
  Fetches the row if available. If the key value is null,
  begin at first key of the index.

  Optimization of the default implementation to take advantage of dynamic
  partition pruning.

  @param[out] buf          Read row in MySQL Row Format
  @param[in]  index        Index to read from
  @param[in]  key          Key
  @param[in]  keypart_map  Which part of key is used
  @param[in]  find_flag    Direction/how to search.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/
int Partition_helper::ph_index_read_idx_map(uchar *buf, uint index,
                                            const uchar *key,
                                            key_part_map keypart_map,
                                            enum ha_rkey_function find_flag) {
  int error = HA_ERR_KEY_NOT_FOUND;
  DBUG_TRACE;

  if (find_flag == HA_READ_KEY_EXACT) {
    uint part;
    m_start_key.key = key;
    m_start_key.keypart_map = keypart_map;
    m_start_key.flag = find_flag;
    m_start_key.length =
        calculate_key_len(m_table, index, m_start_key.keypart_map);

    get_partition_set(m_table, buf, index, &m_start_key, &m_part_spec);

    /*
      We have either found exactly 1 partition
      (in which case start_part == end_part)
      or no matching partitions (start_part > end_part)
    */
    DBUG_ASSERT(m_part_spec.start_part >= m_part_spec.end_part);
    /* The start part is must be marked as used. */
    DBUG_ASSERT(m_part_spec.start_part > m_part_spec.end_part ||
                m_part_info->is_partition_used(m_part_spec.start_part));

    for (part = m_part_spec.start_part; part <= m_part_spec.end_part;
         part = m_part_info->get_next_used_partition(part)) {
      error = index_read_idx_map_in_part(part, buf, index, key, keypart_map,
                                         find_flag);
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
        break;
      }
    }
    if (part <= m_part_spec.end_part) {
      m_last_part = part;
    }
  } else {
    /*
      If not only used with HA_READ_KEY_EXACT, we should investigate if
      possible to optimize for other find_flag's as well.
    */
    DBUG_ASSERT(0);
    error = HA_ERR_INTERNAL_ERROR;
  }
  return error;
}

/**
  Read next record in a forward index scan.

  Used to read forward through the index (left to right, low to high).

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_next(uchar *buf) {
  DBUG_TRACE;

  /*
    TODO(low priority):
    If we want partition to work with the HANDLER commands, we
    must be able to do index_last() -> index_prev() -> index_next()
    and if direction changes, we must step back those partitions in
    the record queue so we don't return a value from the wrong direction.
  */
  DBUG_ASSERT(m_index_scan_type != PARTITION_INDEX_LAST ||
              m_table->open_by_handler);
  if (!m_ordered_scan_ongoing) {
    return handle_unordered_next(buf, false);
  }
  return handle_ordered_next(buf, false);
}

/**
  Read next same record.

  This routine is used to read the next but only if the key is the same
  as supplied in the call.

  @param[out] buf     Read row in MySQL Row Format.
  @param[in]  keylen  Length of key.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_next_same(uchar *buf,
                                         uint keylen MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;

  DBUG_ASSERT(keylen == m_start_key.length);
  DBUG_ASSERT(m_index_scan_type != PARTITION_INDEX_LAST);
  if (!m_ordered_scan_ongoing) return handle_unordered_next(buf, true);
  return handle_ordered_next(buf, true);
}

/**
  Read next record when performing index scan backwards.

  Used to read backwards through the index (right to left, high to low).

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_index_prev(uchar *buf) {
  DBUG_TRACE;

  /* TODO: read comment in index_next */
  DBUG_ASSERT(m_index_scan_type != PARTITION_INDEX_FIRST ||
              m_table->open_by_handler);
  return handle_ordered_prev(buf);
}

/**
  Start a read of one range with start and end key.

  We re-implement read_range_first since we don't want the compare_key
  check at the end. This is already performed in the partition handler.
  read_range_next is very much different due to that we need to scan
  all underlying handlers.

  @param start_key     Specification of start key.
  @param end_key       Specification of end key.
  @param eq_range_arg  Is it equal range.
  @param sorted        Should records be returned in sorted order.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_read_range_first(const key_range *start_key,
                                          const key_range *end_key,
                                          bool eq_range_arg, bool sorted) {
  int error = HA_ERR_END_OF_FILE;
  bool have_start_key = (start_key != nullptr);
  uint part_id = m_part_info->get_first_used_partition();
  DBUG_TRACE;

  if (part_id == MY_BIT_NONE) {
    /* No partition to scan. */
    return error;
  }

  m_ordered = sorted;
  set_eq_range(eq_range_arg);
  m_handler->set_end_range(end_key, handler::RANGE_SCAN_ASC);

  set_range_key_part(m_curr_key_info[0]->key_part);
  if (have_start_key)
    m_start_key = *start_key;
  else
    m_start_key.key = nullptr;

  m_index_scan_type = PARTITION_READ_RANGE;
  error = common_index_read(m_table->record[0], have_start_key);
  return error;
}

/**
  Read next record in read of a range with start and end key.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_helper::ph_read_range_next() {
  DBUG_TRACE;

  if (m_ordered_scan_ongoing) {
    return handle_ordered_next(m_table->record[0], get_eq_range());
  }
  return handle_unordered_next(m_table->record[0], get_eq_range());
}

/**
  Common routine to set up index scans.

  Find out which partitions we'll need to read when scanning the specified
  range.

  If we need to scan only one partition, set m_ordered_scan_ongoing=false
  as we will not need to do merge ordering.

  @param buf            Buffer to later return record in (this function
                        needs it to calculate partitioning function values)

  @param idx_read_flag  true <=> m_start_key has range start endpoint which
                        probably can be used to determine the set of
                        partitions to scan.
                        false <=> there is no start endpoint.

  @return Operation status.
    @retval   0  Success
    @retval !=0  Error code
*/

int Partition_helper::partition_scan_set_up(uchar *buf, bool idx_read_flag) {
  DBUG_TRACE;

  if (idx_read_flag)
    get_partition_set(m_table, buf, m_handler->active_index, &m_start_key,
                      &m_part_spec);
  else {
    // TODO: set to get_first_used_part() instead!
    m_part_spec.start_part = 0;
    // TODO: Implement bitmap_get_last_set() and use that here!
    m_part_spec.end_part = m_tot_parts - 1;
  }
  if (m_part_spec.start_part > m_part_spec.end_part) {
    /*
      We discovered a partition set but the set was empty so we report
      key not found.
    */
    DBUG_PRINT("info", ("scan with no partition to scan"));
    return HA_ERR_END_OF_FILE;
  }
  if (m_part_spec.start_part == m_part_spec.end_part) {
    /*
      We discovered a single partition to scan, this never needs to be
      performed using the ordered index scan.
    */
    DBUG_PRINT("info", ("index scan using the single partition %d",
                        m_part_spec.start_part));
    m_ordered_scan_ongoing = false;
  } else {
    /*
      Set m_ordered_scan_ongoing according how the scan should be done
      Only exact partitions are discovered atm by get_partition_set.
      Verify this, also bitmap must have at least one bit set otherwise
      the result from this table is the empty set.
    */
    uint start_part = m_part_info->get_first_used_partition();
    if (start_part == MY_BIT_NONE) {
      DBUG_PRINT("info", ("scan with no partition to scan"));
      return HA_ERR_END_OF_FILE;
    }
    if (start_part > m_part_spec.start_part)
      m_part_spec.start_part = start_part;
    m_ordered_scan_ongoing = m_ordered;
  }
  DBUG_ASSERT(m_part_spec.start_part < m_tot_parts);
  DBUG_ASSERT(m_part_spec.end_part < m_tot_parts);
  return 0;
}

/**
  Common routine to handle index_next with unordered results.

  These routines are used to scan partitions without considering order.
  This is performed in two situations.
  1) In read_multi_range this is the normal case
  2) When performing any type of index_read, index_first, index_last where
  all fields in the partition function is bound. In this case the index
  scan is performed on only one partition and thus it isn't necessary to
  perform any sort.

  @param[out] buf        Read row in MySQL Row Format.
  @param[in]  is_next_same  Called from index_next_same.

  @return Operation status.
    @retval HA_ERR_END_OF_FILE  End of scan
    @retval 0                   Success
    @retval other               Error code
*/

int Partition_helper::handle_unordered_next(uchar *buf, bool is_next_same) {
  int error;
  DBUG_TRACE;

  if (m_part_spec.start_part >= m_tot_parts) {
    /* Should only happen with SQL HANDLER! */
    DBUG_ASSERT(m_table->open_by_handler);
    return HA_ERR_END_OF_FILE;
  }

  /*
    We should consider if this should be split into three functions as
    partition_read_range is_next_same are always local constants
  */

  if (is_next_same) {
    error = index_next_same_in_part(m_part_spec.start_part, buf,
                                    m_start_key.key, m_start_key.length);
  } else if (m_index_scan_type == PARTITION_READ_RANGE) {
    DBUG_ASSERT(buf == m_table->record[0]);
    error = read_range_next_in_part(m_part_spec.start_part, nullptr);
  } else {
    error = index_next_in_part(m_part_spec.start_part, buf);
  }

  if (error == HA_ERR_END_OF_FILE) {
    m_part_spec.start_part++;  // Start using next part
    error = handle_unordered_scan_next_partition(buf);
  } else {
    m_last_part = m_part_spec.start_part;
  }
  return error;
}

/**
  Handle index_next when changing to new partition.

  This routine is used to start the index scan on the next partition.
  Both initial start and after completing scan on one partition.

  @param[out] buf  Read row in MySQL Row Format

  @return Operation status.
    @retval HA_ERR_END_OF_FILE  End of scan
    @retval 0                   Success
    @retval other               Error code
*/

int Partition_helper::handle_unordered_scan_next_partition(uchar *buf) {
  uint i = m_part_spec.start_part;
  int saved_error = HA_ERR_END_OF_FILE;
  DBUG_TRACE;

  if (i)
    i = m_part_info->get_next_used_partition(i - 1);
  else
    i = m_part_info->get_first_used_partition();

  for (; i <= m_part_spec.end_part;
       i = m_part_info->get_next_used_partition(i)) {
    int error;
    m_part_spec.start_part = i;
    switch (m_index_scan_type) {
      case PARTITION_READ_RANGE:
        DBUG_ASSERT(buf == m_table->record[0]);
        DBUG_PRINT("info", ("read_range_first on partition %d", i));
        error = read_range_first_in_part(
            i, nullptr, m_start_key.key ? &m_start_key : nullptr,
            m_handler->end_range, false);
        break;
      case PARTITION_INDEX_READ:
        DBUG_PRINT("info", ("index_read on partition %d", i));
        error = index_read_map_in_part(
            i, buf, m_start_key.key, m_start_key.keypart_map, m_start_key.flag);
        break;
      case PARTITION_INDEX_FIRST:
        DBUG_PRINT("info", ("index_first on partition %d", i));
        error = index_first_in_part(i, buf);
        break;
      case PARTITION_INDEX_FIRST_UNORDERED:
        /* When is this ever used? */
        DBUG_ASSERT(0);
        /*
          We perform a scan without sorting and this means that we
          should not use the index_first since not all handlers
          support it and it is also unnecessary to restrict sort
          order.
        */
        DBUG_PRINT("info", ("read_range_first on partition %d", i));
        DBUG_ASSERT(buf == m_table->record[0]);
        error = read_range_first_in_part(i, nullptr, nullptr,
                                         m_handler->end_range, false);
        break;
      default:
        DBUG_ASSERT(0);
        return HA_ERR_INTERNAL_ERROR;
    }
    if (!error) {
      m_last_part = i;
      return 0;
    }
    if ((error != HA_ERR_END_OF_FILE) && (error != HA_ERR_KEY_NOT_FOUND))
      return error;

    /*
      If HA_ERR_KEY_NOT_FOUND, we must return that error instead of
      HA_ERR_END_OF_FILE, to be able to continue search.
    */
    if (saved_error != HA_ERR_KEY_NOT_FOUND) saved_error = error;
    DBUG_PRINT("info", ("END_OF_FILE/KEY_NOT_FOUND on partition %d", i));
  }
  if (saved_error == HA_ERR_END_OF_FILE)
    m_part_spec.start_part = NO_CURRENT_PART_ID;
  return saved_error;
}

/**
  Common routine to start index scan with ordered results.

  @param[out] buf  Read row in MySQL Row Format

  @return Operation status
    @retval HA_ERR_END_OF_FILE    End of scan
    @retval HA_ERR_KEY_NOT_FOUND  End of scan
    @retval 0                     Success
    @retval other                 Error code

  @details
    This part contains the logic to handle index scans that require ordered
    output. This includes all except those started by read_range_first with
    the flag ordered set to false. Thus most direct index_read and all
    index_first and index_last.

    We implement ordering by keeping one record plus a key buffer for each
    partition. Every time a new entry is requested we will fetch a new
    entry from the partition that is currently not filled with an entry.
    Then the entry is put into its proper sort position.

    Returning a record is done by getting the top record, copying the
    record to the request buffer and setting the partition as empty on
    entries.
*/

int Partition_helper::handle_ordered_index_scan(uchar *buf) {
  uint i;
  std::vector<uchar *> parts;
  bool found = false;
  uchar *part_rec_buf_ptr = m_ordered_rec_buffer;
  int saved_error = HA_ERR_END_OF_FILE;
  DBUG_TRACE;
  DBUG_ASSERT(part_rec_buf_ptr);

  if (m_key_not_found) {
    m_key_not_found = false;
    bitmap_clear_all(&m_key_not_found_partitions);
    DBUG_PRINT("info", ("Cleared m_key_not_found_partitions"));
  }
  m_top_entry = NO_CURRENT_PART_ID;
  m_queue->clear();
  parts.reserve(m_queue->capacity());
  DBUG_ASSERT(m_part_info->is_partition_used(m_part_spec.start_part));

  /*
    Position part_rec_buf_ptr to point to the first used partition >=
    start_part. There may be partitions marked by used_partitions,
    but is before start_part. These partitions has allocated record buffers
    but is dynamically pruned, so those buffers must be skipped.
  */
  for (i = m_part_info->get_first_used_partition(); i < m_part_spec.start_part;
       i = m_part_info->get_next_used_partition(i)) {
    part_rec_buf_ptr += m_rec_offset + m_rec_length;
  }
  DBUG_PRINT("info", ("m_part_spec.start_part %u first_used_part %u",
                      m_part_spec.start_part, i));
  for (/* continue from above */; i <= m_part_spec.end_part;
       i = m_part_info->get_next_used_partition(i)) {
    DBUG_PRINT("info", ("reading from part %u (scan_type: %u inx: %u)", i,
                        m_index_scan_type, m_handler->active_index));
    DBUG_ASSERT(i == uint2korr(part_rec_buf_ptr));
    uchar *rec_buf_ptr = part_rec_buf_ptr + m_rec_offset;
    uchar *read_buf;
    int error;
    DBUG_PRINT("info", ("part %u, scan_type %d", i, m_index_scan_type));

    /* ICP relies on Item evaluation, which expects the row in record[0]. */
    if (m_handler->pushed_idx_cond)
      read_buf = m_table->record[0];
    else
      read_buf = rec_buf_ptr;

    switch (m_index_scan_type) {
      case PARTITION_INDEX_READ:
        error =
            index_read_map_in_part(i, read_buf, m_start_key.key,
                                   m_start_key.keypart_map, m_start_key.flag);
        break;
      case PARTITION_INDEX_FIRST:
        error = index_first_in_part(i, read_buf);
        break;
      case PARTITION_INDEX_LAST:
        error = index_last_in_part(i, read_buf);
        break;
      case PARTITION_INDEX_READ_LAST:
        error = index_read_last_map_in_part(i, read_buf, m_start_key.key,
                                            m_start_key.keypart_map);
        break;
      case PARTITION_READ_RANGE: {
        /*
          To enable optimization in derived engines, we provide a read buffer
          pointer if we want to read into something different than
          table->record[0] (which read_range_* always uses).
        */
        error = read_range_first_in_part(
            i, read_buf == m_table->record[0] ? nullptr : read_buf,
            m_start_key.key ? &m_start_key : nullptr, m_handler->end_range,
            true);
        break;
      }
      default:
        DBUG_ASSERT(false);
        return HA_ERR_END_OF_FILE;
    }
    DBUG_PRINT("info", ("error %d from partition %u", error, i));
    /* When using ICP, copy record[0] to the priority queue for sorting. */
    if (m_handler->pushed_idx_cond) memcpy(rec_buf_ptr, read_buf, m_rec_length);
    if (!error) {
      found = true;
      if (m_ref_usage != REF_NOT_USED) {
        /* position_in_last_part needs m_last_part set. */
        m_last_part = i;
        position_in_last_part(part_rec_buf_ptr + PARTITION_BYTES_IN_POS,
                              rec_buf_ptr);
      }
      /*
        Save for later insertion in queue;
      */
      parts.push_back(part_rec_buf_ptr);
      DBUG_DUMP("row", read_buf, m_rec_length);
    } else if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
      return error;
    } else if (error == HA_ERR_KEY_NOT_FOUND) {
      DBUG_PRINT("info", ("HA_ERR_KEY_NOT_FOUND from partition %u", i));
      bitmap_set_bit(&m_key_not_found_partitions, i);
      m_key_not_found = true;
      saved_error = error;
    }
    part_rec_buf_ptr += m_rec_offset + m_rec_length;
  }
  if (found) {
    /*
      We found at least one partition with data, now sort all entries and
      after that read the first entry and copy it to the buffer to return in.
    */
    m_queue->m_max_at_top = m_reverse_order;
    m_queue->m_keys = m_curr_key_info;
    DBUG_ASSERT(m_queue->empty());
    /*
      If PK, we should not sort by rowid, since that is already done
      through the KEY setup.
    */
    DBUG_ASSERT(!m_curr_key_info[1] || m_ref_usage == REF_NOT_USED);
    m_queue->assign(parts);
    return_top_record(buf);
    DBUG_PRINT("info", ("Record returned from partition %d", m_top_entry));
    return 0;
  }
  return saved_error;
}

/**
  Return the top record in sort order.

  @param[out] buf  Row returned in MySQL Row Format.
*/

void Partition_helper::return_top_record(uchar *buf) {
  uint part_id;
  uchar *key_buffer = m_queue->top();
  uchar *rec_buffer = key_buffer + m_rec_offset;

  part_id = uint2korr(key_buffer);
  copy_cached_row(buf, rec_buffer);
  DBUG_PRINT("info", ("from part_id %u", part_id));
  DBUG_DUMP("returned_row", buf, m_table->s->reclength);
  m_last_part = part_id;
  m_top_entry = part_id;
}

/**
  Add index_next/prev results from partitions without exact match.

  If there where any partitions that returned HA_ERR_KEY_NOT_FOUND when
  ha_index_read_map was done, those partitions must be included in the
  following index_next/prev call.
*/

int Partition_helper::handle_ordered_index_scan_key_not_found() {
  int error;
  uint i;
  size_t old_elements = m_queue->size();
  uchar *part_buf = m_ordered_rec_buffer;
  uchar *curr_rec_buf = nullptr;
  DBUG_TRACE;
  DBUG_ASSERT(m_key_not_found);
  DBUG_ASSERT(part_buf);
  /*
    Loop over all used partitions to get the correct offset
    into m_ordered_rec_buffer.
  */
  for (i = m_part_info->get_first_used_partition(); i < MY_BIT_NONE;
       i = m_part_info->get_next_used_partition(i)) {
    if (bitmap_is_set(&m_key_not_found_partitions, i)) {
      /*
        This partition is used and did return HA_ERR_KEY_NOT_FOUND
        in index_read_map.
      */
      uchar *read_buf;
      curr_rec_buf = part_buf + m_rec_offset;
      /* ICP relies on Item evaluation, which expects the row in record[0]. */
      if (m_handler->pushed_idx_cond)
        read_buf = m_table->record[0];
      else
        read_buf = curr_rec_buf;

      if (m_reverse_order)
        error = index_prev_in_part(i, read_buf);
      else
        error = index_next_in_part(i, read_buf);
      /* HA_ERR_KEY_NOT_FOUND is not allowed from index_next! */
      DBUG_ASSERT(error != HA_ERR_KEY_NOT_FOUND);
      DBUG_PRINT("info", ("Filling from partition %u reverse %u error %d", i,
                          m_reverse_order, error));
      if (!error) {
        /* When using ICP, copy record[0] to the priority queue for sorting. */
        if (m_handler->pushed_idx_cond)
          memcpy(curr_rec_buf, read_buf, m_rec_length);
        if (m_ref_usage != REF_NOT_USED) {
          /* position_in_last_part needs m_last_part set. */
          m_last_part = i;
          position_in_last_part(part_buf + PARTITION_BYTES_IN_POS,
                                curr_rec_buf);
        }
        m_queue->push(part_buf);
      } else if (error != HA_ERR_END_OF_FILE && error != HA_ERR_KEY_NOT_FOUND)
        return error;
    }
    part_buf += m_rec_offset + m_rec_length;
  }
  DBUG_ASSERT(curr_rec_buf);
  bitmap_clear_all(&m_key_not_found_partitions);
  m_key_not_found = false;

  if (m_queue->size() > old_elements) {
    /* Update m_top_entry, which may have changed. */
    uchar *key_buffer = m_queue->top();
    m_top_entry = uint2korr(key_buffer);
  }
  return 0;
}

/**
  Common routine to handle index_next with ordered results.

  @param[out] buf        Read row in MySQL Row Format.
  @param[in]  is_next_same  Called from index_next_same.

  @return Operation status.
    @retval HA_ERR_END_OF_FILE  End of scan
    @retval 0                   Success
    @retval other               Error code
*/

int Partition_helper::handle_ordered_next(uchar *buf, bool is_next_same) {
  int error;
  uint part_id = m_top_entry;
  uchar *rec_buf = m_queue->empty() ? nullptr : m_queue->top() + m_rec_offset;
  uchar *read_buf;
  DBUG_TRACE;

  if (m_reverse_order) {
    /*
      TODO: To support change of direction (index_prev -> index_next,
      index_read_map(HA_READ_KEY_EXACT) -> index_prev etc.)
      We would need to:
      - Step back all cursors we have a buffered row from a previous next/prev
        call (i.e. for all partitions we previously called index_prev, we must
        call index_next and skip that row.
      - empty the priority queue and initialize it again with reverse ordering.
    */
    DBUG_ASSERT(m_table->open_by_handler);
    return HA_ERR_WRONG_COMMAND;
  }

  if (m_key_not_found) {
    if (is_next_same) {
      /* Only rows which match the key. */
      m_key_not_found = false;
      bitmap_clear_all(&m_key_not_found_partitions);
    } else {
      /* There are partitions not included in the index record queue. */
      size_t old_elements = m_queue->size();
      if ((error = handle_ordered_index_scan_key_not_found())) return error;
      /*
        If the queue top changed, i.e. one of the partitions that gave
        HA_ERR_KEY_NOT_FOUND in index_read_map found the next record,
        return it.
        Otherwise replace the old with a call to index_next (fall through).
      */
      if (old_elements != m_queue->size() && part_id != m_top_entry) {
        return_top_record(buf);
        DBUG_PRINT("info", ("Returning row from part %u (prev KEY_NOT_FOUND)",
                            m_top_entry));
        return 0;
      }
    }
  }
  if (part_id >= m_tot_parts) return HA_ERR_END_OF_FILE;

  DBUG_PRINT("info", ("next row from part %u (inx %u)", part_id,
                      m_handler->active_index));

  /* Assert that buffer for fetch is not NULL */
  DBUG_ASSERT(rec_buf);

  /* ICP relies on Item evaluation, which expects the row in record[0]. */
  if (m_handler->pushed_idx_cond)
    read_buf = m_table->record[0];
  else
    read_buf = rec_buf;

  if (is_next_same) {
    error = index_next_same_in_part(part_id, read_buf, m_start_key.key,
                                    m_start_key.length);
  } else if (m_index_scan_type == PARTITION_READ_RANGE) {
    error = read_range_next_in_part(
        part_id, read_buf == m_table->record[0] ? nullptr : read_buf);
  } else {
    error = index_next_in_part(part_id, read_buf);
  }

  if (error) {
    if (error == HA_ERR_END_OF_FILE) {
      /* Return next buffered row */
      if (!m_queue->empty()) m_queue->pop();
      if (m_queue->empty()) {
        /*
          If priority queue is empty, we have finished fetching rows from all
          partitions. Reset the value of next partition to NONE. This would
          imply HA_ERR_END_OF_FILE for all future calls.
        */
        m_top_entry = NO_CURRENT_PART_ID;
      } else {
        return_top_record(buf);
        DBUG_PRINT("info",
                   ("Record returned from partition %u (2)", m_top_entry));
        error = 0;
      }
    }
    return error;
  }
  /* When using ICP, copy record[0] to the priority queue for sorting. */
  if (m_handler->pushed_idx_cond) memcpy(rec_buf, read_buf, m_rec_length);
  if (m_ref_usage != REF_NOT_USED) {
    /* position_in_last_part needs m_last_part set. */
    m_last_part = part_id;
    position_in_last_part(rec_buf - m_rec_offset + PARTITION_BYTES_IN_POS,
                          rec_buf);
  }
  DBUG_DUMP("rec_buf", rec_buf, m_rec_length);
  m_queue->update_top();
  return_top_record(buf);
  DBUG_PRINT("info", ("Record returned from partition %u", m_top_entry));
  return 0;
}

/**
  Common routine to handle index_prev with ordered results.

  @param[out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval HA_ERR_END_OF_FILE  End of scan
    @retval 0                   Success
    @retval other               Error code
*/

int Partition_helper::handle_ordered_prev(uchar *buf) {
  int error;
  uint part_id = m_top_entry;
  uchar *rec_buf = m_queue->empty() ? nullptr : m_queue->top() + m_rec_offset;
  uchar *read_buf;
  DBUG_TRACE;

  if (!m_reverse_order) {
    /* TODO: See comment in handle_ordered_next(). */
    DBUG_ASSERT(m_table->open_by_handler);
    return HA_ERR_WRONG_COMMAND;
  }

  if (m_key_not_found) {
    /* There are partitions not included in the index record queue. */
    size_t old_elements = m_queue->size();
    if ((error = handle_ordered_index_scan_key_not_found())) return error;
    if (old_elements != m_queue->size() && part_id != m_top_entry) {
      /*
        Should only be possible for when HA_READ_KEY_EXACT was previously used,
        which is not supported to have a subsequent call for PREV.
        I.e. HA_READ_KEY_EXACT is considered to not have reverse order!
      */
      DBUG_ASSERT(0);
      /*
        If the queue top changed, i.e. one of the partitions that gave
        HA_ERR_KEY_NOT_FOUND in index_read_map found the next record,
        return it.
        Otherwise replace the old with a call to index_next (fall through).
      */
      return_top_record(buf);
      return 0;
    }
  }

  if (part_id >= m_tot_parts) {
    /* This should never happen, except for SQL HANDLER calls! */
    DBUG_ASSERT(m_table->open_by_handler);
    return HA_ERR_END_OF_FILE;
  }

  /* Assert that buffer for fetch is not NULL */
  DBUG_ASSERT(rec_buf);

  /* ICP relies on Item evaluation, which expects the row in record[0]. */
  if (m_handler->pushed_idx_cond)
    read_buf = m_table->record[0];
  else
    read_buf = rec_buf;

  if ((error = index_prev_in_part(part_id, read_buf))) {
    if (error == HA_ERR_END_OF_FILE) {
      if (!m_queue->empty()) m_queue->pop();
      if (m_queue->empty()) {
        /*
          If priority queue is empty, we have finished fetching rows from all
          partitions. Reset the value of next partition to NONE. This would
          imply HA_ERR_END_OF_FILE for all future calls.
        */
        m_top_entry = NO_CURRENT_PART_ID;
      } else {
        return_top_record(buf);
        DBUG_PRINT("info",
                   ("Record returned from partition %d (2)", m_top_entry));
        error = 0;
      }
    }
    return error;
  }
  /* When using ICP, copy record[0] to the priority queue for sorting. */
  if (m_handler->pushed_idx_cond) memcpy(rec_buf, read_buf, m_rec_length);

  if (m_ref_usage != REF_NOT_USED) {
    /* position_in_last_part needs m_last_part set. */
    m_last_part = part_id;
    position_in_last_part(rec_buf - m_rec_offset + PARTITION_BYTES_IN_POS,
                          rec_buf);
  }
  m_queue->update_top();
  return_top_record(buf);
  DBUG_PRINT("info", ("Record returned from partition %d", m_top_entry));
  return 0;
}

/**
  Get statistics from a specific partition.

  @param[out] stat_info  Area to report values into.
  @param[out] check_sum  Check sum of partition.
  @param[in]  part_id    Partition to report from.
*/
void Partition_helper::get_dynamic_partition_info_low(ha_statistics *stat_info,
                                                      ha_checksum *check_sum,
                                                      uint part_id) {
  ha_statistics *part_stat = &m_handler->stats;
  DBUG_ASSERT(bitmap_is_set(&m_part_info->read_partitions, part_id));
  DBUG_ASSERT(bitmap_is_subset(&m_part_info->read_partitions,
                               &m_part_info->lock_partitions));
  DBUG_ASSERT(bitmap_is_subset(&m_part_info->lock_partitions,
                               &m_part_info->read_partitions));
  bitmap_clear_all(&m_part_info->read_partitions);
  bitmap_set_bit(&m_part_info->read_partitions, part_id);
  m_handler->info(HA_STATUS_TIME | HA_STATUS_VARIABLE |
                  HA_STATUS_VARIABLE_EXTRA | HA_STATUS_NO_LOCK);
  stat_info->records = part_stat->records;
  stat_info->mean_rec_length = part_stat->mean_rec_length;
  stat_info->data_file_length = part_stat->data_file_length;
  stat_info->max_data_file_length = part_stat->max_data_file_length;
  stat_info->index_file_length = part_stat->index_file_length;
  stat_info->delete_length = part_stat->delete_length;
  stat_info->create_time = part_stat->create_time;
  stat_info->update_time = part_stat->update_time;
  stat_info->check_time = part_stat->check_time;
  if (m_handler->ha_table_flags() & HA_HAS_CHECKSUM) {
    *check_sum = checksum_in_part(part_id);
  }
  bitmap_copy(&m_part_info->read_partitions, &m_part_info->lock_partitions);
}
