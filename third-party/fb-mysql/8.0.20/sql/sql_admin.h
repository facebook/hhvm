/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_TABLE_MAINTENANCE_H
#define SQL_TABLE_MAINTENANCE_H

#include <stddef.h>
#include <set>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "sql/histograms/histogram.h"
#include "sql/mem_root_allocator.h"
#include "sql/sql_cmd.h"            // Sql_cmd
#include "sql/sql_cmd_ddl_table.h"  // Sql_cmd_ddl_table
#include "sql/sql_plugin_ref.h"

class Clone_handler;
class String;
class THD;

struct TABLE_LIST;
template <class T>
class List;

struct KEY_CACHE;
struct LEX_USER;

struct Column_name_comparator {
  bool operator()(const String *lhs, const String *rhs) const;
};

/* Must be able to hold ALTER TABLE t PARTITION BY ... KEY ALGORITHM = 1 ... */
#define SQL_ADMIN_MSG_TEXT_SIZE 128 * 1024

/**
  Sql_cmd_analyze_table represents the ANALYZE TABLE statement.

  Also this class is a base class for Sql_cmd_alter_table_analyze_partition
  which represents the ALTER TABLE ... ANALYZE PARTITION statement.
*/
class Sql_cmd_analyze_table : public Sql_cmd_ddl_table {
 public:
  /**
    Specifies which (if any) of the commands UPDATE HISTOGRAM or DROP HISTOGRAM
    that is specified after ANALYZE TABLE tbl.
  */
  enum class Histogram_command {
    NONE,              ///< Neither UPDATE or DROP histogram is specified
    UPDATE_HISTOGRAM,  ///< UPDATE HISTOGRAM ... is specified after ANALYZE
                       ///< TABLE
    DROP_HISTOGRAM     ///< DROP HISTOGRAM ... is specified after ANALYZE TABLE
  };

  /**
    Constructor, used to represent a ANALYZE TABLE statement.
  */
  Sql_cmd_analyze_table(THD *thd, Alter_info *alter_info,
                        Histogram_command histogram_command,
                        int histogram_buckets);

  bool execute(THD *thd) override;

  enum_sql_command sql_command_code() const override { return SQLCOM_ANALYZE; }

  /**
    Set which fields to (try and) create/update or delete histogram statistics
    for.
  */
  bool set_histogram_fields(List<String> *fields);

 private:
  using columns_set =
      std::set<String *, Column_name_comparator, Mem_root_allocator<String *>>;

  /// Which histogram command (if any) is specified
  Histogram_command m_histogram_command = Histogram_command::NONE;

  /// The fields specified by the user in UPDATE/DROP HISTOGRAM
  columns_set m_histogram_fields;

  /// The number of buckets specified by the user in UPDATE HISTOGRAM
  int m_histogram_buckets;

  /// @return The histogram command specified, if any.
  Histogram_command get_histogram_command() const {
    return m_histogram_command;
  }

  /// @return The number of buckets specified in UPDATE HISTOGRAM.
  int get_histogram_buckets() const { return m_histogram_buckets; }

  /// @return The fields specified in UPDATE/DROP HISTOGRAM
  const columns_set &get_histogram_fields() const { return m_histogram_fields; }

  /**
    Send the result of histogram operations back to the client as a result set.

    @param thd      Thread handle.
    @param results  The messages to send back to the client.
    @param table    The table the operations was performed on.

    @return false on success, true otherwise.
  */
  bool send_histogram_results(THD *thd, const histograms::results_map &results,
                              const TABLE_LIST *table);

  /**
    Update one or more histograms

    This is invoked by running the command "ANALYZE TABLE tbl UPDATE HISTOGRAM
    ON col1, col2 WITH n BUCKETS". Note that the function expects exactly one
    table to be specified, but multiple columns can be specified.

    @param thd Thread handler.
    @param table The table specified in ANALYZE TABLE
    @param results A map where the results of the operations will be stored.

    @return false on success, true on error.
  */
  bool update_histogram(THD *thd, TABLE_LIST *table,
                        histograms::results_map &results);

  /**
    Drops one or more histograms

    This is invoked by running the command "ANALYZE TABLE tbl DROP HISTOGRAM ON
    col1, col2;". Note that the function expects exactly one table to be
    specified, but multiple columns can be specified.

    @param thd Thread handler.
    @param table The table specified in ANALYZE TABLE
    @param results A map where the results of the operations will be stored.

    @return false on success, true on error.
  */
  bool drop_histogram(THD *thd, TABLE_LIST *table,
                      histograms::results_map &results);

  bool handle_histogram_command(THD *thd, TABLE_LIST *table);
};

/**
  Sql_cmd_check_table represents the CHECK TABLE statement.

  Also this is a base class of Sql_cmd_alter_table_check_partition which
  represents the ALTER TABLE ... CHECK PARTITION statement.
*/
class Sql_cmd_check_table : public Sql_cmd_ddl_table {
 public:
  using Sql_cmd_ddl_table::Sql_cmd_ddl_table;

  bool execute(THD *thd);

  virtual enum_sql_command sql_command_code() const { return SQLCOM_CHECK; }
};

/**
  Sql_cmd_optimize_table represents the OPTIMIZE TABLE statement.

  Also this is a base class of Sql_cmd_alter_table_optimize_partition.
  represents the ALTER TABLE ... CHECK PARTITION statement.
*/
class Sql_cmd_optimize_table : public Sql_cmd_ddl_table {
 public:
  using Sql_cmd_ddl_table::Sql_cmd_ddl_table;

  bool execute(THD *thd);

  virtual enum_sql_command sql_command_code() const { return SQLCOM_OPTIMIZE; }
};

/**
  Sql_cmd_repair_table represents the REPAIR TABLE statement.

  Also this is a base class of Sql_cmd_alter_table_repair_partition which
  represents the ALTER TABLE ... REPAIR PARTITION statement.
*/
class Sql_cmd_repair_table : public Sql_cmd_ddl_table {
 public:
  using Sql_cmd_ddl_table::Sql_cmd_ddl_table;

  bool execute(THD *thd);

  virtual enum_sql_command sql_command_code() const { return SQLCOM_REPAIR; }
};

/**
  Sql_cmd_shutdown represents the SHUTDOWN statement.
*/
class Sql_cmd_shutdown : public Sql_cmd {
 public:
  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const { return SQLCOM_SHUTDOWN; }
};

enum class role_enum { ROLE_NONE, ROLE_DEFAULT, ROLE_ALL, ROLE_NAME };

/**
  Sql_cmd_set_role represetns the SET ROLE ... statement.
*/
class Sql_cmd_set_role : public Sql_cmd {
  friend class PT_set_role;

  const role_enum role_type;
  const List<LEX_USER> *role_list;
  const List<LEX_USER> *except_roles;

 public:
  Sql_cmd_set_role(role_enum role_type_arg,
                   const List<LEX_USER> *except_roles_arg)
      : role_type(role_type_arg),
        role_list(nullptr),
        except_roles(except_roles_arg) {
    DBUG_ASSERT(role_type == role_enum::ROLE_NONE ||
                role_type == role_enum::ROLE_DEFAULT ||
                role_type == role_enum::ROLE_ALL);
    DBUG_ASSERT(role_type == role_enum::ROLE_ALL || except_roles == nullptr);
  }
  explicit Sql_cmd_set_role(const List<LEX_USER> *role_arg)
      : role_type(role_enum::ROLE_NAME), role_list(role_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const { return SQLCOM_SET_ROLE; }
};

/**
  Sql_cmd_create_role represetns the CREATE ROLE ... statement.
*/
class Sql_cmd_create_role : public Sql_cmd {
  friend class PT_create_role;

  const bool if_not_exists;
  const List<LEX_USER> *roles;

 public:
  explicit Sql_cmd_create_role(bool if_not_exists_arg,
                               const List<LEX_USER> *roles_arg)
      : if_not_exists(if_not_exists_arg), roles(roles_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_CREATE_ROLE;
  }
};

/**
  Sql_cmd_drop_role represetns the DROP ROLE ... statement.
*/
class Sql_cmd_drop_role : public Sql_cmd {
  friend class PT_drop_role;

  bool ignore_errors;
  const List<LEX_USER> *roles;

 public:
  explicit Sql_cmd_drop_role(bool ignore_errors_arg,
                             const List<LEX_USER> *roles_arg)
      : ignore_errors(ignore_errors_arg), roles(roles_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const { return SQLCOM_DROP_ROLE; }
};

/**
  Sql_cmd_grant_roles represents the GRANT role-list TO ... statement.
*/
class Sql_cmd_grant_roles : public Sql_cmd {
  const List<LEX_USER> *roles;
  const List<LEX_USER> *users;
  const bool with_admin_option;

 public:
  explicit Sql_cmd_grant_roles(const List<LEX_USER> *roles_arg,
                               const List<LEX_USER> *users_arg,
                               bool with_admin_option_arg)
      : roles(roles_arg),
        users(users_arg),
        with_admin_option(with_admin_option_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_GRANT_ROLE;
  }
};

/**
  Sql_cmd_revoke_roles represents the REVOKE [role list] TO ... statement.
*/
class Sql_cmd_revoke_roles : public Sql_cmd {
  const List<LEX_USER> *roles;
  const List<LEX_USER> *users;

 public:
  explicit Sql_cmd_revoke_roles(const List<LEX_USER> *roles_arg,
                                const List<LEX_USER> *users_arg)
      : roles(roles_arg), users(users_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_REVOKE_ROLE;
  }
};

/**
  Sql_cmd_alter_user_default_role ALTER USER ... DEFAULT ROLE ... statement.
*/
class Sql_cmd_alter_user_default_role : public Sql_cmd {
  friend class PT_alter_user_default_role;

  const bool if_exists;
  const List<LEX_USER> *users;
  const List<LEX_USER> *roles;
  const role_enum role_type;

 public:
  explicit Sql_cmd_alter_user_default_role(bool if_exists_arg,
                                           const List<LEX_USER> *users_arg,
                                           const List<LEX_USER> *roles_arg,
                                           const role_enum role_type_arg)
      : if_exists(if_exists_arg),
        users(users_arg),
        roles(roles_arg),
        role_type(role_type_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_ALTER_USER_DEFAULT_ROLE;
  }
};

/**
  Sql_cmd_show_grants SHOW GRANTS ... statement.
*/
class Sql_cmd_show_grants : public Sql_cmd {
  friend class PT_show_grants;

  const LEX_USER *for_user;
  const List<LEX_USER> *using_users;

 public:
  Sql_cmd_show_grants(const LEX_USER *for_user_arg,
                      const List<LEX_USER> *using_users_arg)
      : for_user(for_user_arg), using_users(using_users_arg) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_SHOW_GRANTS;
  }
};

enum alter_instance_action_enum {
  ROTATE_INNODB_MASTER_KEY,
  ALTER_INSTANCE_RELOAD_TLS,
  ALTER_INSTANCE_RELOAD_TLS_ROLLBACK_ON_ERROR,
  ROTATE_BINLOG_MASTER_KEY,
  LAST_MASTER_KEY /* Add new master key type before this */
};

/**
  Sql_cmd_alter_instance represents the ROTATE alter_instance_action MASTER KEY
  statement.
*/
class Alter_instance;

class Sql_cmd_alter_instance : public Sql_cmd {
  friend class PT_alter_instance;
  const enum alter_instance_action_enum alter_instance_action;
  Alter_instance *alter_instance;

 public:
  explicit Sql_cmd_alter_instance(
      enum alter_instance_action_enum alter_instance_action_arg)
      : alter_instance_action(alter_instance_action_arg),
        alter_instance(nullptr) {}

  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_ALTER_INSTANCE;
  }
};

/**
  Sql_cmd_clone implements CLONE ... statement.
*/

class Sql_cmd_clone : public Sql_cmd {
 public:
  /** Construct clone command for clone server */
  explicit Sql_cmd_clone()
      : m_host(),
        m_port(),
        m_user(),
        m_passwd(),
        m_data_dir(),
        m_clone(),
        m_is_local(false) {}

  /** Construct clone command for clone client
  @param[in]  user_info user, password and remote host information
  @param[in]  port port for remote server
  @param[in]  data_dir data directory to clone */
  explicit Sql_cmd_clone(LEX_USER *user_info, ulong port, LEX_CSTRING data_dir);

  /** Construct clone command for local clone
  @param[in]  data_dir data directory to clone */
  explicit Sql_cmd_clone(LEX_CSTRING data_dir)
      : m_host(),
        m_port(),
        m_user(),
        m_passwd(),
        m_data_dir(data_dir),
        m_clone(),
        m_is_local(true) {}

  virtual enum_sql_command sql_command_code() const { return SQLCOM_CLONE; }

  virtual bool execute(THD *thd);

  /** Execute clone server.
  @param[in] thd server session
  @return true, if error */
  bool execute_server(THD *thd);

  /** Load clone plugin for clone server.
  @param[in] thd server session
  @return true, if error */
  bool load(THD *thd);

  /** Re-write clone statement to hide password.
  @param[in,out] thd server session
  @param[in,out] rlb the buffer to return the rewritten query in. empty if none.
  @return true iff query is re-written */
  bool rewrite(THD *thd, String &rlb);

  /** @return true, if it is local clone command */
  bool is_local() const { return (m_is_local); }

 private:
  /** Remote server IP */
  LEX_CSTRING m_host;

  /** Remote server port */
  const ulong m_port;

  /** User name for remote connection */
  LEX_CSTRING m_user;

  /** Password for remote connection */
  LEX_CSTRING m_passwd;

  /** Data directory for cloned data */
  LEX_CSTRING m_data_dir;

  /** Clone handle in server */
  Clone_handler *m_clone;

  /** Loaded clone plugin reference */
  plugin_ref m_plugin;

  /** If it is local clone operation */
  bool m_is_local;
};

/**
  Sql_cmd_show represents the SHOW COLUMNS/SHOW INDEX statements.
*/
class Sql_cmd_show : public Sql_cmd {
 public:
  Sql_cmd_show(enum_sql_command sql_command) : m_sql_command(sql_command) {}
  virtual bool execute(THD *thd);
  virtual enum_sql_command sql_command_code() const { return m_sql_command; }
  virtual bool prepare(THD *thd);

 private:
  enum_sql_command m_sql_command;
};
#endif
