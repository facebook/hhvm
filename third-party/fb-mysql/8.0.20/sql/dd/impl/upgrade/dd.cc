/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/upgrade/dd.h"
#include "sql/dd/impl/bootstrap/bootstrapper.h"
#include "sql/dd/impl/utils.h"

#include <set>

#include "my_dbug.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql_version.h"  // MYSQL_VERSION_ID
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"        // DD_bootstrap_ctx
#include "sql/dd/impl/cache/shared_dictionary_cache.h"  // Shared_dictionary_cache
#include "sql/dd/impl/system_registry.h"                // dd::System_tables
#include "sql/dd/impl/tables/dd_properties.h"  // dd::tables::DD_properties
#include "sql/dd/impl/tables/events.h"         // dd::tables::Events
#include "sql/dd/impl/tables/foreign_key_column_usage.h"  // dd::tables::Fore...
#include "sql/dd/impl/tables/foreign_keys.h"      // dd::tables::Foreign_keys
#include "sql/dd/impl/tables/index_partitions.h"  // dd::tables::Index_partitions
#include "sql/dd/impl/tables/indexes.h"           // dd::tables::Indexes
#include "sql/dd/impl/tables/routines.h"          // dd::tables::Routines
#include "sql/dd/impl/tables/schemata.h"          // dd::tables::Schemata
#include "sql/dd/impl/tables/table_partitions.h"  // dd::tables::Table_partitions
#include "sql/dd/impl/tables/tables.h"            // dd::tables::Tables
#include "sql/dd/impl/tables/tablespaces.h"       // dd::tables::Tablespaces
#include "sql/dd/impl/tables/triggers.h"          // dd::tables::Triggers
#include "sql/dd/object_id.h"
#include "sql/dd/types/object_table.h"             // dd::Object_table
#include "sql/dd/types/object_table_definition.h"  // dd::Object_table_definition
#include "sql/dd/types/schema.h"
#include "sql/sd_notify.h"  // sysd::notify
#include "sql/sql_class.h"  // THD
#include "sql/table.h"      // MYSQL_SCHEMA_NAME

namespace dd {

namespace {
/*
  Create the temporary schemas needed during upgrade, and fetch their ids.
*/
/* purecov: begin inspected */
bool create_temporary_schemas(THD *thd, Object_id *mysql_schema_id,
                              Object_id *target_table_schema_id,
                              String_type *target_table_schema_name,
                              Object_id *actual_table_schema_id) {
  /*
    Find an unused target schema name. Prepare a base name, and append
    a counter, increment until a non-existing name is found
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *schema = nullptr;
  std::stringstream ss;

  DBUG_ASSERT(target_table_schema_name != nullptr);
  *target_table_schema_name = String_type("");
  ss << "dd_upgrade_targets_" << MYSQL_VERSION_ID;
  String_type tmp_schema_name_base{ss.str().c_str()};
  int count = 0;
  do {
    if (thd->dd_client()->acquire(ss.str().c_str(), &schema)) return true;
    if (schema == nullptr) {
      *target_table_schema_name = ss.str().c_str();
      break;
    }
    ss.str("");
    ss.clear();
    ss << tmp_schema_name_base << "_" << count++;
  } while (count < 1000);

  if (target_table_schema_name->empty()) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_SCHEMA_UNAVAILABLE, ss.str().c_str());
    return true;
  }

  /*
    Find an unused schema name where we can temporarily move the actual
    tables to be removed or modified.
  */
  String_type actual_table_schema_name{""};
  ss.str("");
  ss.clear();
  ss << "dd_upgrade_garbage_" << MYSQL_VERSION_ID;
  tmp_schema_name_base = String_type(ss.str().c_str());
  count = 0;
  do {
    if (thd->dd_client()->acquire(ss.str().c_str(), &schema)) return true;
    if (schema == nullptr) {
      actual_table_schema_name = ss.str().c_str();
      break;
    }
    ss.str("");
    ss.clear();
    ss << tmp_schema_name_base << "_" << count++;
  } while (count < 1000);

  if (actual_table_schema_name.empty()) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_SCHEMA_UNAVAILABLE, ss.str().c_str());
    return true;
  }

  /*
    Store the schema names in DD_properties and commit. The schemas will
    now be removed on next restart.
  */
  if (dd::tables::DD_properties::instance().set(thd, "UPGRADE_TARGET_SCHEMA",
                                                *target_table_schema_name) ||
      dd::tables::DD_properties::instance().set(thd, "UPGRADE_ACTUAL_SCHEMA",
                                                actual_table_schema_name)) {
    return dd::end_transaction(thd, true);
  }

  if (dd::end_transaction(thd, false)) return true;

  if (dd::execute_query(
          thd, dd::String_type("CREATE SCHEMA ") + actual_table_schema_name +
                   dd::String_type(" DEFAULT COLLATE '") +
                   dd::String_type(default_charset_info->name) + "'") ||
      dd::execute_query(
          thd, dd::String_type("CREATE SCHEMA ") + *target_table_schema_name +
                   dd::String_type(" DEFAULT COLLATE '") +
                   dd::String_type(default_charset_info->name) + "'") ||
      dd::execute_query(thd,
                        dd::String_type("USE ") + *target_table_schema_name)) {
    return true;
  }

  /*
    Get hold of the schema ids of the temporary target schema, the
    temporary actual schema, and the mysql schema. These are needed
    later in various situations in the upgrade execution.
  */
  if (thd->dd_client()->acquire(MYSQL_SCHEMA_NAME.str, &schema) ||
      schema == nullptr)
    return true;

  DBUG_ASSERT(mysql_schema_id != nullptr);
  *mysql_schema_id = schema->id();
  DBUG_ASSERT(*mysql_schema_id == 1);

  if (thd->dd_client()->acquire(*target_table_schema_name, &schema) ||
      schema == nullptr)
    return true;

  DBUG_ASSERT(target_table_schema_id != nullptr);
  *target_table_schema_id = schema->id();

  if (thd->dd_client()->acquire(actual_table_schema_name, &schema) ||
      schema == nullptr)
    return true;

  DBUG_ASSERT(actual_table_schema_id != nullptr);
  *actual_table_schema_id = schema->id();

  return false;
}
/* purecov: end */

/*
  Establish the sets of names of tables to be created and/or removed.
*/
/* purecov: begin inspected */
void establish_table_name_sets(std::set<String_type> *create_set,
                               std::set<String_type> *remove_set) {
  /*
    Establish the table change sets:
    - The 'remove' set contains the tables that will eventually be removed,
      i.e., they are present in the actual version, and either abandoned
      or replaced by another table definition in the target version.
    - The 'create' set contains the tables that must be created, i.e., they
      are either new tables in the target version, or they replace an
      existing table in the actual version.
  */
  DBUG_ASSERT(create_set != nullptr && create_set->empty());
  DBUG_ASSERT(remove_set != nullptr && remove_set->empty());
  for (System_tables::Const_iterator it = System_tables::instance()->begin();
       it != System_tables::instance()->end(); ++it) {
    if (is_non_inert_dd_or_ddse_table((*it)->property())) {
      /*
        In this context, all tables should have an Object_table. Minor
        downgrade is the only situation where an Object_table may not exist,
        but minor upgrade will never enter this code path.
      */
      DBUG_ASSERT((*it)->entity() != nullptr);

      String_type target_ddl_statement("");
      const Object_table_definition *target_table_def =
          (*it)->entity()->target_table_definition();
      /*
         The target table definition may not be present if the table
         is abandoned.
      */
      if (target_table_def) {
        target_ddl_statement = target_table_def->get_ddl();
      }

      String_type actual_ddl_statement("");
      const Object_table_definition *actual_table_def =
          (*it)->entity()->actual_table_definition();
      /*
        The actual definition may not be present if this is a new table
        which has been added.
      */
      if (actual_table_def) {
        actual_ddl_statement = actual_table_def->get_ddl();
      }

      /*
        Remove and/or create as needed. If the definitions are non-null
        and equal, no change has been done, and hence upgrade of the table
        is irrelevant.
      */
      if (target_table_def == nullptr && actual_table_def != nullptr)
        remove_set->insert((*it)->entity()->name());
      else if (target_table_def != nullptr && actual_table_def == nullptr)
        create_set->insert((*it)->entity()->name());
      else if (target_ddl_statement != actual_ddl_statement) {
        /*
          Abandoned tables that are not present will have target and actual
          statements == "", and will therefore not be added to the create
          nor remove set.
        */
        remove_set->insert((*it)->entity()->name());
        create_set->insert((*it)->entity()->name());
      }
    }
  }
}
/* purecov: end */

/**
  Adjust metadata in source DD tables in mysql schema. This is done by
  mostly executing UPDATE queries on them, but we do not migrate data to
  destination DD tables.

  @param   thd         Thread context.

  @returns false if success. otherwise true.
*/
/* purecov: begin inspected */
bool update_meta_data(THD *thd) {
  /*
    Turn off foreign key checks while migrating the meta data.
  */
  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 0"))
    return dd::end_transaction(thd, true);

  /* Version dependent migration of meta data can be added here. */

  /*
    8.0.11 allowed entries with 0 timestamps to be created. These must
    be updated, otherwise, upgrade will fail since 0 timestamps are not
    allowed with the default SQL mode.
  */
  if (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80012)) {
    if (dd::execute_query(thd,
                          "UPDATE mysql.tables SET last_altered = "
                          "CURRENT_TIMESTAMP WHERE last_altered = 0"))
      return dd::end_transaction(thd, true);
    if (dd::execute_query(thd,
                          "UPDATE mysql.tables SET created = CURRENT_TIMESTAMP "
                          "WHERE created = 0"))
      return dd::end_transaction(thd, true);
  }

  /* Upgrade from 80015. */
  if (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
          bootstrap::DD_VERSION_80016)) {
    // A) REMOVE 'ENCRYPTION' KEY FROM MYSQL.TABLESPACES.OPTIONS FOR
    //    NON-INNODB TABLESPACES/TABLES

    /*
      Remove ENCRYPTION clause for unencrypted non-InnoDB tablespaces.
      Because its only InnoDB that support encryption in 8.0.16.
    */
    static_assert(dd::tables::Tablespaces::NUMBER_OF_FIELDS == 6,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tablespaces ts "
            "SET ts.options=REMOVE_DD_PROPERTY_KEY(ts.options, 'encryption') "
            "WHERE ts.engine!='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') IS NOT "
            "NULL")) {
      return dd::end_transaction(thd, true);
    }

    // Remove ENCRYPTION clause for non-InnoDB tables.
    static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tables tbl "
            "SET tbl.options=REMOVE_DD_PROPERTY_KEY(tbl.options, "
            "'encrypt_type') "
            "WHERE tbl.tablespace_id IS NULL AND tbl.engine!='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(tbl.options,'encrypt_type') IS NOT "
            "NULL")) {
      return dd::end_transaction(thd, true);
    }

    // B) UPDATE MYSQL.TABLESPACES.OPTIONS 'ENCRYPTION' KEY FOR INNODB SE.

    /*
      Add ENCRYPTION clause for InnoDB file-per-table tablespaces used by
      partitioned table.

      For a partitioned table using file-per-table tablespace, the
      tablespace_id is stored in tables corresponding
      mysql.index_partitions.tablespace_id. The following query finds the
      partitioned tablespace by joining several DD tables and updates
      the 'encryption' key same as that of tables encryption type.

      This is done as we expect all innodb tablespaces to have proper
      'encryption' flag set.
    */
    static_assert(dd::tables::Index_partitions::NUMBER_OF_FIELDS == 5,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Table_partitions::NUMBER_OF_FIELDS == 12,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Indexes::NUMBER_OF_FIELDS == 15,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.index_partitions ip "
            "JOIN mysql.tablespaces ts ON ts.id = ip.tablespace_id "
            "JOIN mysql.table_partitions p ON p.id = ip.partition_id "
            "JOIN mysql.tables t ON t.id = p.table_id "
            "JOIN mysql.indexes i ON i.table_id = t.id "
            "SET ts.options=CONCAT(IFNULL(ts.options,''), "
            "IF(LOWER(GET_DD_PROPERTY_KEY_VALUE(t.options,'encrypt_type'))='y' "
            ", 'encryption=Y;','encryption=N;')) "
            "WHERE t.tablespace_id IS NULL AND i.tablespace_id IS NULL AND "
            "p.tablespace_id IS NULL AND ts.engine='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(t.options,'encrypt_type') IS NOT NULL "
            "AND "
            "GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') IS NULL ")) {
      return dd::end_transaction(thd, true);
    }

    /*
      Add ENCRYPTION clause for InnoDB file-per-table tablespaces same as
      encryption type of the table.

      For a tables using file-per-table tablespace, the tablespace_id is
      stored in tables corresponding mysql.indexes.tablespace_id.
      The following query finds the tablespace by joining
      several DD tables and updates the 'encryption' key same as that of
      tables encryption type.

      This is done as we expect all innodb tablespaces to have proper
      'encryption' flag set.
    */
    static_assert(dd::tables::Indexes::NUMBER_OF_FIELDS == 15,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Tablespaces::NUMBER_OF_FIELDS == 6,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.indexes i "
            "JOIN mysql.tablespaces ts ON ts.id = i.tablespace_id "
            "JOIN mysql.tables t ON t.id = i.table_id "
            "SET ts.options=CONCAT(IFNULL(ts.options,''), "
            "IF(LOWER(GET_DD_PROPERTY_KEY_VALUE(t.options,'encrypt_type'))='y'"
            ", 'encryption=Y;','encryption=N;')) "
            "WHERE ts.engine='InnoDB' AND t.tablespace_id IS NULL "
            "AND GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') IS NULL")) {
      return dd::end_transaction(thd, true);
    }

    /*
      Update ENCRYPTION clause for unencrypted InnoDB tablespaces.
      Where the 'encryption' key value is empty string ''.
    */
    static_assert(dd::tables::Tablespaces::NUMBER_OF_FIELDS == 6,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tablespaces ts "
            "SET ts.options=CONCAT(IFNULL(REMOVE_DD_PROPERTY_KEY(ts.options, "
            "'encryption'),''), 'encryption=N;') "
            "WHERE ts.engine='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') = ''")) {
      return dd::end_transaction(thd, true);
    }

    /*
      Store ENCRYPTION clause for unencrypted InnoDB general tablespaces,
      when the 'encryption' key is not yet present.
    */
    static_assert(dd::tables::Tablespaces::NUMBER_OF_FIELDS == 6,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tablespaces ts "
            "SET ts.options=CONCAT(IFNULL(ts.options,''), 'encryption=N;') "
            "WHERE ts.engine='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') IS NULL ")) {
      return dd::end_transaction(thd, true);
    }

    // C) UPDATE MYSQL.TABLES.OPTIONS 'ENCRYPT_TYPE' KEY FOR INNODB TABLES.

    /*
      Update 'encrypt_type' flag for innodb tables using general tablespace.
      It is not possible to have general tablespaces used in partitioned
      table as of 8.0.15, so we ignore to check for partitioned tables
      using general tablespace.
    */
    static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                  "SQL statements rely on a specific table definition");
    static_assert(dd::tables::Tablespaces::NUMBER_OF_FIELDS == 6,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tables t "
            "JOIN mysql.tablespaces ts ON ts.id = t.tablespace_id "
            "SET t.options=CONCAT(IFNULL(t.options,''), "
            "IF(LOWER(GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption'))='y'"
            ", 'encrypt_type=Y;','encrypt_type=N;')) "
            "WHERE t.engine='InnoDB' AND t.tablespace_id IS NOT NULL AND "
            "GET_DD_PROPERTY_KEY_VALUE(ts.options,'encryption') IS NOT NULL "
            "AND GET_DD_PROPERTY_KEY_VALUE(t.options,'encrypt_type') IS "
            "NULL")) {
      return dd::end_transaction(thd, true);
    }

    /*
      Store 'encrypt_type=N' for unencrypted InnoDB file-per-table tables,
      for tables which does not have a 'encrypt_type' key stored already.
    */
    static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                  "SQL statements rely on a specific table definition");
    if (dd::execute_query(
            thd,
            "UPDATE mysql.tables t "
            "SET t.options=CONCAT(IFNULL(t.options,''), 'encrypt_type=N;') "
            "WHERE t.tablespace_id IS NULL AND t.engine='InnoDB' AND "
            "GET_DD_PROPERTY_KEY_VALUE(t.options,'encrypt_type') IS NULL")) {
      return dd::end_transaction(thd, true);
    }
  }

  /*
    Turn foreign key checks back on and commit explicitly.
  */
  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 1"))
    return dd::end_transaction(thd, true);

  return false;
}
/* purecov: end */

/**
  Copy meta data from the actual tables to the target tables.

  The default is to copy all data. This is sufficient if we e.g. add a
  new index in the new DD version. If there are changes to the table
  columns, e.g. if we add or remove a column, we must add code to handle
  each case specifically. Suppose e.g. we add a new column to allow defining
  a default tablespace for each schema, and store the tablespace id
  in that column. Then, we could migrate the meta data for 'schemata' and
  set a default value for all existing schemas:

  ...
  migrated_set.insert("schemata");
  if (dd::execute_query(thd, "INSERT INTO schemata "
         "SELECT id, catalog_id, name, default_collation_id, 1, "
         "       created, last_altered, options FROM mysql.schemata"))
  ...

  The code block above would go into the 'Version dependent migration'
  part of the function below.

  @param   thd         Thread context.
  @param   create_set  Set of new or modified tables to be created.
  @param   remove_set  Set of abandoned or modified tables to be removed.

  @returns false if success. otherwise true.
*/
/* purecov: begin inspected */
bool migrate_meta_data(THD *thd, const std::set<String_type> &create_set,
                       const std::set<String_type> &remove_set) {
  /*
    Turn off foreign key checks while migrating the meta data.
  */
  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 0"))
    return dd::end_transaction(thd, true);

  /*
    Explicitly migrate meta data for each table which has been modified.
    Register the table name in the migrated_set to skip it in the default
    handling below.
  */
  std::set<String_type> migrated_set{};

  /*
    Version dependent migration of meta data can be added here. The migration
    should be grouped by table with a conditional expression for each table,
    branching out to do one single migration step for each table. Note that
    if more than one migration step (i.e., an INSERT into the target table)
    being executed for a table, then each step will overwrite the result of
    the previous one.
  */
  auto migrate_table = [&](const String_type &name, const String_type &stmt) {
    DBUG_ASSERT(create_set.find(name) != create_set.end());
    /* A table must be migrated only once. */
    DBUG_ASSERT(migrated_set.find(name) == migrated_set.end());
    migrated_set.insert(name);
    if (dd::execute_query(thd, stmt)) {
      return dd::end_transaction(thd, true);
    }
    return false;
  };

  auto is_dd_upgrade_from_before = [](uint dd_version) {
    return bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade_from_before(
        dd_version);
  };

  /********************* Migration of mysql.tables *********************/
  /* Upgrade from 80012 or earlier. */
  static_assert(dd::tables::Tables::NUMBER_OF_FIELDS == 35,
                "SQL statements rely on a specific table definition");
  if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80013)) {
    /* Column 'last_checked_for_upgrade' was added. */
    if (migrate_table("tables",
                      "INSERT INTO tables SELECT *, 0 FROM mysql.tables")) {
      return true;
    }
  }

  /********************* Migration of mysql.events *********************/
  /* Upgrade from 80013 or earlier. */
  static_assert(dd::tables::Events::NUMBER_OF_FIELDS == 24,
                "SQL statements rely on a specific table definition");
  if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80014)) {
    /*
      SQL mode 'INVALID_DATES' was renamed to 'ALLOW_INVALID_DATES'.
      Migrate the SQL mode as an integer.
    */
    if (migrate_table(
            "events",
            "INSERT INTO events SELECT  id, schema_id, name, definer, "
            "  time_zone, definition, definition_utf8, execute_at, "
            "  interval_value, interval_field, sql_mode+0, starts, ends, "
            "  status, on_completion, created, last_altered, "
            "  last_executed, comment, originator, client_collation_id, "
            "  connection_collation_id, schema_collation_id, options "
            "  FROM mysql.events")) {
      return true;
    }
  }

  /********************* Migration of mysql.routines *********************/
  /* Upgrade from 80013 or earlier. */
  static_assert(dd::tables::Routines::NUMBER_OF_FIELDS == 29,
                "SQL statements rely on a specific table definition");
  if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80014)) {
    /*
      SQL mode 'INVALID_DATES' was renamed to 'ALLOW_INVALID_DATES'.
      Migrate the SQL mode as an integer.
    */
    if (migrate_table(
            "routines",
            "INSERT INTO routines SELECT id, schema_id, name, type, "
            "  result_data_type, result_data_type_utf8, result_is_zerofill, "
            "  result_is_unsigned, result_char_length, "
            "  result_numeric_precision, result_numeric_scale, "
            "  result_datetime_precision, result_collation_id, "
            "  definition, definition_utf8, parameter_str, is_deterministic, "
            "  sql_data_access, security_type, definer, sql_mode+0, "
            "  client_collation_id, connection_collation_id, "
            "  schema_collation_id, created, last_altered, comment, options, "
            "  external_language FROM mysql.routines")) {
      return true;
    }
  }

  /********************* Migration of mysql.triggers *********************/
  /* Upgrade from 80013 or earlier. */
  static_assert(dd::tables::Triggers::NUMBER_OF_FIELDS == 17,
                "SQL statements rely on a specific table definition");
  if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80014)) {
    /*
      SQL mode 'INVALID_DATES' was renamed to 'ALLOW_INVALID_DATES'.
      Migrate the SQL mode as an integer.
    */
    if (migrate_table(
            "triggers",
            "INSERT INTO triggers SELECT id, schema_id, name, event_type, "
            "  table_id, action_timing, action_order, action_statement, "
            "  action_statement_utf8, created, last_altered, sql_mode+0, "
            "  definer, client_collation_id, connection_collation_id, "
            "  schema_collation_id, options FROM mysql.triggers")) {
      return true;
    }
  }

  /********************* Migration of mysql.schemata *********************/
  /*
    DD version 80016 adds a new column 'default_encryption' and
    DD version 80017 adds a new column 'se_private_data' to the schemata table.
    Handle them both during upgrade.
  */
  static_assert(dd::tables::Schemata::NUMBER_OF_FIELDS == 9,
                "SQL statements rely on a specific table definition");
  if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80016)) {
    /*
      Upgrade from 80014 and before.
      Store 'NO' for new mysql.schemata.default_encryption column and
      store NULL for new mysql.schemata.se_private_data column
    */
    if (migrate_table(
            "schemata",
            "INSERT INTO schemata SELECT *, 'NO', NULL FROM mysql.schemata")) {
      return true;
    }
  } else if (is_dd_upgrade_from_before(bootstrap::DD_VERSION_80017)) {
    /*
      Upgrade from 80016.
      Store NULL for new mysql.schemata.se_private_data column
    */
    if (migrate_table(
            "schemata",
            "INSERT INTO schemata SELECT *, NULL FROM mysql.schemata")) {
      return true;
    }
  }

  /*
    Default handling: Copy all meta data for the tables that have been
    modified (i.e., all tables which are both in the remove- and create set),
    unless they were migrated explicitly above.
  */
  for (std::set<String_type>::const_iterator it = create_set.begin();
       it != create_set.end(); ++it) {
    if (migrated_set.find(*it) == migrated_set.end() &&
        remove_set.find(*it) != remove_set.end()) {
      std::stringstream ss;
      ss << "INSERT INTO " << (*it) << " SELECT * FROM "
         << MYSQL_SCHEMA_NAME.str << "." << (*it);
      if (dd::execute_query(thd, ss.str().c_str()))
        return dd::end_transaction(thd, true);
    }
  }

  /*
    Turn foreign key checks back on and commit explicitly.
  */
  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 1"))
    return dd::end_transaction(thd, true);

  return false;
}
/* purecov: end */

/*
  Adjust the object ids to "move" tables between schemas by using DML.

  At this point, we have a set of old DD tables, in the 'remove_set', that
  should be removed. These are a subset of the actual DD tables. And we
  have a set of new DD tables, in the 'create_set', that should replace the
  old ones. The tables in the 'create_set' are a subset of the target DD
  tables.

  What we want to do is to move the tables in the 'remove_set' out of the
  'mysql' schema and into a different schema with id 'actual_table_schema_id',
  and then move the tables in the 'create_set' (which are in a schema with
  id 'target_table_schema_id' and name 'target_table_schema_name') out of
  that schema and into the 'mysql' schema.

  We could do this by 'RENAME TABLE' statements, but that would not be atomic
  since the statements will be auto committing. So instead, we manipulate the
  DD tables directly, and update the schema ids related to the relevant tables.
  This is possible since the tables are stored in a general tablespace, and
  moving them to a different schema will not affect the DDSE.

  The updates we need to do on the DD tables are the following:

  - For the tables in the 'remove_set' and the 'create_set', we must change
    the schema id of the entry in the 'tables' table according to where we
    want to move the tables.
  - For the tables in the 'remove_set', we delete all foreign keys where the
    table to be removed is a child.
  - For the tables in the 'create_set', we change the schema id and name of
    all foreign keys, where the table to be created is a child, from the
    'target_table_schema_name' to that of the 'mysql' schema.

  See also additional comments in the code below.

  @param  create_set               Set of tables to be created.
  @param  remove_set               Set of tables to be removed.
  @param  mysql_schema_id          Id of the 'mysql' schema.
  @param  target_table_schema_id   Id of the schema where the tables in the
                                   create_set are located.
  @param  target_table_schema_name Name of the schema where the tables in the
                                   create_set are located.
  @param  actual_table_schema_id   Id of the schema where the tables in the
                                   remove_set will be moved.

  @returns false if success, true otherwise.
*/
/* purecov: begin inspected */
bool update_object_ids(THD *thd, const std::set<String_type> &create_set,
                       const std::set<String_type> &remove_set,
                       Object_id mysql_schema_id,
                       Object_id target_table_schema_id,
                       const String_type &target_table_schema_name,
                       Object_id actual_table_schema_id) {
  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 0"))
    return dd::end_transaction(thd, true);

  /*
    If mysql.tables has been modified, do the change on the copy, otherwise
    do the change on mysql.tables
  */
  String_type tables_table = tables::Tables::instance().name();
  if (create_set.find(tables_table) != create_set.end()) {
    tables_table = target_table_schema_name + String_type(".") + tables_table;
  } else {
    tables_table =
        String_type(MYSQL_SCHEMA_NAME.str) + String_type(".") + tables_table;
  }

  /*
    For each actual table to be removed (i.e., modified or abandoned),
    change tables.schema_id to the actual table schema id.
  */
  for (std::set<String_type>::const_iterator it = remove_set.begin();
       it != remove_set.end(); ++it) {
    std::stringstream ss;
    ss << "UPDATE " << tables_table
       << " SET schema_id= " << actual_table_schema_id
       << " WHERE schema_id= " << mysql_schema_id << " AND name LIKE '" << (*it)
       << "'";

    if (dd::execute_query(thd, ss.str().c_str()))
      return dd::end_transaction(thd, true);
  }

  /*
    For each target table to be created (i.e., modified or added),
    change tables.schema_id to the mysql schema id, and set the hidden
    property according to the corresponding Object_table.
  */
  for (std::set<String_type>::const_iterator it = create_set.begin();
       it != create_set.end(); ++it) {
    // Get the corresponding Object_table instance.
    String_type hidden{""};
    if (System_tables::instance()
            ->find_table(MYSQL_SCHEMA_NAME.str, (*it))
            ->is_hidden())
      hidden = String_type(", hidden= 'System'");

    std::stringstream ss;
    ss << "UPDATE " << tables_table << " SET schema_id= " << mysql_schema_id
       << hidden << " WHERE schema_id= " << target_table_schema_id
       << " AND name LIKE '" << (*it) << "'";

    if (dd::execute_query(thd, ss.str().c_str()))
      return dd::end_transaction(thd, true);
  }

  /*
    If mysql.foreign_keys has been modified, do the change on the copy,
    otherwise do the change on mysql.foreign_keys. And likewise, if
    mysql.foreign_key_column_usage has been modified, do the change on
    the copy, otherwise do the change on mysql.foreign_key_column_usage.
  */
  String_type foreign_keys_table = tables::Foreign_keys::instance().name();
  ;
  if (create_set.find(foreign_keys_table) != create_set.end()) {
    foreign_keys_table =
        target_table_schema_name + String_type(".") + foreign_keys_table;
  } else {
    foreign_keys_table = String_type(MYSQL_SCHEMA_NAME.str) + String_type(".") +
                         foreign_keys_table;
  }

  String_type foreign_key_column_usage_table =
      tables::Foreign_key_column_usage::instance().name();
  ;
  if (create_set.find(foreign_key_column_usage_table) != create_set.end()) {
    foreign_key_column_usage_table = target_table_schema_name +
                                     String_type(".") +
                                     foreign_key_column_usage_table;
  } else {
    foreign_key_column_usage_table = String_type(MYSQL_SCHEMA_NAME.str) +
                                     String_type(".") +
                                     foreign_key_column_usage_table;
  }

  /*
    For each actual (i.e., modified or abandoned) table to be removed,
    remove the entries from the foreign_keys and foreign_key_column_usage
    table. There is no point in trying to maintain the foreign keys since
    the tables will be removed eventually anyway.
  */
  for (std::set<String_type>::const_iterator it = remove_set.begin();
       it != remove_set.end(); ++it) {
    std::stringstream ss;
    ss << "DELETE FROM " << foreign_key_column_usage_table
       << " WHERE foreign_key_id IN ("
       << "  SELECT id FROM " << foreign_keys_table
       << "   WHERE table_id= (SELECT id FROM " << tables_table
       << "     WHERE name LIKE '" << (*it) << "' AND "
       << "     schema_id= " << actual_table_schema_id << "))";
    if (dd::execute_query(thd, ss.str().c_str()))

      return dd::end_transaction(thd, true);

    ss.str("");
    ss.clear();
    ss << "DELETE FROM " << foreign_keys_table
       << "   WHERE table_id= (SELECT id FROM " << tables_table
       << "     WHERE name LIKE '" << (*it) << "' AND "
       << "     schema_id= " << actual_table_schema_id << ")";
    if (dd::execute_query(thd, ss.str().c_str()))
      return dd::end_transaction(thd, true);
  }

  /*
    For each target (i.e., modified or added)  table to be moved, change
    foreign_keys.schema_id and foreign_keys.referenced_schema_name to the
    mysql schema id and name. For the created tables, the target schema id
    and name are reflected in the foreign_keys tables, so we don't need a
    subquery based on table names.
  */
  std::stringstream ss;
  ss << "UPDATE " << foreign_keys_table << " SET schema_id= " << mysql_schema_id
     << ", "
     << "     referenced_table_schema= '" << MYSQL_SCHEMA_NAME.str << "'"
     << " WHERE schema_id= " << target_table_schema_id
     << "       AND referenced_table_schema= '" << target_table_schema_name
     << "'";

  if (dd::execute_query(thd, ss.str().c_str()))
    return dd::end_transaction(thd, true);

  if (dd::execute_query(thd, "SET FOREIGN_KEY_CHECKS= 1"))
    return dd::end_transaction(thd, true);

  // Delay commit in the case of success, since we need to do this atomically.
  return false;
}
/* purecov: end */

}  // namespace

namespace upgrade {
// Create the target tables for upgrade and migrate the meta data.
/* purecov: begin inspected */
bool upgrade_tables(THD *thd) {
  if (!bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade()) return false;

  /*
    Create the temporary schemas used for target and actual tables,
    and get hold of their ids.
  */
  Object_id mysql_schema_id = INVALID_OBJECT_ID;
  Object_id target_table_schema_id = INVALID_OBJECT_ID;
  Object_id actual_table_schema_id = INVALID_OBJECT_ID;
  String_type target_table_schema_name;
  if (create_temporary_schemas(thd, &mysql_schema_id, &target_table_schema_id,
                               &target_table_schema_name,
                               &actual_table_schema_id))
    return true;

  /*
    Establish the sets of table names to be removed and/or created.
  */
  std::set<String_type> remove_set = {};
  std::set<String_type> create_set = {};
  establish_table_name_sets(&create_set, &remove_set);

  /*
    Loop over all DD tables, and create the target tables. We may do version
    specific handling, but the default is to create the target table if it is
    different from the actual table (or if there is no corresponding actual
    table). The table creation is done by executing DDL statements that are
    auto committed.
  */
  if (create_tables(thd, &create_set)) return true;

  /*
    Loop over all DD tables and migrate the meta data. We may do version
    specific handling, but the default is to just copy all meta data from
    the actual to the target table, assuming the number and type of columns
    are the same (e.g. if an index is added). The data migration is committed.

    We achieve data migration in two steps:

    1) update_meta_data() is used to adjust metadata in source DD tables in
    mysql schema. This is done by mostly executing UPDATE queries on
    them, but we do not migrate data to destination DD tables.

    2) migrate_meta_data() is used to adjust metadata in destination DD
    tables using UPDATE command and also migrate data to destination DD
    tables using INSERT command.

    Note that the changes done during migration of meta data are committed
    in next step at the end of 'atomic switch' described below.
  */
  if (update_meta_data(thd) || migrate_meta_data(thd, create_set, remove_set))
    return true;

  /*
    We are now ready to do the atomic switch of the actual and target DD
    tables. Thus, the next three steps must be done without intermediate
    commits. Note that in case of failure, rollback is done immediately.
    In case of success, no commit is done until at the very end of
    update_versions(). The switch is done as follows:

    - First, update the DD properties. Note that we must acquire the
      modified DD tables from the temporary target schema. This is done
      before the object ids are modified, because that step also may mess
      up object acquisition (if we change the schema id of a newly created
      table to that of the 'mysql' schema, and then try acquire(), we will
      get the table from the core registry in the storage adapter, and that
      is not what we want).

    - Then, update the object ids and schema names to simulate altering the
      schema of the modified tables. The changes are done on the 'tables',
      'foreign_keys' and 'foreign_key_column_usage' tables. If these tables
      are modified, the changes must be done on the corresponding new table
      in the target schema. If not, the change must be done on the actual
      table in the 'mysql' schema.

    - Finally, update the version numbers and commit. In update_versions(),
      the atomic switch will either be committed.
  */
  if (update_properties(thd, &create_set, &remove_set,
                        target_table_schema_name) ||
      update_object_ids(thd, create_set, remove_set, mysql_schema_id,
                        target_table_schema_id, target_table_schema_name,
                        actual_table_schema_id) ||
      update_versions(thd, false))
    return true;

  LogErr(SYSTEM_LEVEL, ER_DD_UPGRADE_COMPLETED,
         bootstrap::DD_bootstrap_ctx::instance().get_actual_dd_version(),
         dd::DD_VERSION);
  log_sink_buffer_check_timeout();
  sysd::notify("STATUS=Data Dictionary upgrade complete\n");

  /*
    At this point, the DD upgrade is committed. Below, we will reset the
    DD cache and re-initialize based on 'mysql.dd_properties', hence,
    we will lose track of the fact that we have done a DD upgrade as part
    of this restart. Thus, we record this fact in the bootstrap context
    so we can check it e.g. when initializeing the information schema,
    where we need to regenerate the meta data if the underlying tables
    have changed.
  */
  bootstrap::DD_bootstrap_ctx::instance().set_dd_upgrade_done();

  /*
    Flush tables, reset the shared dictionary cache and the storage adapter.
    Start over DD bootstrap from the beginning.
  */
  if (dd::execute_query(thd, "FLUSH TABLES")) return true;

  dd::cache::Shared_dictionary_cache::instance()->reset(false);

  /*
    Reset the encryption attribute in object table def since we will now
    start over by creating the scaffolding, which expectes an unencrypted
    DD tablespace.
  */
  Object_table_definition_impl::set_dd_tablespace_encrypted(false);

  // Reset the DDSE local dictionary cache.
  handlerton *ddse = ha_resolve_by_legacy_type(thd, DB_TYPE_INNODB);
  if (ddse->dict_cache_reset == nullptr) return true;

  for (System_tables::Const_iterator it =
           System_tables::instance()->begin(System_tables::Types::CORE);
       it != System_tables::instance()->end();
       it = System_tables::instance()->next(it, System_tables::Types::CORE)) {
    ddse->dict_cache_reset(MYSQL_SCHEMA_NAME.str,
                           (*it)->entity()->name().c_str());
    ddse->dict_cache_reset(target_table_schema_name.c_str(),
                           (*it)->entity()->name().c_str());
  }

  /*
    We need to start over DD initialization. This is done by executing the
    first stages of the procedure followed at restart. Note that this
    will see and use the newly upgraded DD that was created above. Cleanup
    of the temporary schemas is done at the end of 'sync_meta_data()'.
  */
  bootstrap::DD_bootstrap_ctx::instance().set_stage(bootstrap::Stage::STARTED);

  store_predefined_tablespace_metadata(thd);
  if (create_dd_schema(thd) || initialize_dd_properties(thd) ||
      create_tables(thd, nullptr) || sync_meta_data(thd)) {
    return true;
  }

  bootstrap::DD_bootstrap_ctx::instance().set_stage(
      bootstrap::Stage::UPGRADED_TABLES);

  return false;
}

}  // namespace upgrade
/* purecov: end */
}  // namespace dd
