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

#ifndef DD_UPGRADE__SERVER_H_INCLUDED
#define DD_UPGRADE__SERVER_H_INCLUDED

#include <stdio.h>

#include "typelib.h"

class THD;
class Time_zone;

extern TYPELIB upgrade_mode_typelib;
enum enum_upgrade_mode : int {
  UPGRADE_NONE,
  UPGRADE_MINIMAL,
  UPGRADE_AUTO,
  UPGRADE_FORCE,
  UPGRADE_FORCE_AND_SHUTDOWN
};

namespace dd {
namespace upgrade {
/**
  Upgrades/restores the system tables to defaults of the current MySQL version.
  This is a replacement for the mysql_upgrade client.

  There are four SQL scripts executed:
  1. mysql_system_tables.sql - Creates the system tables
  2. mysql_system_tables_fix.sql - Updates the system table
  3. mysql_system_tables_data_fix.sql - Fills the system tables with meta data
  4. mysql_sys_schema.sql - Create and/or updates the sys schema

  Then the system tables are checked by executing CHECK TABLE SQL statements.

  This function is called during startup if the MySQL version present in
  DD_properties in not the same as the current MySQL version. This function can
  also be called if the server is started with --upgrade=FORCE option.

  If the server is started with --upgrade=MINIMAL option with a newer MySQL
  server version Z on an older data directory of MySQL server version X, this
  function checks if server upgrade has been skipped before using another MySQL
  server version Y such that Y != Z, X < Y and X < Z. If yes, we abort.

  The server upgrade ends with updating the MYSQLD_UPGRADED_VERSION value in
  DD_properties to the current server version (MYSQL_VERSION_ID).

  @param[in]  thd   Thread handle.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
bool upgrade_system_schemas(THD *thd);

bool no_server_upgrade_required();

bool I_S_upgrade_required();

bool is_force_upgrade();
}  // namespace upgrade

}  // namespace dd
#endif  // DD_UPGRADE__SERVER_H_INCLUDED
