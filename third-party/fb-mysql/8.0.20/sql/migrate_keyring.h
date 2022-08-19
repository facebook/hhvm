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

#ifndef MIGRATE_KEYRING_H_INCLUDED
#define MIGRATE_KEYRING_H_INCLUDED

#include <string>
#include "mysql.h"
#include "mysql/plugin_keyring.h"
#include "sql_common.h"  // NET_SERVER

class THD;

#define MAX_KEY_LEN 16384

enum class enum_plugin_type { SOURCE_PLUGIN = 0, DESTINATION_PLUGIN };

class Key_info {
 public:
  Key_info() {}
  Key_info(char *key_id, char *user_id) {
    m_key_id = key_id;
    m_user_id = user_id;
  }
  Key_info(const Key_info &ki) {
    this->m_key_id = ki.m_key_id;
    this->m_user_id = ki.m_user_id;
  }

 public:
  std::string m_key_id;
  std::string m_user_id;
};

class Migrate_keyring {
 public:
  /**
    Standard constructor.
  */
  Migrate_keyring();
  /**
    Initialize all needed parameters to proceed with migration process.
  */
  bool init(int argc, char **argv, char *source_plugin,
            char *destination_plugin, char *user, char *host, char *password,
            char *socket, ulong port);
  /**
    Migrate keys from source keyring to destination keyring.
  */
  bool execute();
  /**
    Standard destructor
  */
  ~Migrate_keyring();

 private:
  /**
    Load source or destination plugin.
  */
  bool load_plugin(enum_plugin_type plugin_type);
  /**
    Fetch keys from source plugin and store in destination plugin.
  */
  bool fetch_and_store_keys();
  /**
    Disable @@keyring_operations variable.
  */
  bool disable_keyring_operations();
  /**
    Enable @@keyring_operations variable.
  */
  bool enable_keyring_operations();

 private:
  int m_argc;
  char **m_argv;
  std::string m_source_plugin_option;
  std::string m_destination_plugin_option;
  std::string m_source_plugin_name;
  std::string m_destination_plugin_name;
  std::string m_internal_option[2];
  st_mysql_keyring *m_source_plugin_handle;
  st_mysql_keyring *m_destination_plugin_handle;
  std::vector<Key_info> m_source_keys;
  MYSQL *mysql;
  NET_SERVER server_extn;
};

#endif /* MIGRATE_KEYRING_H_INCLUDED */
