/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <algorithm>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "sql/mysqld.h"
#include "sql/rpl_lag_manager.h"
#include "sql/rpl_master.h"
#include "sql/sql_class.h"

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WRITE_STATISTICS
************************************************************************/

/* WRITE_STATS - stores write statistics for a sql statement, shard, client or
 * user */
struct WRITE_STATS {
  ulonglong binlog_bytes_written; /* Bytes written into binlog */
  ulonglong cpu_write_time_us;    /* CPU write time spent in micro-seconds */
};
/*
  Map integer representatin of write stats dimensions to string
  constants. The integer value of these dimensions map to the
  index in TIME_BUCKET_STATS array
*/
const std::string WRITE_STATS_TYPE_STRING[] = {"USER", "CLIENT", "SHARD",
                                               "SQL_ID"};
typedef std::array<std::unordered_map<std::string, WRITE_STATS>,
                   WRITE_STATISTICS_DIMENSION_COUNT>
    TIME_BUCKET_STATS;
/* Global write statistics map */
std::list<std::pair<int, TIME_BUCKET_STATS>> global_write_statistics_map;

/*
  free_global_write_statistics
    Frees global_write_statistics
*/
void free_global_write_statistics() {
  mysql_mutex_lock(&LOCK_global_write_statistics);
  global_write_statistics_map.clear();
  mysql_mutex_unlock(&LOCK_global_write_statistics);
}

/*
  populate_write_statistics
    Populate the write statistics

  Input:
    thd                 in:  - THD
    time_bucket_stats  out:  - Array structure containing write stats populated.
*/
static void populate_write_statistics(THD *thd,
                                      TIME_BUCKET_STATS &time_bucket_stats) {
  ulonglong binlog_bytes_written = thd->get_row_binlog_bytes_written();
  ulonglong total_write_time = thd->get_stmt_total_write_time();

  // Get keys for all the target dimensions to update write stats for
  std::array<std::string, WRITE_STATISTICS_DIMENSION_COUNT> keys;
  thd->get_mt_keys_for_write_query(keys);

  // Add/Update the write stats
  for (uint i = 0; i < WRITE_STATISTICS_DIMENSION_COUNT; i++) {
    auto iter = time_bucket_stats[i].find(keys[i]);
    if (iter == time_bucket_stats[i].end()) {
      WRITE_STATS ws;
      ws.binlog_bytes_written = binlog_bytes_written;
      ws.cpu_write_time_us = total_write_time;
      time_bucket_stats[i].insert(std::make_pair(keys[i], ws));
    } else {
      WRITE_STATS &ws = iter->second;
      ws.binlog_bytes_written += binlog_bytes_written;
      ws.cpu_write_time_us += total_write_time;
    }
  }
}

/*
  store_write_statistics
    Store the write statistics for the executed statement.
    The bulk of the work is done in populate_write_stats()

  Input:
    thd         in:  - THD
*/
void store_write_statistics(THD *thd) {
  // write_stats_frequency may be updated dynamically. Caching it for the
  // logic below
  ulong write_stats_frequency_cached = write_stats_frequency;
  if (write_stats_frequency_cached == 0) {
    return;
  }

  mysql_mutex_lock(&LOCK_global_write_statistics);
  time_t timestamp = my_time(0);
  int time_bucket_key = timestamp - (timestamp % write_stats_frequency_cached);
  auto time_bucket_iter = global_write_statistics_map.begin();

  DBUG_EXECUTE_IF("dbug.add_write_stats_to_most_recent_bucket", {
    if (time_bucket_iter != global_write_statistics_map.end()) {
      time_bucket_key = time_bucket_iter->first;
    }
  });

  if (time_bucket_iter == global_write_statistics_map.end() ||
      time_bucket_key > time_bucket_iter->first) {
    // time_bucket is newer than last registered bucket...
    // need to insert a new one
    while (!global_write_statistics_map.empty() &&
           (uint)global_write_statistics_map.size() >= write_stats_count) {
      // We are over the configured size. Erase older entries first.
      global_write_statistics_map.pop_back();
    }
    TIME_BUCKET_STATS time_bucket_stats;
    populate_write_statistics(thd, time_bucket_stats);
    global_write_statistics_map.push_front(
        std::make_pair(time_bucket_key, time_bucket_stats));
  } else {
    populate_write_statistics(thd, time_bucket_iter->second);
  }
  mysql_mutex_unlock(&LOCK_global_write_statistics);
}

std::vector<write_statistics_row> get_all_write_statistics() {
  std::vector<write_statistics_row> write_statistics;
  mysql_mutex_lock(&LOCK_global_write_statistics);
  for (auto time_bucket_iter = global_write_statistics_map.cbegin();
       time_bucket_iter != global_write_statistics_map.cend();
       ++time_bucket_iter) {
    const TIME_BUCKET_STATS &time_bucket = time_bucket_iter->second;

    for (int type = 0; type != (int)time_bucket.size(); ++type) {
      for (auto stats_iter = time_bucket[type].begin();
           stats_iter != time_bucket[type].end(); ++stats_iter) {
        write_statistics.emplace_back(
            time_bucket_iter->first, WRITE_STATS_TYPE_STRING[type],
            stats_iter->first.c_str(), stats_iter->second.binlog_bytes_written,
            // cpu_write_time_ms (convert from micro-secs to mill-secs)
            stats_iter->second.cpu_write_time_us / 1000);
      }
    }
  }
  mysql_mutex_unlock(&LOCK_global_write_statistics);
  return write_statistics;
}

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WRITE_THROTTLING_RULES
************************************************************************/

GLOBAL_WRITE_THROTTLING_RULES_MAP global_write_throttling_rules;
/* Queue to store all the entities being currently auto throttled. It is used to
release entities in order they were throttled when replication lag goes below
safe threshold  */
std::list<std::pair<std::string, enum_wtr_dimension>>
    currently_throttled_entities;

/*
  free_global_write_throttling_rules
    Frees auto and manual rules from global_write_throttling_rules data
    structure
*/
void free_global_write_throttling_rules() {
  mysql_mutex_lock(&LOCK_global_write_throttling_rules);
  for (uint i = 0; i < WRITE_STATISTICS_DIMENSION_COUNT; i++) {
    global_write_throttling_rules[i].clear();
  }
  mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
}

/*
  free_global_write_auto_throttling_rules
    Frees only auto rules from global_write_throttling_rules data structure
*/
void free_global_write_auto_throttling_rules() {
  mysql_mutex_lock(&LOCK_global_write_throttling_rules);
  for (auto &rules : global_write_throttling_rules) {
    auto rules_iter = rules.begin();
    while (rules_iter != rules.end()) {
      if (rules_iter->second.mode == WTR_AUTO) {
        rules_iter = rules.erase(rules_iter);
      } else {
        ++rules_iter;
      }
    }
  }
  mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
}

/*
  Utility method to convert a dimension(client, sql_id, user, shard)
  string to integer index. These indices are used in
  global_write_throttling_rules data structure. If the string doesn't represent
  a dimension, WTR_DIM_UNKNOWN is returned.
*/
enum_wtr_dimension get_wtr_dimension_from_str(std::string type_str) {
  int type = WRITE_STATISTICS_DIMENSION_COUNT - 1;

  while (type >= 0 && WRITE_STATS_TYPE_STRING[type] != type_str) type--;

  return static_cast<enum_wtr_dimension>(type);
}

/*
  Stores a user specified order in which throttling system should throttle
  write throttling dimensions in case of replication lag
*/
bool store_write_throttle_permissible_dimensions_in_order(char *new_value) {
  if (strcmp(new_value, "OFF") == 0) {
    mysql_mutex_lock(&LOCK_replication_lag_auto_throttling);
    write_throttle_permissible_dimensions_in_order.clear();
    mysql_mutex_unlock(&LOCK_replication_lag_auto_throttling);
    return false;
  }

  // copy the string to avoid mutating new var value.
  char *new_value_copy = (char *)my_malloc(PSI_NOT_INSTRUMENTED,
                                           strlen(new_value) + 1, MYF(MY_WME));
  if(new_value_copy == nullptr) {
    return true; // failure allocating memory
  }
  strcpy(new_value_copy, new_value);
  char *wtr_dim_str;
  enum_wtr_dimension wtr_dim;
  std::vector<enum_wtr_dimension> new_dimensions;
  bool result = false;

  wtr_dim_str = strtok(new_value_copy, ",");
  while (wtr_dim_str != nullptr) {
    wtr_dim = get_wtr_dimension_from_str(wtr_dim_str);
    if (wtr_dim == WTR_DIM_UNKNOWN) {
      result = true;  // not a valid dimension string, failure
      break;
    }
    auto it = std::find(new_dimensions.begin(), new_dimensions.end(), wtr_dim);
    if (it != new_dimensions.end()) {
      result = true;  // duplicate dimension, failure
      break;
    }
    new_dimensions.push_back(wtr_dim);
    wtr_dim_str = strtok(nullptr, ",");
  }
  if (!result) {
    mysql_mutex_lock(&LOCK_replication_lag_auto_throttling);
    write_throttle_permissible_dimensions_in_order = new_dimensions;
    mysql_mutex_unlock(&LOCK_replication_lag_auto_throttling);
  }
  my_free(new_value_copy);
  return result;
}

/*
  Stores a user specified throttling rule from write_throttling_patterns
  sys_var into global_write_throttling_rules
*/
bool store_write_throttling_rules() {
  char *wtr_string_cur_pos;
  std::string type_str;
  std::string value_str;
  enum_wtr_dimension wtr_dim;

  char op = latest_write_throttling_rule[0];

  // first character is + or -
  if (op == '+' || op == '-') {
    wtr_string_cur_pos = latest_write_throttling_rule;
    wtr_string_cur_pos++;

    type_str = strtok_r(wtr_string_cur_pos, "=", &wtr_string_cur_pos);
    wtr_dim = get_wtr_dimension_from_str(type_str);
    value_str = strtok_r(wtr_string_cur_pos, "", &wtr_string_cur_pos);
    if (wtr_dim == WTR_DIM_UNKNOWN || value_str == "") {
      return true;
    }
    WRITE_THROTTLING_RULE rule;
    rule.mode = WTR_MANUAL;  // manual
    rule.create_time = my_time(0);
    rule.throttle_rate = 100;  // manual rules are fully throttled

    mysql_mutex_lock(&LOCK_global_write_throttling_rules);
    auto &rules_map = global_write_throttling_rules[wtr_dim];
    auto iter = rules_map.find(value_str);
    bool remove_from_currently_throttled_entities = false;
    if (op == '+') {
      if (iter != rules_map.end()) {
        // If manually overriding an auto rule, remove it from
        // currently_throttled_entities queue
        if (rules_map[value_str].mode == WTR_AUTO)
          remove_from_currently_throttled_entities = true;
        rules_map[value_str] = rule;
      } else
        rules_map.insert(std::make_pair(value_str, rule));
    } else {  // op == '-'
      if (iter != rules_map.end()) {
        // If manually overriding an auto rule, remove it from
        // currently_throttled_entities queue
        if (rules_map[value_str].mode == WTR_AUTO)
          remove_from_currently_throttled_entities = true;
        rules_map.erase(iter);
      }
    }
    if (remove_from_currently_throttled_entities) {
      for (auto q_iter = currently_throttled_entities.begin();
           q_iter != currently_throttled_entities.end(); q_iter++) {
        if (q_iter->first == value_str) {
          currently_throttled_entities.erase(q_iter);
          break;
        }
      }
    }
    mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
    return false;  // success
  }
  return true;  // failure
}

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WHITE_THROTTLING_LOG
************************************************************************/

/*
** enum_wtr_thrlog_txn_type
**
** valid values for the COLUMN I_S.write_throttling_log."TRANSACTION_TYPE"
*/
enum enum_wtr_thrlog_txn_type {
  WTR_THRLOG_TXN_TYPE_SHORT = 0,
  WTR_THRLOG_TXN_TYPE_LONG = 1,
};

/*
** WRITE_THRLOG_TXN_TYPE
**
** valid values for the COLUMN I_S.write_throttling_log."TRANSACTION_TYPE"
*/
const std::string WRITE_THRLOG_TXN_TYPE[] = {"SHORT", "LONG"};

std::array<std::array<std::unordered_map<std::string, WRITE_THROTTLING_LOG>,
                      WRITE_THROTTLING_MODE_COUNT>,
           WRITE_STATISTICS_DIMENSION_COUNT>
    global_write_throttling_log;

/*
  free_global_write_throttling_log
    Frees global_write_throttling_log
*/
void free_global_write_throttling_log(void) {
  mysql_mutex_lock(&LOCK_global_write_throttling_log);
  for (uint i = 0; i < WRITE_STATISTICS_DIMENSION_COUNT; i++) {
    for (uint j = 0; j < WRITE_THROTTLING_MODE_COUNT; j++) {
      global_write_throttling_log[i][j].clear();
    }
  }
  mysql_mutex_unlock(&LOCK_global_write_throttling_log);
}

/*
** global_long_qry_abort_log
**
** global map that stores the long queries information to populate
** the table performance_schema.write_throttling_log
*/
std::unordered_map<std::string, WRITE_THROTTLING_LOG> global_long_qry_abort_log;

/*
  store_write_throttling_log
    Stores a log for when a query was throttled due to a throttling
    rule in I_S.WRITE_THROTTLING_RULES
*/
void store_write_throttling_log(THD *, int type, std::string value,
                                WRITE_THROTTLING_RULE &rule) {
  mysql_mutex_lock(&LOCK_global_write_throttling_log);
  WRITE_THROTTLING_LOG log{};
  time_t timestamp = my_time(0);
  auto &log_map = global_write_throttling_log[type][rule.mode];
  auto &inserted_log = log_map.insert(std::make_pair(value, log)).first->second;
  inserted_log.last_time = timestamp;
  inserted_log.count++;
  mysql_mutex_unlock(&LOCK_global_write_throttling_log);
}

/*
  store_long_qry_abort_log
    Stores a log for a query that is aborted due to it being identified
    as long running query. This will be added to
    performance_schema.write_throttling_log.
*/
void store_long_qry_abort_log(THD *thd) {
  // SQL ID
  char sql_id[DIGEST_HASH_TO_STRING_LENGTH + 1];
  thd->mt_hex_value(THD::SQL_ID, sql_id, DIGEST_HASH_TO_STRING_LENGTH + 1);

  mysql_mutex_lock(&LOCK_global_write_throttling_log);
  WRITE_THROTTLING_LOG log{};
  time_t timestamp = my_time(0);
  auto &inserted_log =
      global_long_qry_abort_log
          .insert(std::make_pair(
              std::string(sql_id, DIGEST_HASH_TO_STRING_LENGTH), log))
          .first->second;
  inserted_log.last_time = timestamp;
  inserted_log.count++;
  mysql_mutex_unlock(&LOCK_global_write_throttling_log);
}

std::vector<write_throttling_rules_row> get_all_write_throttling_rules() {
  std::vector<write_throttling_rules_row> write_throttling_rules;
  mysql_mutex_lock(&LOCK_global_write_throttling_rules);

  for (int type = 0; type != (int)global_write_throttling_rules.size();
       ++type) {
    for (auto rules_iter = global_write_throttling_rules[type].begin();
         rules_iter != global_write_throttling_rules[type].end();
         ++rules_iter) {
      write_throttling_rules.emplace_back(
          WRITE_THROTTLING_MODE_STRING[rules_iter->second.mode],
          rules_iter->second.create_time, WRITE_STATS_TYPE_STRING[type],
          rules_iter->first, rules_iter->second.throttle_rate);
    }
  }
  mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
  return write_throttling_rules;
}

std::vector<write_throttling_log_row> get_all_write_throttling_log() {
  std::vector<write_throttling_log_row> write_throttling_log;
  mysql_mutex_lock(&LOCK_global_write_throttling_log);

  for (size_t type = 0; type != global_write_throttling_log.size(); ++type) {
    for (size_t mode = 0; mode != global_write_throttling_log[type].size();
         ++mode) {
      for (auto log_iter = global_write_throttling_log[type][mode].begin();
           log_iter != global_write_throttling_log[type][mode].end();
           ++log_iter) {
        WRITE_THROTTLING_LOG &log = log_iter->second;
        write_throttling_log.emplace_back(
            WRITE_THROTTLING_MODE_STRING[mode], log.last_time,
            WRITE_STATS_TYPE_STRING[type], log_iter->first,
            WRITE_THRLOG_TXN_TYPE[WTR_THRLOG_TXN_TYPE_SHORT], log.count);
      }
    }
  }

  /* populate rows from aborting long running queries */
  for (auto log_iter = global_long_qry_abort_log.begin();
       log_iter != global_long_qry_abort_log.end(); ++log_iter) {
    WRITE_THROTTLING_LOG &log = log_iter->second;

    // mode: AUTO
    auto mode = WTR_AUTO;
    write_throttling_log.emplace_back(
        WRITE_THROTTLING_MODE_STRING[mode], log.last_time,
        WRITE_STATS_TYPE_STRING[WTR_DIM_SQL_ID], log_iter->first,
        WRITE_THRLOG_TXN_TYPE[WTR_THRLOG_TXN_TYPE_LONG], log.count);
  }

  mysql_mutex_unlock(&LOCK_global_write_throttling_log);
  return write_throttling_log;
}

/***********************************************************************
OBJECTS & METHODS TO SUPPORT AUTO_THROTTLING OF WRITE QUERIES
************************************************************************/

/* timestamp when replication lag check was last done */
std::atomic<time_t> last_replication_lag_check_time(0);
/* Stores the info about the entity that is currently being monitored for
 * replication lag */
WRITE_MONITORED_ENTITY currently_monitored_entity;

/*
  update_monitoring_status_for_entity
    Given a potential entity that is causing replication lag, this method either
  marks it to be monitored for next cycle or marks it to be throttled if we have
  already monitored it for enough cycles.
*/
void static update_monitoring_status_for_entity(std::string name,
                                                enum_wtr_dimension dimension) {
  if (write_throttle_monitor_cycles == 0 ||
      (currently_monitored_entity.dimension == dimension &&
       currently_monitored_entity.name == name)) {
    currently_monitored_entity.hits++;
    if (currently_monitored_entity.hits >= write_throttle_monitor_cycles) {
      // throttle the entity, create a rule if not already created
      WRITE_THROTTLING_RULE rule;
      rule.mode = WTR_AUTO;  // auto
      rule.create_time = my_time(0);
      rule.throttle_rate = write_throttle_rate_step;

      mysql_mutex_lock(&LOCK_global_write_throttling_rules);
      auto &rules_map = global_write_throttling_rules[dimension];
      auto iter = rules_map.find(name);
      if (iter == rules_map.end()) {
        rules_map.insert(std::make_pair(name, rule));
        // insert the entity into currently_throttled_entities queue
        currently_throttled_entities.push_back(std::make_pair(name, dimension));
      }
      mysql_mutex_unlock(&LOCK_global_write_throttling_rules);

      // reset currently_monitored_entity
      currently_monitored_entity.reset();
    }
  } else {
    // update the currently monitored entity
    currently_monitored_entity.dimension = dimension;
    currently_monitored_entity.name = name;
    currently_monitored_entity.hits = 0;
  }
}

/*
  get_top_two_entities
    Given a map of string entities mapped to WRITE_STATS, this method returns
  the keys for top two entities with highest binlog_bytes_written. Defaults to
  empty string keys in return value if there aren't enough entries in provided
  map.

  @retval pair<first_key, second key>
*/
std::pair<std::string, std::string> get_top_two_entities(
    std::unordered_map<std::string, WRITE_STATS> &dim_stats) {
  std::string first_entity = "";
  std::string second_entity = "";
  ulonglong first_bytes_written = 0;
  ulonglong second_bytes_written = 0;
  for (auto iter = dim_stats.begin(); iter != dim_stats.end(); iter++) {
    if (iter->second.binlog_bytes_written > first_bytes_written) {
      second_bytes_written = first_bytes_written;
      second_entity = first_entity;
      first_bytes_written = iter->second.binlog_bytes_written;
      first_entity = iter->first;
    } else if (iter->second.binlog_bytes_written > second_bytes_written) {
      second_bytes_written = iter->second.binlog_bytes_written;
      second_entity = iter->first;
    }
  }
  return std::make_pair(first_entity, second_entity);
}

/*
  check_lag_and_throttle
    Main method responsible for auto throttling to avoid replication lag.
    It checks if there is lag in the replication topology.
    If yes, it finds the entity that it should throttle. Otherwise, it
  optionally releases one of the previously throttled entities if replication
  lag is below safe threshold.
*/
void check_lag_and_throttle(time_t time_now) {
  ulong lag = get_current_replication_lag();

  if (lag < write_stop_throttle_lag_milliseconds) {
    // Replication lag below safe threshold, reduce throttle rate or release
    // at most one throttled entity. If releasing, erase corresponding
    // throttling rule.
    if (currently_throttled_entities.empty()) return;
    auto throttled_entity = currently_throttled_entities.front();

    enum_wtr_dimension wtr_dim = throttled_entity.second;
    std::string name = throttled_entity.first;

    mysql_mutex_lock(&LOCK_global_write_throttling_rules);
    auto &rules_map = global_write_throttling_rules[wtr_dim];
    auto iter = rules_map.find(name);
    if (iter != rules_map.end()) {
      if (iter->second.mode == WTR_MANUAL) {
        // Safe guard. A manual rule should not end up in.
        // currently_throttled_entities. But if it does, simply pop it out.
        currently_throttled_entities.pop_front();
      } else if (iter->second.throttle_rate > write_throttle_rate_step) {
        iter->second.throttle_rate -= write_throttle_rate_step;
      } else {
        global_write_throttling_rules[wtr_dim].erase(iter);
        currently_throttled_entities.pop_front();
      }
    }
    mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
  }

  if (lag > write_start_throttle_lag_milliseconds) {
    // Replication lag above threshold, Check if we can increase throttle rate
    // for last throttled entity
    if (!currently_throttled_entities.empty()) {
      auto last_throttled_entity = currently_throttled_entities.back();
      bool throttle_rate_increased = false;
      mysql_mutex_lock(&LOCK_global_write_throttling_rules);
      auto &rules_map =
          global_write_throttling_rules[last_throttled_entity.second];
      auto iter = rules_map.find(last_throttled_entity.first);
      if (iter != rules_map.end() && iter->second.throttle_rate < 100) {
        iter->second.throttle_rate = std::min(
            (uint)100, iter->second.throttle_rate + write_throttle_rate_step);
        throttle_rate_increased = true;
      }
      mysql_mutex_unlock(&LOCK_global_write_throttling_rules);
      if (throttle_rate_increased) return;
    }

    // Replication lag above threshold, find an entity to throttle
    mysql_mutex_lock(&LOCK_global_write_statistics);
    if (global_write_statistics_map.size() == 0) {
      // no stats collected so far
      mysql_mutex_unlock(&LOCK_global_write_statistics);
      return;
    }

    // write_stats_frequency may be updated dynamically. Caching it for the
    // logic below
    ulong write_stats_frequency_cached = write_stats_frequency;
    if (write_stats_frequency_cached == 0) {
      return;
    }

    // Find latest write_statistics time bucket that is complete.
    // Example - For write_stats_frequency=6s, At t=8s, this method should
    // return write_stats bucket for stats between t=0 to t=6 i.e. bucket_key=0
    // and not bucket_key=6 which is incomplete and only has 2s worth of
    // write_stats data;
    int time_bucket_key = time_now - (time_now % write_stats_frequency_cached) -
                          write_stats_frequency_cached;
    auto latest_write_stats_iter = global_write_statistics_map.begin();

    // For testing purpose, force use the latest write stats bucket for culprit
    // analysis
    bool dbug_skip_last_complete_bucket_check = false;
    DBUG_EXECUTE_IF("dbug.skip_last_complete_bucket_check",
                    { dbug_skip_last_complete_bucket_check = true; });

    if (!dbug_skip_last_complete_bucket_check) {
      if (latest_write_stats_iter->first != time_bucket_key) {
        // move to the second from front time bucket
        latest_write_stats_iter++;
      }
      if (latest_write_stats_iter == global_write_statistics_map.end() ||
          latest_write_stats_iter->first != time_bucket_key) {
        // no complete write statistics bucket for analysis
        // reset currently monitored entity, if any, as there's been a
        // significant gap in time since we last did culprit analysis. It is
        // outdated.
        currently_monitored_entity.reset();
        mysql_mutex_unlock(&LOCK_global_write_statistics);
        return;
      }
    }
    TIME_BUCKET_STATS &latest_write_stats = latest_write_stats_iter->second;

    std::vector<enum_wtr_dimension> &dimensions =
        write_throttle_permissible_dimensions_in_order;

    bool is_fallback_entity_set = false;
    std::pair<std::string, enum_wtr_dimension> fallback_entity;
    bool is_entity_to_throttle_set = false;
    std::pair<std::string, enum_wtr_dimension> entity_to_throttle;

    for (auto dim_iter = dimensions.begin(); dim_iter != dimensions.end();
         dim_iter++) {
      enum_wtr_dimension dim = *dim_iter;
      auto &dim_stats = latest_write_stats[dim];
      std::pair<std::string, std::string> top_entities =
          get_top_two_entities(dim_stats);

      // Set the fallback entity as the top entity in the highest cardinality
      // dimension This entity is throttled if there is no conclusive entity
      // causing the lag.
      if (!is_fallback_entity_set && !dim_stats.empty()) {
        fallback_entity = std::make_pair(top_entities.first, dim);
        is_fallback_entity_set = true;
      }

      // For testing purpose, skip to throttle fallback entity
      bool dbug_simulate_fallback_sql_throttling = false;
      DBUG_EXECUTE_IF("dbug.simulate_fallback_sql_throttling",
                      { dbug_simulate_fallback_sql_throttling = true; });

      if (dim_stats.empty() || dbug_simulate_fallback_sql_throttling) {
        // move on to the next dimension
        continue;
      } else if (dim_stats.size() == 1) {
        // throttle the first entity
        entity_to_throttle = std::make_pair(top_entities.first, dim);
        is_entity_to_throttle_set = true;
        break;
      } else {
        // compare the top two entities in this dimension
        auto first_bytes_written =
            dim_stats[top_entities.first].binlog_bytes_written;
        auto second_bytes_written =
            dim_stats[top_entities.second].binlog_bytes_written;
        if (first_bytes_written >
            second_bytes_written * write_throttle_min_ratio) {
          // first entity can be throttled
          entity_to_throttle = std::make_pair(top_entities.first, dim);
          is_entity_to_throttle_set = true;
          break;
        }
      }
    }
    mysql_mutex_unlock(&LOCK_global_write_statistics);

    if (is_entity_to_throttle_set) {
      // throttle the culprit entity if set
      update_monitoring_status_for_entity(entity_to_throttle.first,
                                          entity_to_throttle.second);
    } else if (is_fallback_entity_set) {
      // throttle fallback sql in case of no conclusive culprit
      update_monitoring_status_for_entity(fallback_entity.first,
                                          fallback_entity.second);
    }
  } else {
    // reset the currently monitored entity since the replication lag has fallen
    // down
    currently_monitored_entity.reset();
  }
}
