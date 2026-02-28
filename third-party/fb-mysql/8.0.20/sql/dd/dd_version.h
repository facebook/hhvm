/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__DD_VERSION_INCLUDED
#define DD__DD_VERSION_INCLUDED

#include "mysql_version.h"  // MYSQL_VERSION_ID

/**
  @file sql/dd/dd_version.h
  Data dictionary version.
*/

/**
  The version of the current data dictionary table definitions.

  This version number is stored on disk in the data dictionary. Every time
  the data dictionary schema structure changes, this version number must
  change. The table that stores the data dictionary version number is never
  allowed to change.

  The data dictionary version number is the MySQL server version number
  of the first MySQL server version that published a given database schema.
  The format is Mmmdd with M=Major, m=minor, d=dot, so that MySQL 8.0.4 is
  encoded as 80004. This is the same version numbering scheme as the
  information schema and performance schema are using.

  When a data dictionary version is made public, the next change to a
  dictionary table will be associated with the next available MySQL server
  version number. So if DD version 80004 is made available in MySQL 8.0.4,
  and 8.0.5 is an MRU with no changes to the DD tables, then the DD version
  will stay 80004 also in MySQL 8.0.5. If MySQL 9.0.4 is the first GA of
  9.0, and if there are no changes to the DD tables compared to 8.0.4, then
  the DD version number will stay 80004 also in MySQL 9.0.4. Then, if there
  are changes to the DD tables after MySQL 9.0.4, then the new DD version will
  be 90005. In day to day builds internally, changes to the DD tables may be
  done incrementally, so there may be different builds having the same DD
  version number, yet with different DD table definitions.

  Historical version number published in the data dictionary:


  1: Published in 8.0.3-RC.
  ----------------------------------------------------------------------------
  Introduced in MySQL 8.0.0 by WL#6378. Never published in a GA version.
  Last changes were:

  - WL#6049: Removed foreign_keys.unique_constraint_id and the corresponding
    FK constraint, added foreign_keys.unique_constraint_name.

  - Bug#2620373: Added index_stats.cached_time and table_stats.cached_time.


  80004: Published in 8.0.4-RC.
  ----------------------------------------------------------------------------
  Changes from version 1:

  - WL#9059: Added collation clause for spatial_reference_systems.organization

  - WL#9553: Added new 'options' column to the following DD tables:
    catalogs, character_sets, collations, column_statistics, events,
    foreign_keys, resource_groups, routines, schemata,
    st_spatial_reference_systems, triggers.

    (Other relevant DD tables have this column already: columns,
     indexes, parameters, tables, tablespaces).

    Also added explicit indexes for foreign keys instead of relying
    on these to be created implicitly for the following tables/columns:
    character_sets.default_collation_id,  collations.character_set_id,
    columns.collation_id, columns.srs_id, events.client_collation_id,
    events.connection_collation_id, events.schema_collation_id,
    foreign_key_column_usage.column_id, index_column_usage.column_id,
    index_partitions.index_id, index_partitions.tablespace_id,
    indexes.tablespace_id, parameters.collation_id,
    routines.result_collation_id, routines.client_collation_id,
    routines.connection_collation_id, routines.schema_collation_id,
    schemata.default.collation_id, table_partitions.tablespace_id,
    tables.collation_id, tables.tablespace_id, triggers.client_collation_id,
    triggers.connection_collation_id, triggers.schema_collation_id,


  80011: Published in 8.0 GA.
  ----------------------------------------------------------------------------
  Changes from version 80004:

  - WL#8383 and WL#9465: Removed obsolete SQL modes from enums in 'events',
    'routines' and 'triggers'.

  - WL#10774 removed NO_AUTO_CREATE_USER as a valid sql mode value.
    As a result events, routines and triggers table are updated.

  - Bug#27499518 changed the data type for the column 'hidden' in table
    'columns' from BOOL to ENUM('Visible', 'SE', 'SQL').

  - Bug#11754608 "MYSQL DOESN'T SHOW WHAT COLLATION WAS USED IF THAT
    COLLATION IS THE DEFAU"
    Added a new column 'is_explicit_collation' to the 'columns' DD table.

  - BUG#27309116: Add a new column `external_language` to `mysql`.`routines`
    and update `information_schema`.`routines` to reflect this column.

  - Bug#27690593: CHANGE TYPE OF MYSQL.DD_PROPERTIES.PROPERTIES.
    Changed type of 'dd_properties.properties' from MEDIUMTEXT to
    MEDIUMBLOB.


  80012: Published in 8.0.12
  ----------------------------------------------------------------------------
  Changes from version 80011:

  - Bug#27745526: Various adjustments to make the DD table definitions
    in sync with WL#6379.


  80013: Published in 8.0.13
  ----------------------------------------------------------------------------
  Changes from version 80012:

  - Bug#24741307: add last_checked_for_upgrade column to msyql.tables table


  80014: Published in 8.0.14
  ----------------------------------------------------------------------------
  Changes from version 80013:

  - Bug#28492272: Synchronize sql_mode in server with that in DD.


  80015: Not published. DD version still at 80014 in server 8.0.15.
  ----------------------------------------------------------------------------
  No changes from version 80014.


  80016: Published in 8.0.16
  ----------------------------------------------------------------------------
  Changes from version 80014:

  - WL#929 - CHECK CONSTRAINTS
      New DD table check_constraints is introduced for the check
      constraints metadata.

  - WL#12261 adds new mysql.schemata.default_encryption DD column.

  - Bug#29053560 Increases DD column mysql.tablespaces.name length to 268.

  80017: Current
  ----------------------------------------------------------------------------
  Changes from version 80016:

  - WL#12731 adds new mysql.schemata.se_private_data DD column.
  - WL#12571 Support fully qualified hostnames longer than 60 characters
    Server metadata table columns size is increased to 255.

  80018: Next DD version number after the previous is public.
  ----------------------------------------------------------------------------
  Changes from version 80017:
  - No changes, this version number is not active yet.


  If a new DD version is published in a MRU, that version may or may not
  be possible to downgrade to previous MRUs within the same GA. If
  downgrade is supported, the constant DD_VERSION_MINOR_DOWNGRADE_THRESHOLD
  should be set to the lowest DD_VERSION that we may downgrade to. If minor
  downgrade is not supported at all, DD_VERSION_MINOR_DOWNGRADE_THRESHOLD
  should be set to DD_VERSION.
*/
namespace dd {

static const uint DD_VERSION = 80017;
static_assert(DD_VERSION <= MYSQL_VERSION_ID,
              "This release can not use a version number from the future");

static const uint DD_VERSION_MINOR_DOWNGRADE_THRESHOLD = 80017;
static_assert(DD_VERSION_MINOR_DOWNGRADE_THRESHOLD <= MYSQL_VERSION_ID,
              "This release can not use a version number from the future");

}  // namespace dd

#endif /* DD__DD_VERSION_INCLUDED */
