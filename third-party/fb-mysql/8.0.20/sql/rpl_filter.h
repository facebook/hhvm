/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_FILTER_H
#define RPL_FILTER_H

#include "my_config.h"

#include <stddef.h>
#include <sys/types.h>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "map_helpers.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "prealloced_array.h"    // Prealloced_arrray
#include "sql/options_mysqld.h"  // options_mysqld
#include "sql/rpl_gtid.h"
#include "sql/sql_cmd.h"   // Sql_cmd
#include "sql/sql_list.h"  // I_List
#include "sql_string.h"

class Item;
class THD;
struct TABLE_LIST;

/*
There are five classes related to replication filters:
- Rpl_filter contains all the seven filters do-db, ignore-db, do-table,
  ignore-table, wild-do-table, wild-ignore-table, rewrite-db. We
  instantiate one Rpl_filter for each replication channel and one for
  the binlog. This contains member functions to apply the filters.
- Rpl_pfs_filter has one instance for each row in
  P_S.replication_applier_filters and one instance for each row of
  P_S.replication_applier_global_filters.
- Rpl_filter_statistics contains the data other than the key for each
  P_S row. Each Rpl_filter object owns seven instances of this class
  (one for each filter type) and each Rpl_pfs_filter points to one of
  those instances.
- Rpl_channel_filters contains a map that maps channel names to
  Rpl_filter objects, as well as a vector that references all
  the Rpl_pfs_filter objects used to represent
  P_S.replication_applier_filters.
- Rpl_global_filter is a Rpl_filter representing global replication
  filters, with a vector that references all Rpl_pfs_filter objects
  used to represent P_S.replication_applier_global_filters table.

The vectors of Rpl_pfs_filters objects are rebuilt whenever filters
are modified (i.e., channels created/dropped or filters changed).
*/

struct TABLE_RULE_ENT {
  char *db;
  char *tbl_name;
  uint key_len;
};

/** Enum values for CONFIGURED_BY column. */
enum enum_configured_by {
  CONFIGURED_BY_STARTUP_OPTIONS = 1,
  CONFIGURED_BY_CHANGE_REPLICATION_FILTER,
  CONFIGURED_BY_STARTUP_OPTIONS_FOR_CHANNEL,
  CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL
};

/**
  The class Rpl_filter_statistics encapsulates the following three
  statistics of replication filter:
  The configured_by indicates that how the rpl filter is configured.
  The active_since indicates when the configuration took place.
  The counter indicates the hit amount of the filter since last
  configuration.
  Instances of this class are created in Rpl_filter for each filter
  type for each channel and the global filter, and have the same
  life cycle as the instance of Rpl_filter.
  The reference of this class is used in Rpl_pfs_filter for
  displaying the three statistics of replication filter in
  performance_schema.replication_applier_filters table and
  performance_schema.replication_applier_global_filters table.
*/
class Rpl_filter_statistics {
 public:
  Rpl_filter_statistics();
  ~Rpl_filter_statistics();
  void reset();
  /*
    Set all member variables. The caller just needs to pass argument
    for configured_by, since counter and active_since are set in the
    funtion. We do that, since counter must be set to 0 and
    active_since must be set to current time for any case.
  */
  void set_all(enum_configured_by configured_by);

  enum_configured_by get_configured_by() { return m_configured_by; }
  ulonglong get_active_since() { return m_active_since; }
  ulonglong get_counter() { return m_atomic_counter; }
  void increase_counter() { m_atomic_counter++; }

 private:
  /*
    The replication filters can be configured with the following four states:
    STARTUP_OPTIONS, //STARTUP_OPTIONS: --REPLICATE-*
    CHANGE_REPLICATION_FILTER, //CHANGE REPLICATION FILTER filter [, filter...]
    STARTUP_OPTIONS_FOR_CHANNEL, //STARTUP_OPTIONS: --REPLICATE-* (FOR_CHANNEL)
    CHANGE_REPLICATION_FILTER_FOR_CHANNEL //CHANGE REPLICATION FILTER filter [,
                                          filter...] FOR CHANNEL <channel_name>
  */
  enum_configured_by m_configured_by;

  /* Timestamp of when the configuration took place */
  ulonglong m_active_since;

  /*
    The hit amount of the filter since last configuration.
    The m_atomic_counter may be increased by concurrent slave
    workers, so we use the atomic<uint64>.
  */
  std::atomic<uint64> m_atomic_counter{0};

  /* Prevent user from invoking default constructor function. */
  Rpl_filter_statistics(Rpl_filter_statistics const &);

  /* Prevent user from invoking default assignment function. */
  Rpl_filter_statistics &operator=(Rpl_filter_statistics const &);
};

/**
  The class Rpl_pfs_filter is introduced to serve the
  performance_schema.replication_applier_filters table
  and performance_schema.replication_applier_global_filters
  table to collect data for a row. The class Rpl_filter
  does not use it directly, since it contains channel_name,
  which does not belong to Rpl_filter. To decouple code,
  it depends on Rpl_filter_statistics, does not inherit
  Rpl_filter_statistics.
  Instances of this class are created in Rpl_filter for
  each filter type for each channel and the global filter.
  Each instance is created when creating, changing or
  deleting the filter, destroyed when creating, changing
  or deleting the filter next time.
*/
class Rpl_pfs_filter {
 public:
  Rpl_pfs_filter();
  Rpl_pfs_filter(const char *channel_name, const char *filter_name,
                 const String &filter_rule,
                 Rpl_filter_statistics *rpl_filter_statistics);
  Rpl_pfs_filter(const Rpl_pfs_filter &other);
  ~Rpl_pfs_filter();

  const char *get_channel_name() { return m_channel_name; }
  const char *get_filter_name() { return m_filter_name; }
  const String &get_filter_rule() { return m_filter_rule; }
  Rpl_filter_statistics *get_rpl_filter_statistics() {
    return m_rpl_filter_statistics;
  }

 private:
  /* A pointer to the channel name. */
  const char *m_channel_name;

  /* A pointer to the filer name. */
  const char *m_filter_name;

  /* A pointer to replication filter statistics. */
  Rpl_filter_statistics *m_rpl_filter_statistics;

  /* A filter rule. */
  String m_filter_rule;

  /* Prevent user from invoking default assignment function. */
  Rpl_pfs_filter &operator=(Rpl_pfs_filter const &);
};

/**
  Rpl_filter

  Inclusion and exclusion rules of tables and databases.
  Also handles rewrites of db.
  Used for replication and binlogging.
  - Instances of this class are created in Rpl_channel_filters
    for replication filter for each channel. Each instance is
    created when the channel is configured, destroyed when the
    channel is removed.
  - There is one instance, binlog_filter, created for binlog filter.
    The instance is created when the server is started, destroyed
    when the server is stopped.
 */
class Rpl_filter {
 public:
  Rpl_filter();
  virtual ~Rpl_filter();
  Rpl_filter(Rpl_filter const &);
  Rpl_filter &operator=(Rpl_filter const &);

  /* Checks - returns true if ok to replicate/log */

  bool tables_ok(const char *db, TABLE_LIST *tables);
  bool db_ok(const char *db, bool need_increase_counter = true);
  bool db_ok_with_wild_table(const char *db);

  bool is_on();
  /**
    Check if the replication filter is empty or not.

    @retval true if the replication filter is empty.
    @retval false if the replication filter is not empty.
  */
  bool is_empty();
  /**
    Copy global replication filters to its per-channel replication filters
    if there are no per-channel replication filters and there are global
    filters on the filter type on channel creation.

    @retval 0 OK
    @retval 1 Error
  */
  int copy_global_replication_filters();

  bool is_rewrite_empty();

  /* Setters - add filtering rules */
  int build_do_table_hash();
  int build_ignore_table_hash();

  int add_string_list(I_List<i_string> *list, const char *spec);
  int add_string_pair_list(I_List<i_string_pair> *list, const char *key,
                           const char *val);
  int add_do_table_array(const char *table_spec);
  int add_ignore_table_array(const char *table_spec);

  int add_wild_do_table(const char *table_spec);
  int add_wild_ignore_table(const char *table_spec);

  int set_do_db(List<Item> *list, enum_configured_by configured_by);
  int set_ignore_db(List<Item> *list, enum_configured_by configured_by);
  int set_do_table(List<Item> *list, enum_configured_by configured_by);
  int set_ignore_table(List<Item> *list, enum_configured_by configured_by);
  int set_wild_do_table(List<Item> *list, enum_configured_by configured_by);
  int set_wild_ignore_table(List<Item> *list, enum_configured_by configured_by);
  int set_db_rewrite(List<Item> *list, enum_configured_by configured_by);
  typedef int (Rpl_filter::*Add_filter)(char const *);
  int parse_filter_list(List<Item> *item_list, Add_filter func);
  /**
    Execute the specified func with elements of the list as input.

    @param list A list with I_List\<i_string\> type
    @param add A function with Add_filter type

    @retval 0 OK
    @retval 1 Error
  */
  int parse_filter_list(I_List<i_string> *list, Add_filter add);
  int add_do_db(const char *db_spec);
  int add_ignore_db(const char *db_spec);

  int add_db_rewrite(const char *from_db, const char *to_db);

  /* Getters - to get information about current rules */

  void get_do_table(String *str);
  void get_ignore_table(String *str);

  void get_wild_do_table(String *str);
  void get_wild_ignore_table(String *str);

  const char *get_rewrite_db(const char *db, size_t *new_len);
  void get_rewrite_db(String *str);

  I_List<i_string> *get_do_db();
  /*
    Get do_db rule.

    @param[out] str the db_db rule.
  */
  void get_do_db(String *str);

  I_List<i_string> *get_ignore_db();
  /*
    Get ignore_db rule.

    @param[out] str the ignore_db rule.
  */
  void get_ignore_db(String *str);
  /*
    Get rewrite_db_statistics.

    @retval A pointer to a rewrite_db_statistics object.
  */
  Rpl_filter_statistics *get_rewrite_db_statistics() {
    return &rewrite_db_statistics;
  }

  void free_string_list(I_List<i_string> *l);
  void free_string_pair_list(I_List<i_string_pair> *l);

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  /**
    Put replication filters with attached channel name into a vector.

    @param rpl_pfs_filter_vec the vector.
    @param channel_name the name of the channel attached or NULL if
                        there is no channel attached.
  */
  void put_filters_into_vector(std::vector<Rpl_pfs_filter> &rpl_pfs_filter_vec,
                               const char *channel_name);
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

  /**
    Acquire the write lock.
  */
  void wrlock() { m_rpl_filter_lock->wrlock(); }

  /**
    Acquire the read lock.
  */
  void rdlock() { m_rpl_filter_lock->rdlock(); }

  /**
    Release the lock (whether it is a write or read lock).
  */
  void unlock() { m_rpl_filter_lock->unlock(); }

  /**
    Assert that some thread holds the write lock.
  */
  void assert_some_wrlock() { m_rpl_filter_lock->assert_some_wrlock(); }

  /**
    Assert that some thread holds the read lock.
  */
  void assert_some_rdlock() { m_rpl_filter_lock->assert_some_rdlock(); }

  /**
    Check if the relation between the per-channel filter and
    the channel's Relay_log_info is established.

    @retval true if the relation is established
    @retval false if the relation is not established
  */
  bool is_attached() { return attached; }

  /**
    Set attached to true when the relation between the per-channel filter
    and the channel's Relay_log_info is established.
  */
  void set_attached() { attached = true; }

  void reset();

  Rpl_filter_statistics do_table_statistics;
  Rpl_filter_statistics ignore_table_statistics;
  Rpl_filter_statistics wild_do_table_statistics;
  Rpl_filter_statistics wild_ignore_table_statistics;
  Rpl_filter_statistics do_db_statistics;
  Rpl_filter_statistics ignore_db_statistics;
  Rpl_filter_statistics rewrite_db_statistics;

 private:
  bool table_rules_on;
  /*
    State if the relation between the per-channel filter
    and the channel's Relay_log_info is established.
  */
  bool attached;

  /*
    While slave is not running after server startup, the replication filter
    can be modified by CHANGE REPLICATION FILTER filter [, filter...]
    [FOR CHANNEL <channel_name>] and CHANGE MASTER TO ... FOR CHANNEL,
    and read by querying P_S.replication_applier_global_filters,
    querying P_S.replication_applier_filters, and SHOW SLAVE STATUS
    [FOR CHANNEL <channel_name>]. So the lock is introduced to protect
    some member functions called by above commands. See below.

    The read lock should be held when calling the following member functions:
      get_do_table(String* str);  // SHOW SLAVE STATUS
      get_ignore_table(String* str); // SHOW SLAVE STATUS
      get_wild_do_table(String* str); // SHOW SLAVE STATUS
      get_wild_ignore_table(String* str); // SHOW SLAVE STATUS
      get_rewrite_db(const char* db, size_t *new_len); // SHOW SLAVE STATUS
      get_rewrite_db(String *str); // SHOW SLAVE STATUS
      get_do_db(); // SHOW SLAVE STATUS
      get_do_db(String *str);  // SHOW SLAVE STATUS
      get_ignore_db();  // SHOW SLAVE STATUS
      get_ignore_db(String *str);  // SHOW SLAVE STATUS
      put_filters_into_vector(...);  // query P_S tables
      get_filter_count();  // query P_S tables

    The write lock should be held when calling the following member functions:
      set_do_db(List<Item> *list); // CHANGE REPLICATION FILTER
      set_ignore_db(List<Item> *list);  // CHANGE REPLICATION FILTER
      set_do_table(List<Item> *list);  // CHANGE REPLICATION FILTER
      set_ignore_table(List<Item> *list); // CHANGE REPLICATION FILTER
      set_wild_do_table(List<Item> *list); // CHANGE REPLICATION FILTER
      set_wild_ignore_table(List<Item> *list); // CHANGE REPLICATION FILTER
      set_db_rewrite(List<Item> *list); // CHANGE REPLICATION FILTER
      copy_global_replication_filters(); // CHANGE MASTER TO ... FOR CHANNEL

    Please acquire a wrlock when modifying the replication filter (CHANGE
    REPLICATION FILTER filter [, filter...] [FOR CHANNEL <channel_name>]
    and CHANGE MASTER TO ... FOR CHANNEL).
    Please acqurie a rdlock when reading the replication filter (
    SELECT * FROM performance_schema.replication_applier_global_filters,
    SELECT * FROM performance_schema.replication_applier_filters and
    SHOW SLAVE STATUS [FOR CHANNEL <channel_name>]).

    Other member functions do not need the protection of the lock and we can
    access thd->rli_slave->rpl_filter to filter log event without the
    protection of the lock while slave is running, since the replication
    filter is read/modified by a single thread during server startup and
    there is no command can change it while slave is running.
  */
  Checkable_rwlock *m_rpl_filter_lock;

  typedef Prealloced_array<TABLE_RULE_ENT *, 16> Table_rule_array;
  typedef collation_unordered_map<std::string,
                                  unique_ptr_my_free<TABLE_RULE_ENT>>
      Table_rule_hash;

  void init_table_rule_hash(Table_rule_hash **h, bool *h_inited);
  void init_table_rule_array(Table_rule_array *, bool *a_inited);

  int add_table_rule_to_array(Table_rule_array *a, const char *table_spec);
  int add_table_rule_to_hash(Table_rule_hash *h, const char *table_spec,
                             uint len);

  void free_string_array(Table_rule_array *a);

  void table_rule_ent_hash_to_str(String *s, Table_rule_hash *h, bool inited);
  /**
    Builds a Table_rule_array from a hash of TABLE_RULE_ENT. Cannot be used for
    any other hash, as it assumes that the hash entries are TABLE_RULE_ENT.

    @param table_array Pointer to the Table_rule_array to fill
    @param h Pointer to the hash to read
    @param inited True if the hash is initialized

    @retval 0 OK
    @retval 1 Error
  */
  int table_rule_ent_hash_to_array(Table_rule_array *table_array,
                                   Table_rule_hash *h, bool inited);
  /**
    Builds a destination Table_rule_array from a source Table_rule_array
    of TABLE_RULE_ENT.

    @param dest_array Pointer to the destination Table_rule_array to fill
    @param source_array Pointer to the source Table_rule_array to read
    @param inited True if the source Table_rule_array is initialized

    @retval 0 OK
    @retval 1 Error
  */
  int table_rule_ent_array_to_array(Table_rule_array *dest_array,
                                    Table_rule_array *source_array,
                                    bool inited);
  void table_rule_ent_dynamic_array_to_str(String *s, Table_rule_array *a,
                                           bool inited);
  TABLE_RULE_ENT *find_wild(Table_rule_array *a, const char *key, size_t len);

  int build_table_hash_from_array(Table_rule_array *table_array,
                                  Table_rule_hash **table_hash,
                                  bool array_inited, bool *hash_inited);

  /*
    Those 6 structures below are uninitialized memory unless the
    corresponding *_inited variables are "true".
  */
  /* For quick search */
  Table_rule_hash *do_table_hash{nullptr};
  Table_rule_hash *ignore_table_hash{nullptr};

  Table_rule_array do_table_array;
  Table_rule_array ignore_table_array;

  Table_rule_array wild_do_table;
  Table_rule_array wild_ignore_table;

  bool do_table_hash_inited;
  bool ignore_table_hash_inited;
  bool do_table_array_inited;
  bool ignore_table_array_inited;
  bool wild_do_table_inited;
  bool wild_ignore_table_inited;

  I_List<i_string> do_db;
  I_List<i_string> ignore_db;

  I_List<i_string_pair> rewrite_db;
};

/**
  The class is a Rpl_filter representing global replication filters,
  with a vector that references all Rpl_pfs_filter objects used to
  represent P_S.replication_applier_global_filters table.
  There is one instance, rpl_global_filter, created globally for
  replication global filter. The rpl_global_filter is created when
  the server is started, destroyed when the server is stopped.
*/
class Rpl_global_filter : public Rpl_filter {
 public:
  Rpl_global_filter() {}
  ~Rpl_global_filter() {}

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  /**
    Used only by replication performance schema indices to get the count
    of global replication filters.

    @retval the count of global replication filters.
  */
  uint get_filter_count();
  /**
    Used only by replication performance schema indices to get the global
    replication filter at the position 'pos' from the
    rpl_pfs_filter_vec vector.

    @param pos the index in the rpl_pfs_filter_vec vector.

    @retval Rpl_pfs_filter A pointer to a Rpl_pfs_filter, or NULL if it
                           arrived the end of the rpl_pfs_filter_vec.
  */
  Rpl_pfs_filter *get_filter_at_pos(uint pos);
  /**
    This member function is called everytime the rules of the global
    repliation filter are changed. Once that happens the PFS view of
    global repliation filter is recreated.
  */
  void reset_pfs_view();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

 private:
  /*
    Store pointers of all Rpl_pfs_filter objects in
    replication filter.
  */
  std::vector<Rpl_pfs_filter> rpl_pfs_filter_vec;
  /* Prevent user from invoking default assignment function. */
  Rpl_global_filter &operator=(const Rpl_global_filter &info);
  /* Prevent user from invoking default copy constructor function. */
  Rpl_global_filter(const Rpl_global_filter &info);
};

/** Sql_cmd_change_repl_filter represents the command CHANGE REPLICATION
 * FILTER.
 */
class Sql_cmd_change_repl_filter : public Sql_cmd {
 public:
  /** Constructor.  */
  Sql_cmd_change_repl_filter()
      : do_db_list(nullptr),
        ignore_db_list(nullptr),
        do_table_list(nullptr),
        ignore_table_list(nullptr),
        wild_do_table_list(nullptr),
        wild_ignore_table_list(nullptr),
        rewrite_db_pair_list(nullptr) {}

  ~Sql_cmd_change_repl_filter() {}

  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_CHANGE_REPLICATION_FILTER;
  }
  bool execute(THD *thd);

  void set_filter_value(List<Item> *item_list, options_mysqld filter_type);
  bool change_rpl_filter(THD *thd);

 private:
  List<Item> *do_db_list;
  List<Item> *ignore_db_list;
  List<Item> *do_table_list;
  List<Item> *ignore_table_list;
  List<Item> *wild_do_table_list;
  List<Item> *wild_ignore_table_list;
  List<Item> *rewrite_db_pair_list;
};

extern Rpl_filter *rpl_filter;
extern Rpl_filter *binlog_filter;

#endif  // RPL_FILTER_H
