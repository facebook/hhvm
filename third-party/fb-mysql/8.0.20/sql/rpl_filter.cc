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

#include "sql/rpl_filter.h"

#include "my_config.h"

#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <algorithm>
#include <map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "mf_wcomp.h"  // wild_one, wild_many
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/item.h"    // Item
#include "sql/mysqld.h"  // table_alias_charset
#include "sql/psi_memory_key.h"
#include "sql/rpl_mi.h"     // Master_info
#include "sql/rpl_msr.h"    // channel_map
#include "sql/rpl_rli.h"    // Relay_log_info
#include "sql/rpl_slave.h"  // SLAVE_SQL
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/table.h"  // TABLE_LIST
#include "sql/thr_malloc.h"
#include "sql_string.h"
#include "template_utils.h"  // my_free_container_pointers

extern PSI_memory_key key_memory_array_buffer;

Rpl_pfs_filter::Rpl_pfs_filter()
    : m_channel_name(nullptr),
      m_filter_name(nullptr),
      m_rpl_filter_statistics(nullptr) {}

Rpl_pfs_filter::Rpl_pfs_filter(const char *channel_name,
                               const char *filter_name,
                               const String &filter_rule,
                               Rpl_filter_statistics *rpl_filter_statistics) {
  m_channel_name = channel_name;
  m_filter_name = filter_name;
  if (!filter_rule.is_empty()) m_filter_rule.copy(filter_rule);
  m_rpl_filter_statistics = rpl_filter_statistics;
}

Rpl_pfs_filter::Rpl_pfs_filter(const Rpl_pfs_filter &other) {
  m_channel_name = other.m_channel_name;
  m_filter_name = other.m_filter_name;
  m_rpl_filter_statistics = other.m_rpl_filter_statistics;
  /* Deep copy */
  if (!other.m_filter_rule.is_empty()) m_filter_rule.copy(other.m_filter_rule);
}

Rpl_pfs_filter::~Rpl_pfs_filter() {}

Rpl_filter_statistics::Rpl_filter_statistics() { reset(); }

Rpl_filter_statistics::~Rpl_filter_statistics() {}

void Rpl_filter_statistics::reset() {
  m_configured_by = CONFIGURED_BY_STARTUP_OPTIONS;
  m_atomic_counter = 0;
  m_active_since = 0;
}

void Rpl_filter_statistics::set_all(enum_configured_by configured_by) {
  m_configured_by = configured_by;
  m_atomic_counter = 0;

  /* Set m_active_since to current time. */
  THD *thd = current_thd;
  if (thd == nullptr)
    m_active_since = my_micro_time();
  else {
    /*
      Calculate time stamp up to tenths of milliseconds elapsed
      from 1 Jan 1970 00:00:00.
    */
    struct timeval stmt_start_time = thd->query_start_timeval_trunc(6);
    m_active_since = static_cast<ulonglong>(stmt_start_time.tv_sec) * 1000000 +
                     stmt_start_time.tv_usec;
  }
}

Rpl_filter::Rpl_filter()
    : table_rules_on(false),
      attached(false),
      do_table_array(key_memory_TABLE_RULE_ENT),
      ignore_table_array(key_memory_TABLE_RULE_ENT),
      wild_do_table(key_memory_TABLE_RULE_ENT),
      wild_ignore_table(key_memory_TABLE_RULE_ENT),
      do_table_hash_inited(false),
      ignore_table_hash_inited(false),
      do_table_array_inited(false),
      ignore_table_array_inited(false),
      wild_do_table_inited(false),
      wild_ignore_table_inited(false) {
  do_db.empty();
  ignore_db.empty();
  rewrite_db.empty();

  m_rpl_filter_lock = new Checkable_rwlock(
#ifdef HAVE_PSI_INTERFACE
      key_rwlock_rpl_filter_lock
#endif
  );
}

Rpl_filter::~Rpl_filter() {
  reset();
  delete m_rpl_filter_lock;
}

void Rpl_filter::reset() {
  if (do_table_hash_inited) delete do_table_hash;
  if (ignore_table_hash_inited) delete ignore_table_hash;

  do_table_hash_inited = false;
  ignore_table_hash_inited = false;
  do_table_array_inited = false;
  ignore_table_array_inited = false;
  wild_do_table_inited = false;
  wild_ignore_table_inited = false;
  table_rules_on = false;

  free_string_array(&do_table_array);
  free_string_array(&ignore_table_array);
  free_string_array(&wild_do_table);
  free_string_array(&wild_ignore_table);
  free_string_list(&do_db);
  free_string_list(&ignore_db);
  free_string_pair_list(&rewrite_db);

  do_table_statistics.reset();
  ignore_table_statistics.reset();
  wild_do_table_statistics.reset();
  wild_ignore_table_statistics.reset();
  do_db_statistics.reset();
  ignore_db_statistics.reset();
  rewrite_db_statistics.reset();
}

bool Rpl_filter::is_empty() {
  rdlock();
  bool res = do_table_hash_inited == 0 && ignore_table_hash_inited == 0 &&
             do_table_array_inited == 0 && ignore_table_array_inited == 0 &&
             wild_do_table_inited == 0 && wild_ignore_table_inited == 0 &&
             do_db.is_empty() && ignore_db.is_empty() && rewrite_db.is_empty();

  unlock();
  return res;
}

int Rpl_filter::copy_global_replication_filters() {
  DBUG_TRACE;
  int res = 0;
  bool need_unlock = false;

  /* Assert that it is not self copy. */
  DBUG_ASSERT(this != &rpl_global_filter);

  /* Check if the source is empty. */
  if (rpl_global_filter.is_empty()) return 0;

  THD *thd = current_thd;
  if (thd != nullptr && thd->lex->sql_command == SQLCOM_CHANGE_MASTER) {
    /*
      Acquire the write lock when copying global replication filter if
      a new channel is being created by CHANGE MASTER TO ... FOR CHANNEL
      command after server startup, in case SHOW SLAVE STATUS or
      SELECT * FROM performance_schema.replication_applier_filters is
      querying the filter in parallel. We do not have the race problem
      when creating a new channel from repository during server startup.
      Note: we hold a write lock of channel_map when executing
      CHANGE MASTER TO ... FOR CHANNEL <channel_name>, and hold a read
      lock of channel_map when executing CHANGE REPLICATION FILTER
      (the global replication filters). So we do not need to lock the
      global replication filters for read.
    */
    wrlock();
    need_unlock = true;
  }

  if (!do_table_hash_inited && rpl_global_filter.do_table_hash_inited) {
    /*
      Build this->do_table_array from rpl_global_filter.do_table_hash since
      rpl_global_filter.do_table_array is freed after building do table hash.
    */
    res = table_rule_ent_hash_to_array(&do_table_array,
                                       rpl_global_filter.do_table_hash,
                                       rpl_global_filter.do_table_hash_inited);
    if (res != 0) goto err;

    do_table_array_inited = true;
    table_rules_on = true;

    res = build_do_table_hash();
    if (res != 0) goto err;

    if (do_table_hash_inited && do_table_hash->empty()) {
      delete do_table_hash;
      do_table_hash = nullptr;
      do_table_hash_inited = false;
    }

    do_table_statistics.set_all(
        rpl_global_filter.do_table_statistics.get_configured_by());
  }

  if (!ignore_table_hash_inited && rpl_global_filter.ignore_table_hash_inited) {
    /*
      Build this->ignore_table_array from rpl_global_filter.ignore_table_hash
      since rpl_global_filter.ignore_table_array is freed after building
      ignore table hash.
    */
    res = table_rule_ent_hash_to_array(
        &ignore_table_array, rpl_global_filter.ignore_table_hash,
        rpl_global_filter.ignore_table_hash_inited);
    if (res != 0) goto err;

    ignore_table_array_inited = true;
    table_rules_on = true;

    res = build_ignore_table_hash();
    DBUG_EXECUTE_IF("simulate_out_of_memory_on_copy_ignore_table", res = 1;);
    if (res != 0) goto err;

    if (ignore_table_hash_inited && ignore_table_hash->empty()) {
      delete ignore_table_hash;
      ignore_table_hash = nullptr;
      ignore_table_hash_inited = false;
    }

    ignore_table_statistics.set_all(
        rpl_global_filter.ignore_table_statistics.get_configured_by());
  }

  if (!wild_do_table_inited && rpl_global_filter.wild_do_table_inited) {
    res = table_rule_ent_array_to_array(&wild_do_table,
                                        &rpl_global_filter.wild_do_table,
                                        rpl_global_filter.wild_do_table_inited);
    if (res != 0) goto err;

    DBUG_ASSERT(!wild_do_table.empty());

    wild_do_table_inited = true;
    table_rules_on = true;

    wild_do_table_statistics.set_all(
        rpl_global_filter.wild_do_table_statistics.get_configured_by());
  }

  if (!wild_ignore_table_inited && rpl_global_filter.wild_ignore_table_inited) {
    res = table_rule_ent_array_to_array(
        &wild_ignore_table, &rpl_global_filter.wild_ignore_table,
        rpl_global_filter.wild_ignore_table_inited);
    DBUG_EXECUTE_IF("simulate_out_of_memory_on_copy_wild_ignore_table",
                    res = 1;);
    if (res != 0) goto err;

    DBUG_ASSERT(!wild_ignore_table.empty());

    wild_ignore_table_inited = true;
    table_rules_on = true;

    wild_ignore_table_statistics.set_all(
        rpl_global_filter.wild_ignore_table_statistics.get_configured_by());
  }

  if (do_db.is_empty() && !rpl_global_filter.do_db.is_empty()) {
    /* Copy content from rpl_global_filter.do_db to this->do_db */
    res = parse_filter_list(&rpl_global_filter.do_db, &Rpl_filter::add_do_db);
    if (res != 0) goto err;

    do_db_statistics.set_all(
        rpl_global_filter.do_db_statistics.get_configured_by());
  }

  if (ignore_db.is_empty() && !rpl_global_filter.ignore_db.is_empty()) {
    /* Copy content from rpl_global_filter.ignore_db to this->ignore_db */
    res = parse_filter_list(&rpl_global_filter.ignore_db,
                            &Rpl_filter::add_ignore_db);
    DBUG_EXECUTE_IF("simulate_out_of_memory_on_copy_ignore_db", res = 1;);
    if (res != 0) goto err;

    ignore_db_statistics.set_all(
        rpl_global_filter.ignore_db_statistics.get_configured_by());
  }

  if (rewrite_db.is_empty() && !rpl_global_filter.rewrite_db.is_empty()) {
    /* Copy content from rpl_global_filter.rewrite_db to this->rewrite_db */
    I_List_iterator<i_string_pair> it(rpl_global_filter.rewrite_db);
    i_string_pair *str_pair;
    while ((str_pair = it++)) {
      res = add_db_rewrite(str_pair->key, str_pair->val);
      DBUG_EXECUTE_IF("simulate_out_of_memory_on_copy_rewrite_db", res = 1;);
      if (res) break;
    }
    if (res != 0) goto err;

    rewrite_db_statistics.set_all(
        rpl_global_filter.rewrite_db_statistics.get_configured_by());
  }

  if (need_unlock) unlock();

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  rpl_channel_filters.wrlock();
  rpl_channel_filters.reset_pfs_view();
  rpl_channel_filters.unlock();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

  return 0;

err:
  if (need_unlock) unlock();
  my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
  return 1;
}

/*
  Returns true if table should be logged/replicated

  SYNOPSIS
    tables_ok()
    db              db to use if db in TABLE_LIST is undefined for a table
    tables          list of tables to check

  NOTES
    Changing table order in the list can lead to different results.

    Note also order of precedence of do/ignore rules (see code).  For
    that reason, users should not set conflicting rules because they
    may get unpredicted results (precedence order is explained in the
    manual).

    If no table in the list is marked "updating", then we always
    return 0, because there is no reason to execute this statement on
    slave if it updates nothing.  (Currently, this can only happen if
    statement is a multi-delete (SQLCOM_DELETE_MULTI) and "tables" are
    the tables in the FROM):

    In the case of SQLCOM_DELETE_MULTI, there will be a second call to
    tables_ok(), with tables having "updating==TRUE" (those after the
    DELETE), so this second call will make the decision (because
    all_tables_not_ok() = !tables_ok(1st_list) &&
    !tables_ok(2nd_list)).

  TODO
    "Include all tables like "abc.%" except "%.EFG"". (Can't be done now.)
    If we supported Perl regexps, we could do it with pattern: /^abc\.(?!EFG)/
    (I could not find an equivalent in the regex library MySQL uses).

  RETURN VALUES
    0           should not be logged/replicated
    1           should be logged/replicated
*/

bool Rpl_filter::tables_ok(const char *db, TABLE_LIST *tables) {
  bool some_tables_updating = false;
  DBUG_TRACE;

  for (; tables; tables = tables->next_global) {
    char hash_key[2 * NAME_LEN + 2];
    char *end;
    uint len;

    if (!tables->updating) continue;
    some_tables_updating = true;
    end = my_stpcpy(hash_key, tables->db ? tables->db : db);
    *end++ = '.';
    len = (uint)(my_stpcpy(end, tables->table_name) - hash_key);
    if (do_table_hash_inited)  // if there are any do's
    {
      if (do_table_hash->count(std::string(hash_key, len)) != 0) {
        do_table_statistics.increase_counter();
        return true;
      }
    }
    if (ignore_table_hash_inited)  // if there are any ignores
    {
      if (ignore_table_hash->count(std::string(hash_key, len)) != 0) {
        ignore_table_statistics.increase_counter();
        return false;
      }
    }
    if (wild_do_table_inited && find_wild(&wild_do_table, hash_key, len)) {
      wild_do_table_statistics.increase_counter();
      return true;
    }
    if (wild_ignore_table_inited &&
        find_wild(&wild_ignore_table, hash_key, len)) {
      wild_ignore_table_statistics.increase_counter();
      return false;
    }
  }

  /*
    If no table was to be updated, ignore statement (no reason we play it on
    slave, slave is supposed to replicate _changes_ only).
    If no explicit rule found and there was a do list, do not replicate.
    If there was no do list, go ahead
  */
  return some_tables_updating && !do_table_hash_inited && !wild_do_table_inited;
}

/*
  Checks whether a db matches some do_db and ignore_db rules

  SYNOPSIS
    db_ok()
    db              name of the db to check
    need_increase_counter true if need to increase do_db/ignore_db counter

  RETURN VALUES
    0           should not be logged/replicated
    1           should be logged/replicated
*/

bool Rpl_filter::db_ok(const char *db, bool need_increase_counter) {
  DBUG_TRACE;

  if (do_db.is_empty() && ignore_db.is_empty())
    return true;  // Ok to replicate if the user puts no constraints

  /*
    Previous behaviour "if the user has specified restrictions on which
    databases to replicate and db was not selected, do not replicate" has
    been replaced with "do replicate".
    Since the filtering criteria is not equal to "NULL" the statement should
    be logged into binlog.
  */
  if (!db) return true;

  if (!do_db.is_empty())  // if the do's are not empty
  {
    I_List_iterator<i_string> it(do_db);
    i_string *tmp;

    while ((tmp = it++)) {
      /*
        Filters will follow the setting of lower_case_table_name
        to be case sensitive when setting lower_case_table_name=0.
        Otherwise they will be case insensitive but accent sensitive.
      */
      if (!my_strcasecmp(table_alias_charset, tmp->ptr, db)) {
        if (need_increase_counter) do_db_statistics.increase_counter();
        return true;  // match
      }
    }
    return false;
  } else  // there are some elements in the don't, otherwise we cannot get here
  {
    I_List_iterator<i_string> it(ignore_db);
    i_string *tmp;

    while ((tmp = it++)) {
      /*
        Filters will follow the setting of lower_case_table_name
        to be case sensitive when setting lower_case_table_name=0.
        Otherwise they will be case insensitive but accent sensitive.
      */
      if (!my_strcasecmp(table_alias_charset, tmp->ptr, db)) {
        if (need_increase_counter) ignore_db_statistics.increase_counter();
        return false;  // match
      }
    }
    return true;
  }
}

/*
  Checks whether a db matches wild_do_table and wild_ignore_table
  rules (for replication)

  SYNOPSIS
    db_ok_with_wild_table()
    db		name of the db to check.
                Is tested with check_db_name() before calling this function.

  NOTES
    Here is the reason for this function.
    We advise users who want to exclude a database 'db1' safely to do it
    with replicate_wild_ignore_table='db1.%' instead of binlog_ignore_db or
    replicate_ignore_db because the two lasts only check for the selected db,
    which won't work in that case:
    USE db2;
    UPDATE db1.t SET ... #this will be replicated and should not
    whereas replicate_wild_ignore_table will work in all cases.
    With replicate_wild_ignore_table, we only check tables. When
    one does 'DROP DATABASE db1', tables are not involved and the
    statement will be replicated, while users could expect it would not (as it
    rougly means 'DROP db1.first_table, DROP db1.second_table...').
    In other words, we want to interpret 'db1.%' as "everything touching db1".
    That is why we want to match 'db1' against 'db1.%' wild table rules.

  RETURN VALUES
    0           should not be logged/replicated
    1           should be logged/replicated
*/

bool Rpl_filter::db_ok_with_wild_table(const char *db) {
  DBUG_TRACE;

  char hash_key[NAME_LEN + 2];
  char *end;
  size_t len;
  end = my_stpcpy(hash_key, db);
  *end++ = '.';
  len = end - hash_key;
  if (wild_do_table_inited && find_wild(&wild_do_table, hash_key, len)) {
    wild_do_table_statistics.increase_counter();
    DBUG_PRINT("return", ("1"));
    return true;
  }
  if (wild_ignore_table_inited &&
      find_wild(&wild_ignore_table, hash_key, len)) {
    wild_ignore_table_statistics.increase_counter();
    DBUG_PRINT("return", ("0"));
    return false;
  }

  /*
    If no explicit rule found and there was a do list, do not replicate.
    If there was no do list, go ahead
  */
  DBUG_PRINT("return", ("db=%s,retval=%d", db, !wild_do_table_inited));
  return !wild_do_table_inited;
}

bool Rpl_filter::is_on() { return table_rules_on; }

bool Rpl_filter::is_rewrite_empty() { return rewrite_db.is_empty(); }

int Rpl_filter::add_do_table_array(const char *table_spec) {
  DBUG_TRACE;
  if (!do_table_array_inited)
    init_table_rule_array(&do_table_array, &do_table_array_inited);
  table_rules_on = true;
  return add_table_rule_to_array(&do_table_array, table_spec);
}

int Rpl_filter::add_ignore_table_array(const char *table_spec) {
  DBUG_TRACE;
  if (!ignore_table_array_inited)
    init_table_rule_array(&ignore_table_array, &ignore_table_array_inited);
  table_rules_on = true;
  return add_table_rule_to_array(&ignore_table_array, table_spec);
}

int Rpl_filter::add_wild_do_table(const char *table_spec) {
  DBUG_TRACE;
  if (!wild_do_table_inited)
    init_table_rule_array(&wild_do_table, &wild_do_table_inited);
  table_rules_on = true;
  return add_table_rule_to_array(&wild_do_table, table_spec);
}

int Rpl_filter::add_wild_ignore_table(const char *table_spec) {
  DBUG_TRACE;
  if (!wild_ignore_table_inited)
    init_table_rule_array(&wild_ignore_table, &wild_ignore_table_inited);
  table_rules_on = true;
  int ret = add_table_rule_to_array(&wild_ignore_table, table_spec);
  return ret;
}

int Rpl_filter::add_db_rewrite(const char *from_db, const char *to_db) {
  DBUG_TRACE;
  int ret = add_string_pair_list(&rewrite_db, from_db, to_db);
  return ret;
}

/*
  Build do_table rules to hash from dynamic array
  for faster filter checking.

  @return
             0           ok
             1           error
*/
int Rpl_filter::build_do_table_hash() {
  DBUG_TRACE;

  if (build_table_hash_from_array(&do_table_array, &do_table_hash,
                                  do_table_array_inited, &do_table_hash_inited))
    return 1;

  /* Free do table ARRAY as it is a copy in do table hash */
  if (do_table_array_inited) {
    free_string_array(&do_table_array);
    do_table_array_inited = false;
  }

  return 0;
}

/*
  Build ignore_table rules to hash from dynamic array
  for faster filter checking.

  @return
             0           ok
             1           error
*/
int Rpl_filter::build_ignore_table_hash() {
  DBUG_TRACE;

  if (build_table_hash_from_array(&ignore_table_array, &ignore_table_hash,
                                  ignore_table_array_inited,
                                  &ignore_table_hash_inited))
    return 1;

  /* Free ignore table ARRAY as it is a copy in ignore table hash */
  if (ignore_table_array_inited) {
    free_string_array(&ignore_table_array);
    ignore_table_array_inited = false;
  }

  return 0;
}

/**
  Table rules are initially added to DYNAMIC_LIST, and then,
  when the charset to use for tables has been established,
  inserted into a hash for faster filter checking.

  @param[in] table_array         dynamic array stored table rules
  @param[in] table_hash          hash for storing table rules
  @param[in] array_inited        Table rules are added to dynamic array
  @param[in] hash_inited         Table rules are added to hash

  @return
             0           ok
             1           error
*/
int Rpl_filter::build_table_hash_from_array(Table_rule_array *table_array,
                                            Table_rule_hash **table_hash,
                                            bool array_inited,
                                            bool *hash_inited) {
  DBUG_TRACE;

  if (array_inited) {
    init_table_rule_hash(table_hash, hash_inited);
    for (size_t i = 0; i < table_array->size(); i++) {
      TABLE_RULE_ENT *e = table_array->at(i);
      if (add_table_rule_to_hash(*table_hash, e->db, e->key_len)) return 1;
    }
  }

  return 0;
}

/**
  Added one table rule to hash.

  @param[in] h                   hash for storing table rules
  @param[in] table_spec          Table name with db
  @param[in] len                 The length of table_spec

  @return
             0           ok
             1           error
*/
int Rpl_filter::add_table_rule_to_hash(Table_rule_hash *h,
                                       const char *table_spec, uint len) {
  const char *dot = strchr(table_spec, '.');
  if (!dot) return 1;
  // len is always > 0 because we know the there exists a '.'
  TABLE_RULE_ENT *e = (TABLE_RULE_ENT *)my_malloc(
      key_memory_TABLE_RULE_ENT, sizeof(TABLE_RULE_ENT) + len, MYF(MY_WME));
  if (!e) return 1;
  e->db = (char *)e + sizeof(TABLE_RULE_ENT);
  e->tbl_name = e->db + (dot - table_spec) + 1;
  e->key_len = len;
  memcpy(e->db, table_spec, len);

  h->emplace(std::string(e->db, e->key_len),
             unique_ptr_my_free<TABLE_RULE_ENT>(e));
  return 0;
}

/*
  Add table expression to dynamic array
*/

int Rpl_filter::add_table_rule_to_array(Table_rule_array *a,
                                        const char *table_spec) {
  const char *dot = strchr(table_spec, '.');
  if (!dot) return 1;
  size_t len = strlen(table_spec);
  TABLE_RULE_ENT *e = (TABLE_RULE_ENT *)my_malloc(
      key_memory_TABLE_RULE_ENT, sizeof(TABLE_RULE_ENT) + len, MYF(MY_WME));
  if (!e) return 1;
  e->db = (char *)e + sizeof(TABLE_RULE_ENT);
  e->tbl_name = e->db + (dot - table_spec) + 1;
  e->key_len = len;
  memcpy(e->db, table_spec, len);

  if (a->push_back(e)) {
    my_free(e);
    return 1;
  }
  return 0;
}

int Rpl_filter::parse_filter_list(List<Item> *item_list, Add_filter add) {
  DBUG_TRACE;
  int status = 0;
  if (item_list->is_empty()) /* to support '()' syntax */
    return status;
  List_iterator_fast<Item> it(*item_list);
  Item *item;
  while ((item = it++)) {
    String buf;
    status = (this->*add)(item->val_str(&buf)->c_ptr());
    if (status) break;
  }
  return status;
}

int Rpl_filter::parse_filter_list(I_List<i_string> *list, Add_filter add) {
  DBUG_TRACE;
  int status = 0;
  if (list->is_empty()) /* to support '()' syntax */
    return status;
  I_List_iterator<i_string> it(*list);
  i_string *istr;
  while ((istr = it++)) {
    status = (this->*add)(istr->ptr);
    DBUG_EXECUTE_IF("simulate_out_of_memory_on_copy_do_db", status = 1;);
    if (status) break;
  }
  return status;
}

int Rpl_filter::set_do_db(List<Item> *do_db_list,
                          enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!do_db_list) return 0;
  free_string_list(&do_db);
  int ret = parse_filter_list(do_db_list, &Rpl_filter::add_do_db);
  do_db_statistics.set_all(configured_by);
  return ret;
}

int Rpl_filter::set_ignore_db(List<Item> *ignore_db_list,
                              enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!ignore_db_list) return 0;
  free_string_list(&ignore_db);
  int ret = parse_filter_list(ignore_db_list, &Rpl_filter::add_ignore_db);
  ignore_db_statistics.set_all(configured_by);
  return ret;
}

int Rpl_filter::set_do_table(List<Item> *do_table_list,
                             enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!do_table_list) return 0;
  int status;
  if (do_table_hash_inited) {
    delete do_table_hash;
    do_table_hash = nullptr;
    do_table_hash_inited = false;
  }
  if (do_table_array_inited)
    free_string_array(&do_table_array); /* purecov: inspected */
  status = parse_filter_list(do_table_list, &Rpl_filter::add_do_table_array);
  if (!status) {
    status = build_do_table_hash();
    if (do_table_hash_inited && do_table_hash->empty()) {
      delete do_table_hash;
      do_table_hash = nullptr;
      do_table_hash_inited = false;
    }
  }
  do_table_statistics.set_all(configured_by);
  return status;
}

int Rpl_filter::set_ignore_table(List<Item> *ignore_table_list,
                                 enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!ignore_table_list) return 0;
  int status;
  if (ignore_table_hash_inited) {
    delete ignore_table_hash;
    ignore_table_hash = nullptr;
    ignore_table_hash_inited = false;
  }
  if (ignore_table_array_inited)
    free_string_array(&ignore_table_array); /* purecov: inspected */
  status =
      parse_filter_list(ignore_table_list, &Rpl_filter::add_ignore_table_array);
  if (!status) {
    status = build_ignore_table_hash();
    if (ignore_table_hash_inited && ignore_table_hash->empty()) {
      delete ignore_table_hash;
      ignore_table_hash = nullptr;
      ignore_table_hash_inited = false;
    }
  }
  ignore_table_statistics.set_all(configured_by);
  return status;
}

int Rpl_filter::set_wild_do_table(List<Item> *wild_do_table_list,
                                  enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!wild_do_table_list) return 0;
  int status;
  if (wild_do_table_inited) free_string_array(&wild_do_table);

  status =
      parse_filter_list(wild_do_table_list, &Rpl_filter::add_wild_do_table);

  if (wild_do_table.empty()) {
    wild_do_table.shrink_to_fit();
    wild_do_table_inited = false;
  }
  wild_do_table_statistics.set_all(configured_by);
  return status;
}

int Rpl_filter::set_wild_ignore_table(List<Item> *wild_ignore_table_list,
                                      enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!wild_ignore_table_list) return 0;
  int status;
  if (wild_ignore_table_inited) free_string_array(&wild_ignore_table);

  status = parse_filter_list(wild_ignore_table_list,
                             &Rpl_filter::add_wild_ignore_table);

  if (wild_ignore_table.empty()) {
    wild_ignore_table.shrink_to_fit();
    wild_ignore_table_inited = false;
  }
  wild_ignore_table_statistics.set_all(configured_by);
  return status;
}

int Rpl_filter::set_db_rewrite(List<Item> *rewrite_db_pair_list,
                               enum_configured_by configured_by) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_wrlock();
  if (!rewrite_db_pair_list) return 0;
  int status = 0;
  free_string_pair_list(&rewrite_db);

  List_iterator_fast<Item> it(*rewrite_db_pair_list);
  Item *db_key, *db_val;

  if (rewrite_db_pair_list->is_empty()) /* to support '()' syntax */
    goto end;

  /* Please note that grammer itself allows only even number of db values. So
   * it is ok to do it++ twice without checking anything. */
  db_key = it++;
  db_val = it++;
  while (db_key && db_val) {
    String buf1, buf2;
    status = add_db_rewrite(db_key->val_str(&buf1)->c_ptr(),
                            db_val->val_str(&buf2)->c_ptr());
    if (status) break;
    db_key = it++;
    db_val = it++;
  }
end:
  rewrite_db_statistics.set_all(configured_by);
  return status;
}

int Rpl_filter::add_string_list(I_List<i_string> *list, const char *spec) {
  char *str;
  i_string *node;

  if (!(str = my_strdup(key_memory_rpl_filter, spec, MYF(MY_WME))))
    return true; /* purecov: inspected */

  if (!(node = new i_string(str))) {
    /* purecov: begin inspected */
    my_free(str);
    return true;
    /* purecov: end */
  }

  list->push_back(node);

  return false;
}

int Rpl_filter::add_string_pair_list(I_List<i_string_pair> *list,
                                     const char *key, const char *val) {
  char *dup_key, *dup_val;
  i_string_pair *node;

  if (!(dup_key = my_strdup(key_memory_rpl_filter, key, MYF(MY_WME))))
    return true; /* purecov: inspected */
  if (!(dup_val = my_strdup(key_memory_rpl_filter, val, MYF(MY_WME)))) {
    /* purecov: begin inspected */
    my_free(dup_key);
    return true;
    /* purecov: end */
  }

  if (!(node = new i_string_pair(dup_key, dup_val))) {
    /* purecov: begin inspected */
    my_free(dup_key);
    my_free(dup_val);
    return true;
    /* purecov: end */
  }

  list->push_back(node);

  return false;
}

int Rpl_filter::add_do_db(const char *table_spec) {
  DBUG_TRACE;
  int ret = add_string_list(&do_db, table_spec);
  return ret;
}

int Rpl_filter::add_ignore_db(const char *table_spec) {
  DBUG_TRACE;
  int ret = add_string_list(&ignore_db, table_spec);
  return ret;
}

void Rpl_filter::init_table_rule_hash(Table_rule_hash **h, bool *h_inited) {
  *h = new Table_rule_hash(table_alias_charset, key_memory_TABLE_RULE_ENT);
  *h_inited = true;
}

void Rpl_filter::init_table_rule_array(Table_rule_array *a, bool *a_inited) {
  a->clear();
  *a_inited = true;
}

TABLE_RULE_ENT *Rpl_filter::find_wild(Table_rule_array *a, const char *key,
                                      size_t len) {
  const char *key_end = key + len;

  for (size_t i = 0; i < a->size(); i++) {
    TABLE_RULE_ENT *e = a->at(i);
    /*
      Filters will follow the setting of lower_case_table_name
      to be case sensitive when setting lower_case_table_name=0.
      Otherwise they will be case insensitive but accent sensitive.
    */
    if (!my_wildcmp(table_alias_charset, key, key_end, (const char *)e->db,
                    (const char *)(e->db + e->key_len), '\\', wild_one,
                    wild_many))
      return e;
  }

  return nullptr;
}

void Rpl_filter::free_string_array(Table_rule_array *a) {
  my_free_container_pointers(*a);
  a->shrink_to_fit();
}

void Rpl_filter::free_string_list(I_List<i_string> *l) {
  char *ptr;
  i_string *tmp;

  while ((tmp = l->get())) {
    ptr = const_cast<char *>(tmp->ptr);
    my_free(ptr);
    delete tmp;
  }

  l->empty();
}

void Rpl_filter::free_string_pair_list(I_List<i_string_pair> *pl) {
  i_string_pair *tmp;
  while ((tmp = pl->get())) {
    my_free(const_cast<char *>(tmp->key));
    my_free(const_cast<char *>(tmp->val));
    delete tmp;
  }

  pl->empty();
}

/*
  Builds a String from a hash of TABLE_RULE_ENT.

  SYNOPSIS
    table_rule_ent_hash_to_str()
    s               pointer to the String to fill
    h               pointer to the hash to read

  RETURN VALUES
    none
*/

void Rpl_filter::table_rule_ent_hash_to_str(String *s, Table_rule_hash *h,
                                            bool inited) {
  s->length(0);
  if (inited) {
    /*
      Return the entries in sorted order. This isn't a protocol requirement
      (and thus, we don't need to care about collations), but it makes for
      easier testing when things are deterministic and not in hash order.
    */
    std::vector<TABLE_RULE_ENT *> entries;
    entries.reserve(h->size());
    for (const auto &key_and_value : *h) {
      entries.push_back(key_and_value.second.get());
    }
    std::sort(entries.begin(), entries.end(),
              [](const TABLE_RULE_ENT *a, const TABLE_RULE_ENT *b) {
                return std::string(a->db, a->key_len) <
                       std::string(b->db, b->key_len);
              });
    for (const TABLE_RULE_ENT *e : entries) {
      if (s->length()) s->append(',');
      s->append(e->db, e->key_len);
    }
  }
}

int Rpl_filter::table_rule_ent_hash_to_array(Table_rule_array *table_array,
                                             Table_rule_hash *h, bool inited) {
  if (inited) {
    /*
      Build do_table_array from other.do_table_hash since other.do_table_array
      is freed after building do table hash.
    */
    for (const auto &key_and_value : *h) {
      const TABLE_RULE_ENT *ori_e = key_and_value.second.get();

      const char *dot = strchr(ori_e->db, '.');
      if (!dot) return 1;
      size_t len = ori_e->key_len;
      TABLE_RULE_ENT *e = (TABLE_RULE_ENT *)my_malloc(
          key_memory_TABLE_RULE_ENT, sizeof(TABLE_RULE_ENT) + len, MYF(MY_WME));
      if (!e) return 1;
      e->db = (char *)e + sizeof(TABLE_RULE_ENT);
      e->tbl_name = e->db + (dot - ori_e->db) + 1;
      e->key_len = len;
      memcpy(e->db, ori_e->db, len);

      if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_copy_do_table", 1, 0) ||
          table_array->push_back(e)) {
        my_free(e);
        return 1;
      }
    }
  }

  return 0;
}

int Rpl_filter::table_rule_ent_array_to_array(Table_rule_array *dest_array,
                                              Table_rule_array *source_array,
                                              bool inited) {
  if (inited) {
    size_t array_size = source_array->size();
    for (size_t i = 0; i < array_size; i++) {
      TABLE_RULE_ENT *ori_e = source_array->at(i);

      const char *dot = strchr(ori_e->db, '.');
      if (!dot) return 1;
      size_t len = ori_e->key_len;
      TABLE_RULE_ENT *e = (TABLE_RULE_ENT *)my_malloc(
          key_memory_TABLE_RULE_ENT, sizeof(TABLE_RULE_ENT) + len, MYF(MY_WME));
      if (!e) return 1;
      e->db = (char *)e + sizeof(TABLE_RULE_ENT);
      e->tbl_name = e->db + (dot - ori_e->db) + 1;
      e->key_len = len;
      memcpy(e->db, ori_e->db, len);

      if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_copy_wild_do_table", 1,
                           0) ||
          dest_array->push_back(e)) {
        my_free(e);
        return 1;
      }
    }
  }

  return 0;
}

void Rpl_filter::table_rule_ent_dynamic_array_to_str(String *s,
                                                     Table_rule_array *a,
                                                     bool inited) {
  s->length(0);
  if (inited) {
    for (size_t i = 0; i < a->size(); i++) {
      TABLE_RULE_ENT *e = a->at(i);
      if (s->length()) s->append(',');
      s->append(e->db, e->key_len);
    }
  }
}

void Rpl_filter::get_do_table(String *str) {
  table_rule_ent_hash_to_str(str, do_table_hash, do_table_hash_inited);
}

void Rpl_filter::get_ignore_table(String *str) {
  table_rule_ent_hash_to_str(str, ignore_table_hash, ignore_table_hash_inited);
}

void Rpl_filter::get_wild_do_table(String *str) {
  table_rule_ent_dynamic_array_to_str(str, &wild_do_table,
                                      wild_do_table_inited);
}

void Rpl_filter::get_wild_ignore_table(String *str) {
  table_rule_ent_dynamic_array_to_str(str, &wild_ignore_table,
                                      wild_ignore_table_inited);
}

void Rpl_filter::get_rewrite_db(String *str) {
  str->length(0);
  if (!rewrite_db.is_empty()) {
    I_List_iterator<i_string_pair> it(rewrite_db);
    i_string_pair *s;
    while ((s = it++)) {
      str->append('(');
      str->append(s->key);
      str->append(',');
      str->append(s->val);
      str->append(')');
      str->append(',');
    }
    // Remove last ',' str->chop();
    str->chop();
  }
}

const char *Rpl_filter::get_rewrite_db(const char *db, size_t *new_len) {
  if (rewrite_db.is_empty() || !db) return db;
  I_List_iterator<i_string_pair> it(rewrite_db);
  i_string_pair *tmp;

  while ((tmp = it++)) {
    /*
      Filters will follow the setting of lower_case_table_name
      to be case sensitive when setting lower_case_table_name=0.
      Otherwise they will be case insensitive but accent sensitive.
    */
    if (!my_strcasecmp(table_alias_charset, tmp->key, db)) {
      *new_len = strlen(tmp->val);
      return tmp->val;
    }
  }
  return db;
}

I_List<i_string> *Rpl_filter::get_do_db() { return &do_db; }

void Rpl_filter::get_do_db(String *str) {
  str->length(0);
  if (!do_db.is_empty()) {
    I_List_iterator<i_string> it(do_db);
    i_string *s;
    while ((s = it++)) {
      str->append(s->ptr);
      str->append(',');
    }
    // Remove last ','
    str->chop();
  }
}

I_List<i_string> *Rpl_filter::get_ignore_db() { return &ignore_db; }

void Rpl_filter::get_ignore_db(String *str) {
  str->length(0);
  if (!ignore_db.is_empty()) {
    I_List_iterator<i_string> it(ignore_db);
    i_string *s;
    while ((s = it++)) {
      str->append(s->ptr);
      str->append(',');
    }
    // Remove last ','
    str->chop();
  }
}

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE

void Rpl_filter::put_filters_into_vector(
    std::vector<Rpl_pfs_filter> &rpl_pfs_filter_vec, const char *channel_name) {
  DBUG_TRACE;
  m_rpl_filter_lock->assert_some_lock();

  String filter_rule;

  if (do_db_statistics.get_active_since() > 0) {
    // Fill REPLICATE_DO_DB filter.
    get_do_db(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_DO_DB",
                                    filter_rule, &do_db_statistics);
  }

  if (ignore_db_statistics.get_active_since() > 0) {
    // Fill REPLICATE_IGNORE_DB filter.
    get_ignore_db(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_IGNORE_DB",
                                    filter_rule, &ignore_db_statistics);
  }

  if (do_table_statistics.get_active_since() > 0) {
    // Fill REPLICATE_DO_TABLE filter.
    get_do_table(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_DO_TABLE",
                                    filter_rule, &do_table_statistics);
  }

  if (ignore_table_statistics.get_active_since() > 0) {
    // Fill REPLICATE_IGNORE_TABLE filter.
    get_ignore_table(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_IGNORE_TABLE",
                                    filter_rule, &ignore_table_statistics);
  }

  if (wild_do_table_statistics.get_active_since() > 0) {
    // Fill REPLICATE_WILD_DO_TABLE filter.
    get_wild_do_table(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_WILD_DO_TABLE",
                                    filter_rule, &wild_do_table_statistics);
  }

  if (wild_ignore_table_statistics.get_active_since() > 0) {
    // Fill REPLICATE_WILD_IGNORE_TABLE filter.
    get_wild_ignore_table(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_WILD_IGNORE_TABLE",
                                    filter_rule, &wild_ignore_table_statistics);
  }

  if (rewrite_db_statistics.get_active_since() > 0) {
    // Fill REPLICATE_REWRITE_DB filter.
    get_rewrite_db(&filter_rule);
    rpl_pfs_filter_vec.emplace_back(channel_name, "REPLICATE_REWRITE_DB",
                                    filter_rule, &rewrite_db_statistics);
  }
}

void Rpl_global_filter::reset_pfs_view() {
  DBUG_TRACE;
  assert_some_wrlock();

  rpl_pfs_filter_vec.clear();

  // Pass NULL since rpl_global_filter does not attach a channel.
  put_filters_into_vector(rpl_pfs_filter_vec, nullptr);
}

Rpl_pfs_filter *Rpl_global_filter::get_filter_at_pos(uint pos) {
  DBUG_TRACE;
  assert_some_rdlock();

  if (pos < rpl_pfs_filter_vec.size())
    return &rpl_pfs_filter_vec[pos];
  else
    return nullptr;
}

uint Rpl_global_filter::get_filter_count() {
  DBUG_TRACE;
  assert_some_rdlock();

  return rpl_pfs_filter_vec.size();
}

#endif /*WITH_PERFSCHEMA_STORAGE_ENGINE */

bool Sql_cmd_change_repl_filter::execute(THD *thd) {
  DBUG_TRACE;
  bool rc = change_rpl_filter(thd);
  return rc;
}

void Sql_cmd_change_repl_filter::set_filter_value(List<Item> *item_list,
                                                  options_mysqld filter_type) {
  DBUG_TRACE;
  switch (filter_type) {
    case OPT_REPLICATE_DO_DB:
      do_db_list = item_list;
      break;
    case OPT_REPLICATE_IGNORE_DB:
      ignore_db_list = item_list;
      break;
    case OPT_REPLICATE_DO_TABLE:
      do_table_list = item_list;
      break;
    case OPT_REPLICATE_IGNORE_TABLE:
      ignore_table_list = item_list;
      break;
    case OPT_REPLICATE_WILD_DO_TABLE:
      wild_do_table_list = item_list;
      break;
    case OPT_REPLICATE_WILD_IGNORE_TABLE:
      wild_ignore_table_list = item_list;
      break;
    case OPT_REPLICATE_REWRITE_DB:
      rewrite_db_pair_list = item_list;
      break;
    default:
      /* purecov: begin deadcode */
      DBUG_ASSERT(0);
      break;
      /* purecov: end */
  }
}

/**
  Execute a CHANGE REPLICATION FILTER statement to set filter rules.

  @param thd A pointer to the thread handler object.

  @retval false success
  @retval true error
 */
bool Sql_cmd_change_repl_filter::change_rpl_filter(THD *thd) {
  DBUG_TRACE;
  bool ret = false;
  int thread_mask = 0;
  Master_info *mi = nullptr;
  LEX *lex = thd->lex;
  Rpl_filter *rpl_filter;

  Security_context *sctx = thd->security_context();
  if (!sctx->check_access(SUPER_ACL) &&
      !sctx->has_global_grant(STRING_WITH_LEN("REPLICATION_SLAVE_ADMIN"))
           .first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER or REPLICATION_SLAVE_ADMIN");
    return ret = true;
  }

  channel_map.rdlock();

  if (!lex->mi.for_channel) {
    if (channel_map.get_num_instances(true) == 0) {
      my_error(ER_SLAVE_CONFIGURATION, MYF(0));
      ret = true;
      goto err;
    }
    /*
      CHANGE REPLICATION FILTER filter [, filter...] with no FOR CHANNEL clause
      does the following, both for every configured slave replication channel's
      per-channel filter and for the global filters: For every filter type, if
      the filter type is listed in the statement, then any existing filter
      rules of that type are replaced by the filter rules specified in the
      statement. The statement does not act on group replication channels,
      because replication filters on group replication channels are
      disallowed.
    */
    mi_map::iterator it_end = channel_map.end();
    for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
         it++) {
      mi = it->second;
      if (Master_info::is_configured(mi)) {
        /* lock slave_sql_thread */
        mysql_mutex_lock(&mi->rli->run_lock);
        /* Check the running status of all SQL threads */
        init_thread_mask(&thread_mask, mi, false /*not inverse*/);
        if (thread_mask & SLAVE_SQL) {
          /* We refuse if any slave thread is running */
          my_error(ER_SLAVE_CHANNEL_SQL_THREAD_MUST_STOP, MYF(0),
                   mi->get_channel());
          ret = true;
          /*
            Stop acquiring the run_locks once finding a SQL thread running
            and record the stop position.
          */
          it_end = ++it;
          break;
        }
      }
    }

    if (!ret) {
      for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
           it++) {
        mi = it->second;
        if (Master_info::is_configured(mi)) {
          /* filter for this channel */
          rpl_filter = mi->rli->rpl_filter;
          if (rpl_filter != nullptr) {
            rpl_filter->wrlock();
            if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_CRF", 1, 0) ||
                rpl_filter->set_do_db(
                    do_db_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_ignore_db(
                    ignore_db_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_do_table(
                    do_table_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_ignore_table(
                    ignore_table_list,
                    CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_wild_do_table(
                    wild_do_table_list,
                    CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_wild_ignore_table(
                    wild_ignore_table_list,
                    CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
                rpl_filter->set_db_rewrite(
                    rewrite_db_pair_list,
                    CONFIGURED_BY_CHANGE_REPLICATION_FILTER)) {
              my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
              ret = true;
            }
            rpl_filter->unlock();
          }
        }
      }
    }

    if (!ret) {
      rpl_global_filter.wrlock();
      if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_global_CRF", 1, 0) ||
          rpl_global_filter.set_do_db(
              do_db_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_ignore_db(
              ignore_db_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_do_table(
              do_table_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_ignore_table(
              ignore_table_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_wild_do_table(
              wild_do_table_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_wild_ignore_table(
              wild_ignore_table_list,
              CONFIGURED_BY_CHANGE_REPLICATION_FILTER) ||
          rpl_global_filter.set_db_rewrite(
              rewrite_db_pair_list, CONFIGURED_BY_CHANGE_REPLICATION_FILTER)) {
        my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
        ret = true;
      }
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
      rpl_global_filter.reset_pfs_view();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */
      rpl_global_filter.unlock();
    }

    /* Release the run_locks until the stop position recorded in above. */
    for (mi_map::iterator it = channel_map.begin(); it != it_end; it++) {
      mi = it->second;
      if (Master_info::is_configured(mi))
        mysql_mutex_unlock(&mi->rli->run_lock);
    }
  } else {
    if (channel_map.is_group_replication_channel_name(lex->mi.channel)) {
      /*
        If an explicit FOR CHANNEL clause is provided, the statement
        is disallowed on group replication channels.
      */
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "CHANGE REPLICATION FILTER", lex->mi.channel);
      ret = true;
      goto err;
    }

    /*
      If an explicit FOR CHANNEL clause is provided, the statement acts on
      that configured slave replication channel removing any existing
      replication filter if it has the same filter type as one of
      specified replication filters, and replacing them with the
      specified ones. Filter types that were not explicitly listed
      in the statement are not modified.
    */
    mi = channel_map.get_mi(lex->mi.channel);

    if (!Master_info::is_configured(mi)) {
      my_error(ER_SLAVE_CONFIGURATION, MYF(0));
      ret = true;
      goto err;
    }

    /* lock slave_sql_thread */
    mysql_mutex_lock(&mi->rli->run_lock);

    /* check the status of SQL thread */
    init_thread_mask(&thread_mask, mi, false /*not inverse*/);
    /* We refuse if the slave thread is running */
    if (thread_mask & SLAVE_SQL) {
      my_error(ER_SLAVE_CHANNEL_SQL_THREAD_MUST_STOP, MYF(0),
               mi->get_channel());
      ret = true;
    }

    if (!ret) {
      /* filter for this channel */
      rpl_filter = mi->rli->rpl_filter;
      if (rpl_filter != nullptr) {
        rpl_filter->wrlock();
        if (DBUG_EVALUATE_IF("simulate_out_of_memory_on_CRF_FOR_CHA", 1, 0) ||
            rpl_filter->set_do_db(
                do_db_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_ignore_db(
                ignore_db_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_do_table(
                do_table_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_ignore_table(
                ignore_table_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_wild_do_table(
                wild_do_table_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_wild_ignore_table(
                wild_ignore_table_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL) ||
            rpl_filter->set_db_rewrite(
                rewrite_db_pair_list,
                CONFIGURED_BY_CHANGE_REPLICATION_FILTER_FOR_CHANNEL)) {
          /* purecov: begin inspected */
          my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
          ret = true;
          /* purecov: end */
        }
        rpl_filter->unlock();
      }
    }

    mysql_mutex_unlock(&mi->rli->run_lock);
  }

  if (ret) goto err;

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  rpl_channel_filters.wrlock();
  rpl_channel_filters.reset_pfs_view();
  rpl_channel_filters.unlock();
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

  my_ok(thd);

err:
  channel_map.unlock();
  return ret;
}
