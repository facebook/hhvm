#ifndef SQL_SERVERS_INCLUDED
#define SQL_SERVERS_INCLUDED

/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>

#include "lex_string.h"
#include "my_sqlcommand.h"
#include "sql/sql_cmd.h"  // Sql_cmd

class THD;
struct MEM_ROOT;
struct TABLE;

class FOREIGN_SERVER {
 public:
  char *server_name;
  long port;
  size_t server_name_length;
  char *db, *scheme, *username, *password, *socket, *owner, *host, *sport;

  FOREIGN_SERVER()
      : server_name(nullptr),
        port(-1),
        server_name_length(0),
        db(nullptr),
        scheme(nullptr),
        username(nullptr),
        password(nullptr),
        socket(nullptr),
        owner(nullptr),
        host(nullptr),
        sport(nullptr) {}
};

/* cache handlers */
bool servers_init(bool dont_read_server_table);
bool servers_reload(THD *thd);
void servers_free(bool end = false);

/* lookup functions */
FOREIGN_SERVER *get_server_by_name(MEM_ROOT *mem, const char *server_name,
                                   FOREIGN_SERVER *server_buffer);

/**
   This class represent server options as set by the parser.
 */

class Server_options {
 public:
  static const long PORT_NOT_SET = -1;
  LEX_STRING m_server_name;

 private:
  long m_port;
  LEX_STRING m_host;
  LEX_STRING m_db;
  LEX_STRING m_username;
  LEX_STRING m_password;
  LEX_STRING m_scheme;
  LEX_STRING m_socket;
  LEX_STRING m_owner;

 public:
  void set_port(long port) { m_port = port; }
  void set_host(LEX_STRING host) { m_host = host; }
  void set_db(LEX_STRING db) { m_db = db; }
  void set_username(LEX_STRING username) { m_username = username; }
  void set_password(LEX_STRING password) { m_password = password; }
  void set_scheme(LEX_STRING scheme) { m_scheme = scheme; }
  void set_socket(LEX_STRING socket) { m_socket = socket; }
  void set_owner(LEX_STRING owner) { m_owner = owner; }

  long get_port() const { return m_port; }
  const char *get_host() const { return m_host.str; }
  const char *get_db() const { return m_db.str; }
  const char *get_username() const { return m_username.str; }
  const char *get_password() const { return m_password.str; }
  const char *get_scheme() const { return m_scheme.str; }
  const char *get_socket() const { return m_socket.str; }
  const char *get_owner() const { return m_owner.str; }

  /**
     Reset all strings to NULL and port to PORT_NOT_SET.
     This prepares the structure for being used by a new statement.
  */
  void reset();

  /**
     Create a cache entry and insert it into the cache.

     @returns false if entry was created and inserted, true otherwise.
  */
  bool insert_into_cache() const;

  /**
     Update a cache entry.

     @param existing  Cache entry to update

     @returns false if the entry was updated, true otherwise.
  */
  bool update_cache(FOREIGN_SERVER *existing) const;

  /**
     Create a record representing these server options,
     ready to be inserted into the mysql.servers table.

     @param table  Table to be inserted into.
  */
  void store_new_server(TABLE *table) const;

  /**
     Create a record for updating a row in the mysql.servers table.

     @param table     Table to be updated.
     @param existing  Cache entry represeting the existing values.
  */
  void store_altered_server(TABLE *table, FOREIGN_SERVER *existing) const;
};

/**
   This class has common code for CREATE/ALTER/DROP SERVER statements.
*/

class Sql_cmd_common_server : public Sql_cmd {
 protected:
  TABLE *table;

  Sql_cmd_common_server() : table(nullptr) {}

  virtual ~Sql_cmd_common_server() {}

  /**
     Check permissions and open the mysql.servers table.

     @param thd  Thread context

     @returns false if success, true otherwise
  */
  bool check_and_open_table(THD *thd);
};

/**
   This class implements the CREATE SERVER statement.
*/

class Sql_cmd_create_server : public Sql_cmd_common_server {
  /**
     Server_options::m_server_name contains the name of the
     server to create. The remaining Server_options fields
     contain options as set by the parser.
     Unset options are NULL (or PORT_NOT_SET for port).
  */
  const Server_options *m_server_options;

 public:
  Sql_cmd_create_server(Server_options *server_options)
      : Sql_cmd_common_server(), m_server_options(server_options) {}

  enum_sql_command sql_command_code() const { return SQLCOM_CREATE_SERVER; }

  /**
     Create a new server by inserting a row into the
     mysql.server table and creating a cache entry.

     @param thd  Thread context

     @returns false if success, true otherwise
  */
  bool execute(THD *thd);
};

/**
   This class implements the ALTER SERVER statement.
*/

class Sql_cmd_alter_server : public Sql_cmd_common_server {
  /**
     Server_options::m_server_name contains the name of the
     server to change. The remaining Server_options fields
     contain changed options as set by the parser.
     Unchanged options are NULL (or PORT_NOT_SET for port).
  */
  const Server_options *m_server_options;

 public:
  Sql_cmd_alter_server(Server_options *server_options)
      : Sql_cmd_common_server(), m_server_options(server_options) {}

  enum_sql_command sql_command_code() const { return SQLCOM_ALTER_SERVER; }

  /**
     Alter an existing server by updating the matching row in the
     mysql.servers table and updating the cache entry.

     @param thd  Thread context

     @returns false if success, true otherwise
  */
  bool execute(THD *thd);
};

/**
   This class implements the DROP SERVER statement.
*/

class Sql_cmd_drop_server : public Sql_cmd_common_server {
  /// Name of server to drop
  LEX_STRING m_server_name;

  /// Is this DROP IF EXISTS?
  bool m_if_exists;

 public:
  Sql_cmd_drop_server(LEX_STRING server_name, bool if_exists)
      : Sql_cmd_common_server(),
        m_server_name(server_name),
        m_if_exists(if_exists) {}

  enum_sql_command sql_command_code() const { return SQLCOM_DROP_SERVER; }

  /**
     Drop an existing server by deleting the matching row from the
     mysql.servers table and removing the cache entry.

     @param thd  Thread context

     @returns false if success, true otherwise
  */
  bool execute(THD *thd);
};

#endif /* SQL_SERVERS_INCLUDED */
