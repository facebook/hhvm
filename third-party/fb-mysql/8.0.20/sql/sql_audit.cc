/* Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_audit.h"

#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auto_thd.h"  // Auto_THD
#include "sql/current_thd.h"
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/log.h"
#include "sql/mysqld.h"     // sql_statement_names
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_plugin.h"  // my_plugin_foreach
#include "sql/sql_plugin_ref.h"
#include "sql/sql_rewrite.h"  // mysql_rewrite_query
#include "sql/table.h"
#include "sql_string.h"
#include "thr_mutex.h"

/**
  @class Audit_error_handler

  Error handler that controls error reporting by plugin.
*/
class Audit_error_handler : public Internal_error_handler {
 private:
  /**
    @brief Blocked copy constructor (private).
  */
  Audit_error_handler(const Audit_error_handler &obj MY_ATTRIBUTE((unused)))
      : m_thd(nullptr),
        m_warning_message(nullptr),
        m_error_reported(false),
        m_active(false) {}

 public:
  /**
    @brief Construction.

    @param thd            Current thread data.
    @param warning_message Warning message used when error has been
                               suppressed.
    @param active              Specifies whether the handler is active or not.
                               Optional parameter (default is true).
  */
  Audit_error_handler(THD *thd, const char *warning_message, bool active = true)
      : m_thd(thd),
        m_warning_message(warning_message),
        m_error_reported(false),
        m_active(active) {
    if (m_active) {
      /* Activate the error handler. */
      m_thd->push_internal_handler(this);
    }
  }

  /**
    @brief Destruction.
  */
  virtual ~Audit_error_handler() {
    if (m_active) {
      /* Deactivate this handler. */
      m_thd->pop_internal_handler();
    }
  }

  /**
    @brief Simplified custom handler.

    @returns True on error rejection, otherwise false.
  */
  virtual bool handle() = 0;

  /**
    @brief Error handler.

    @see Internal_error_handler::handle_condition

    @returns True on error rejection, otherwise false.
  */
  virtual bool handle_condition(THD *, uint sql_errno, const char *sqlstate,
                                Sql_condition::enum_severity_level *,
                                const char *msg) {
    if (m_active && handle()) {
      /* Error has been rejected. Write warning message. */
      print_warning(m_warning_message, sql_errno, sqlstate, msg);

      m_error_reported = true;

      return true;
    }

    return false;
  }

  /**
    @brief Warning print routine.

    Also prints the underlying error attributes if supplied.

    @param warn_msg  Warning message to be printed.
    @param sql_errno The error number of the underlying error
    @param sqlstate  The SQL state of the underlying error. NULL if none
    @param msg       The text of the underlying error. NULL if none
  */
  virtual void print_warning(const char *warn_msg, uint sql_errno,
                             const char *sqlstate, const char *msg) {
    LogErr(WARNING_LEVEL, ER_AUDIT_WARNING, warn_msg, sql_errno,
           sqlstate ? sqlstate : "<NO_STATE>", msg ? msg : "<NO_MESSAGE>");
  }

  /**
    @brief Convert the result value returned from the audit api.

    @param result Result value received from the plugin function.

    @returns Converted result value.
  */
  int get_result(int result) { return m_error_reported ? 0 : result; }

 private:
  /** Current thread data. */
  THD *m_thd;

  /** Warning message used when the error is rejected. */
  const char *m_warning_message;

  /** Error has been reported. */
  bool m_error_reported;

  /** Handler has been activated. */
  const bool m_active;
};

struct st_mysql_event_generic {
  mysql_event_class_t event_class;
  const void *event;
};

/**
  @struct st_mysql_subscribe_event

  Plugin event subscription structure. Used during acquisition of the plugins
  into user session.
*/
struct st_mysql_subscribe_event {
  /*
    Event class.
  */
  mysql_event_class_t event_class;
  /*
    Event subclass.
  */
  unsigned long event_subclass;
  /*
    The array that keeps sum (OR) mask of all plugins that subscribe
    to the event specified by the event_class and event_subclass.

    lookup_mask is acquired during build_lookup_mask call.
  */
  unsigned long lookup_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];
  /*
    The array that keeps sum (OR) mask of all plugins that are acquired
    to the current session as a result of acquire_plugins call.

    subscribed_mask is acquired during acquisition of the plugins
    (acquire_plugins call).
  */
  unsigned long subscribed_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];
  /*
    The array that keeps sum (OR) mask of all plugins that were not acquired
    to the current session as a result of acquire_plugins call.
  */
  unsigned long not_subscribed_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];
};

unsigned long mysql_global_audit_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];

static mysql_mutex_t LOCK_audit_mask;

static int event_class_dispatch(THD *thd, mysql_event_class_t event_class,
                                const void *event);

static int event_class_dispatch_error(THD *thd, mysql_event_class_t event_class,
                                      const char *event_name,
                                      const void *event);

/**
  Add mask specified by the rhs parameter to the mask parameter.

  @param mask Mask, to which rhs mask is to be added.
  @param rhs  Mask to be added to mask parameter.
*/
static inline void add_audit_mask(unsigned long *mask, unsigned long rhs) {
  *mask |= rhs;
}

/**
  Add entire audit mask specified by the src to dst.

  @param dst Destination mask array pointer.
  @param src Source mask array pointer.
*/
static inline void add_audit_mask(unsigned long *dst,
                                  const unsigned long *src) {
  int i;
  for (i = MYSQL_AUDIT_GENERAL_CLASS; i < MYSQL_AUDIT_CLASS_MASK_SIZE; i++)
    add_audit_mask(dst++, *src++);
}

/**
  Check, whether masks specified by lhs parameter and rhs parameters overlap.

  @param lhs First mask to check.
  @param rhs Second mask to check.

  @return false, when masks overlap, otherwise true.
*/
static inline bool check_audit_mask(const unsigned long lhs,
                                    const unsigned long rhs) {
  return !(lhs & rhs);
}

/**
  Check, whether mask arrays specified by the lhs parameter and rhs parameter
  overlap.

  @param lhs First mask array to check.
  @param rhs Second mask array to check.

  @return false, when mask array overlap, otherwise true.
*/
static inline bool check_audit_mask(const unsigned long *lhs,
                                    const unsigned long *rhs) {
  int i;
  for (i = MYSQL_AUDIT_GENERAL_CLASS; i < MYSQL_AUDIT_CLASS_MASK_SIZE; i++)
    if (!check_audit_mask(*lhs++, *rhs++)) return false;

  return true;
}

/**
  Fill query info extracted from the thread object and return
  the thread object charset info.

  @param[in]  thd     Thread data.
  @param[out] query   SQL query text.

  @return SQL query charset.
*/
inline const CHARSET_INFO *thd_get_audit_query(THD *thd,
                                               MYSQL_LEX_CSTRING *query) {
  /*
    If we haven't tried to rewrite the query to obfuscate passwords
    etc. yet, do so now.
  */
  if (thd->rewritten_query().length() == 0) mysql_rewrite_query(thd);

  /*
    If there was something to rewrite, use the rewritten query;
    otherwise, just use the original as submitted by the client.
  */
  if (thd->rewritten_query().length() > 0) {
    query->str = thd->rewritten_query().ptr();
    query->length = thd->rewritten_query().length();
    return thd->rewritten_query().charset();
  } else {
    query->str = thd->query().str;
    query->length = thd->query().length;
    return thd->charset();
  }
}

/**
  @class Ignore_event_error_handler

  Ignore all errors notified from within plugin.
*/
class Ignore_event_error_handler : public Audit_error_handler {
 public:
  /**
    @brief Construction.

    @param thd             Current thread data.
    @param event_name
  */
  Ignore_event_error_handler(THD *thd, const char *event_name)
      : Audit_error_handler(thd, ""), m_event_name(event_name) {}

  /**
    @brief Ignore all errors.

    @retval True on error rejection, otherwise false.
  */
  virtual bool handle() { return true; }

  /**
    @brief Custom warning print routine.

    Also prints the underlying error attributes if supplied.

    @param warn_msg  Warning message to be printed.
    @param sql_errno The error number of the underlying error
    @param sqlstate  The SQL state of the underlying error. NULL if none
    @param msg       The text of the underlying error. NULL if none
  */
  virtual void print_warning(const char *warn_msg MY_ATTRIBUTE((unused)),
                             uint sql_errno, const char *sqlstate,
                             const char *msg) {
    LogErr(WARNING_LEVEL, ER_AUDIT_CANT_ABORT_EVENT, m_event_name, sql_errno,
           sqlstate ? sqlstate : "<NO_STATE>", msg ? msg : "<NO_MESSAGE>");
  }

 private:
  /**
  @brief Event name used in the warning message.
  */
  const char *m_event_name;
};

int mysql_audit_notify(THD *thd, mysql_event_general_subclass_t subclass,
                       const char *subclass_name, int error_code,
                       const char *msg, size_t msg_len) {
  mysql_event_general event;

  DBUG_ASSERT(thd);

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_GENERAL_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.general_error_code = error_code;
  event.general_thread_id = thd->thread_id();

  Security_context *sctx = thd->security_context();

  event.general_user.str = sctx->user().str;
  event.general_user.length = sctx->user().str ? sctx->user().length : 0;
  event.general_ip = sctx->ip();
  event.database.str = thd->db().str;
  event.database.length = thd->db().length;
  event.query_id = thd->query_id;
  event.general_host = sctx->host();
  event.general_external_user = sctx->external_user();
  event.general_rows = thd->get_stmt_da()->current_row_for_condition();
  event.general_sql_command = sql_statement_names[thd->lex->sql_command];
  event.affected_rows = thd->get_row_count_func();
  event.port = mysqld_port;

  std::string shard_str = thd->shard_id();
  event.shard.str = shard_str.c_str();
  event.shard.length = shard_str.size();

  event.general_charset = const_cast<CHARSET_INFO *>(
      thd_get_audit_query(thd, &event.general_query));

  event.general_time = thd->query_start_in_secs();

  DBUG_EXECUTE_IF("audit_log_negative_general_error_code",
                  event.general_error_code *= -1;);

  event.general_command.str = msg;
  event.general_command.length = msg_len;

  if (subclass == MYSQL_AUDIT_GENERAL_ERROR ||
      subclass == MYSQL_AUDIT_GENERAL_STATUS ||
      subclass == MYSQL_AUDIT_GENERAL_RESULT ||
      subclass == MYSQL_AUDIT_GENERAL_WARNING_INSTR ||
      subclass == MYSQL_AUDIT_GENERAL_ERROR_INSTR) {
    Ignore_event_error_handler handler(thd, subclass_name);

    return handler.get_result(
        event_class_dispatch(thd, MYSQL_AUDIT_GENERAL_CLASS, &event));
  }

  return event_class_dispatch_error(thd, MYSQL_AUDIT_GENERAL_CLASS,
                                    subclass_name, &event);
}

int mysql_audit_notify(THD *thd, mysql_event_connection_subclass_t subclass,
                       const char *subclass_name, int errcode) {
  mysql_event_connection event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_CONNECTION_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.status = errcode;
  event.connection_id = thd->thread_id();
  event.user.str = thd->security_context()->user().str;
  event.user.length = thd->security_context()->user().length;
  event.priv_user.str = thd->security_context()->priv_user().str;
  event.priv_user.length = thd->security_context()->priv_user().length;
  event.external_user.str = thd->security_context()->external_user().str;
  event.external_user.length = thd->security_context()->external_user().length;
  event.proxy_user.str = thd->security_context()->proxy_user().str;
  event.proxy_user.length = thd->security_context()->proxy_user().length;
  event.host.str = thd->security_context()->host().str;
  event.host.length = thd->security_context()->host().length;
  event.ip.str = thd->security_context()->ip().str;
  event.ip.length = thd->security_context()->ip().length;
  event.database.str = thd->db().str;
  event.database.length = thd->db().length;
  event.connection_type = thd->get_vio_type();
  event.connection_certificate.str = thd->get_connection_certificate().c_str();
  event.connection_certificate.length =
      thd->get_connection_certificate().size();
  event.port = mysqld_port;

  std::string shard_str = thd->shard_id();
  event.shard.str = shard_str.c_str();
  event.shard.length = shard_str.size();

  if (subclass == MYSQL_AUDIT_CONNECTION_DISCONNECT) {
    Ignore_event_error_handler handler(thd, subclass_name);

    return handler.get_result(event_class_dispatch_error(
        thd, MYSQL_AUDIT_CONNECTION_CLASS, subclass_name, &event));
  }

  return event_class_dispatch_error(thd, MYSQL_AUDIT_CONNECTION_CLASS,
                                    subclass_name, &event);
}

int mysql_audit_notify(THD *thd, mysql_event_connection_subclass_t subclass,
                       const char *subclass_name) {
  return mysql_audit_notify(
      thd, subclass, subclass_name,
      thd->get_stmt_da()->is_error() ? thd->get_stmt_da()->mysql_errno() : 0);
}

int mysql_audit_notify(THD *thd, mysql_event_parse_subclass_t subclass,
                       const char *subclass_name,
                       mysql_event_parse_rewrite_plugin_flag *flags,
                       LEX_CSTRING *rewritten_query) {
  mysql_event_parse event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_PARSE_CLASS, subclass))
    return 0;

  event.event_subclass = subclass;
  event.flags = flags;
  event.query.str = thd->query().str;
  event.query.length = thd->query().length;
  event.rewritten_query = rewritten_query;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_PARSE_CLASS, subclass_name,
                                    &event);
}

/**
  Check whether the table access event for a specified table will
  be generated.

  Events for Views, table catogories other than 'SYSTEM' or 'USER' and
  temporary tables are not generated.

  @param table Table that is to be check.

  @retval true - generate event, otherwise not.
*/
inline bool generate_table_access_event(TABLE_LIST *table) {
  /* Discard views or derived tables. */
  if (table->is_view_or_derived()) return false;

  /* TRUNCATE query on Storage Engine supporting HTON_CAN_RECREATE flag. */
  if (!table->table) return true;

  /* Do not generate events, which come from PS preparation. */
  if (table->table->in_use->lex->is_ps_or_view_context_analysis()) return false;

  /* Generate event for SYSTEM and USER tables, which are not temp tables. */
  if ((table->table->s->table_category == TABLE_CATEGORY_SYSTEM ||
       table->table->s->table_category == TABLE_CATEGORY_USER) &&
      table->table->s->tmp_table == NO_TMP_TABLE)
    return true;

  return false;
}

/**
  Function that allows to use AUDIT_EVENT macro for setting subclass
  and subclass name values.

  @param [out] out_subclass      Subclass value pointer to be set.
  @param [out] out_subclass_name Subclass name pointer to be set.
  @param subclass                Subclass that sets out_subclass value.
  @param subclass_name           Subclass name that sets out_subclass_name.
*/
inline static void set_table_access_subclass(
    mysql_event_table_access_subclass_t *out_subclass,
    const char **out_subclass_name,
    mysql_event_table_access_subclass_t subclass, const char *subclass_name) {
  *out_subclass = subclass;
  *out_subclass_name = subclass_name;
}

/**
  Generate table access event for a specified table. Table is being
  verified, whether the event for this table is to be generated.

  @see generate_event

  @param thd           Current thread data.
  @param subclass      Subclass value.
  @param subclass_name Subclass name.
  @param table         Table, for which table access event is to be generated.

  @return Abort execution on 'true', otherwise continue execution.
*/
static int mysql_audit_notify(THD *thd,
                              mysql_event_table_access_subclass_t subclass,
                              const char *subclass_name, TABLE_LIST *table) {
  LEX_CSTRING str;
  mysql_event_table_access event;

  if (!generate_table_access_event(table) ||
      mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_TABLE_ACCESS_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.connection_id = thd->thread_id();
  event.sql_command_id = thd->lex->sql_command;

  event.query_charset = thd_get_audit_query(thd, &event.query);

  lex_cstring_set(&str, table->db);
  event.table_database.str = str.str;
  event.table_database.length = str.length;

  lex_cstring_set(&str, table->table_name);
  event.table_name.str = str.str;
  event.table_name.length = str.length;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_TABLE_ACCESS_CLASS,
                                    subclass_name, &event);
}

int mysql_audit_table_access_notify(THD *thd, TABLE_LIST *table) {
  mysql_event_table_access_subclass_t subclass;
  const char *subclass_name;
  int ret;

  /* Do not generate events for non query table access. */
  if (!thd->lex->query_tables) return 0;

  switch (thd->lex->sql_command) {
    case SQLCOM_REPLACE_SELECT:
    case SQLCOM_INSERT_SELECT: {
      /*
        INSERT/REPLACE SELECT generates Insert event for the first table in the
        list and Read for remaining tables.
      */
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_INSERT));

      if ((ret = mysql_audit_notify(thd, subclass, subclass_name, table)))
        return ret;

      /* Skip this table (event already generated). */
      table = table->next_global;

      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_READ));
      break;
    }
    case SQLCOM_INSERT:
    case SQLCOM_REPLACE:
    case SQLCOM_LOAD:
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_INSERT));
      break;
    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
    case SQLCOM_TRUNCATE:
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_DELETE));
      break;
    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
      /* Update state is taken from the table instance in the
         mysql_audit_notify function. */
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_UPDATE));
      break;
    case SQLCOM_SELECT:
    case SQLCOM_HA_READ:
    case SQLCOM_ANALYZE:
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_READ));
      break;
    default:
      /* Do not generate event for not supported command. */
      return 0;
  }

  for (; table; table = table->next_global) {
    /*
      Do not generate audit logs for opening DD tables when processing I_S
      queries.
    */
    if (table->referencing_view && table->referencing_view->is_system_view)
      continue;

    /*
      Update-Multi query can have several updatable tables as well as readable
      tables. This is taken from table->updating field, which holds info,
      whether table is being updated or not. table->updating holds invalid
      info, when the updatable table is referenced by a view. View status is
      taken into account in that case.
    */
    if (subclass == MYSQL_AUDIT_TABLE_ACCESS_UPDATE &&
        !table->referencing_view && !table->updating)
      set_table_access_subclass(&subclass, &subclass_name,
                                AUDIT_EVENT(MYSQL_AUDIT_TABLE_ACCESS_READ));

    if ((ret = mysql_audit_notify(thd, subclass, subclass_name, table)))
      return ret;
  }

  return 0;
}

int mysql_audit_notify(THD *thd,
                       mysql_event_global_variable_subclass_t subclass,
                       const char *subclass_name, const char *name,
                       const char *value, const unsigned int value_length) {
  mysql_event_global_variable event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.connection_id = thd->thread_id();
  event.sql_command_id = thd->lex->sql_command;

  LEX_CSTRING name_str;
  lex_cstring_set(&name_str, name);
  event.variable_name.str = name_str.str;
  event.variable_name.length = name_str.length;

  event.variable_value.str = value;
  event.variable_value.length = value_length;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS,
                                    subclass_name, &event);
}

int mysql_audit_notify(mysql_event_server_startup_subclass_t subclass,
                       const char *subclass_name, const char **argv,
                       unsigned int argc) {
  mysql_event_server_startup event;
  Auto_THD thd;

  if (mysql_audit_acquire_plugins(thd.thd, MYSQL_AUDIT_SERVER_STARTUP_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.argv = argv;
  event.argc = argc;

  return event_class_dispatch_error(thd.thd, MYSQL_AUDIT_SERVER_STARTUP_CLASS,
                                    subclass_name, &event);
}

/**
  Call audit plugins of SERVER SHUTDOWN audit class.

  @param[in] thd       Client thread info or NULL.
  @param[in] subclass  Type of the server abort audit event.
  @param[in] reason    Reason code of the shutdown.
  @param[in] exit_code Abort exit code.

  @result Value returned is not taken into consideration by the server.
*/
int mysql_audit_notify(THD *thd,
                       mysql_event_server_shutdown_subclass_t subclass,
                       mysql_server_shutdown_reason_t reason, int exit_code) {
  mysql_event_server_shutdown event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.exit_code = exit_code;
  event.reason = reason;

  return event_class_dispatch(thd, MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS, &event);
}

int mysql_audit_notify(mysql_event_server_shutdown_subclass_t subclass,
                       mysql_server_shutdown_reason_t reason, int exit_code) {
  if (error_handler_hook == my_message_sql) {
    Auto_THD thd;

    return mysql_audit_notify(thd.thd, subclass, reason, exit_code);
  }

  return mysql_audit_notify(nullptr, subclass, reason, exit_code);
}

/*
Function commented out. No Audit API calls yet.

int mysql_audit_notify(THD *thd, mysql_event_authorization_subclass_t subclass,
                       const char* subclass_name,
                       const char *database, const char *table,
                       const char *object)
{
  mysql_event_authorization event;

  mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_AUTHORIZATION_CLASS,
                              static_cast<unsigned long>(subclass));

  event.event_subclass= subclass;
  event.connection_id= thd->thread_id();
  event.sql_command_id= thd->lex->sql_command;

  event.query_charset = thd_get_audit_query(thd, &event.query);

  LEX_CSTRING obj_str;

  lex_cstring_set(&obj_str, database ? database : "");
  event.database.str= database;
  event.database.length= obj_str.length;

  lex_cstring_set(&obj_str, table ? table : "");
  event.table.str= table;
  event.table.length = obj_str.length;

  lex_cstring_set(&obj_str, object ? object : "");
  event.object.str= object;
  event.object.length= obj_str.length;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_AUTHORIZATION_CLASS,
                                    subclass_name, &event);
}
*/

/**
  @class Ignore_command_start_error_handler

  Ignore error for specified commands.
*/
class Ignore_command_start_error_handler : public Audit_error_handler {
 public:
  /**
    @brief Construction.

    @param thd     Current thread data.
    @param command Current command that the handler will be active against.
    @param command_text
  */
  Ignore_command_start_error_handler(THD *thd, enum_server_command command,
                                     const char *command_text)
      : Audit_error_handler(thd, "", ignore_command(command)),
        m_command(command),
        m_command_text(command_text) {}

  /**
    @brief Error for specified command handling routine.

    @retval True on error rejection, otherwise false.
  */
  virtual bool handle() { return ignore_command(m_command); }

  /**
    @brief Custom warning print routine.

    Also prints the underlying error attributes if supplied.

    @param warn_msg  Warning message to be printed.
    @param sql_errno The error number of the underlying error
    @param sqlstate  The SQL state of the underlying error. NULL if none
    @param msg       The text of the underlying error. NULL if none
  */
  virtual void print_warning(const char *warn_msg MY_ATTRIBUTE((unused)),
                             uint sql_errno, const char *sqlstate,
                             const char *msg) {
    LogErr(WARNING_LEVEL, ER_AUDIT_CANT_ABORT_COMMAND, m_command_text,
           sql_errno, sqlstate ? sqlstate : "<NO_STATE>",
           msg ? msg : "<NO_MESSAGE>");
  }

  /**
    @brief Check whether the command is to be ignored.

    @retval True whether the command is to be ignored. Otherwise false.
  */
  static bool ignore_command(enum_server_command command) {
    /* Ignore these commands. The plugin cannot abort on these commands. */
    if (command == COM_QUIT || command == COM_PING ||
        command == COM_SLEEP || /* Deprecated commands from here. */
        command == COM_CONNECT || command == COM_TIME ||
        command == COM_DELAYED_INSERT || command == COM_END ||
        command == COM_TOP_BEGIN || command == COM_TOP_END) {
      return true;
    }

    return false;
  }

 private:
  /** Command that the handler is active against. */
  enum_server_command m_command;

  /** Command string. */
  const char *m_command_text;
};

int mysql_audit_notify(THD *thd, mysql_event_command_subclass_t subclass,
                       const char *subclass_name, enum_server_command command,
                       const char *command_text) {
  mysql_event_command event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_COMMAND_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.status =
      thd->get_stmt_da()->is_error() ? thd->get_stmt_da()->mysql_errno() : 0;
  event.connection_id = thd && thd->thread_id();
  event.command_id = command;

  if (subclass == MYSQL_AUDIT_COMMAND_START) {
    Ignore_command_start_error_handler handler(thd, command, command_text);

    return handler.get_result(event_class_dispatch_error(
        thd, MYSQL_AUDIT_COMMAND_CLASS, subclass_name, &event));
  }

  /* MYSQL_AUDIT_COMMAND_END event handling. */
  Ignore_event_error_handler handler(thd, subclass_name);

  return handler.get_result(event_class_dispatch_error(
      thd, MYSQL_AUDIT_COMMAND_CLASS, subclass_name, &event));
}

int mysql_audit_notify(THD *thd, mysql_event_query_subclass_t subclass,
                       const char *subclass_name) {
  mysql_event_query event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_QUERY_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.status =
      thd->get_stmt_da()->is_error() ? thd->get_stmt_da()->mysql_errno() : 0;
  event.connection_id = thd->thread_id();

  event.sql_command_id = thd->lex->sql_command;

  event.query_charset = thd_get_audit_query(thd, &event.query);

  return event_class_dispatch_error(thd, MYSQL_AUDIT_QUERY_CLASS, subclass_name,
                                    &event);
}

int mysql_audit_notify(THD *thd, mysql_event_stored_program_subclass_t subclass,
                       const char *subclass_name, const char *database,
                       const char *name, void *parameters) {
  mysql_event_stored_program event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_STORED_PROGRAM_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.connection_id = thd->thread_id();
  event.sql_command_id = thd->lex->sql_command;

  event.query_charset = thd_get_audit_query(thd, &event.query);

  LEX_CSTRING obj_str;

  lex_cstring_set(&obj_str, database ? database : "");
  event.database.str = obj_str.str;
  event.database.length = obj_str.length;

  lex_cstring_set(&obj_str, name ? name : "");
  event.name.str = obj_str.str;
  event.name.length = obj_str.length;

  event.parameters = parameters;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_STORED_PROGRAM_CLASS,
                                    subclass_name, &event);
}

int mysql_audit_notify(THD *thd, mysql_event_authentication_subclass_t subclass,
                       const char *subclass_name, int status, const char *user,
                       const char *host, const char *authentication_plugin,
                       bool is_role, const char *new_user,
                       const char *new_host) {
  mysql_event_authentication event;

  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_AUTHENTICATION_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  event.event_subclass = subclass;
  event.status = status;
  event.connection_id = thd->thread_id();
  event.sql_command_id = thd->lex->sql_command;

  event.query_charset = thd_get_audit_query(thd, &event.query);

  LEX_CSTRING obj_str;

  lex_cstring_set(&obj_str, user ? user : "");
  event.user.str = obj_str.str;
  event.user.length = obj_str.length;

  lex_cstring_set(&obj_str, host ? host : "");
  event.host.str = obj_str.str;
  event.host.length = obj_str.length;

  lex_cstring_set(&obj_str, authentication_plugin ? authentication_plugin : "");
  event.authentication_plugin.str = obj_str.str;
  event.authentication_plugin.length = obj_str.length;

  event.is_role = is_role;

  lex_cstring_set(&obj_str, new_user ? new_user : "");
  event.new_user.str = obj_str.str;
  event.new_user.length = obj_str.length;

  lex_cstring_set(&obj_str, new_host ? new_host : "");
  event.new_host.str = obj_str.str;
  event.new_host.length = obj_str.length;

  Ignore_event_error_handler handler(thd, subclass_name);

  return handler.get_result(event_class_dispatch_error(
      thd, MYSQL_AUDIT_AUTHENTICATION_CLASS, subclass_name, &event));
}

int mysql_audit_notify(THD *thd, mysql_event_message_subclass_t subclass,
                       const char *subclass_name, const char *component,
                       size_t component_length, const char *producer,
                       size_t producer_length, const char *message,
                       size_t message_length,
                       mysql_event_message_key_value_t *key_value_map,
                       size_t key_value_map_length) {
  if (mysql_audit_acquire_plugins(thd, MYSQL_AUDIT_MESSAGE_CLASS,
                                  static_cast<unsigned long>(subclass)))
    return 0;

  mysql_event_message event;

  event.event_subclass = subclass;
  event.component.str = component;
  event.component.length = component_length;
  event.producer.str = producer;
  event.producer.length = producer_length;
  event.message.str = message;
  event.message.length = message_length;
  event.key_value_map = key_value_map;
  event.key_value_map_length = key_value_map_length;

  return event_class_dispatch_error(thd, MYSQL_AUDIT_MESSAGE_CLASS,
                                    subclass_name, &event);
}

/**
  Acquire plugin masks subscribing to the specified event of the specified
  class, passed by arg parameter. lookup_mask of the st_mysql_subscribe_event
  structure is filled, when the plugin is interested in receiving the event.

  @param         plugin Plugin reference.
  @param[in,out] arg    Opaque st_mysql_subscribe_event pointer.

  @return false is always returned.
*/
static bool acquire_lookup_mask(THD *thd, plugin_ref plugin, void *arg) {
  st_mysql_subscribe_event *evt = static_cast<st_mysql_subscribe_event *>(arg);
  st_mysql_audit *audit = plugin_data<st_mysql_audit *>(plugin);

  MUTEX_LOCK(lock, &thd->LOCK_thd_audit_data);

  /* Check if this plugin is interested in the event */
  if (!check_audit_mask(audit->class_mask[evt->event_class],
                        evt->event_subclass))
    add_audit_mask(evt->lookup_mask, audit->class_mask);

  return false;
}

/**
  Acquire and lock any additional audit plugins, whose subscription
  mask overlaps with the lookup_mask.

  @param         thd    Current session THD.
  @param         plugin Plugin reference.
  @param[in,out] arg    Opaque st_mysql_subscribe_event pointer.

  @return This function always returns false.
*/
static bool acquire_plugins(THD *thd, plugin_ref plugin, void *arg) {
  st_mysql_subscribe_event *evt = static_cast<st_mysql_subscribe_event *>(arg);
  st_mysql_audit *data = plugin_data<st_mysql_audit *>(plugin);

  MUTEX_LOCK(lock, &thd->LOCK_thd_audit_data);

  /* Check if this plugin is interested in the event */
  if (check_audit_mask(data->class_mask, evt->lookup_mask)) {
    add_audit_mask(evt->not_subscribed_mask, data->class_mask);
    return false;
  }

  /* Prevent from adding the same plugin more than one time. */
  if (!thd->audit_class_plugins.exists(plugin)) {
    /* lock the plugin and add it to the list */
    plugin = my_plugin_lock(nullptr, &plugin);

    /* The plugin could not be acquired. */
    if (plugin == nullptr) {
      /* Add this plugin mask to non subscribed mask. */
      add_audit_mask(evt->not_subscribed_mask, data->class_mask);
      return false;
    }

    thd->audit_class_plugins.push_back(plugin);
  }

  /* Copy subscription mask from the plugin into the array. */
  add_audit_mask(evt->subscribed_mask, data->class_mask);

  return false;
}

/**
  Acquire audit plugins. Ensure that audit plugins interested in given event
  class are locked by current thread.

  @param thd            MySQL thread handle.
  @param event_class    Audit event class.
  @param event_subclass Audit event subclass.

  @return Zero, when there is a plugins interested in the event specified
          by event_class and event_subclass. Otherwise non zero value is
          returned.
*/
int mysql_audit_acquire_plugins(THD *thd, mysql_event_class_t event_class,
                                unsigned long event_subclass) {
  DBUG_TRACE;
  unsigned long global_mask = mysql_global_audit_mask[event_class];

  if (thd && !check_audit_mask(global_mask, event_subclass) &&
      check_audit_mask(thd->audit_class_mask[event_class], event_subclass)) {
    /*
      There is a plugin registered for the subclass, but THD has not
      registered yet for this event. Refresh THD class mask.
    */
    st_mysql_subscribe_event evt = {event_class,
                                    event_subclass,
                                    {
                                        0,
                                    },
                                    {
                                        0,
                                    },
                                    {
                                        0,
                                    }};
    plugin_foreach_func *funcs[] = {acquire_lookup_mask, acquire_plugins,
                                    nullptr};
    /*
      Acquire lookup_mask, which contains mask of all plugins that subscribe
      event specified by the event_class and event_subclass
      (acquire_lookup_mask).
      Load plugins that overlap with the lookup_mask (acquire_plugins).
    */
    plugin_foreach(thd, funcs, MYSQL_AUDIT_PLUGIN, &evt);
    /*
      Iterate through event masks of the acquired plugin, excluding masks
      of the the plugin not acquired. It's more likely that these plugins will
      be acquired during the next audit plugin acquisition.
    */
    int i;
    for (i = MYSQL_AUDIT_GENERAL_CLASS; i < MYSQL_AUDIT_CLASS_MASK_SIZE; i++)
      add_audit_mask(&thd->audit_class_mask[i],
                     (evt.subscribed_mask[i] ^ evt.not_subscribed_mask[i]) &
                         evt.subscribed_mask[i]);

    global_mask = thd->audit_class_mask[event_class];
  }

  /* Check whether there is a plugin registered for this event. */
  return check_audit_mask(global_mask, event_subclass) ? 1 : 0;
}

/**
  Release any resources associated with the current thd.

  @param[in] thd

*/

void mysql_audit_release(THD *thd) {
  MUTEX_LOCK(lock, &thd->LOCK_thd_audit_data);
  plugin_ref *plugins, *plugins_last;

  if (!thd || thd->audit_class_plugins.empty()) return;

  plugins = thd->audit_class_plugins.begin();
  plugins_last = thd->audit_class_plugins.end();
  for (; plugins != plugins_last; plugins++) {
    st_mysql_audit *data = plugin_data<st_mysql_audit *>(*plugins);

    /* Check to see if the plugin has a release method */
    if (!(data->release_thd)) continue;

    /* Tell the plugin to release its resources */
    data->release_thd(thd);
  }

  /* Now we actually unlock the plugins */
  plugin_unlock_list(nullptr, thd->audit_class_plugins.begin(),
                     thd->audit_class_plugins.size());

  /* Reset the state of thread values */
  thd->audit_class_plugins.clear();
  thd->audit_class_mask.clear();
  thd->audit_class_mask.resize(MYSQL_AUDIT_CLASS_MASK_SIZE);
}

/**
  Initialize thd variables used by Audit

  @param[in] thd

*/

void mysql_audit_init_thd(THD *thd) {
  MUTEX_LOCK(lock, &thd->LOCK_thd_audit_data);
  thd->audit_class_mask.clear();
  thd->audit_class_mask.resize(MYSQL_AUDIT_CLASS_MASK_SIZE);
}

/**
  Free thd variables used by Audit

  @param thd Current thread
*/

void mysql_audit_free_thd(THD *thd) {
  mysql_audit_release(thd);
  DBUG_ASSERT(thd->audit_class_plugins.empty());
}

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_audit_mask;

static PSI_mutex_info all_audit_mutexes[] = {
    {&key_LOCK_audit_mask, "LOCK_audit_mask", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static void init_audit_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_audit_mutexes));
  mysql_mutex_register(category, all_audit_mutexes, count);
}
#endif /* HAVE_PSI_INTERFACE */

/**
  Initialize Audit global variables
*/

void mysql_audit_initialize() {
#ifdef HAVE_PSI_INTERFACE
  init_audit_psi_keys();
#endif

  mysql_mutex_init(key_LOCK_audit_mask, &LOCK_audit_mask, MY_MUTEX_INIT_FAST);
  memset(mysql_global_audit_mask, 0, sizeof(mysql_global_audit_mask));
}

/**
  Finalize Audit global variables
*/

void mysql_audit_finalize() { mysql_mutex_destroy(&LOCK_audit_mask); }

/**
  Initialize an Audit plug-in

  @param[in] plugin

  @retval false  OK
  @retval true   There was an error.
*/

int initialize_audit_plugin(st_plugin_int *plugin) {
  st_mysql_audit *data = (st_mysql_audit *)plugin->plugin->info;
  int i;
  unsigned long masks = 0;

  for (i = MYSQL_AUDIT_GENERAL_CLASS; i < MYSQL_AUDIT_CLASS_MASK_SIZE; i++) {
    masks |= data->class_mask[i];
  }

  if (data->class_mask[MYSQL_AUDIT_AUTHORIZATION_CLASS]) {
    LogErr(ERROR_LEVEL, ER_AUDIT_PLUGIN_DOES_NOT_SUPPORT_AUDIT_AUTH_EVENTS,
           plugin->name.str);
    return 1;
  }

  if (!data->event_notify || !masks) {
    LogErr(ERROR_LEVEL, ER_AUDIT_PLUGIN_HAS_INVALID_DATA, plugin->name.str);
    return 1;
  }

  if (plugin->plugin->init && plugin->plugin->init(plugin)) {
    LogErr(ERROR_LEVEL, ER_PLUGIN_INIT_FAILED, plugin->name.str);
    return 1;
  }

  /* Make the interface info more easily accessible */
  plugin->data = plugin->plugin->info;

  /* Add the bits the plugin is interested in to the global mask */
  mysql_mutex_lock(&LOCK_audit_mask);
  add_audit_mask(mysql_global_audit_mask, data->class_mask);
  mysql_mutex_unlock(&LOCK_audit_mask);

  return 0;
}

/**
  Performs a bitwise OR of the installed plugins event class masks

  @param[in] plugin
  @param[in] arg

  @retval false  always
*/
static bool calc_class_mask(THD *, plugin_ref plugin, void *arg) {
  st_mysql_audit *data = plugin_data<st_mysql_audit *>(plugin);
  if (data)
    add_audit_mask(reinterpret_cast<unsigned long *>(arg), data->class_mask);
  return false;
}

/**
  Finalize an Audit plug-in

  @param[in] plugin

  @retval false  OK
  @retval true   There was an error.
*/
int finalize_audit_plugin(st_plugin_int *plugin) {
  unsigned long event_class_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];

  if (plugin->plugin->deinit && plugin->plugin->deinit(nullptr)) {
    DBUG_PRINT("warning", ("Plugin '%s' deinit function returned error.",
                           plugin->name.str));
    DBUG_EXECUTE("finalize_audit_plugin", return 1;);
  }

  plugin->data = nullptr;
  memset(&event_class_mask, 0, sizeof(event_class_mask));

  /* Iterate through all the installed plugins to create new mask */

  /*
    LOCK_audit_mask/LOCK_plugin order is not fixed, but serialized with table
    lock on mysql.plugin.
  */
  mysql_mutex_lock(&LOCK_audit_mask);
  plugin_foreach(current_thd, calc_class_mask, MYSQL_AUDIT_PLUGIN,
                 &event_class_mask);

  /* Set the global audit mask */
  memmove(mysql_global_audit_mask, event_class_mask, sizeof(event_class_mask));
  mysql_mutex_unlock(&LOCK_audit_mask);

  return 0;
}

/**
  Dispatches an event by invoking the plugin's event_notify method.

  @param[in] thd
  @param[in] plugin
  @param[in] arg

  @retval false  always
*/

static int plugins_dispatch(THD *thd, plugin_ref plugin, void *arg) {
  const struct st_mysql_event_generic *event_generic =
      (const struct st_mysql_event_generic *)arg;
  unsigned long subclass = static_cast<unsigned long>(
      *static_cast<const int *>(event_generic->event));
  st_mysql_audit *data = plugin_data<st_mysql_audit *>(plugin);

  /* Check to see if the plugin is interested in this event */
  if (check_audit_mask(data->class_mask[event_generic->event_class], subclass))
    return 0;

  /* Actually notify the plugin */
  return data->event_notify(thd, event_generic->event_class,
                            event_generic->event);
}

static bool plugins_dispatch_bool(THD *thd, plugin_ref plugin, void *arg) {
  return plugins_dispatch(thd, plugin, arg) ? true : false;
}

/**
  Distributes an audit event to plug-ins

  @param[in] thd
  @param     event_class
  @param[in] event
*/

static int event_class_dispatch(THD *thd, mysql_event_class_t event_class,
                                const void *event) {
  int result = 0;
  struct st_mysql_event_generic event_generic;
  event_generic.event_class = event_class;
  event_generic.event = event;
  /*
    Check if we are doing a slow global dispatch. This event occurs when
    thd == NULL as it is not associated with any particular thread.
  */
  if (unlikely(!thd)) {
    return plugin_foreach(thd, plugins_dispatch_bool, MYSQL_AUDIT_PLUGIN,
                          &event_generic)
               ? 1
               : 0;
  } else {
    decltype(thd->audit_class_plugins) plugins_copy{PSI_NOT_INSTRUMENTED};
    {
      MUTEX_LOCK(lock, &thd->LOCK_thd_audit_data);
      plugins_copy = thd->audit_class_plugins;
    }
    plugin_ref *plugins, *plugins_last;

    /* Use the cached set of audit plugins */
    plugins = plugins_copy.begin();
    plugins_last = plugins_copy.end();

    for (; plugins != plugins_last; plugins++)
      result |= plugins_dispatch(thd, *plugins, &event_generic);
  }

  return result;
}

static int event_class_dispatch_error(THD *thd, mysql_event_class_t event_class,
                                      const char *event_name,
                                      const void *event) {
  int result = 0;
  bool err = thd ? thd->get_stmt_da()->is_error() : true;

  if (err) /* Audit API cannot modify the already set DA's error state. */
    event_class_dispatch(thd, event_class, event);
  else {
    /* We are not is the error state, we can modify the existing one. */
    thd->get_stmt_da()->set_overwrite_status(true);

    result = event_class_dispatch(thd, event_class, event);

    if (result) {
      if (!thd->get_stmt_da()->is_error()) {
        my_error(ER_AUDIT_API_ABORT, MYF(0), event_name, result);
      }
    }

    thd->get_stmt_da()->set_overwrite_status(false);

    /* Because we rely on the error state, we have to notify our
    caller that the Audit API returned with error state. */
    if (thd->get_stmt_da()->is_error()) result = result != 0 ? result : 1;
  }

  return result;
}

/**  There's at least one active audit plugin tracking a specified class */
bool is_audit_plugin_class_active(THD *thd MY_ATTRIBUTE((unused)),
                                  unsigned long event_class) {
  return mysql_global_audit_mask[event_class] != 0;
}

/**
  @brief Checks presence of active audit plugin

  @retval      TRUE             At least one audit plugin is present
  @retval      FALSE            No audit plugin is present
*/
bool is_global_audit_mask_set() {
  for (int i = MYSQL_AUDIT_GENERAL_CLASS; i < MYSQL_AUDIT_CLASS_MASK_SIZE;
       i++) {
    if (mysql_global_audit_mask[i] != 0) return true;
  }
  return false;
}

size_t make_user_name(Security_context *sctx, char *buf) {
  LEX_CSTRING sctx_user = sctx->user();
  LEX_CSTRING sctx_host = sctx->host();
  LEX_CSTRING sctx_ip = sctx->ip();
  LEX_CSTRING sctx_priv_user = sctx->priv_user();
  return static_cast<size_t>(
      strxnmov(buf, MAX_USER_HOST_SIZE,
               sctx_priv_user.str[0] ? sctx_priv_user.str : "", "[",
               sctx_user.length ? sctx_user.str : "", "] @ ",
               sctx_host.length ? sctx_host.str : "", " [",
               sctx_ip.length ? sctx_ip.str : "", "]", NullS) -
      buf);
}
