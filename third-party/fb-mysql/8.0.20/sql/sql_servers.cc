/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  The servers are saved in the system table "servers"

  Currently, when the user performs an ALTER SERVER or a DROP SERVER
  operation, it will cause all open tables which refer to the named
  server connection to be flushed. This may cause some undesirable
  behaviour with regard to currently running transactions. It is
  expected that the DBA knows what s/he is doing when s/he performs
  the ALTER SERVER or DROP SERVER operation.

  TODO:
  It is desirable for us to implement a callback mechanism instead where
  callbacks can be registered for specific server protocols. The callback
  will be fired when such a server name has been created/altered/dropped
  or when statistics are to be gathered such as how many actual connections.
  Storage engines etc will be able to make use of the callback so that
  currently running transactions etc will not be disrupted.
*/

#include "sql/sql_servers.h"

#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/psi_memory_key.h"  // key_memory_servers
#include "sql/records.h"         // init_read_record
#include "sql/row_iterator.h"
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_base.h"         // close_mysql_tables
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_rollback_stmt, trans_commit_stmt
#include "thr_lock.h"

/*
  We only use 1 mutex to guard the data structures - THR_LOCK_servers.
  Read locked when only reading data and write-locked for all other access.
*/

static collation_unordered_map<std::string, FOREIGN_SERVER *> *servers_cache;
static MEM_ROOT mem;
static mysql_rwlock_t THR_LOCK_servers;

/**
   This enum describes the structure of the mysql.servers table.
*/
enum enum_servers_table_field {
  SERVERS_FIELD_NAME = 0,
  SERVERS_FIELD_HOST,
  SERVERS_FIELD_DB,
  SERVERS_FIELD_USERNAME,
  SERVERS_FIELD_PASSWORD,
  SERVERS_FIELD_PORT,
  SERVERS_FIELD_SOCKET,
  SERVERS_FIELD_SCHEME,
  SERVERS_FIELD_OWNER
};

static bool get_server_from_table_to_cache(TABLE *table);

#ifdef HAVE_PSI_INTERFACE
static PSI_rwlock_key key_rwlock_THR_LOCK_servers;

static PSI_rwlock_info all_servers_cache_rwlocks[] = {
    {&key_rwlock_THR_LOCK_servers, "THR_LOCK_servers", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static PSI_memory_info all_servers_cache_memory[] = {
    {&key_memory_servers, "servers_cache", PSI_FLAG_ONLY_GLOBAL_STAT, 0,
     PSI_DOCUMENT_ME}};

static void init_servers_cache_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_servers_cache_rwlocks));
  mysql_rwlock_register(category, all_servers_cache_rwlocks, count);

  count = static_cast<int>(array_elements(all_servers_cache_memory));
  mysql_memory_register(category, all_servers_cache_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

/*
  Initialize structures responsible for servers used in federated
  server scheme information for them from the server
  table in the 'mysql' database.

  SYNOPSIS
    servers_init()
      dont_read_server_table  true if we want to skip loading data from
                            server table and disable privilege checking.

  NOTES
    This function is mostly responsible for preparatory steps, main work
    on initialization and grants loading is done in servers_reload().

  RETURN VALUES
    0	ok
    1	Could not initialize servers
*/

bool servers_init(bool dont_read_servers_table) {
  THD *thd;
  bool return_val = false;
  DBUG_TRACE;

#ifdef HAVE_PSI_INTERFACE
  init_servers_cache_psi_keys();
#endif

  /* init the mutex */
  if (mysql_rwlock_init(key_rwlock_THR_LOCK_servers, &THR_LOCK_servers))
    return true;

  /* initialise our servers cache */
  servers_cache = new collation_unordered_map<std::string, FOREIGN_SERVER *>(
      system_charset_info, key_memory_servers);

  /* Initialize the mem root for data */
  init_sql_alloc(key_memory_servers, &mem, ACL_ALLOC_BLOCK_SIZE, 0);

  if (dont_read_servers_table) goto end;

  /*
    To be able to run this from boot, we allocate a temporary THD
  */
  if (!(thd = new THD)) return true;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();
  /*
    It is safe to call servers_reload() since servers_* arrays and hashes which
    will be freed there are global static objects and thus are initialized
    by zeros at startup.
  */
  return_val = servers_reload(thd);
  delete thd;

end:
  return return_val;
}

/*
  Initialize server structures

  SYNOPSIS
    servers_load()
      thd     Current thread
      tables  List containing open "mysql.servers"

  RETURN VALUES
    false  Success
    true   Error

  TODO
    Revert back to old list if we failed to load new one.
*/

static bool servers_load(THD *thd, TABLE *table) {
  DBUG_TRACE;

  if (servers_cache != nullptr) {
    servers_cache->clear();
  }
  free_root(&mem, MYF(0));
  init_sql_alloc(key_memory_servers, &mem, ACL_ALLOC_BLOCK_SIZE, 0);

  unique_ptr_destroy_only<RowIterator> iterator =
      init_table_iterator(thd, table, nullptr, false,
                          /*ignore_not_found_rows=*/false);
  if (iterator == nullptr) return true;

  while (!(iterator->Read())) {
    if ((get_server_from_table_to_cache(table))) return true;
  }

  return false;
}

/*
  Forget current servers cache and read new servers
  from the conneciton table.

  SYNOPSIS
    servers_reload()
      thd  Current thread

  NOTE
    All tables of calling thread which were open and locked by LOCK TABLES
    statement will be unlocked and closed.
    This function is also used for initialization of structures responsible
    for user/db-level privilege checking.

  RETURN VALUE
    false  Success
    true   Failure
*/

bool servers_reload(THD *thd) {
  bool return_val = true;
  DBUG_TRACE;

  DBUG_PRINT("info", ("locking servers_cache"));
  mysql_rwlock_wrlock(&THR_LOCK_servers);

  TABLE_LIST tables("mysql", "servers", TL_READ);
  if (open_trans_system_tables_for_read(thd, &tables)) {
    /*
      Execution might have been interrupted; only print the error message
      if an error condition has been raised.
    */
    if (thd->get_stmt_da()->is_error())
      LogErr(ERROR_LEVEL, ER_CANT_OPEN_AND_LOCK_PRIVILEGE_TABLES,
             thd->get_stmt_da()->message_text());
    goto end;
  }

  if ((return_val =
           servers_load(thd, tables.table))) {  // Error. Revert to old list
    /* blast, for now, we have no servers, discuss later way to preserve */

    DBUG_PRINT("error", ("Reverting to old privileges"));
    servers_free();
  }

  close_trans_system_tables(thd);
end:
  DBUG_PRINT("info", ("unlocking servers_cache"));
  mysql_rwlock_unlock(&THR_LOCK_servers);
  return return_val;
}

/*
  Initialize structures responsible for servers used in federated
  server scheme information for them from the server
  table in the 'mysql' database.

  SYNOPSIS
    get_server_from_table_to_cache()
      TABLE *table         open table pointer


  NOTES
    This function takes a TABLE pointer (pointing to an opened
    table). With this open table, a FOREIGN_SERVER struct pointer
    is allocated into root memory, then each member of the FOREIGN_SERVER
    struct is populated. A char pointer takes the return value of get_field
    for each column we're interested in obtaining, and if that pointer
    isn't 0x0, the FOREIGN_SERVER member is set to that value, otherwise,
    is set to the value of an empty string, since get_field would set it to
    0x0 if the column's value is empty, even if the default value for that
    column is NOT NULL.

  RETURN VALUES
    0	ok
    1	could not insert server struct into global servers cache
*/

static bool get_server_from_table_to_cache(TABLE *table) {
  /* alloc a server struct */
  char *ptr;
  char *blank = const_cast<char *>("");
  FOREIGN_SERVER *server = new (&mem) FOREIGN_SERVER();

  DBUG_TRACE;
  table->use_all_columns();

  /* get each field into the server struct ptr */
  ptr = get_field(&mem, table->field[SERVERS_FIELD_NAME]);
  server->server_name = ptr ? ptr : blank;
  server->server_name_length = strlen(server->server_name);
  ptr = get_field(&mem, table->field[SERVERS_FIELD_HOST]);
  server->host = ptr ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_DB]);
  server->db = ptr ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_USERNAME]);
  server->username = ptr ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_PASSWORD]);
  server->password = ptr ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_PORT]);
  server->sport = ptr ? ptr : blank;

  server->port = server->sport ? atoi(server->sport) : 0;

  ptr = get_field(&mem, table->field[SERVERS_FIELD_SOCKET]);
  server->socket = ptr && strlen(ptr) ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_SCHEME]);
  server->scheme = ptr ? ptr : blank;
  ptr = get_field(&mem, table->field[SERVERS_FIELD_OWNER]);
  server->owner = ptr ? ptr : blank;
  DBUG_PRINT("info", ("server->server_name %s", server->server_name));
  DBUG_PRINT("info", ("server->host %s", server->host));
  DBUG_PRINT("info", ("server->db %s", server->db));
  DBUG_PRINT("info", ("server->username %s", server->username));
  DBUG_PRINT("info", ("server->password %s", server->password));
  DBUG_PRINT("info", ("server->socket %s", server->socket));
  servers_cache->emplace(server->server_name, server);
  return false;
}

/**
  Close all tables which match specified connection string or
  if specified string is NULL, then any table with a connection string.
*/

static bool close_cached_connection_tables(THD *thd,
                                           const char *connection_string,
                                           size_t connection_length) {
  TABLE_LIST tmp, *tables = nullptr;
  bool result = false;
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  mysql_mutex_lock(&LOCK_open);

  for (const auto &key_and_value : *table_def_cache) {
    TABLE_SHARE *share = key_and_value.second.get();

    /*
      Skip table shares being opened to avoid comparison reading into
      uninitialized memory further below.

      Thus, in theory, there is a risk that shares are left in the
      cache that should really be closed (matching the submitted
      connection string), and this risk is already present since
      LOCK_open is unlocked before calling this function. However,
      this function is called as the final step of DROP/ALTER SERVER,
      so its goal is to flush all tables which were open before
      DROP/ALTER SERVER started. Thus, if a share gets opened after
      this function is called, the information about the server has
      already been updated, so the new table will use the new
      definition of the server.

      It might have been an issue, however if one thread started
      opening a federated table, read the old server definition into a
      share, and then a switch to another thread doing ALTER SERVER
      happened right before setting m_open_in_progress to false for
      the share. Because in this case ALTER SERVER would not flush
      the share opened by the first thread as it should have been. But
      luckily, server definitions affected by * SERVER statements are
      not read into TABLE_SHARE structures, but are read when we
      create the TABLE object in ha_federated::open().

      This means that ignoring shares that are in the process of being
      opened is safe, because such shares don't have TABLE objects
      associated with them yet.
    */
    if (share->m_open_in_progress) continue;

    /* Ignore if table is not open or does not have a connect_string */
    if (!share->connect_string.length || share->ref_count() == 0) continue;

    /* Compare the connection string */
    if (connection_string &&
        (connection_length > share->connect_string.length ||
         (connection_length < share->connect_string.length &&
          (share->connect_string.str[connection_length] != '/' &&
           share->connect_string.str[connection_length] != '\\')) ||
         native_strncasecmp(connection_string, share->connect_string.str,
                            connection_length)))
      continue;

    /* close_cached_tables() only uses these elements */
    tmp.db = share->db.str;
    tmp.table_name = share->table_name.str;
    tmp.next_local = tables;

    tables = new (thd->mem_root) TABLE_LIST(tmp);
  }
  mysql_mutex_unlock(&LOCK_open);

  if (tables)
    result = close_cached_tables_nsec(thd, tables, false, LONG_TIMEOUT_NSEC);

  return result;
}

void Server_options::reset() {
  m_server_name.str = nullptr;
  m_server_name.length = 0;
  m_port = PORT_NOT_SET;
  m_host.str = nullptr;
  m_host.length = 0;
  m_db.str = nullptr;
  m_db.length = 0;
  m_username.str = nullptr;
  m_db.length = 0;
  m_password.str = nullptr;
  m_password.length = 0;
  m_scheme.str = nullptr;
  m_scheme.length = 0;
  m_socket.str = nullptr;
  m_socket.length = 0;
  m_owner.str = nullptr;
  m_owner.length = 0;
}

bool Server_options::insert_into_cache() const {
  char *unset_ptr = const_cast<char *>("");
  DBUG_TRACE;

  FOREIGN_SERVER *server = new (&mem) FOREIGN_SERVER();
  if (!server) return true;

  /* these two MUST be set */
  if (!(server->server_name = strdup_root(&mem, m_server_name.str)))
    return true;
  server->server_name_length = m_server_name.length;

  if (!(server->host = m_host.str ? strdup_root(&mem, m_host.str) : unset_ptr))
    return true;

  if (!(server->db = m_db.str ? strdup_root(&mem, m_db.str) : unset_ptr))
    return true;

  if (!(server->username =
            m_username.str ? strdup_root(&mem, m_username.str) : unset_ptr))
    return true;

  if (!(server->password =
            m_password.str ? strdup_root(&mem, m_password.str) : unset_ptr))
    return true;

  /* set to 0 if not specified */
  server->port = m_port != PORT_NOT_SET ? m_port : 0;

  if (!(server->socket =
            m_socket.str ? strdup_root(&mem, m_socket.str) : unset_ptr))
    return true;

  if (!(server->scheme =
            m_scheme.str ? strdup_root(&mem, m_scheme.str) : unset_ptr))
    return true;

  if (!(server->owner =
            m_owner.str ? strdup_root(&mem, m_owner.str) : unset_ptr))
    return true;

  servers_cache->emplace(
      std::string(server->server_name, server->server_name_length), server);
  return false;
}

bool Server_options::update_cache(FOREIGN_SERVER *existing) const {
  DBUG_TRACE;

  /*
    Note: Since the name can't change, we don't need to set it.
    This also means we can just update the existing cache entry.
  */

  /*
    The logic here is this: is this value set AND is it different
    than the existing value?
  */
  if (m_host.str && strcmp(m_host.str, existing->host) &&
      !(existing->host = strdup_root(&mem, m_host.str)))
    return true;

  if (m_db.str && strcmp(m_db.str, existing->db) &&
      !(existing->db = strdup_root(&mem, m_db.str)))
    return true;

  if (m_username.str && strcmp(m_username.str, existing->username) &&
      !(existing->username = strdup_root(&mem, m_username.str)))
    return true;

  if (m_password.str && strcmp(m_password.str, existing->password) &&
      !(existing->password = strdup_root(&mem, m_password.str)))
    return true;

  /*
    port is initialised to PORT_NOT_SET, so if unset, it will be -1
  */
  if (m_port != PORT_NOT_SET && m_port != existing->port)
    existing->port = m_port;

  if (m_socket.str && strcmp(m_socket.str, existing->socket) &&
      !(existing->socket = strdup_root(&mem, m_socket.str)))
    return true;

  if (m_scheme.str && strcmp(m_scheme.str, existing->scheme) &&
      !(existing->scheme = strdup_root(&mem, m_scheme.str)))
    return true;

  if (m_owner.str && strcmp(m_owner.str, existing->owner) &&
      !(existing->owner = strdup_root(&mem, m_owner.str)))
    return true;

  return false;
}

/**
   Helper function for creating a record for inserting
   a new server into the mysql.servers table.

   Set a field to the given parser string. If the parser
   string is empty, set the field to "" instead.
*/

static inline void store_new_field(TABLE *table, enum_servers_table_field field,
                                   const LEX_STRING *val) {
  if (val->str)
    table->field[field]->store(val->str, val->length, system_charset_info);
  else
    table->field[field]->store("", 0U, system_charset_info);
}

void Server_options::store_new_server(TABLE *table) const {
  store_new_field(table, SERVERS_FIELD_HOST, &m_host);
  store_new_field(table, SERVERS_FIELD_DB, &m_db);
  store_new_field(table, SERVERS_FIELD_USERNAME, &m_username);
  store_new_field(table, SERVERS_FIELD_PASSWORD, &m_password);

  if (m_port != PORT_NOT_SET)
    table->field[SERVERS_FIELD_PORT]->store(m_port);
  else
    table->field[SERVERS_FIELD_PORT]->store(0);

  store_new_field(table, SERVERS_FIELD_SOCKET, &m_socket);
  store_new_field(table, SERVERS_FIELD_SCHEME, &m_scheme);
  store_new_field(table, SERVERS_FIELD_OWNER, &m_owner);
}

/**
   Helper function for creating a record for updating
   an existing server in the mysql.servers table.

   Set a field to the given parser string unless
   the parser string is empty or equal to the existing value.
*/

static inline void store_updated_field(TABLE *table,
                                       enum_servers_table_field field,
                                       const char *existing_val,
                                       const LEX_STRING *new_val) {
  if (new_val->str && strcmp(new_val->str, existing_val))
    table->field[field]->store(new_val->str, new_val->length,
                               system_charset_info);
}

void Server_options::store_altered_server(TABLE *table,
                                          FOREIGN_SERVER *existing) const {
  store_updated_field(table, SERVERS_FIELD_HOST, existing->host, &m_host);
  store_updated_field(table, SERVERS_FIELD_DB, existing->db, &m_db);
  store_updated_field(table, SERVERS_FIELD_USERNAME, existing->username,
                      &m_username);
  store_updated_field(table, SERVERS_FIELD_PASSWORD, existing->password,
                      &m_password);

  if (m_port != PORT_NOT_SET && m_port != existing->port)
    table->field[SERVERS_FIELD_PORT]->store(m_port);

  store_updated_field(table, SERVERS_FIELD_SOCKET, existing->socket, &m_socket);
  store_updated_field(table, SERVERS_FIELD_SCHEME, existing->scheme, &m_scheme);
  store_updated_field(table, SERVERS_FIELD_OWNER, existing->owner, &m_owner);
}

bool Sql_cmd_common_server::check_and_open_table(THD *thd) {
  if (check_global_access(thd, SUPER_ACL) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  TABLE_LIST tables("mysql", "servers", TL_WRITE);

  table = open_ltable(thd, &tables, TL_WRITE, MYSQL_LOCK_IGNORE_TIMEOUT);
  return (table == nullptr);
}

bool Sql_cmd_create_server::execute(THD *thd) {
  DBUG_TRACE;

  if (Sql_cmd_common_server::check_and_open_table(thd)) return true;

  // Check for existing cache entries with same name
  mysql_rwlock_wrlock(&THR_LOCK_servers);
  const auto it =
      servers_cache->find(to_string(m_server_options->m_server_name));
  if (it != servers_cache->end()) {
    mysql_rwlock_unlock(&THR_LOCK_servers);
    my_error(ER_FOREIGN_SERVER_EXISTS, MYF(0),
             m_server_options->m_server_name.str);
    trans_rollback_stmt(thd);
    close_mysql_tables(thd);
    return true;
  }

  int error;
  {
    Disable_binlog_guard binlog_guard(thd);
    table->use_all_columns();
    empty_record(table);

    /* set the field that's the PK to the value we're looking for */
    table->field[SERVERS_FIELD_NAME]->store(
        m_server_options->m_server_name.str,
        m_server_options->m_server_name.length, system_charset_info);

    /* read index until record is that specified in server_name */
    error = table->file->ha_index_read_idx_map(
        table->record[0], 0, table->field[SERVERS_FIELD_NAME]->ptr,
        HA_WHOLE_KEY, HA_READ_KEY_EXACT);

    if (!error) {
      my_error(ER_FOREIGN_SERVER_EXISTS, MYF(0),
               m_server_options->m_server_name.str);
      error = 1;
    } else if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
      /* if not found, err */
      table->file->print_error(error, MYF(0));
    } else {
      /* store each field to be inserted */
      m_server_options->store_new_server(table);

      /* write/insert the new server */
      if ((error = table->file->ha_write_row(table->record[0])))
        table->file->print_error(error, MYF(0));
      else {
        /* insert the server into the cache */
        if ((error = m_server_options->insert_into_cache()))
          my_error(ER_OUT_OF_RESOURCES, MYF(0));
      }
    }
  }

  mysql_rwlock_unlock(&THR_LOCK_servers);

  if (error)
    trans_rollback_stmt(thd);
  else
    trans_commit_stmt(thd);
  close_mysql_tables(thd);

  if (error == 0 && !thd->killed) my_ok(thd, 1);
  return error != 0 || thd->killed;
}

bool Sql_cmd_alter_server::execute(THD *thd) {
  DBUG_TRACE;

  if (Sql_cmd_common_server::check_and_open_table(thd)) return true;

  // Find existing cache entry to update
  mysql_rwlock_wrlock(&THR_LOCK_servers);
  const auto it =
      servers_cache->find(to_string(m_server_options->m_server_name));
  if (it == servers_cache->end()) {
    my_error(ER_FOREIGN_SERVER_DOESNT_EXIST, MYF(0),
             m_server_options->m_server_name.str);
    mysql_rwlock_unlock(&THR_LOCK_servers);
    trans_rollback_stmt(thd);
    close_mysql_tables(thd);
    return true;
  }

  FOREIGN_SERVER *existing = it->second;

  int error;
  {
    Disable_binlog_guard binlog_guard(table->in_use);
    table->use_all_columns();

    /* set the field that's the PK to the value we're looking for */
    table->field[SERVERS_FIELD_NAME]->store(
        m_server_options->m_server_name.str,
        m_server_options->m_server_name.length, system_charset_info);

    error = table->file->ha_index_read_idx_map(
        table->record[0], 0, table->field[SERVERS_FIELD_NAME]->ptr,
        ~(longlong)0, HA_READ_KEY_EXACT);
    if (error) {
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE)
        table->file->print_error(error, MYF(0));
      else
        my_error(ER_FOREIGN_SERVER_DOESNT_EXIST, MYF(0),
                 m_server_options->m_server_name.str);
    } else {
      /* ok, so we can update since the record exists in the table */
      store_record(table, record[1]);
      m_server_options->store_altered_server(table, existing);
      if ((error = table->file->ha_update_row(table->record[1],
                                              table->record[0])) &&
          error != HA_ERR_RECORD_IS_THE_SAME)
        table->file->print_error(error, MYF(0));
      else {
        // Update cache entry
        if ((error = m_server_options->update_cache(existing)))
          my_error(ER_OUT_OF_RESOURCES, MYF(0));
      }
    }
  }

  /* Perform a reload so we don't have a 'hole' in our mem_root */
  servers_load(thd, table);

  // NOTE: servers_load() must be called under acquired THR_LOCK_servers.
  mysql_rwlock_unlock(&THR_LOCK_servers);

  if (error)
    trans_rollback_stmt(thd);
  else
    trans_commit_stmt(thd);
  close_mysql_tables(thd);

  if (close_cached_connection_tables(thd, m_server_options->m_server_name.str,
                                     m_server_options->m_server_name.length)) {
    push_warning(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                 "Server connection in use");
  }

  if (error == 0 && !thd->killed) my_ok(thd, 1);
  return error != 0 || thd->killed;
}

bool Sql_cmd_drop_server::execute(THD *thd) {
  DBUG_TRACE;

  if (Sql_cmd_common_server::check_and_open_table(thd)) return true;

  int error;
  mysql_rwlock_wrlock(&THR_LOCK_servers);
  {
    Disable_binlog_guard binlog_guard(table->in_use);
    table->use_all_columns();

    /* set the field that's the PK to the value we're looking for */
    table->field[SERVERS_FIELD_NAME]->store(
        m_server_name.str, m_server_name.length, system_charset_info);

    error = table->file->ha_index_read_idx_map(
        table->record[0], 0, table->field[SERVERS_FIELD_NAME]->ptr,
        HA_WHOLE_KEY, HA_READ_KEY_EXACT);
    if (error) {
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE)
        table->file->print_error(error, MYF(0));
      else if (!m_if_exists)
        my_error(ER_FOREIGN_SERVER_DOESNT_EXIST, MYF(0), m_server_name.str);
      else
        error = 0;  // Reset error - we will report my_ok() in this case.
    } else {
      // Delete from table
      if ((error = table->file->ha_delete_row(table->record[0])))
        table->file->print_error(error, MYF(0));
      else {
        // Remove from cache
        size_t num_erased = servers_cache->erase(to_string(m_server_name));
        if (num_erased == 0 && !m_if_exists) {
          my_error(ER_FOREIGN_SERVER_DOESNT_EXIST, MYF(0), m_server_name.str);
          error = 1;
        }
      }
    }
  }

  mysql_rwlock_unlock(&THR_LOCK_servers);

  if (error)
    trans_rollback_stmt(thd);
  else
    trans_commit_stmt(thd);
  close_mysql_tables(thd);

  if (close_cached_connection_tables(thd, m_server_name.str,
                                     m_server_name.length)) {
    push_warning(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                 "Server connection in use");
  }

  if (error == 0 && !thd->killed) my_ok(thd, 1);
  return error != 0 || thd->killed;
}

void servers_free(bool end) {
  DBUG_TRACE;
  if (servers_cache == nullptr) return;
  if (!end) {
    free_root(&mem, MYF(MY_MARK_BLOCKS_FREE));
    servers_cache->clear();
    return;
  }
  mysql_rwlock_destroy(&THR_LOCK_servers);
  free_root(&mem, MYF(0));
  delete servers_cache;
  servers_cache = nullptr;
}

/*
  SYNOPSIS

  clone_server(MEM_ROOT *mem_root, FOREIGN_SERVER *orig, FOREIGN_SERVER *buff)

  Create a clone of FOREIGN_SERVER. If the supplied mem_root is of
  thd->mem_root then the copy is automatically disposed at end of statement.

  NOTES

  ARGS
   MEM_ROOT pointer (strings are copied into this mem root)
   FOREIGN_SERVER pointer (made a copy of)
   FOREIGN_SERVER buffer (if not-NULL, this pointer is returned)

  RETURN VALUE
   FOREIGN_SEVER pointer (copy of one supplied FOREIGN_SERVER)
*/

static FOREIGN_SERVER *clone_server(MEM_ROOT *mem, const FOREIGN_SERVER *server,
                                    FOREIGN_SERVER *buffer) {
  DBUG_TRACE;

  if (!buffer) buffer = new (mem) FOREIGN_SERVER();

  buffer->server_name =
      strmake_root(mem, server->server_name, server->server_name_length);
  buffer->port = server->port;
  buffer->server_name_length = server->server_name_length;

  /* TODO: We need to examine which of these can really be NULL */
  buffer->db = server->db ? strdup_root(mem, server->db) : nullptr;
  buffer->scheme = server->scheme ? strdup_root(mem, server->scheme) : nullptr;
  buffer->username =
      server->username ? strdup_root(mem, server->username) : nullptr;
  buffer->password =
      server->password ? strdup_root(mem, server->password) : nullptr;
  buffer->socket = server->socket ? strdup_root(mem, server->socket) : nullptr;
  buffer->owner = server->owner ? strdup_root(mem, server->owner) : nullptr;
  buffer->host = server->host ? strdup_root(mem, server->host) : nullptr;

  return buffer;
}

FOREIGN_SERVER *get_server_by_name(MEM_ROOT *mem, const char *server_name,
                                   FOREIGN_SERVER *buff) {
  size_t server_name_length;
  FOREIGN_SERVER *server;
  DBUG_TRACE;
  DBUG_PRINT("info", ("server_name %s", server_name));

  server_name_length = strlen(server_name);

  if (!server_name || !strlen(server_name)) {
    DBUG_PRINT("info", ("server_name not defined!"));
    return (FOREIGN_SERVER *)nullptr;
  }

  DBUG_PRINT("info", ("locking servers_cache"));
  mysql_rwlock_rdlock(&THR_LOCK_servers);
  const auto it =
      servers_cache->find(std::string(server_name, server_name_length));
  if (it == servers_cache->end()) {
    DBUG_PRINT("info", ("server_name %s length %u not found!", server_name,
                        (unsigned)server_name_length));
    server = (FOREIGN_SERVER *)nullptr;
  }
  /* otherwise, make copy of server */
  else
    server = clone_server(mem, it->second, buff);

  DBUG_PRINT("info", ("unlocking servers_cache"));
  mysql_rwlock_unlock(&THR_LOCK_servers);
  return server;
}
