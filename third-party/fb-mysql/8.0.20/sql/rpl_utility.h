/* Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_UTILITY_H
#define RPL_UTILITY_H

#ifndef __cplusplus
#error "Don't include this C++ header file from a non-C++ file!"
#endif

#include <sys/types.h>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "field_types.h"  // enum_field_types
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "sql/psi_memory_key.h"

struct MY_BITMAP;

#ifdef MYSQL_SERVER
#include <memory>
#include <unordered_map>

#include "map_helpers.h"
#include "prealloced_array.h"  // Prealloced_array
#include "sql/table.h"         // TABLE_LIST

class Log_event;
class Relay_log_info;
class THD;

/**
   Hash table used when applying row events on the slave and there is
   no index on the slave's table.
 */

struct HASH_ROW_POS {
  /**
      Points at the position where the row starts in the
      event buffer (ie, area in memory before unpacking takes
      place).
  */
  const uchar *bi_start;
  const uchar *bi_ends;
};

struct HASH_ROW_ENTRY;

struct hash_slave_rows_free_entry {
  void operator()(HASH_ROW_ENTRY *entry) const;
};

/**
   Internal structure that acts as a preamble for HASH_ROW_POS
   in memory structure.

   Allocation is done in Hash_slave_rows::make_entry as part of
   the entry allocation.
 */
struct HASH_ROW_PREAMBLE {
  HASH_ROW_PREAMBLE() = default;
  /*
    The actual key.
   */
  uint hash_value;

  /**
    The search state used to iterate over multiple entries for a
    given key.
   */
  malloc_unordered_multimap<
      uint, std::unique_ptr<HASH_ROW_ENTRY, hash_slave_rows_free_entry>>::
      const_iterator search_state;

  /**
    Wether this search_state is usable or not.
   */
  bool is_search_state_inited;
};

struct HASH_ROW_ENTRY {
  HASH_ROW_PREAMBLE *preamble;
  HASH_ROW_POS *positions;
};

class Hash_slave_rows {
 public:
  /**
     Allocates an empty entry to be added to the hash table.
     It should be called before calling member function @c put.

     @returns NULL if a problem occurred, a valid pointer otherwise.
  */
  HASH_ROW_ENTRY *make_entry();

  /**
     Allocates an entry to be added to the hash table. It should be
     called before calling member function @c put.

     @param bi_start the position to where in the rows buffer the
                     before image begins.
     @param bi_ends  the position to where in the rows buffer the
                     before image ends.
     @returns NULL if a problem occurred, a valid pointer otherwise.
   */
  HASH_ROW_ENTRY *make_entry(const uchar *bi_start, const uchar *bi_ends);

  /**
     Puts data into the hash table. It calculates the key taking
     the data on @c TABLE::record as the input for hash computation.

     @param table   The table holding the buffer used to calculate the
                    key, ie, table->record[0].
     @param cols    The read_set bitmap signaling which columns are used.
     @param entry   The entry with the values to store.

     @returns true if something went wrong, false otherwise.
   */
  bool put(TABLE *table, MY_BITMAP *cols, HASH_ROW_ENTRY *entry);

  /**
     Gets the entry, from the hash table, that matches the data in
     table->record[0] and signaled using cols.

     @param table   The table holding the buffer containing data used to
                    make the entry lookup.
     @param cols    Bitmap signaling which columns, from
                    table->record[0], should be used.

     @returns a pointer that will hold a reference to the entry
              found. If the entry is not found then NULL shall be
              returned.
   */
  HASH_ROW_ENTRY *get(TABLE *table, MY_BITMAP *cols);

  /**
     Gets the entry that stands next to the one pointed to by
     *entry. Before calling this member function, the entry that one
     uses as parameter must have: 1. been obtained through get() or
     next() invocations; and 2. must have not been used before in a
     next() operation.

     @param[in,out] entry contains a pointer to an entry that we can
                          use to search for another adjacent entry
                          (ie, that shares the same key).

     @returns true if something went wrong, false otherwise. In the
              case that this entry was already used in a next()
              operation this member function returns true and does not
              update the pointer.
   */
  bool next(HASH_ROW_ENTRY **entry);

  /**
     Deletes the entry pointed by entry. It also frees memory used
     holding entry contents. This is the way to release memeory
     used for entry, freeing it explicitly with my_free will cause
     undefined behavior.

     @param entry  Pointer to the entry to be deleted.
     @returns true if something went wrong, false otherwise.
   */
  bool del(HASH_ROW_ENTRY *entry);

  /**
     Initializes the hash table.

     @returns true if something went wrong, false otherwise.
   */
  bool init(void);

  /**
     De-initializes the hash table.

     @returns true if something went wrong, false otherwise.
   */
  bool deinit(void);

  /**
     Checks if the hash table is empty or not.

     @returns true if the hash table has zero entries, false otherwise.
   */
  bool is_empty(void);

  /**
     Returns the number of entries in the hash table.

     @returns the number of entries in the hash table.
   */
  int size();

 private:
  /**
     The hashtable itself.
   */
  malloc_unordered_multimap<
      uint, std::unique_ptr<HASH_ROW_ENTRY, hash_slave_rows_free_entry>>
      m_hash{key_memory_HASH_ROW_ENTRY};

  /**
     Auxiliary and internal method used to create an hash key, based on
     the data in table->record[0] buffer and signaled as used in cols.

     @param table  The table that is being scanned
     @param cols   The read_set bitmap signaling which columns are used.

     @returns the hash key created.
   */
  uint make_hash_key(TABLE *table, MY_BITMAP *cols);
};

#endif

/**
  A table definition from the master.

  The responsibilities of this class is:
  - Extract and decode table definition data from the table map event
  - Check if table definition in table map is compatible with table
    definition on slave
  - expose the type information so that it can be used when encoding
    or decoding row event data.
*/
class table_def {
 public:
  /**
    No-op constructor. Instances of RPL_TABLE_LIST are created by first
    allocating memory, then placement-new-ing an RPL_TABLE_LIST object
    containing an uninitialized table_def object which is only conditionally
    initialized. See Table_map_log_event::do_apply_event().
  */
  table_def() {}

  /**
    Constructor.

    @param types Array of types, each stored as a byte
    @param size  Number of elements in array 'types'
    @param field_metadata Array of extra information about fields
    @param metadata_size Size of the field_metadata array
    @param null_bitmap The bitmap of fields that can be null
    @param flags Table flags
    @param column_names Column names
    @param sign_bits Sign bits
   */
  table_def(unsigned char *types, ulong size, uchar *field_metadata,
            int metadata_size, uchar *null_bitmap, uint16 flags,
            const uchar *column_names, unsigned long column_names_size,
            const uchar *sign_bits);

  ~table_def();

  bool is_unsigned(uint index) const {
    DBUG_ASSERT(index < m_size);
    return (m_sign_bits[(index / 8)] & (1 << (index % 8)));
  }

  /**
    Return the number of fields there is type data for.

    @return The number of fields that there is type data for.
   */
  ulong size() const { return m_size; }

  /*
    Returns internal binlog type code for one field,
    without translation to real types.
  */
  enum_field_types binlog_type(ulong index) const {
    return static_cast<enum_field_types>(m_type[index]);
  }

  /// Return the number of JSON columns in this table.
  int json_column_count() const {
    // Cache in member field to make successive calls faster.
    if (m_json_column_count == -1) {
      int c = 0;
      for (uint i = 0; i < size(); i++)
        if (type(i) == MYSQL_TYPE_JSON) c++;
      m_json_column_count = c;
    }
    return m_json_column_count;
  }

  /*
    Return a representation of the type data for one field.

    @param index Field index to return data for

    @return Will return a representation of the type data for field
    <code>index</code>. Currently, only the type identifier is
    returned.
   */
  enum_field_types type(ulong index) const {
    DBUG_ASSERT(index < m_size);
    /*
      If the source type is MYSQL_TYPE_STRING, it can in reality be
      either MYSQL_TYPE_STRING, MYSQL_TYPE_ENUM, or MYSQL_TYPE_SET, so
      we might need to modify the type to get the real type.
    */
    enum_field_types source_type = binlog_type(index);
    uint source_metadata = m_field_metadata[index];
    switch (source_type) {
      case MYSQL_TYPE_STRING: {
        int real_type = source_metadata >> 8;
        if (real_type == MYSQL_TYPE_ENUM || real_type == MYSQL_TYPE_SET)
          source_type = static_cast<enum_field_types>(real_type);
        break;
      }

      /*
        This type has not been used since before row-based replication,
        so we can safely assume that it really is MYSQL_TYPE_NEWDATE.
       */
      case MYSQL_TYPE_DATE:
        source_type = MYSQL_TYPE_NEWDATE;
        break;

      default:
        /* Do nothing */
        break;
    }

    return source_type;
  }

  /*
    This function allows callers to get the extra field data from the
    table map for a given field. If there is no metadata for that field
    or there is no extra metadata at all, the function returns 0.

    The function returns the value for the field metadata for column at
    position indicated by index. As mentioned, if the field was a type
    that stores field metadata, that value is returned else zero (0) is
    returned. This method is used in the unpack() methods of the
    corresponding fields to properly extract the data from the binary log
    in the event that the master's field is smaller than the slave.
  */
  uint field_metadata(uint index) const {
    DBUG_ASSERT(index < m_size);
    if (m_field_metadata_size)
      return m_field_metadata[index];
    else
      return 0;
  }

  /**
    Returns whether or not the field at `index` is a typed array.
   */
  bool is_array(uint index) const {
    DBUG_ASSERT(index < m_size);
    if (m_field_metadata_size)
      return m_is_array[index];
    else
      return false;
  }

  /*
    This function returns whether the field on the master can be null.
    This value is derived from field->maybe_null().
  */
  bool maybe_null(uint index) const {
    DBUG_ASSERT(index < m_size);
    return ((m_null_bits[(index / 8)] & (1 << (index % 8))) ==
            (1 << (index % 8)));
  }

  /*
    This function returns the field size in raw bytes based on the type
    and the encoded field data from the master's raw data. This method can
    be used for situations where the slave needs to skip a column (e.g.,
    WL#3915) or needs to advance the pointer for the fields in the raw
    data from the master to a specific column.
  */
  uint32 calc_field_size(uint col, const uchar *master_data) const;

#ifdef MYSQL_SERVER
  /**
    Decide if the table definition is compatible with a table.

    Compare the definition with a table to see if it is compatible
    with it.

    A table definition is compatible with a table if:
      - The columns types of the table definition is a (not
        necessarily proper) prefix of the column type of the table.

      - The other way around.

      - Each column on the master that also exists on the slave can be
        converted according to the current settings of @c
        SLAVE_TYPE_CONVERSIONS.

    @param thd   Current thread
    @param rli   Pointer to relay log info
    @param table Pointer to table to compare with.

    @param[out] conv_table_var Pointer to temporary table for holding
    conversion table.

    @retval 1  if the table definition is not compatible with @c table
    @retval 0  if the table definition is compatible with @c table
  */
  bool compatible_with(THD *thd, Relay_log_info *rli, TABLE *table,
                       TABLE **conv_table_var) const;

  /**
   Create a virtual in-memory temporary table structure.

   The table structure has records and field array so that a row can
   be unpacked into the record for further processing.

   In the virtual table, each field that requires conversion will
   have a non-NULL value, while fields that do not require
   conversion will have a NULL value.

   Some information that is missing in the events, such as the
   character set for string types, are taken from the table that the
   field is going to be pushed into, so the target table that the data
   eventually need to be pushed into need to be supplied.

   @param thd Thread to allocate memory from.
   @param rli Relay log info structure, for error reporting.
   @param target_table Target table for fields.

   @return A pointer to a temporary table with memory allocated in the
   thread's memroot, NULL if the table could not be created
   */
  TABLE *create_conversion_table(THD *thd, Relay_log_info *rli,
                                 TABLE *target_table) const;

  bool use_column_names(TABLE *table);
#endif

  bool have_column_names() const { return !m_column_names.empty(); }

  const char *get_column_name(uint index) const {
    return (index < m_column_names.size()) ? m_column_names[index] : nullptr;
  }

  // return: if col is found <index, true>, <0, false> otherwise
  std::pair<uint, bool> get_column_index(const char *name) const {
    const auto itr = m_column_indices.find(name);
    if (itr == m_column_indices.end()) return std::make_pair(0, false);
    return std::make_pair(itr->second, true);
  }

 private:
  ulong m_size;           // Number of elements in the types array
  unsigned char *m_type;  // Array of type descriptors
  uint m_field_metadata_size;
  uint *m_field_metadata;
  uchar *m_null_bits;
  uint16 m_flags;  // Table flags
  uchar *m_memory;
  mutable int m_json_column_count;  // Number of JSON columns
  bool *m_is_array;
  std::vector<char *> m_column_names;
  std::unordered_map<std::string, uint> m_column_indices;
  uchar *m_sign_bits;
  int m_slave_schema_is_different;
};

#ifdef MYSQL_SERVER
/**
   Extend the normal table list with a few new fields needed by the
   slave thread, but nowhere else.
 */
struct RPL_TABLE_LIST : public TABLE_LIST {
  RPL_TABLE_LIST(const char *db_name_arg, size_t db_length_arg,
                 const char *table_name_arg, size_t table_name_length_arg,
                 const char *alias_arg, enum thr_lock_type lock_type_arg)
      : TABLE_LIST(db_name_arg, db_length_arg, table_name_arg,
                   table_name_length_arg, alias_arg, lock_type_arg) {}

  bool m_tabledef_valid;
  table_def m_tabledef;
  TABLE *m_conv_table;
  bool master_had_triggers;
};

class Deferred_log_events {
 private:
  Prealloced_array<Log_event *, 32> m_array;

 public:
  Deferred_log_events();
  ~Deferred_log_events();
  /* queue for exection at Query-log-event time prior the Query */
  int add(Log_event *ev);
  bool is_empty();
  bool execute(Relay_log_info *rli);
  void rewind();
};

#endif

/**
  Decode field metadata from a char buffer (serialized form) into an int
  (packed form).

  @note On little-endian platforms (e.g Intel) this function effectively
  inverts order of bytes compared to what Field::save_field_metadata()
  writes. E.g for MYSQL_TYPE_NEWDECIMAL save_field_metadata writes precision
  into the first byte and decimals into the second, this function puts
  precision into the second byte and decimals into the first. This layout
  is expected by replication code that reads metadata in the uint form.
  Due to this design feature show_sql_type() can't correctly print
  immediate output of save_field_metadata(), this function have to be used
  as translator.

  @param buffer Field metadata, in the character stream form produced by
                save_field_metadata.
  @param binlog_type The type of the field, in the form returned by
                      Field::binlog_type and stored in Table_map_log_event.
  @retval pair where:
  - the first component is the length of the metadata within 'buffer',
    i.e., how much the buffer pointer should move forward in order to skip it.
  - the second component is pair containing:
    - the metadata, encoded as an 'uint', in the form required by e.g.
      show_sql_type.
    - bool indicating whether the field is array (true) or a scalar (false)
*/

std::pair<my_off_t, std::pair<uint, bool>> read_field_metadata(
    const uchar *buffer, enum_field_types binlog_type);

// NB. number of printed bit values is limited to sizeof(buf) - 1
#define DBUG_PRINT_BITSET(N, FRM, BS)                                   \
  do {                                                                  \
    char buf[256];                                                      \
    uint i;                                                             \
    for (i = 0; i < std::min(uint{sizeof(buf) - 1}, (BS)->n_bits); i++) \
      buf[i] = bitmap_is_set((BS), i) ? '1' : '0';                      \
    buf[i] = '\0';                                                      \
    DBUG_PRINT((N), ((FRM), buf));                                      \
  } while (0)

#ifdef MYSQL_SERVER
/**
  Sentry class for managing the need to create and dispose of a local `THD`
  instance.

  If the given `THD` object pointer passed on the constructor is `nullptr`, a
  new instance will be initialized within the constructor and disposed of in the
  destructor.

  If the given `THD` object poitner passed on the constructor is not `nullptr`,
  the reference is kept and nothing is disposed on the destructor.

  Casting operator to `THD*` is also provided, to easy code replacemente.

  Usage example:

       THD_instance_guard thd{current_thd != nullptr ? current_thd :
                                                       this->info_thd};
       Acl_cache_lock_guard guard{thd, Acl_cache_lock_mode::READ_MODE};
       if (guard.lock())
         ...

 */
class THD_instance_guard {
 public:
  /**
    If the given `THD` object pointer is `nullptr`, a new instance will be
    initialized within the constructor and disposed of in the destructor.

    If the given `THD` object poitner is not `nullptr`, the reference is kept
    and nothing is disposed on the destructor.

    @param thd `THD` object reference that determines if an existence instance
    is used or a new instance of `THD` must be created.
   */
  THD_instance_guard(THD *thd);
  /**
    If a new instance of `THD` was created in the constructor, it will be
    disposed here.
   */
  virtual ~THD_instance_guard();

  /**
    Returns the active `THD` object pointer.

    @return a not-nullptr `THD` object pointer.
   */
  operator THD *();

 private:
  /** The active `THD` object pointer. */
  THD *m_target{nullptr};
  /**
    Tells whether or not the active `THD` object was created in this object
    constructor.
   */
  bool m_is_locally_initialized{false};
};
#endif  // MYSQL_SERVER

/**
  Replaces every occurrence of the string `find` by the string `replace`, within
  the string `from` and return the resulting string.

  The original string `from` remains untouched.

  @param from the string to search within.
  @param find the string to search for.
  @param replace the string to replace every occurrence of `from`

  @return a new string, holding the result of the search and replace operation.
 */
std::string replace_all_in_str(std::string from, std::string find,
                               std::string replace);

#ifdef MYSQL_SERVER

/**
  This method shall evaluate if a command being executed goes against any of
  the restrictions of server variable session.require_row_format.

  @param thd The thread associated to the command
  @return true if it violates any restrictions
          false otherwise
 */
bool evaluate_command_row_only_restrictions(THD *thd);

#endif  // MYSQL_SERVER

#endif /* RPL_UTILITY_H */
