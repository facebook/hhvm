/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_DD_METADATA_H
#define SQL_DD_METADATA_H

#include <mysql/plugin.h>  // st_plugin_int

#include "mysql_version.h"       // MYSQL_VERSION_ID
#include "sql/dd/string_type.h"  // dd::String_type

class THD;
struct st_plugin_int;

namespace dd {
namespace info_schema {

/**
  The version of the current information_schema system views.

  This version number is stored on disk in the data dictionary.
  Every time the information_schema structure changes,
  this version number must change.

  The numbering to use is the MySQL version number
  of the first MySQL version that published a given database schema.
  The format is Mmmdd with M=Major, m=minor, d=dot,
  so that MySQL 8.0.4 is encoded as 80004.

  Historical I_S version number published:

  1: Published in 8.0.3-RC.
  ------------------------
  Introduced in MySQL 8.0.0 by WL#6599. Never published in a GA version.

  80011: Published in 8.0 GA.
  ------------------------------------
  Changes from version 1:

  - Bug#27309116: Add a new column `external_language` to `mysql`.`routines`
    and update `information_schema`.`routines` to reflect this column.

  - Bug#27593348: INFORMATION_SCHEMA.STATISTICS FIELD TYPE CHANGE.
    Changes the column I_S.STATISTICS.NON_UNIQUE type from VARCHAR
    to INT.

  80012: Published in 8.0.12
  ------------------------------------
  Changes from version 80011:

  - Bug#27945704 UNABLE TO JOIN TABLE_CONSTRAINTS AND REFERENTIAL_CONSTRAINTS
    Changes the collation of I_S columns that project index name and
    constraint name to use utf8_tolower_ci.

  - WL#11864 Implement I_S.VIEW_TABLE_USAGE and I_S.VIEW_ROUTINE_USAGE

  - WL#1075 adds one column to INFORMATION_SCHEMA.STATISTICS: "EXPRESSION".
    This column prints out the expression for functional key parts, or SQL NULL
    if it is a regular key part. For functional key parts, COLUMN_NAME is set to
    SQL NULL.

  80013: Published in 8.0.13
  ------------------------------------
  Changes from version 80012

  - WL#11000 ST_Distance with units
    Adds a new view `information_schema`.`st_units_of_measure` with columns
    `UNIT_NAME`, `CONVERSION_FACTOR`, `DESCRIPTION`, and `UNIT_TYPE`. This view
    contains the supported spatial units.

  80014: Published in 8.0.14
  ------------------------------------
  There are no changes from version 80013. Hence server version 80014 used
  I_S version 80013.

  80015: Not published.
  ----------------------------------------------------------------------------
  There are no changes from version 80014. Hence server version 80015 used
  I_S version 80013.

  80016: Published in 8.0.16
  ------------------------------------
  Changes from version 80015.

  - WL#929 - CHECK CONSTRAINTS
    New INFORMATION_SCHEMA table CHECK_CONSTRAINTS is introduced and
    INFORMATION_SCHEMA.TABLE_CONSTRAINTS is modified to include check
    constraints defined on the table.

  - WL#12261 Control (enforce and disable) table encryption
    - Add new column information_schema.schemata.default_encryption
    - information_schema.tables.options UDF definition is changed to pass
      schema default encryption.

  80017: Published in 8.0.17
  ------------------------------------
  Changes from version 80016:

  - WL#12984 INFORMATION_SCHEMA and metadata related to secondary engine.
    Changes system view definitions of
    INFORMATION_SCHEMA.TABLES.CREATE_OPTIONS and
  INFORMATION_SCHEMA.COLUMNS.EXTRA.

  - Bug#29406053: OPTIMIZER_SWITCH DERIVED_MERGE=OFF CAUSES TABLE COMMENTS
                  "... IS NOT BASE TABLE"
    Modifies the INFORMATION_SCHEMA.TABLES dynamic column definitions to
    return NULL, if it finds a view.

  80018: Published in 8.0.18
  ------------------------------------
  Changes from version 80017:

  - Bug#28278220: wrong column type , view , binary
    Changes type of following I_S table column's
      KEY_COLUMN_USAGE:    CONSTRAINT_NAME, POSITION_IN_UNIQUE_CONSTRAINT,
                           REFERENCED_TABLE_SCHEMA, FIELD_REFERENCED_TABLE_NAME,
                           REFERENCED_COLUMN_NAME

      TABLE_CONSTRAINTS:   CONSTRAINT_NAME.
    Column metadata of views on these system views or tables created using
    CREATE TABLE SELECT from these system views will *not* be similar to one
    created with previous version of system views.

  - Bug#29870919: INFORMATION SCHEMA STATS EXPIRY RESULTS IN BAD
                  STATS FOR PARTITIONED TABLES
    This bug changes definition of I_S.STATISTICS.

  80019: Not published (see below)
  ------------------------------------
  Changes from version 80018:

  - WL#10895 INFORMATION_SCHEMA views for Roles.
    Adds new system view definitions for roles.
       INFORMATION_SCHEMA.APPLICABLE_ROLES;
       INFORMATION_SCHEMA.ADMINISTRABLE_ROLE_AUTHORIZATIONS;
       INFORMATION_SCHEMA.ENABLED_ROLES;
       INFORMATION_SCHEMA.ROLE_TABLE_GRANTS;
       INFORMATION_SCHEMA.ROLE_COLUMN_GRANTS;
       INFORMATION_SCHEMA.ROLE_ROUTINE_GRANTS;

  80020: Published by mistake in server version 8.0.19. To correct this,
  we set the IS version number to 800201 in mysql server version 8.0.20.
  Then, in server version 8.0.21, we're back on track with IS_version 80021.
  ------------------------------------
  Changes from version 80018:

  - Bug#29871530: MYSQL 8.0 INFORMATION_SCHEMA.EVENTS NOT
                  OBSERVING CUSTOM TIMEZONE
    This bug updates LAST_EXECUTED to include time zones in
    I_S.EVENTS.

  800201: Current
  ------------------------------------
  Changes from version 80020:

  - Bug#30263373: INCORRECT OUTPUT FROM TABLE_FUNCTION_JSON::PRINT()
    This bug updates the character set of columns returned from JSON_TABLE
    expressions in INFORMATION_SCHEMA views.

  80021: Next IS version number after the previous is public.
  ------------------------------------
*/

static const uint IS_DD_VERSION = 800201;
static_assert((IS_DD_VERSION <= MYSQL_VERSION_ID) ||
                  ((IS_DD_VERSION == 800201) && (MYSQL_VERSION_ID >= 80020)),
              "This release can not use a version number from the future");

/**
  Initialize INFORMATION_SCHEMA system views.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool initialize(THD *thd);

/**
  Initialize non DD based INFORMATION_SCHEMA system views.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool init_non_dd_based_system_view(THD *thd);

/**
  Create INFORMATION_SCHEMA system views.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool create_system_views(THD *thd);

/**
  Store the server I_S table metadata into dictionary, once during MySQL
  server bootstrap.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool store_server_I_S_metadata(THD *thd);

/**
  Store I_S table metadata into dictionary, during MySQL server startup.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool update_I_S_metadata(THD *thd);

/**
  Store dynamic I_S plugin table metadata into dictionary, during INSTALL
  command execution.

  @param thd         Thread context.
  @param plugin_int  I_S Plugin of which the metadata is to be stored.

  @return       Upon failure, return true, otherwise false.
*/
bool store_dynamic_plugin_I_S_metadata(THD *thd, st_plugin_int *plugin_int);

/**
  Remove I_S view metadata from dictionary. This is used
  UNINSTALL and server restart procedure when I_S version is changed.

  @param thd         Thread context.
  @param view_name   I_S view name of which the metadata is to be stored.

  @return       Upon failure, return true, otherwise false.
*/
bool remove_I_S_view_metadata(THD *thd, const dd::String_type &view_name);

/**
  Get create view definition for the given I_S system view.

  @param schema_name Schema name.
  @param view_name   I_S view name.
  @param definition  [out] The CREATE VIEW command to create sytem view.
                           A pointer to a preallocated string should be
                           supplied.

  @return       Upon failure, return true, otherwise false.
*/
bool get_I_S_view_definition(const dd::String_type &schema_name,
                             const dd::String_type &view_name,
                             dd::String_type *definition);

}  // namespace info_schema
}  // namespace dd

#endif  // SQL_DD_METADATA_H
