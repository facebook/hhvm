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

#ifndef SQL_INFO_H
#define SQL_INFO_H

#include <string>
#include <vector>

#include "sql/sql_error.h"
#include "storage/perfschema/table_sql_findings.h"

/*
  Possible values used for gap_lock_raise_error session variable
  - OFF: silent
  - WARNING: raise a warning
  - ERROR: raise an error
*/
enum enum_gap_lock_raise_values {
  GAP_LOCK_RAISE_OFF = 0,
  GAP_LOCK_RAISE_WARNING = 1,
  GAP_LOCK_RAISE_ERROR = 2,
  /* Add new control before the following line */
  GAP_LOCK_RAISE_INVALID
};

/*
  Possible values used for variables to control collection of MySQL stats
  - sql_findings_control,
  - sql_plans_control,
  - column_stats_control
  Values
  - OFF_HARD: stop the collection and all data in the corresponding
              in-memory structures is evicted
  - OFF_SOFT: stop collecting the stats but keep the data collected so far
  - ON:       (re-)start the collection

  Keep the enum in the sync with sql_info_control_values[] (sys_vars.cc)
*/
enum enum_sql_info_control {
  SQL_INFO_CONTROL_OFF_HARD = 0,
  SQL_INFO_CONTROL_OFF_SOFT = 1,
  SQL_INFO_CONTROL_ON = 2,
  /* Add new control before the following line */
  SQL_INFO_CONTROL_INVALID
};

/***********************************************************************
              Begin - Functions to support SQL findings
************************************************************************/

/* SQL Finding - stores information about one SQL finding */
typedef struct st_sql_finding {
  uint code;                                /* error code */
  Sql_condition::enum_severity_level level; /* warning level */
  std::string message;                      /* message */
  std::string query_text;                   /* query text */
  ulonglong count;         /* number of times the finding was recorded */
  ulonglong last_recorded; /* last recorded, seconds since epoch */
} SQL_FINDING;

/* SQL_FINDING_VEC - stores all the findings for a SQL statement.
   The lookup key is the code/error number
 */
typedef std::vector<SQL_FINDING> SQL_FINDING_VEC;

void free_global_sql_findings(bool limits_updated);
void store_sql_findings(THD *thd, char *query_text);
std::vector<sql_findings_row> get_all_sql_findings();

/* initializes sql info related variables/structures at instance start */
void init_sql_info();

/***********************************************************************
                End - Functions to support SQL findings
************************************************************************/

/***********************************************************************
 Begin - Functions to support capping the number of duplicate executions
************************************************************************/

void free_global_active_sql(void);
bool register_active_sql(THD *thd, const char *query_text, size_t query_length);
void remove_active_sql(THD *thd);

/*********************************************************************
 End - Functions to support capping the number of duplicate executions
**********************************************************************/

/* Stores the client attribute names */
void store_client_attribute_names(char *new_value);

#endif
