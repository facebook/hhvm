#ifndef PARTITION_HANDLER_INCLUDED
#define PARTITION_HANDLER_INCLUDED

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

#include <string.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <vector>

#include "my_base.h"  // ha_rows.
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/mysql_mutex.h"
#include "priority_queue.h"
#include "sql/handler.h"        // Handler_share
#include "sql/key.h"            // key_rec_cmp
#include "sql/sql_partition.h"  // part_id_range

class Field;
class THD;
class partition_info;
struct mysql_mutex_t;
template <class Key, class Value>
class collation_unordered_map;

namespace dd {
class Table;
}  // namespace dd
struct TABLE;
struct TABLE_SHARE;

#define PARTITION_BYTES_IN_POS 2

struct MEM_ROOT;

static const uint NO_CURRENT_PART_ID = UINT_MAX32;

/**
  bits in Partition_handler::alter_flags():

  HA_PARTITION_FUNCTION_SUPPORTED indicates that the function is
  supported at all.
  HA_INPLACE_CHANGE_PARTITION means that changes to partitioning can be done
  through in-place ALTER TABLE API but special mark-up in partition_info
  object is required for this.
*/
#define HA_PARTITION_FUNCTION_SUPPORTED (1L << 0)
#define HA_INPLACE_CHANGE_PARTITION (1L << 1)

enum enum_part_operation {
  OPTIMIZE_PARTS = 0,
  ANALYZE_PARTS,
  CHECK_PARTS,
  REPAIR_PARTS,
  ASSIGN_KEYCACHE_PARTS,
  PRELOAD_KEYS_PARTS
};

/** Struct used for partition_name_hash */
struct PART_NAME_DEF {
  uchar *partition_name;
  uint length;
  uint32 part_id;
  bool is_subpart;
};

/**
  Initialize partitioning (currently only PSI keys).
*/
void partitioning_init();

/**
  Partition specific Handler_share.
*/
class Partition_share : public Handler_share {
 public:
  Partition_share();
  ~Partition_share();

  /** Set if auto increment is used an initialized. */
  bool auto_inc_initialized;
  /**
    Mutex protecting next_auto_inc_val.
    Initialized if table uses auto increment.
  */
  mysql_mutex_t *auto_inc_mutex;
  /** First non reserved auto increment value. */
  ulonglong next_auto_inc_val;
  /**
    Hash of partition names. Initialized by the first handler instance of a
    table_share calling populate_partition_name_hash().
    After that it is read-only, i.e. no locking required for reading.
  */
  std::unique_ptr<
      collation_unordered_map<std::string, unique_ptr_my_free<PART_NAME_DEF>>>
      partition_name_hash;

  /**
    Initializes and sets auto_inc_mutex.
    Only needed to be called if the table have an auto increment.
    Must hold TABLE_SHARE::LOCK_ha_data when calling.
  */
  bool init_auto_inc_mutex(TABLE_SHARE *table_share);
  /**
    Release reserved auto increment values not used.
    @param thd             Thread.
    @param table_share     Table Share
    @param next_insert_id  Next insert id (first non used auto inc value).
    @param max_reserved    End of reserved auto inc range.
  */
  void release_auto_inc_if_possible(THD *thd, TABLE_SHARE *table_share,
                                    const ulonglong next_insert_id,
                                    const ulonglong max_reserved);

  /** lock mutex protecting auto increment value next_auto_inc_val. */
  inline void lock_auto_inc() {
    DBUG_ASSERT(auto_inc_mutex);
    mysql_mutex_lock(auto_inc_mutex);
  }
  /** unlock mutex protecting auto increment value next_auto_inc_val. */
  inline void unlock_auto_inc() {
    DBUG_ASSERT(auto_inc_mutex);
    mysql_mutex_unlock(auto_inc_mutex);
  }
  /**
    Populate partition_name_hash with partition and subpartition names
    from part_info.
    @param part_info  Partition info containing all partitions metadata.

    @return Operation status.
      @retval false Success.
      @retval true  Failure.
  */
  bool populate_partition_name_hash(partition_info *part_info);
  /** Get partition name.

  @param part_id  Partition id (for subpartitioned table only subpartition
                  names will be returned.)

  @return partition name or NULL if error.
  */
  const char *get_partition_name(size_t part_id) const;

 private:
  const uchar **partition_names;
  /**
    Insert [sub]partition name into  partition_name_hash
    @param name        Partition name.
    @param part_id     Partition id.
    @param is_subpart  True if subpartition else partition.

    @return Operation status.
      @retval false Success.
      @retval true  Failure.
  */
  bool insert_partition_name_in_hash(const char *name, uint part_id,
                                     bool is_subpart);
};

/**
  Class for partitioning specific operations.

  Returned from handler::get_partition_handler().
*/
class Partition_handler {
 public:
  Partition_handler() {}
  virtual ~Partition_handler() {}

  /**
    Get dynamic table information from partition.

    @param[out] stat_info  Statistics struct to fill in.
    @param[out] check_sum  Check sum value to fill in if supported.
    @param[in]  part_id    Partition to report for.

    @note stat_info and check_sum are initialized by caller.
    check_sum is only expected to be updated if HA_HAS_CHECKSUM.
  */
  virtual void get_dynamic_partition_info(ha_statistics *stat_info,
                                          ha_checksum *check_sum,
                                          uint part_id) = 0;
  /**
    Get default number of partitions.

    Used during creating a partitioned table.

    @param info  Create info.
    @return Number of default partitions.
  */
  virtual int get_default_num_partitions(
      HA_CREATE_INFO *info MY_ATTRIBUTE((unused))) {
    return 1;
  }
  /**
    Setup auto partitioning.

    Called for engines with HA_USE_AUTO_PARTITION to setup the partition info
    object

    @param[in,out] part_info  Partition object to setup.
  */
  virtual void set_auto_partitions(
      partition_info *part_info MY_ATTRIBUTE((unused))) {}
  /**
    Get number of partitions for table in SE

    @param name normalized path(same as open) to the table

    @param[out] num_parts Number of partitions

    @retval false for success
    @retval true for failure, for example table didn't exist in engine
  */
  virtual bool get_num_parts(const char *name MY_ATTRIBUTE((unused)),
                             uint *num_parts) {
    *num_parts = 0;
    return false;
  }
  /**
    Set the partition info object to be used by the handler.

    @param part_info  Partition info to be used by the handler.
    @param early      True if called when part_info only created and parsed,
                      but not setup, checked or fixed.
  */
  virtual void set_part_info(partition_info *part_info, bool early) = 0;

  /**
    Truncate partitions.

    Truncate all partitions matching table->part_info->read_partitions.
    Handler level wrapper for truncating partitions, will ensure that
    mark_trx_read_write() is called and also checks locking assertions.

    @param[in,out]  table_def    dd::Table object for the table. Engines
                                 which support atomic DDL are allowed to
                                 adjust this object. Changes will be saved
                                 to the data-dictionary.

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  int truncate_partition(dd::Table *table_def);

  /**
    Exchange partition.

    @param[in]      part_id           Id of partition to be exchanged.
    @param[in,out]  part_table_def    dd::Table object for partitioned table.
    @param[in,out]  swap_table_def    dd::Table object for non-partitioned
                                      table.

    @note   Both tables are locked in exclusive mode.

    @note   Changes to dd::Table object done by this method will be saved
            to data-dictionary only if storage engine supporting atomic
            DDL (i.e. with HTON_SUPPORTS_ATOMIC_DDL flag).

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  int exchange_partition(uint part_id, dd::Table *part_table_def,
                         dd::Table *swap_table_def);

  /**
    Alter flags.

    Given a set of alter table flags, return which is supported.

    @param flags  Alter table operation flags.

    @return Supported alter table flags.
  */
  virtual uint alter_flags(uint flags MY_ATTRIBUTE((unused))) const {
    return 0;
  }

  /**
    Get partition row type from SE
    @param       table      partition table
    @param       part_id    Id of partition for which row type to be retrieved
    @return      Partition row type.
  */
  virtual enum row_type get_partition_row_type(const dd::Table *table,
                                               uint part_id) = 0;

 private:
  /**
    Truncate partition.

    Low-level primitive for handler, implementing
    Partition_handler::truncate_partition().

    @sa Partition_handler::truncate_partition().
  */
  virtual int truncate_partition_low(dd::Table *) {
    return HA_ERR_WRONG_COMMAND;
  }

  /**
    Exchange partition.

    Low-level primitive which implementation to be provided by SE.

    @sa Partition_handler::exchange_partition().
  */
  virtual int exchange_partition_low(
      uint part_id MY_ATTRIBUTE((unused)),
      dd::Table *part_table_def MY_ATTRIBUTE((unused)),
      dd::Table *swap_table_def MY_ATTRIBUTE((unused))) {
    return HA_ERR_WRONG_COMMAND;
  }

  /**
    Return the table handler.

    For some partitioning specific functions it is still needed to access
    the handler directly for transaction handling (mark_trx_read_write())
    and to assert correct locking.

    @return handler or NULL if not supported.
  */
  virtual handler *get_handler() { return nullptr; }
};

/// Maps compare function to strict weak ordering required by Priority_queue.
struct Key_rec_less {
  typedef int (*key_compare_fun)(KEY **, uchar *, uchar *);

  explicit Key_rec_less(KEY **keys)
      : m_keys(keys), m_fun(key_rec_cmp), m_max_at_top(false) {}

  bool operator()(uchar *first, uchar *second) {
    const int cmpval =
        (*m_fun)(m_keys, first + m_rec_offset, second + m_rec_offset);
    return m_max_at_top ? cmpval < 0 : cmpval > 0;
  }

  KEY **m_keys;
  key_compare_fun m_fun;
  uint m_rec_offset;
  bool m_max_at_top;
};

/**
  Partition_helper is a helper class that implements most generic partitioning
  functionality such as:
  table scan, index scan (both ordered and non-ordered),
  insert (write_row()), delete and update.
  And includes ALTER TABLE ... ADD/COALESCE/DROP/REORGANIZE/... PARTITION
  support.
  It also implements a cache for the auto increment value and check/repair for
  rows in wrong partition.

  How to use it:
  Inherit it and implement:
  - *_in_part() functions for row operations.
  - write_row_in_new_part() for handling 'fast' alter partition.
*/
class Partition_helper {
  typedef Priority_queue<uchar *, std::vector<uchar *>, Key_rec_less>
      Prio_queue;

 public:
  Partition_helper(handler *main_handler);
  virtual ~Partition_helper();

  void set_active_index(uint inx) { m_handler->active_index = inx; }

  /**
    Set partition info.

    To be called from Partition_handler.

    @param  part_info  Partition info to use.
    @param  early      True if called when part_info only created and parsed,
                       but not setup, checked or fixed.
  */
  virtual void set_part_info_low(partition_info *part_info, bool early);
  /**
    Initialize variables used before the table is opened.

    @param mem_root  Memory root to allocate things from (not yet used).

    @return Operation status.
      @retval false success.
      @retval true  failure.
  */
  bool init_partitioning(MEM_ROOT *mem_root MY_ATTRIBUTE((unused))) {
#ifndef DBUG_OFF
    m_key_not_found_partitions.bitmap = nullptr;
#endif
    return false;
  }

  /**
    INSERT/UPDATE/DELETE functions.
    @see handler.h
    @{
  */

  /**
    Insert a row to the partitioned table.

    @param buf The row in MySQL Row Format.

    @return Operation status.
      @retval    0 Success
      @retval != 0 Error code
  */
  int ph_write_row(uchar *buf);
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
  int ph_update_row(const uchar *old_data, uchar *new_data);
  /**
    Delete an existing row in the partitioned table.

    This will delete a row. buf will contain a copy of the row to be deleted.
    The server will call this right after the current row has been read
    (from either a previous rnd_xxx() or index_xxx() call).
    If you keep a pointer to the last row or can access a primary key it will
    make doing the deletion quite a bit easier.
    Keep in mind that the server does no guarantee consecutive deletions.
    ORDER BY clauses can be used.

    buf is either record[0] or record[1]

    @param buf  The record in MySQL Row Format.

    @return Operation status.
      @retval    0 Success
      @retval != 0 Error code
  */
  int ph_delete_row(const uchar *buf);

  /** @} */

  /** Release unused auto increment values. */
  void ph_release_auto_increment();
  /**
    Calculate key hash value from an null terminated array of fields.
    Support function for KEY partitioning.

    @param field_array   An array of the fields in KEY partitioning

    @return hash_value calculated

    @note Uses the hash function on the character set of the field.
    Integer and floating point fields use the binary character set by default.
  */
  static uint32 ph_calculate_key_hash_value(Field **field_array);

  /**
    MODULE full table scan

    This module is used for the most basic access method for any table
    handler. This is to fetch all data through a full table scan. No
    indexes are needed to implement this part.
    It contains one method to start the scan (rnd_init) that can also be
    called multiple times (typical in a nested loop join). Then proceeding
    to the next record (rnd_next) and closing the scan (rnd_end).
    To remember a record for later access there is a method (position)
    and there is a method used to retrieve the record based on the stored
    position.
    The position can be a file position, a primary key, a ROWID dependent
    on the handler below.

    unlike index_init(), rnd_init() can be called two times
    without rnd_end() in between (it only makes sense if scan=1).
    then the second call should prepare for the new table scan
    (e.g if rnd_init allocates the cursor, second call should
    position it to the start of the table, no need to deallocate
    and allocate it again.
    @see handler.h
    @{
  */

  int ph_rnd_init(bool scan);
  int ph_rnd_end();
  int ph_rnd_next(uchar *buf);
  void ph_position(const uchar *record);

  /** @} */

  /**
    MODULE index scan

    This part of the handler interface is used to perform access through
    indexes. The interface is defined as a scan interface but the handler
    can also use key lookup if the index is a unique index or a primary
    key index.
    Index scans are mostly useful for SELECT queries but are an important
    part also of UPDATE, DELETE, REPLACE and CREATE TABLE table AS SELECT
    and so forth.
    Naturally an index is needed for an index scan and indexes can either
    be ordered, hash based. Some ordered indexes can return data in order
    but not necessarily all of them.
    There are many flags that define the behavior of indexes in the
    various handlers. These methods are found in the optimizer module.
    -------------------------------------------------------------------------

    index_read is called to start a scan of an index. The find_flag defines
    the semantics of the scan. These flags are defined in
    include/my_base.h
    index_read_idx is the same but also initializes index before calling doing
    the same thing as index_read. Thus it is similar to index_init followed
    by index_read. This is also how we implement it.

    index_read/index_read_idx does also return the first row. Thus for
    key lookups, the index_read will be the only call to the handler in
    the index scan.

    index_init initializes an index before using it and index_end does
    any end processing needed.
    @{
  */

  int ph_index_init_setup(uint key_nr, bool sorted);
  /*
    These methods are used to jump to next or previous entry in the index
    scan. There are also methods to jump to first and last entry.
  */
  int ph_index_first(uchar *buf);
  int ph_index_last(uchar *buf);
  int ph_index_next(uchar *buf);
  int ph_index_next_same(uchar *buf, uint keylen);
  int ph_index_prev(uchar *buf);
  int ph_index_read_map(uchar *buf, const uchar *key, key_part_map keypart_map,
                        enum ha_rkey_function find_flag);
  int ph_index_read_last_map(uchar *buf, const uchar *key,
                             key_part_map keypart_map);
  int ph_index_read_idx_map(uchar *buf, uint index, const uchar *key,
                            key_part_map keypart_map,
                            enum ha_rkey_function find_flag);
  int ph_read_range_first(const key_range *start_key, const key_range *end_key,
                          bool eq_range_arg, bool sorted);
  int ph_read_range_next();
  /** @} */

  /**
    Functions matching Partition_handler API.
    @{
  */

  /**
    Get statistics from a specific partition.
    @param[out] stat_info  Area to report values into.
    @param[out] check_sum  Check sum of partition.
    @param[in]  part_id    Partition to report from.
  */
  virtual void get_dynamic_partition_info_low(ha_statistics *stat_info,
                                              ha_checksum *check_sum,
                                              uint part_id);

  /**
    Prepare for reorganizing partitions by setting up
    partition_info::read_partitions according to the partition_info
    mark-up.

    This is helper method which can also be used by SEs implementing
    support for reorganizing partitions through ALTER TABLE INPLACE
    SE API.
  */
  void prepare_change_partitions();

  /** @} */

 protected:
  /* Common helper functions to be used by inheriting engines. */

  /*
    open/close functions.
  */

  /**
    Set m_part_share, Allocate internal bitmaps etc. used by open tables.

    @return Operation status.
      @retval false success.
      @retval true  failure.
  */
  bool open_partitioning(Partition_share *part_share);
  /**
    Close partitioning for a table.

    Frees memory and release other resources.
  */
  void close_partitioning();

  /**
    Lock auto increment value if needed.
  */
  void lock_auto_increment();

  /**
    unlock auto increment.
  */
  inline void unlock_auto_increment() {
    /*
      If m_auto_increment_safe_stmt_log_lock is true, we have to keep the lock.
      It will be set to false and thus unlocked at the end of the statement by
      ha_partition::release_auto_increment.
    */
    if (m_auto_increment_lock && !m_auto_increment_safe_stmt_log_lock) {
      m_part_share->unlock_auto_inc();
      m_auto_increment_lock = false;
    }
  }
  /**
    Get auto increment.

    Only to be used for auto increment values that are the first field in
    an unique index.

    @param[in]  increment           Increment between generated numbers.
    @param[in]  nb_desired_values   Number of values requested.
    @param[out] first_value         First reserved value (ULLONG_MAX on error).
    @param[out] nb_reserved_values  Number of values reserved.
  */
  void get_auto_increment_first_field(ulonglong increment,
                                      ulonglong nb_desired_values,
                                      ulonglong *first_value,
                                      ulonglong *nb_reserved_values);

  /**
    Initialize the record priority queue used for sorted index scans.
    @return Operation status.
      @retval    0   Success.
      @retval != 0   Error code.
  */
  int init_record_priority_queue();
  /**
    Destroy the record priority queue used for sorted index scans.
  */
  void destroy_record_priority_queue();
  /*
    Administrative support functions.
  */

  /** Print partitioning specific error.
    @param error   Error code.
    @return false if error is printed else true.
  */
  bool print_partition_error(int error);
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
  bool print_admin_msg(THD *thd, uint len, const char *msg_type,
                       const char *db_name, const char *table_name,
                       const char *op_name, const char *fmt, ...)
      MY_ATTRIBUTE((format(printf, 8, 9)));

  /**
    Check/fix misplaced rows.

    @param read_part_id  Partition to check/fix.
    @param repair   If true, move misplaced rows to correct partition.

    @return Operation status.
      @retval    0  Success
      @retval != 0  Error
  */
  int check_misplaced_rows(uint read_part_id, bool repair);
  /**
    Set used partitions bitmap from Alter_info.

    @return false if success else true.
  */
  bool set_altered_partitions();

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
  virtual int copy_partitions(ulonglong *const deleted);

 private:
  enum partition_index_scan_type {
    PARTITION_INDEX_READ = 1,
    PARTITION_INDEX_FIRST,
    PARTITION_INDEX_FIRST_UNORDERED,
    PARTITION_INDEX_LAST,
    PARTITION_INDEX_READ_LAST,
    PARTITION_READ_RANGE,
    PARTITION_NO_INDEX_SCAN
  };

  /** handler to use (ha_innopart etc.) */
  handler *m_handler;
  /*
    Access methods to protected areas in handler to avoid adding
    friend class Partition_helper in class handler.
  */
  virtual THD *get_thd() const = 0;
  virtual TABLE *get_table() const = 0;
  virtual bool get_eq_range() const = 0;
  virtual void set_eq_range(bool eq_range) = 0;
  virtual void set_range_key_part(KEY_PART_INFO *key_part) = 0;

  /*
    Implementation of per partition operation by instantiated engine.
    These must be implemented in the 'real' partition_helper subclass.
  */

  /**
    Write a row in the specified partition.

    @see handler::write_row().

    @param  part_id  Partition to write to.
    @param  buf      Buffer with data to write.

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  virtual int write_row_in_part(uint part_id, uchar *buf) = 0;
  /**
    Update a row in the specified partition.

    @see handler::update_row().

    @param  part_id   Partition to update in.
    @param  old_data  Buffer containing old row.
    @param  new_data  Buffer containing new row.

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  virtual int update_row_in_part(uint part_id, const uchar *old_data,
                                 uchar *new_data) = 0;
  /**
    Delete an existing row in the specified partition.

    @see handler::delete_row().

    @param  part_id  Partition to delete from.
    @param  buf      Buffer containing row to delete.

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  virtual int delete_row_in_part(uint part_id, const uchar *buf) = 0;
  /**
    Initialize the shared auto increment value.

    @param no_lock  If HA_STATUS_NO_LOCK should be used in info(HA_STATUS_AUTO).

    Also sets stats.auto_increment_value.
  */
  virtual int initialize_auto_increment(bool no_lock) = 0;
  /** Release auto_increment in all underlying partitions. */
  virtual void release_auto_increment_all_parts() {}
  /** Save or persist the current max auto increment. */
  virtual void save_auto_increment(ulonglong nr MY_ATTRIBUTE((unused))) {}
  /**
    Per partition equivalent of rnd_* and index_* functions.

    @see class handler.
  */
  virtual int rnd_init_in_part(uint part_id, bool table_scan) = 0;
  int ph_rnd_next_in_part(uint part_id, uchar *buf);
  virtual int rnd_next_in_part(uint part_id, uchar *buf) = 0;
  virtual int rnd_end_in_part(uint part_id, bool scan) = 0;
  virtual void position_in_last_part(uchar *ref, const uchar *row) = 0;
  virtual int index_first_in_part(uint part, uchar *buf) = 0;
  virtual int index_last_in_part(uint part, uchar *buf) = 0;
  virtual int index_prev_in_part(uint part, uchar *buf) = 0;
  virtual int index_next_in_part(uint part, uchar *buf) = 0;
  virtual int index_next_same_in_part(uint part, uchar *buf, const uchar *key,
                                      uint length) = 0;
  virtual int index_read_map_in_part(uint part, uchar *buf, const uchar *key,
                                     key_part_map keypart_map,
                                     enum ha_rkey_function find_flag) = 0;
  virtual int index_read_last_map_in_part(uint part, uchar *buf,
                                          const uchar *key,
                                          key_part_map keypart_map) = 0;
  /**
    Do read_range_first in the specified partition.
    If buf is set, then copy the result there instead of table->record[0].
  */
  virtual int read_range_first_in_part(uint part, uchar *buf,
                                       const key_range *start_key,
                                       const key_range *end_key,
                                       bool sorted) = 0;
  /**
    Do read_range_next in the specified partition.
    If buf is set, then copy the result there instead of table->record[0].
  */
  virtual int read_range_next_in_part(uint part, uchar *buf) = 0;
  virtual int index_read_idx_map_in_part(uint part, uchar *buf, uint index,
                                         const uchar *key,
                                         key_part_map keypart_map,
                                         enum ha_rkey_function find_flag) = 0;
  /**
    Initialize engine specific resources for the record priority queue
    used duing ordered index reads for multiple partitions.

    @param used_parts  Number of partitions used in query
                       (number of set bits in m_part_info->read_partitions).

    @return Operation status.
      @retval    0   Success.
      @retval != 0   Error code.
  */
  virtual int init_record_priority_queue_for_parts(
      uint used_parts MY_ATTRIBUTE((unused))) {
    return 0;
  }
  /**
    Destroy and release engine specific resources used by the record
    priority queue.
  */
  virtual void destroy_record_priority_queue_for_parts() {}
  /**
    Checksum for a partition.

    @param part_id  Partition to checksum.
  */
  virtual ha_checksum checksum_in_part(
      uint part_id MY_ATTRIBUTE((unused))) const {
    DBUG_ASSERT(0);
    return 0;
  }
  /**
    Copy a cached row.

    Used when copying a row from the record priority queue to the return buffer.
    For some engines, like InnoDB, only marked columns must be copied,
    to preserve non-read columns.

    @param[out] to_rec    Buffer to copy to.
    @param[in]  from_rec  Buffer to copy from.
  */
  virtual void copy_cached_row(uchar *to_rec, const uchar *from_rec) {
    memcpy(to_rec, from_rec, m_rec_length);
  }

  /**
    write row to new partition.
    @param  new_part   New partition to write to.

    @return Operation status.
      @retval    0  Success.
      @retval != 0  Error code.
  */
  virtual int write_row_in_new_part(uint new_part) = 0;

  /* Internal helper functions*/
  /**
    Update auto increment value if current row contains a higher value.
  */
  inline void set_auto_increment_if_higher();
  /**
    Common routine to set up index scans.

    Find out which partitions we'll need to read when scanning the specified
    range.

    If we need to scan only one partition, set m_ordered_scan_ongoing=false
    as we will not need to do merge ordering.

    @param buf            Buffer to later return record in (this function
                          needs it to calculate partitioning function values)

    @param idx_read_flag  True <=> m_start_key has range start endpoint which
                          probably can be used to determine the set of
                          partitions to scan.
                          False <=> there is no start endpoint.

    @return Operation status.
      @retval   0  Success
      @retval !=0  Error code
  */
  int partition_scan_set_up(uchar *buf, bool idx_read_flag);
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
  int handle_unordered_next(uchar *buf, bool is_next_same);
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
  int handle_unordered_scan_next_partition(uchar *buf);
  /**
    Common routine to start index scan with ordered results.

    @param[out] buf  Read row in MySQL Row Format

    @return Operation status
      @retval HA_ERR_END_OF_FILE    End of scan
      @retval HA_ERR_KEY_NOT_FOUND  End of scan
      @retval 0                     Success
      @retval other                 Error code
  */
  int handle_ordered_index_scan(uchar *buf);
  /**
    Add index_next/prev results from partitions without exact match.

    If there where any partitions that returned HA_ERR_KEY_NOT_FOUND when
    ha_index_read_map was done, those partitions must be included in the
    following index_next/prev call.

    @return Operation status
      @retval HA_ERR_END_OF_FILE    End of scan
      @retval 0                     Success
      @retval other                 Error code
  */
  int handle_ordered_index_scan_key_not_found();
  /**
    Common routine to handle index_prev with ordered results.

    @param[out] buf  Read row in MySQL Row Format.

    @return Operation status.
      @retval HA_ERR_END_OF_FILE  End of scan
      @retval 0                   Success
      @retval other               Error code
  */
  int handle_ordered_prev(uchar *buf);
  /**
    Common routine to handle index_next with ordered results.

    @param[out] buf        Read row in MySQL Row Format.
    @param[in]  is_next_same  Called from index_next_same.

    @return Operation status.
      @retval HA_ERR_END_OF_FILE  End of scan
      @retval 0                   Success
      @retval other               Error code
  */
  int handle_ordered_next(uchar *buf, bool is_next_same);
  /**
    Common routine for a number of index_read variants.

    @param[out] buf             Buffer where the record should be returned.
    @param[in]  have_start_key  true <=> the left endpoint is available, i.e.
                                we're in index_read call or in read_range_first
                                call and the range has left endpoint.
                                false <=> there is no left endpoint (we're in
                                read_range_first() call and the range has no
                                left endpoint).

    @return Operation status
      @retval 0                    OK
      @retval HA_ERR_END_OF_FILE   Whole index scanned, without finding the
    record.
      @retval HA_ERR_KEY_NOT_FOUND Record not found, but index cursor
    positioned.
      @retval other                Error code.
  */
  int common_index_read(uchar *buf, bool have_start_key);
  /**
    Common routine for index_first/index_last.

    @param[out] buf  Read row in MySQL Row Format.

    @return Operation status.
      @retval    0  Success
      @retval != 0  Error code
  */
  int common_first_last(uchar *buf);
  /**
    Return the top record in sort order.

    @param[out] buf  Row returned in MySQL Row Format.
  */
  void return_top_record(uchar *buf);

  /**
    Set table->read_set taking partitioning expressions into account.
  */
  void set_partition_read_set();

  /*
    These could be private as well,
    but easier to expose them to derived classes to use.
  */
 protected:
  /** Convenience pointer to table from m_handler (i.e. m_handler->table). */
  TABLE *m_table;

  /** All internal partitioning data! @{ */
  /** Tables partitioning info (same as table->part_info) */
  partition_info *m_part_info;
  /** Is primary key clustered. */
  bool m_pkey_is_clustered;
  /** Cached value of m_part_info->is_sub_partitioned(). */
  bool m_is_sub_partitioned;
  /** Total number of partitions. */
  uint m_tot_parts;
  uint m_last_part;        // Last accessed partition.
  const uchar *m_err_rec;  // record which gave error.
  bool m_auto_increment_safe_stmt_log_lock;
  bool m_auto_increment_lock;
  part_id_range m_part_spec;                         // Which parts to scan
  uint m_scan_value;                                 // Value passed in rnd_init
                                                     // call
  key_range m_start_key;                             // index read key range
  enum partition_index_scan_type m_index_scan_type;  // What type of index
                                                     // scan
  uint m_rec_length;  // Local copy of record length

  bool m_ordered;               // Ordered/Unordered index scan.
  bool m_ordered_scan_ongoing;  // Ordered index scan ongoing.
  bool m_reverse_order;         // Scanning in reverse order (prev).
  /** Row and key buffer for ordered index scan. */
  uchar *m_ordered_rec_buffer;
  /** Prio queue used by sorted read. */
  Prio_queue *m_queue;
  /** Which partition is to deliver next result. */
  uint m_top_entry;
  /** Offset in m_ordered_rec_buffer from part buffer to its record buffer. */
  uint m_rec_offset;
  /**
    Current index used for sorting.
    If clustered PK exists, then it will be used as secondary index to
    sort on if the first is equal in key_rec_cmp.
    So if clustered pk: m_curr_key_info[0]= current index and
    m_curr_key_info[1]= pk and [2]= NULL.
    Otherwise [0]= current index, [1]= NULL, and we will
    sort by rowid as secondary sort key if equal first key.
  */
  KEY *m_curr_key_info[3];
  enum enum_using_ref {
    /** handler::ref is not copied to the PQ. */
    REF_NOT_USED = 0,
    /**
      handler::ref is copied to the PQ but does not need to be used in sorting.
    */
    REF_STORED_IN_PQ,
    /** handler::ref is copied to the PQ and must be used during sorting. */
    REF_USED_FOR_SORT
  };
  /** How handler::ref is used in the priority queue. */
  enum_using_ref m_ref_usage;
  /** Set if previous index_* call returned HA_ERR_KEY_NOT_FOUND. */
  bool m_key_not_found;
  /** Partitions that returned HA_ERR_KEY_NOT_FOUND. */
  MY_BITMAP m_key_not_found_partitions;
  /** @} */

 private:
  /** Partition share for auto_inc handling. */
  Partition_share *m_part_share;
};
#endif /* PARTITION_HANDLER_INCLUDED */
