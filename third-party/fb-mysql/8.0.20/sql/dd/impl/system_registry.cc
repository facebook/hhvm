/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_registry.h"

#include "sql/dd/impl/system_views/administrable_role_authorizations.h"
#include "sql/dd/impl/system_views/applicable_roles.h"   // Applicable_roles
#include "sql/dd/impl/system_views/character_sets.h"     // Character_sets
#include "sql/dd/impl/system_views/check_constraints.h"  // Check_constraints
#include "sql/dd/impl/system_views/collation_charset_applicability.h"  // Collati...
#include "sql/dd/impl/system_views/collations.h"           // Collations
#include "sql/dd/impl/system_views/column_statistics.h"    // Column_statistics
#include "sql/dd/impl/system_views/columns.h"              // Columns
#include "sql/dd/impl/system_views/enabled_roles.h"        // Enabled_roles
#include "sql/dd/impl/system_views/events.h"               // Events
#include "sql/dd/impl/system_views/files.h"                // Files
#include "sql/dd/impl/system_views/innodb_datafiles.h"     // Innodb_datafiles
#include "sql/dd/impl/system_views/innodb_fields.h"        // Innodb_fields
#include "sql/dd/impl/system_views/innodb_foreign.h"       // Innodb_foreign
#include "sql/dd/impl/system_views/innodb_foreign_cols.h"  // Innodb_foreign_cols
#include "sql/dd/impl/system_views/innodb_tablespaces_brief.h"  // Innodb_tablespace_brief
#include "sql/dd/impl/system_views/key_column_usage.h"  // key_column_usage
#include "sql/dd/impl/system_views/keywords.h"          // keywords
#include "sql/dd/impl/system_views/parameters.h"        // Parameters
#include "sql/dd/impl/system_views/partitions.h"        // Partitions
#include "sql/dd/impl/system_views/referential_constraints.h"  // Referential_con...
#include "sql/dd/impl/system_views/resource_groups.h"      // Resource_groups
#include "sql/dd/impl/system_views/role_column_grants.h"   // Role_column_grant
#include "sql/dd/impl/system_views/role_routine_grants.h"  // Role_routine_gran
#include "sql/dd/impl/system_views/role_table_grants.h"    // Role_table_grants
#include "sql/dd/impl/system_views/routines.h"             // Routines
#include "sql/dd/impl/system_views/schemata.h"             // Schemata
#include "sql/dd/impl/system_views/schemata_ext.h"         // Schemata_ext
#include "sql/dd/impl/system_views/st_geometry_columns.h"  // st_geometry_columns
#include "sql/dd/impl/system_views/st_spatial_reference_systems.h"  // St_spatial...
#include "sql/dd/impl/system_views/st_units_of_measure.h"  // St_units_of_measure
#include "sql/dd/impl/system_views/statistics.h"           // Statistics
#include "sql/dd/impl/system_views/table_constraints.h"    // Table_constraints
#include "sql/dd/impl/system_views/tables.h"               // Tables
#include "sql/dd/impl/system_views/triggers.h"             // Triggers
#include "sql/dd/impl/system_views/view_routine_usage.h"   // View_routine_usage
#include "sql/dd/impl/system_views/view_table_usage.h"     // View_table_usage
#include "sql/dd/impl/system_views/views.h"                // Views

#include "sql/dd/impl/tables/catalogs.h"              // Catalog
#include "sql/dd/impl/tables/character_sets.h"        // Character_sets
#include "sql/dd/impl/tables/check_constraints.h"     // Check_constraints
#include "sql/dd/impl/tables/collations.h"            // Collations
#include "sql/dd/impl/tables/column_statistics.h"     // Column_statistics
#include "sql/dd/impl/tables/column_type_elements.h"  // Column_type_elements
#include "sql/dd/impl/tables/columns.h"               // Columns
#include "sql/dd/impl/tables/dd_properties.h"         // DD_properties
#include "sql/dd/impl/tables/events.h"                // Events
#include "sql/dd/impl/tables/foreign_key_column_usage.h"  // Foreign_key_column_usage
#include "sql/dd/impl/tables/foreign_keys.h"              // Foreign_keys
#include "sql/dd/impl/tables/index_column_usage.h"        // Index_column_usage
#include "sql/dd/impl/tables/index_partitions.h"          // Index_partitions
#include "sql/dd/impl/tables/index_stats.h"               // Index_stats
#include "sql/dd/impl/tables/indexes.h"                   // Indexes
#include "sql/dd/impl/tables/parameter_type_elements.h"  // Parameter_type_elements
#include "sql/dd/impl/tables/parameters.h"               // Parameters
#include "sql/dd/impl/tables/resource_groups.h"          // Resource_groups
#include "sql/dd/impl/tables/routines.h"                 // Routines
#include "sql/dd/impl/tables/schemata.h"                 // Schemata
#include "sql/dd/impl/tables/spatial_reference_systems.h"  // Spatial_reference_systems
#include "sql/dd/impl/tables/table_partition_values.h"  // Table_partition_values
#include "sql/dd/impl/tables/table_partitions.h"        // Table_partitions
#include "sql/dd/impl/tables/table_stats.h"             // Table_stats
#include "sql/dd/impl/tables/tables.h"                  // Tables
#include "sql/dd/impl/tables/tablespace_files.h"        // Tablespace_files
#include "sql/dd/impl/tables/tablespaces.h"             // Tablespaces
#include "sql/dd/impl/tables/triggers.h"                // Triggers
#include "sql/dd/impl/tables/view_routine_usage.h"      // View_routine_usage
#include "sql/dd/impl/tables/view_table_usage.h"        // View_table_usage
#include "sql/table.h"                                  // MYSQL_SYSTEM_SCHEMA

using namespace dd::tables;

///////////////////////////////////////////////////////////////////////////

namespace {
template <typename X>
void register_table(dd::System_tables::Types type) {
  dd::System_tables::instance()->add(
      MYSQL_SCHEMA_NAME.str, X::instance().name(), type, &X::instance());
}
void register_table(const dd::String_type table,
                    dd::System_tables::Types type) {
  dd::System_tables::instance()->add(MYSQL_SCHEMA_NAME.str, table, type,
                                     nullptr);
}

template <typename X>
void register_view(dd::System_views::Types type) {
  DBUG_EXECUTE_IF("test_i_s_metadata_version", {
    if (X::view_name() == "EVENTS") return;
  });

  dd::System_views::instance()->add(INFORMATION_SCHEMA_NAME.str,
                                    X::instance().name(), type, &X::instance());
}
}  // namespace

namespace dd {

System_tables *System_tables::instance() {
  static System_tables s_instance;
  return &s_instance;
}

System_views *System_views::instance() {
  static System_views s_instance;
  return &s_instance;
}

System_tablespaces *System_tablespaces::instance() {
  static System_tablespaces s_instance;
  return &s_instance;
}

/*
  Initialize the System_tables registry with the INERT DD
  tables.
*/
void System_tables::add_inert_dd_tables() {
  // Se header file for explanation of table categories.
  dd::System_tables::Types inert = dd::System_tables::Types::INERT;
  register_table<DD_properties>(inert);
}

/*
  Initialize the System_tables registry with the non-INERT DD
  tables. All actual tables should have an entry in the
  System_tables registry, except in the case of a minor
  downgrade. Thus, the System_tables registry should contain
  all tables from all supported versions. Tables that are
  abandoned should be marked as such, see below.
*/
void System_tables::add_remaining_dd_tables() {
  // Se header file for explanation of table categories.
  dd::System_tables::Types core = dd::System_tables::Types::CORE;
  dd::System_tables::Types second = dd::System_tables::Types::SECOND;
  dd::System_tables::Types system = dd::System_tables::Types::SYSTEM;

  register_table<Catalogs>(core);
  register_table<Character_sets>(core);
  register_table<Check_constraints>(core);
  register_table<Collations>(core);
  register_table<dd::tables::Column_statistics>(core);
  register_table<Column_type_elements>(core);
  register_table<Columns>(core);
  register_table<Events>(second);
  register_table<Foreign_key_column_usage>(core);
  register_table<Foreign_keys>(core);
  register_table<Index_column_usage>(core);
  register_table<Index_partitions>(core);
  register_table<Index_stats>(second);
  register_table<Indexes>(core);
  register_table<Parameter_type_elements>(second);
  register_table<Parameters>(second);
  register_table<Resource_groups>(core);
  register_table<Routines>(second);
  register_table<Schemata>(core);
  register_table<Spatial_reference_systems>(second);
  register_table<Table_partition_values>(core);
  register_table<Table_partitions>(core);
  register_table<Table_stats>(second);
  register_table<Tables>(core);
  register_table<Tablespace_files>(core);
  register_table<Tablespaces>(core);
  register_table<Triggers>(core);
  register_table<View_routine_usage>(core);
  register_table<View_table_usage>(core);

  /*
    Mark abandoned tables. E.g., if done as shown below, note that when
    the last version becomes unsupported, we will get a build error here
    that makes sure the abandoned table will not be left unnoticed. When
    that happens, the source code for the table can be removed:

    Collations::instance().set_abandoned(bootstrap::DD_VERSION_80004);
  */
  register_table("backup_history", system);
  register_table("backup_progress", system);
  register_table("columns_priv", system);
  register_table("component", system);
  register_table("db", system);
  register_table("default_roles", system);
  register_table("engine_cost", system);
  register_table("func", system);
  register_table("global_grants", system);
  register_table("gtid_executed", system);
  register_table("help_category", system);
  register_table("help_keyword", system);
  register_table("help_relation", system);
  register_table("help_topic", system);
  register_table("host", system);
  register_table("ndb_binlog_index", system);
  register_table("plugin", system);
  register_table("password_history", system);
  register_table("procs_priv", system);
  register_table("proxies_priv", system);
  register_table("role_edges", system);
  register_table("servers", system);
  register_table("server_cost", system);
  register_table("slave_master_info", system);
  register_table("slave_master_info_backup", system);
  register_table("slave_worker_info", system);
  register_table("slave_relay_log_info", system);
  register_table("tables_priv", system);
  register_table("temp_user", system);
  register_table("tmp_user", system);
  register_table("time_zone", system);
  register_table("time_zone_name", system);
  register_table("time_zone_leap_second", system);
  register_table("time_zone_transition", system);
  register_table("time_zone_transition_type", system);
  register_table("user", system);
  register_table("user_backup", system);
}

void System_views::init() {
  // Register system views with the server.
  dd::System_views::Types is = dd::System_views::Types::INFORMATION_SCHEMA;
  dd::System_views::Types non_dd_based_is =
      dd::System_views::Types::NON_DD_BASED_INFORMATION_SCHEMA;

  register_view<dd::system_views::Enabled_roles>(non_dd_based_is);
  register_view<dd::system_views::Applicable_roles>(non_dd_based_is);
  register_view<dd::system_views::Administrable_role_authorizations>(
      non_dd_based_is);
  register_view<dd::system_views::Character_sets>(is);
  register_view<dd::system_views::Check_constraints>(is);
  register_view<dd::system_views::Collations>(is);
  register_view<dd::system_views::Collation_charset_applicability>(is);
  register_view<dd::system_views::Columns>(is);
  register_view<dd::system_views::Column_statistics>(is);
  register_view<dd::system_views::Events>(is);
  register_view<dd::system_views::Files>(is);
  register_view<dd::system_views::Innodb_datafiles>(is);
  register_view<dd::system_views::Innodb_foreign>(is);
  register_view<dd::system_views::Innodb_foreign_cols>(is);
  register_view<dd::system_views::Innodb_fields>(is);
  register_view<dd::system_views::Innodb_tablespaces_brief>(is);
  register_view<dd::system_views::Key_column_usage>(is);
  register_view<dd::system_views::Keywords>(is);
  register_view<dd::system_views::Parameters>(is);
  register_view<dd::system_views::Partitions>(is);
  register_view<dd::system_views::Referential_constraints>(is);
  register_view<dd::system_views::Resource_groups>(is);
  register_view<dd::system_views::Role_column_grants>(non_dd_based_is);
  register_view<dd::system_views::Role_routine_grants>(non_dd_based_is);
  register_view<dd::system_views::Role_table_grants>(non_dd_based_is);
  register_view<dd::system_views::Routines>(is);
  register_view<dd::system_views::Schemata>(is);
  register_view<dd::system_views::Schemata_ext>(is);
  register_view<dd::system_views::Show_statistics>(is);
  register_view<dd::system_views::St_spatial_reference_systems>(is);
  register_view<dd::system_views::St_units_of_measure>(is);
  register_view<dd::system_views::St_geometry_columns>(is);
  register_view<dd::system_views::Statistics>(is);
  register_view<dd::system_views::Table_constraints>(is);
  register_view<dd::system_views::Tables>(is);
  register_view<dd::system_views::Triggers>(is);
  register_view<dd::system_views::View_routine_usage>(is);
  register_view<dd::system_views::View_table_usage>(is);
  register_view<dd::system_views::Views>(is);
}

}  // namespace dd
