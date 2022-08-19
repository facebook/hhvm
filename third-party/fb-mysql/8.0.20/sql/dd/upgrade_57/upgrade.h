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

#ifndef DD_UPGRADE__UPGRADE_H_INCLUDED
#define DD_UPGRADE__UPGRADE_H_INCLUDED

#include <stdio.h>

#include "sql/dd/string_type.h"

class THD;

namespace dd {
namespace upgrade_57 {

/**
  Check if the server is starting on a data directory without dictionary
  tables or not.

  If the dictionary tables are present, continue with restart of the server.

  If the dicionary tables are not present, create the dictionary tables in
  existing data directory.  This function marks dd_upgrade_flag as true to
  indicate to the server that Data dictionary is being upgraded.

  metadata mysql.plugin table is migrated to the DD tables in case of upgrade.
  mysql.plugin table is used to initialize all other Storage Engines.
  This is necessary before migrating other user tables.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool do_pre_checks_and_initialize_dd(THD *thd);

/**
  Finalize upgrade to the new data-dictionary by populating
  Data Dictionary tables with metadata.

  Populate metadata in Data dictionary tables.
  This will be done for following database objects:
  - Databases
  - Tables
  - Views
  - Stored Procedures and Stored Functions
  - Events
  - Triggers

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool fill_dd_and_finalize(THD *thd);

/**
  Drop all DD tables in case there is an error while upgrading server.

  @param[in] thd               Thread context.

  @return       Upon failure, return true, otherwise false.
*/
bool terminate(THD *thd);

/**
  Check if upgrade is in progress
*/
bool in_progress();

/**
  Check if creation of SDI file is allowed by upgrade.
*/
bool allow_sdi_creation();

/**
  Class to manage a temporary file to maintain the progess of the
  upgrade. This file will help in error handling for crashes
  during upgrade. After upgrade is successful, this file will be
  deleted.
*/
class Upgrade_status {
 public:
  // Stages of upgrade to be maintained in the file.
  enum class enum_stage {
    // Upgrade not started.
    NONE,

    // Upgrade from 5.7 detected, create this file and write 0 to it.
    STARTED,

    /*
      Started InnoDB in upgrade mode, i.e., undo and redo logs are
      upgraded and mysql.ibd is created.
    */
    DICT_SPACE_CREATED,

    // Dictionary tables are created.
    DICT_TABLES_CREATED,

    /*
      Dictionary initialization is complete and upgrade will start
      processing user tables now.
    */
    DICTIONARY_CREATED,

    // Upgrade of user tables is complete.
    USER_TABLE_UPGRADED,

    // SDI information is added to tablespaces.
    SDI_INFO_UPDATED
  };

 public:
  Upgrade_status();

  /**
    Check if status file exists.

    @returns true if exists, else false.
  */
  bool exists();

  /**
    Create status file.

    @returns false on success, else true.
  */
  bool create();

  /**
    Get status from file.

    @returns enum_stage.
  */
  enum_stage get();

  /**
    Update upgrade status.

    @returns false on success, else true.
  */
  bool update(enum_stage status);

  /**
    Remove upgrade status file.

    @returns false on success, else true.
  */
  bool remove();

 private:
  bool open(int flags);
  enum_stage read();
  bool write(enum_stage status);
  bool close();

 private:
  FILE *m_file;
  const String_type m_filename;
};

}  // namespace upgrade_57
}  // namespace dd
#endif  // DD_UPGRADE__UPGRADE_H_INCLUDED
