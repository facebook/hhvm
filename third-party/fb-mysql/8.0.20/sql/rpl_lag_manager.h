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

#ifndef RPL_LAG_MANAGER_H
#define RPL_LAG_MANAGER_H

#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "storage/perfschema/table_write_statistics.h"
#include "storage/perfschema/table_write_throttling_log.h"
#include "storage/perfschema/table_write_throttling_rules.h"

class THD;

const uint WRITE_STATISTICS_DIMENSION_COUNT = 4;
const uint WRITE_THROTTLING_MODE_COUNT = 2;

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WRITE_STATISTICS
***********************************************************************/

void store_write_statistics(THD *thd);
void free_global_write_statistics();
std::vector<write_statistics_row> get_all_write_statistics();

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WRITE_THROTTLING_RULES
***********************************************************************/

/* write_control_level:
 * Global variable to control write throttling for short running queries and
 * abort for long running queries.
 */
/* values of write_control_level / sql_duplicate_executions_control
 * CONTROL_LEVEL_OFF: write abort is disabled
 * CONTROL_LEVEL_NOTE: write abort warning is raised as a note
 * CONTROL_LEVEL_WARN: write abort warning is raised
 * CONTROL_LEVEL_ERROR: error is raised and query is aborted
 */
enum enum_control_level {
  CONTROL_LEVEL_OFF = 0,
  CONTROL_LEVEL_NOTE = 1,
  CONTROL_LEVEL_WARN = 2,
  CONTROL_LEVEL_ERROR = 3,
  /* add new control before the following line */
  CONTROL_LEVEL_INVALID
};

/*
  Map integer representation of write throttling rules mode to string
  constants.
*/
const std::string WRITE_THROTTLING_MODE_STRING[] = {"MANUAL", "AUTO"};

/*
** enum_wtr_mode
**
** Different modes for write_throttling_rules
*/
enum enum_wtr_mode {
  WTR_MANUAL = 0,
  WTR_AUTO = 1,
};

/*
** enum_wtr_dimension
**
** Different dimensions(shard, user, client id, sql_id) for write statistics
** throttling rules
*/
enum enum_wtr_dimension {
  WTR_DIM_UNKNOWN = -1,
  WTR_DIM_USER = 0,
  WTR_DIM_CLIENT = 1,
  WTR_DIM_SHARD = 2,
  WTR_DIM_SQL_ID = 3,
};

/* WRITE_THROTTLING_RULE - Stores metadata for a throttling rule for write
 * workload */
struct WRITE_THROTTLING_RULE {
  time_t create_time; /* creation time */
  enum_wtr_mode mode; /* Auto or manual */
  uint throttle_rate; /* Rate between [0, 100] with which this entity is
                        throttled */
};

typedef std::array<std::unordered_map<std::string, WRITE_THROTTLING_RULE>,
                   WRITE_STATISTICS_DIMENSION_COUNT>
    GLOBAL_WRITE_THROTTLING_RULES_MAP;

extern GLOBAL_WRITE_THROTTLING_RULES_MAP global_write_throttling_rules;
/* Queue to store all the entities being currently auto throttled. It is used to
release entities in order they were throttled when replication lag goes below
safe threshold  */
extern std::list<std::pair<std::string, enum_wtr_dimension>>
    currently_throttled_entities;

void free_global_write_throttling_rules();
void free_global_write_auto_throttling_rules();
bool store_write_throttling_rules();
std::vector<write_throttling_rules_row> get_all_write_throttling_rules();
bool store_write_throttle_permissible_dimensions_in_order(char *new_value);

/***********************************************************************
OBJECTS & METHODS TO SUPPORT WRITE_THROTTLING_LOG
***********************************************************************/

// Stores metadata for an event when a write query was throttled
struct WRITE_THROTTLING_LOG {
  time_t last_time; /* last time this rule was used to throttle a query */
  ulonglong count;  /* Number of times this rule has been used to throttle */
};

void store_write_throttling_log(THD *thd, int type, std::string value,
                                WRITE_THROTTLING_RULE &rule);

void store_long_qry_abort_log(THD *thd);

std::vector<write_throttling_log_row> get_all_write_throttling_log();

/***********************************************************************
OBJECTS & METHODS TO SUPPORT AUTO_THROTTLING OF WRITE QUERIES
***********************************************************************/

// Stores metadata for the currently monitored entity for replication lag
struct WRITE_MONITORED_ENTITY {
  std::string name;
  enum_wtr_dimension dimension;
  uint hits;

  void reset() {
    name = "";
    dimension = WTR_DIM_UNKNOWN;
    hits = 0;
  }

  WRITE_MONITORED_ENTITY() { reset(); }
};

extern WRITE_MONITORED_ENTITY currently_monitored_entity;
extern std::atomic<time_t> last_replication_lag_check_time;

void check_lag_and_throttle(time_t time_now);

#endif
