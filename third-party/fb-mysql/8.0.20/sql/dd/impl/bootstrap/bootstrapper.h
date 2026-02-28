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

#ifndef DD__BOOTSTRAPPER_INCLUDED
#define DD__BOOTSTRAPPER_INCLUDED

#include <sys/types.h>

#include "sql/dd/impl/system_registry.h"  // dd::System_tables
#include "sql/dd/string_type.h"           // dd::String_type
#include "sql/handler.h"                  // dict_init_mode_t

class THD;

/**
  Data dictionary initialization.

  The data dictionary is initialized whenever the mysqld process starts.
  We distinguish between the first time start and the subsequent normal
  restarts/upgrades, as explained below. However, there are three main
  design principles that should be elaborated first.

  1. Two-step process: The dictionary initialization is implemented as
     a two step process. First, scaffolding is built to prepare the
     synchronization with persistent storage, then, the actual synchronization
     is done. The way this is done depends on the context, and is different
     for first time start and the subsequent restarts.

  2. Use SQL: The initialization uses SQL to build the scaffolding. This
     means that we execute SQL statements to create the dictionary tables.
     Since this is done at a stage where the physical tables either do not
     exist yet, or are not known, we must instrument the DDL execution to
     create the physical counterpart of the tables only on first time start.
     The goal is to keep the instrumentation at a minimum.

  3. Fake caching: As a consequence of keeping instrumentation at a minimum,
     we provide uniform behavior of the caching layer in the data dictionary
     also in the scaffolding phase. This means that as seen from the outside,
     dictionary objects can be retrieved from the cache. Internally, below the
     caching layer, the objects are only kept in a separate buffer until all
     the required scaffolding is built. At that point, we can start using the
     underlying physical tables, depending on the circumstances:

     - For first time start (initialization), we can flush the meta data
       generated in the scaffolding phase, to the DD tables.
     - For ordinary restart, we can use the scaffolding to open the physical
       tables, and then sync up the real meta data that is stored persistently.
     - For upgrade, we first build scaffolding based on the actual DD tables,
       then we create the target DD tables, migrate the meta data from the old
       to the new tables, and finally switch from old to new tables
       atomically by means of DML on the DD tables. This means that we update
       the schema ids in the DD tables directly instead of executing
       'RENAME TABLE', which would do auto commit and thus break atomicity.

     After the scaffolding has been flushed or synced, what should be left is
     a collection of the core DD meta data objects. This collection is located
     in the storage adapter, and allows the DD cache to evict core DD objects
     in the same way as other DD objects.

  Please note that dictionary initialization is only a small part of server
  initialization. There is a lot going on before and after dictionary
  initialization while starting the server.

  Please see more elaborated descriptions for the initialize() and restart()
  methods below.
*/

namespace dd {
class Dictionary_impl;

namespace bootstrap {

/**
  Initialize the dictionary while starting the server for the first time.

  At this point, the DDSE has been initialized as a normal plugin. The
  dictionary initialization proceeds as follows:

   1. Preparation phase

  1.1 Call dict_init() to initialize the DDSE. This will make the predefined
      tablespaces be created physically, and their meta data be returned to
      the SQL layer along with the meta data for the DD tables required by
      the DDSE. The tables are not yet created physically.
  1.2 Prepare the dd::Tablespace objects reflecting the predefined tablespace
      objects and add them to the core registry in the storage adapter.

  2. Scaffolding phase

  2.1 Create and use the dictionary schema by executing SQL statements.
      The schema is created physically since this is the first time start,
      and the meta data is generated and stored in the core registry of
      the storage adapter without being written to disk.
  2.2 Create tables by executing SQL statements. Like for the schema, the
      tables are created physically, and the meta data is generated
      and stored in the core registry without being written to disk.
      This is done to prepare enough meta data to actually be able to
      open the DD tables.

  3. Synchronization phase

  3.1 Store meta data for the DD schema, tablespace and tables, i.e., the DD
      objects that were generated in the scaffolding phase, and make sure the
      IDs are maintained when the objects are stored.
  3.2 Populate the DD tables which have some predefined static contents to
      be inserted. This is, e.g., relevant for the 'catalogs' table, which
      only has a single default entry in it. Dynamic contents is added in
      other ways, e.g. by storing generated DD objects (see above) or by
      inserting data from other sources (see re-population of character sets
      in the context of server restart below).
  3.3 Store various properties of the DD tables, including the SE private data,
      a representation of the DDL statement used to create the table etc.
  3.4 Verify that the dictionary objects representing the core DD table meta
      data are present in the core registry of the storage adapter. If an
      object representing the meta data of a core DD table is not available,
      then we loose access to the DD tables, and we will not be able to handle
      cache misses or updates to the meta data.
  3.5 Update the version numbers that are stored, e.g. the DD version and the
      current mysqld server version.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool initialize(THD *thd);

/**
  Initialize the dictionary while restarting the server.

  At this point, the DDSE has been initialized as a normal plugin. The
  dictionary initialization proceeds as follows:

  1. Preparation phase

  1.1 Call dict_init() to initialize the DDSE. This will retrieve the meta data
      of the predefined tablespaces and the DD tables required by the DDSE.
      Both the tables and the tablespaces are already created physically, the
      point here is just to get hold of enough meta data to start using the DD.
  1.2 Prepare the dd::Tablespace objects reflecting the predefined tablespace
      objects and add them to the core registry in the storage adapter.

  2. Scaffolding phase

  2.1 Create and use the dictionary schema by executing SQL statements.
      The schema is not created physically, but the meta data is generated
      and stored in the core registry without being written to disk.
  2.2 Create tables by executing SQL statements. Like for the schema, the
      tables are not created physically, but the meta data is generated
      and stored in the core registry without being written to disk.
      This is done to prepare enough meta data to actually be able to
      open the DD tables. The SQL DDL statements are either retrieved from
      the table definitions that are part of the server binary (for restart),
      or from one of the DD tables (for upgrade).

  3. Synchronization phase

  3.1 Read meta data for the DD tables from the DD tables. Here, we use the
      meta data from the scaffolding phase for the schema, tablespace and the
      DD tables to open the physical DD tables. We read the stored objects,
      and update the in-memory copies in the core registry with the real meta
      data from the objects that are retrieved form persistent storage. Finally,
      we flush the tables to empty the table definition cache to make sure the
      table share structures for the DD tables are re-created based on the
      actual meta data that was read from disk rather than the temporary meta
      data from the scaffolding phase.
  3.2 If this is a restart with a new DD version, we must upgrade the DD
      tables. In that case, we create the new target DD tables in a temporary
      schema, migrate the meta data to the new tables, and then do DML on the
      DD tables to make sure the new DD tables will be used instead of the old
      ones. This DML involves changing the schema ids directly in the DD tables,
      and updating the meta data stored in the 'dd_properties' DD table.
      This will make sure the switch from the old to the new tables is
      atomic. After this is done, we will reset the DD cache and start over
      the initialization from step 1.2. Then, the new DD tables will be used,
      and a normal restart will be done.
  3.3 Re-populate character sets and collations: The character set and
      collation information is read from files and added to a server
      internal data structure when the server starts. This data structure is,
      in turn, used to populate the corresponding DD tables. The tables must
      be re-populated on each server start if new character sets or collations
      have been added. However, we can not do this if in read only mode.
  3.4 Verify that the dictionary objects representing the core DD table meta
      data are present in the core registry of the storage adapter. If an
      object representing the meta data of a core DD table is not available,
      then we loose access to the DD tables, and we will not be able to handle
      cache misses or updates to the meta data.
  3.5 If an upgrade was done, the persistent version numbers are updated,
      e.g. the DD version and the current mysqld server version.

  @param thd            Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool restart(THD *thd);

/**
  Iterate through all the plugins, and store IS table meta data
  into dictionary, once during MySQL server bootstrap.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool store_plugin_IS_table_metadata(THD *thd);

/**
  Initialization and verification of dictionary objects
  after upgrade, similar to what is done after normal server
  restart.

  @param thd    Thread context
*/
bool setup_dd_objects_and_collations(THD *thd);

/**
  This function is used in case of crash during upgrade.
  It tries to initialize dictionary and calls DDSE_dict_recover.
  InnoDB should do the recovery and empty undo log. Upgrade
  process will do the cleanup and exit.

  @param thd    Thread context.
*/
void recover_innodb_upon_upgrade(THD *thd);

/**
  Initialize InnoDB for
  - creating new data directory : InnoDB creates system tablespace and
                                  dictionary tablespace.
  - normal server restart.      : Verifies existence of system and dictionary
                                  tablespaces.
  - in place upgrade            : Verifies existence of system tablespace and
                                  create dictionary tablespace.

  @param thd             Thread context.
  @param dict_init_mode  mode to initialize InnoDB
  @param version         Dictionary version.

  @return       Upon failure, return true, otherwise false.
*/
bool DDSE_dict_init(THD *thd, dict_init_mode_t dict_init_mode, uint version);

/**
  Create mysql schema. Create dictionary tables inside InnoDB.
  Create entry for dictionary tables inside dictionary tables.
  Add hard coded data to dictionary tables.
  Create Foreign key constraint on dictionary tables.

  This function is used in both cases, new data directory initialization
  and in place upgrade.

  @param thd            Thread context.
  @param is_dd_upgrade  Flag to indicate if it is in place upgrade.
  @param d              Dictionary instance

  @return       Upon failure, return true, otherwise false.

*/
bool initialize_dictionary(THD *thd, bool is_dd_upgrade, Dictionary_impl *d);

}  // namespace bootstrap

/**
  This function creates the meta data of the predefined tablespaces.

  @param thd    Thread context.
*/
void store_predefined_tablespace_metadata(THD *thd);

/**
  Executes SQL queries to create and use the dictionary schema.

  @param thd    Thread context.
*/
bool create_dd_schema(THD *thd);

/**
  During --initialize, we create the dd_properties table. During restart,
  create its meta data, and use it to open and read its contents.

  @param thd    Thread context.
*/
bool initialize_dd_properties(THD *thd);

/**
  Predicate to check if a table type is a non-inert DD ot DDSE table.

  @param table_type    Type as defined in the System_tables registry.
  @returns             true if the table is a non-inert DD or DDSE table,
                       false otherwise
*/
bool is_non_inert_dd_or_ddse_table(System_tables::Types table_type);

/**
  Execute SQL statements to create the DD tables.

  The tables created here will be a subset of the target DD tables for this
  DD version. This function is called in the following four cases:

  1. When a server is started the first time, with --initialize. Then, we
     will iterate over all target tables and create them. This will also
     make them be created physically in the DDSE.
  2. When a server is restarted, and the data directory contains a dictionary
     with the same DD version as the target DD version of the starting server.
     In this case, we will iterate over all target tables and create them,
     using the target table SQL DDL definitions. This is done only to create
     the meta data, though; the tables will not be created physically in the
     DDSE since they already exist. But we need to create the meta data to be
     able top open them.
  3. When a server is restarted, and the data directory was last used by a
     more recent MRU within the same GA with a higher target DD version.
     This is considered a 'minor downgrade'. In this case, the restarting
     server will continue to run using the more recent DD version. This is
     possible since only a subset of DD changes are allowed in a DD upgrade
     that can also be downgraded. However, it means that we must create the
     meta data reflecting the *actual* tables, not the target tables. So in
     this case, we iterate over the target tables, but execute the DDL
     statements of the actual tables. We get these statements from the
     'dd_properties' table, where the more recent MRU has stored them.
  4. When a server is restarted, and the data directory was last used by a
     server with a DD version from which the starting server can upgrade. In
     this case, this function is called three times:

     - The first time, we need to create the meta data reflecting the actual
       tables in the persistent DD. This is needed to be able to open the DD
       tables and read the data. This is similar to use case 3. above.
     - The second time, we create the tables that are modified in the new DD
       version. Here, the tables are also created physically in the DDSE.
       In this case, the 'create_set' specifies which subset of the target
       tables should be created. After this stage, we replace the meta data
       in 'dd_properties' by new meta data reflecting the modified tables. We
       also replace the version numbers to make sure a new restart will use
       the upgraded DD.
     - The third time, we do the same as in case 2 above. This is basically
       the same as a shutdown and restart of the server after upgrade was
       completed.

  @param  thd         Thread context.
  @param  create_set  Subset of the target tables which should be created
                      during upgrade.

  @returns false if success, otherwise true.
*/
bool create_tables(THD *thd, const std::set<String_type> *create_set);

/**
  Acquire the DD schema, tablespace and table objects. Read the persisted
  objects from the DD tables, and replace the contents of the core
  registry in the storage adapter

  @param thd    Thread context.
*/
bool sync_meta_data(THD *thd);

/**
  Update properties in the DD_properties table. Note that upon failure, we
  will rollback, whereas upon success, commit will be delayed.

  @param thd                        Thread context.
  @param create_set                 A set of table names created/modified in
                                    this version of DD.
  @param remove_set                 A set of table names removed in this
                                    version of DD.
  @param target_table_schema_name   Schema name in which the final changes are
                                    required.

  @return       Upon failure, return true, otherwise false.
*/
bool update_properties(THD *thd, const std::set<String_type> *create_set,
                       const std::set<String_type> *remove_set,
                       const String_type &target_table_schema_name);

/**
  Updates the DD Version in the DD_properties table to the current version.
  This function is used during initialize and during server upgrade.

  @param thd               Thread context.
  @param is_dd_upgrade_57  Flag to indicate if it is an upgrade from 5.7.

  @return       Upon failure, return true, otherwise false.
*/
bool update_versions(THD *thd, bool is_dd_upgrade_57);

}  // namespace dd
#endif  // DD__BOOTSTRAPPER_INCLUDED
