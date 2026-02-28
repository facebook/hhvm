/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/error_handler.h"
#include "sql/auth/auth_acls.h"

#include <errno.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"  // EE_*
#include "sql/create_field.h"
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/mdl.h"
#include "sql/sql_audit.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE_LIST
#include "sql/transaction_info.h"
#include "sql_string.h"

/**
  Implementation of Drop_table_error_handler::handle_condition().
  The reason in having this implementation is to silence technical low-level
  warnings during DROP TABLE operation. Currently we don't want to expose
  the following warnings during DROP TABLE:
    - Some of table files are missed or invalid (the table is going to be
      deleted anyway, so why bother that something was missed).
    - The table is using an invalid collation.

  @return true if the condition is handled.
*/
bool Drop_table_error_handler::handle_condition(
    THD *, uint sql_errno, const char *, Sql_condition::enum_severity_level *,
    const char *) {
  return (sql_errno == ER_UNKNOWN_COLLATION) ||
         (sql_errno == ER_PLUGIN_IS_NOT_LOADED) ||
         (sql_errno == EE_DELETE && my_errno() == ENOENT);
}

/**
  This handler is used for the statements which support IGNORE keyword.
  If IGNORE is specified in the statement, this error handler converts
  the given errors codes to warnings.
  These errors occur for each record. With IGNORE, statements are not
  aborted and next row is processed.

*/
bool Ignore_error_handler::handle_condition(
    THD *thd, uint sql_errno, const char *,
    Sql_condition::enum_severity_level *level, const char *) {
  /*
    If a statement is executed with IGNORE keyword then this handler
    gets pushed for the statement. If there is trigger on the table
    which contains statements without IGNORE then this handler should
    not convert the errors within trigger to warnings.
  */
  if (!thd->lex->is_ignore()) return false;
  /*
    Error codes ER_DUP_ENTRY_WITH_KEY_NAME is used while calling my_error
    to get the proper error messages depending on the use case.
    The error code used is ER_DUP_ENTRY to call error functions.

    Same case exists for ER_NO_PARTITION_FOR_GIVEN_VALUE_SILENT which uses
    error code of ER_NO_PARTITION_FOR_GIVEN_VALUE to call error function.

    There error codes are added here to force consistency if these error
    codes are used in any other case in future.
  */
  switch (sql_errno) {
    case ER_SUBQUERY_NO_1_ROW:
    case ER_ROW_IS_REFERENCED_2:
    case ER_NO_REFERENCED_ROW_2:
    case ER_NO_REFERENCED_ROW:
    case ER_ROW_IS_REFERENCED:
    case ER_BAD_NULL_ERROR:
    case ER_DUP_ENTRY:
    case ER_DUP_ENTRY_WITH_KEY_NAME:
    case ER_DUP_KEY:
    case ER_VIEW_CHECK_FAILED:
    case ER_NO_PARTITION_FOR_GIVEN_VALUE:
    case ER_NO_PARTITION_FOR_GIVEN_VALUE_SILENT:
    case ER_ROW_DOES_NOT_MATCH_GIVEN_PARTITION_SET:
    case ER_CHECK_CONSTRAINT_VIOLATED:
      (*level) = Sql_condition::SL_WARNING;
      break;
    default:
      break;
  }
  return false;
}

bool View_error_handler::handle_condition(THD *thd, uint sql_errno,
                                          const char *,
                                          Sql_condition::enum_severity_level *,
                                          const char *) {
  /*
    Error will be handled by Show_create_error_handler for
    SHOW CREATE statements.
  */
  if (thd->lex->sql_command == SQLCOM_SHOW_CREATE) return false;

  switch (sql_errno) {
    case ER_BAD_FIELD_ERROR:
    case ER_SP_DOES_NOT_EXIST:
    // ER_FUNC_INEXISTENT_NAME_COLLISION cannot happen here.
    case ER_PROCACCESS_DENIED_ERROR:
    case ER_COLUMNACCESS_DENIED_ERROR:
    case ER_TABLEACCESS_DENIED_ERROR:
    // ER_TABLE_NOT_LOCKED cannot happen here.
    case ER_NO_SUCH_TABLE: {
      TABLE_LIST *top = m_top_view->top_table();
      my_error(ER_VIEW_INVALID, MYF(0), top->view_db.str, top->view_name.str);
      return true;
    }

    case ER_NO_DEFAULT_FOR_FIELD: {
      TABLE_LIST *top = m_top_view->top_table();
      // TODO: make correct error message
      my_error(ER_NO_DEFAULT_FOR_VIEW_FIELD, MYF(0), top->view_db.str,
               top->view_name.str);
      return true;
    }
  }
  return false;
}

/**
  Implementation of STRICT mode.
  Upgrades a set of given conditions from warning to error.
*/
bool Strict_error_handler::handle_condition(
    THD *thd, uint sql_errno, const char *,
    Sql_condition::enum_severity_level *level, const char *msg) {
  /*
    STRICT error handler should not be effective if we have changed the
    variable to turn off STRICT mode. This is the case when a SF/SP/Trigger
    calls another SP/SF. A statement in SP/SF which is affected by STRICT mode
    with push this handler for the statement. If the same statement calls
    another SP/SF/Trigger, we already have the STRICT handler pushed for the
    statement. We dont want the strict handler to be effective for the
    next SP/SF/Trigger call if it was not created in STRICT mode.
  */
  if (!thd->install_strict_handler()) return false;
  /* STRICT MODE should affect only the below statements */
  switch (thd->lex->sql_command) {
    case SQLCOM_SET_OPTION:
    case SQLCOM_SELECT:
      if (m_set_select_behavior == DISABLE_SET_SELECT_STRICT_ERROR_HANDLER)
        return false;
    case SQLCOM_CREATE_TABLE:
    case SQLCOM_CREATE_INDEX:
    case SQLCOM_DROP_INDEX:
    case SQLCOM_INSERT:
    case SQLCOM_REPLACE:
    case SQLCOM_REPLACE_SELECT:
    case SQLCOM_INSERT_SELECT:
    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_LOAD:
    case SQLCOM_CALL:
    case SQLCOM_END:
      break;
    default:
      return false;
  }

  bool is_slave_thread = (thd->system_thread == SYSTEM_THREAD_SLAVE_SQL) ||
                         (thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER);

  /*
    First check whether we need to error out because of
    error_partial_strict system variable. This has higher precedence
    than sql mode check since we end up auditing the errors
    caused by the system variable.
   */
  switch (sql_errno) {
    case ER_DATA_TOO_LONG:
      if ((*level == Sql_condition::SL_WARNING) &&
          thd->really_error_partial_strict && !is_slave_thread) {
        (*level) = Sql_condition::SL_ERROR;

        if (thd->variables.audit_instrumented_event > 1 &&
            !thd->audited_event_for_command) {
          thd->audited_event_for_command = true;
          mysql_audit_notify(thd, AUDIT_EVENT(MYSQL_AUDIT_GENERAL_ERROR_INSTR),
                             sql_errno, msg, strlen(msg));
        }

        return false;
      }
      break;

    default:
      break;
  }

  /*
    We want to audit ER_DUPLICATED_VALUE_IN_TYPE but it is called with
    Sql_condition::SL_NOTE. So set expected logging_level to SL_NOTE
    for ER_DUPLICATED_VALUE_IN_TYPE.
   */
  Sql_condition::enum_severity_level logging_level =
      sql_errno == ER_DUPLICATED_VALUE_IN_TYPE ? Sql_condition::SL_NOTE
                                               : Sql_condition::SL_WARNING;

  switch (sql_errno) {
    case ER_TRUNCATED_WRONG_VALUE:
    case ER_WRONG_VALUE_FOR_TYPE:
    case ER_WARN_DATA_OUT_OF_RANGE:
    case ER_WARN_DATA_OUT_OF_RANGE_FUNCTIONAL_INDEX:
    case ER_DIVISION_BY_ZERO:
    case ER_TRUNCATED_WRONG_VALUE_FOR_FIELD:
    case WARN_DATA_TRUNCATED:
    case ER_WARN_DATA_TRUNCATED_FUNCTIONAL_INDEX:
    case ER_DATA_TOO_LONG:
    case ER_BAD_NULL_ERROR:
    case ER_NO_DEFAULT_FOR_FIELD:
    case ER_TOO_LONG_KEY:
    case ER_NO_DEFAULT_FOR_VIEW_FIELD:
    case ER_WARN_NULL_TO_NOTNULL:
    case ER_CUT_VALUE_GROUP_CONCAT:
    case ER_DATETIME_FUNCTION_OVERFLOW:
    case ER_WARN_TOO_FEW_RECORDS:
    case ER_WARN_TOO_MANY_RECORDS:
    case ER_INVALID_ARGUMENT_FOR_LOGARITHM:
    case ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE:
    case ER_INVALID_JSON_VALUE_FOR_CAST:
    case ER_WARN_ALLOWED_PACKET_OVERFLOWED:
      if (thd->is_strict_sql_mode() && (*level == logging_level) &&
          (!thd->get_transaction()->cannot_safely_rollback(
               Transaction_ctx::STMT) ||
           (thd->variables.sql_mode & MODE_STRICT_ALL_TABLES))) {
        (*level) = Sql_condition::SL_ERROR;
        /*
           Do not audit errors due to strict mode violations so break out
           of the switch case.
        */
        break;
      }
      /* Fall through */
      /*
        The section below handles the audit logging for warning
        messages in strict mode.
       */
    case WARN_COND_ITEM_TRUNCATED:
    case ER_AUTO_CONVERT:
    case ER_DUPLICATED_VALUE_IN_TYPE:
    case ER_TOO_LONG_FIELD_COMMENT:
    case ER_TOO_LONG_INDEX_COMMENT:
    case ER_TOO_LONG_TABLE_COMMENT:
    case ER_TOO_LONG_TABLE_PARTITION_COMMENT:
    case ER_TOO_LONG_TABLESPACE_COMMENT:
    case ER_BLOB_CANT_HAVE_DEFAULT:
      if ((*level == logging_level) &&
          thd->variables.audit_instrumented_event > 0 && !is_slave_thread &&
          !thd->audited_event_for_command) {
        thd->audited_event_for_command = true;
        mysql_audit_notify(thd, AUDIT_EVENT(MYSQL_AUDIT_GENERAL_WARNING_INSTR),
                           sql_errno, msg, strlen(msg));
      }
      break;
    default:
      /*
        There are some error codes (ER_WRONG_ARGUMENTS) that are
        logged throughout the code. However only a subset of the
        positions where they are logged end up being audited. Those
        callers set really_audit_instrumented_event before printing
        the warning.
       */
      if ((*level == logging_level) &&
          thd->really_audit_instrumented_event > 0 &&
          !thd->audited_event_for_command && !is_slave_thread) {
        thd->audited_event_for_command = true;
        mysql_audit_notify(thd, AUDIT_EVENT(MYSQL_AUDIT_GENERAL_WARNING_INSTR),
                           sql_errno, msg, strlen(msg));
      }

      break;
  }

  return false;
}

Functional_index_error_handler::Functional_index_error_handler(
    const Field *field, THD *thd)
    : m_thd(thd), m_pop_error_handler(false), m_force_error_code(-1) {
  DBUG_ASSERT(field != nullptr);

  if (field->is_field_for_functional_index()) {
    m_thd->push_internal_handler(this);
    m_pop_error_handler = true;

    // Get the name of the functional index
    // This field is only used by one functional index, so it's OK to just fetch
    // the first key that matches.
    for (uint i = 0; i < field->table->s->keys; ++i) {
      if (field->part_of_key.is_set(i)) {
        m_functional_index_name.assign(field->table->s->key_info[i].name);
        break;
      }
    }
  }
}

Functional_index_error_handler::Functional_index_error_handler(
    Create_field *field, const std::string &key_name, THD *thd)
    : m_functional_index_name(key_name),
      m_thd(thd),
      m_pop_error_handler(false),
      m_force_error_code(-1) {
  DBUG_ASSERT(field != nullptr);

  if (is_field_for_functional_index(field)) {
    m_thd->push_internal_handler(this);
    m_pop_error_handler = true;
  }
}

Functional_index_error_handler::Functional_index_error_handler(
    const std::string &functional_index_name, THD *thd)
    : m_functional_index_name(functional_index_name),
      m_thd(thd),
      m_pop_error_handler(true),
      m_force_error_code(-1) {
  m_thd->push_internal_handler(this);
}

Functional_index_error_handler::~Functional_index_error_handler() {
  if (m_pop_error_handler) {
    m_thd->pop_internal_handler();
  }
}

template <typename... Args>
static bool report_error(THD *thd, int error_code,
                         Sql_condition::enum_severity_level level,
                         Args... args) {
  switch (level) {
    case Sql_condition::SL_ERROR: {
      my_error(error_code, MYF(0), args...);
      return true;
    }
    case Sql_condition::SL_WARNING: {
      push_warning_printf(thd, level, error_code,
                          ER_THD_NONCONST(thd, error_code), args...);
      return true;
    }
    default: {
      // Do nothing
      return false; /* purecov: deadcode */
    }
  }
  return false;
}

static bool report_error(THD *thd, int error_code,
                         Sql_condition::enum_severity_level level) {
  switch (level) {
    case Sql_condition::SL_ERROR: {
      my_error(error_code, MYF(0));
      return true;
    }
    case Sql_condition::SL_WARNING: {
      push_warning(thd, level, error_code, ER_THD_NONCONST(thd, error_code));
      return true;
    }
    default: {
      // Do nothing
      return false; /* purecov: deadcode */
    }
  }
}

bool Functional_index_error_handler::handle_condition(
    THD *, uint sql_errno, const char *,
    Sql_condition::enum_severity_level *level, const char *) {
  DBUG_ASSERT(!m_functional_index_name.empty());
  uint res_errno = 0;
  bool print_row = false;

  switch (sql_errno) {
    case ER_JSON_USED_AS_KEY: {
      res_errno = ER_FUNCTIONAL_INDEX_ON_JSON_OR_GEOMETRY_FUNCTION;
      break;
    }
    case ER_TRUNCATED_WRONG_VALUE: {
      res_errno = ER_WARN_DATA_TRUNCATED_FUNCTIONAL_INDEX;
      print_row = true;
      break;
    }
    case WARN_DATA_TRUNCATED: {
      push_warning_printf(
          m_thd, *level, ER_WARN_DATA_TRUNCATED_FUNCTIONAL_INDEX,
          ER_THD(m_thd, ER_WARN_DATA_TRUNCATED_FUNCTIONAL_INDEX),
          m_functional_index_name.c_str(),
          m_thd->get_stmt_da()->current_row_for_condition());
      return true;
    }
    case ER_JT_VALUE_OUT_OF_RANGE:
    case ER_WARN_DATA_OUT_OF_RANGE: {
      res_errno = ER_WARN_DATA_OUT_OF_RANGE_FUNCTIONAL_INDEX;
      print_row = true;
      break;
    }
    // Difference with above is that this one doesn't print row number
    case ER_NUMERIC_JSON_VALUE_OUT_OF_RANGE: {
      res_errno = ER_JSON_VALUE_OUT_OF_RANGE_FOR_FUNC_INDEX;
      break;
    }
    case ER_GENERATED_COLUMN_REF_AUTO_INC: {
      res_errno = ER_FUNCTIONAL_INDEX_REF_AUTO_INCREMENT;
      break;
    }
    case ER_GENERATED_COLUMN_NAMED_FUNCTION_IS_NOT_ALLOWED:
    case ER_GENERATED_COLUMN_FUNCTION_IS_NOT_ALLOWED: {
      res_errno = ER_FUNCTIONAL_INDEX_FUNCTION_IS_NOT_ALLOWED;
      break;
    }
    case ER_UNSUPPORTED_ACTION_ON_GENERATED_COLUMN: {
      if (m_force_error_code != -1) {
        return report_error(m_thd, m_force_error_code, *level);
      }
      return false;
    }
    case ER_WRONG_JSON_TABLE_VALUE: {
      res_errno = ER_WRONG_MVI_VALUE;
      break;
    }
    case ER_WARN_INDEX_NOT_APPLICABLE: {
      res_errno = ER_WARN_FUNC_INDEX_NOT_APPLICABLE;
      break;
    }
    case ER_DATA_TOO_LONG: {
      my_error(ER_FUNCTIONAL_INDEX_DATA_IS_TOO_LONG, MYF(0),
               m_functional_index_name.c_str());
      return true;
    }
    case ER_INVALID_JSON_VALUE_FOR_CAST: {
      res_errno = ER_INVALID_JSON_VALUE_FOR_FUNC_INDEX;
      break;
    }
    case ER_GENERATED_COLUMN_ROW_VALUE: {
      my_error(ER_FUNCTIONAL_INDEX_ROW_VALUE_IS_NOT_ALLOWED, MYF(0),
               m_functional_index_name.c_str());
      return true;
    }
    default: {
      // Do nothing
      return false;
    }
  }
  if (!print_row)
    report_error(m_thd, res_errno, *level, m_functional_index_name.c_str());
  else
    report_error(m_thd, res_errno, *level, m_functional_index_name.c_str(),
                 m_thd->get_stmt_da()->current_row_for_condition());
  return true;
}

/**
  This internal handler is used to trap ER_NO_SUCH_TABLE and
  ER_WRONG_MRG_TABLE errors during CHECK/REPAIR TABLE for MERGE
  tables.
*/

class Repair_mrg_table_error_handler : public Internal_error_handler {
 public:
  Repair_mrg_table_error_handler()
      : m_handled_errors(false), m_unhandled_errors(false) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_NO_SUCH_TABLE || sql_errno == ER_WRONG_MRG_TABLE) {
      m_handled_errors = true;
      return true;
    }

    m_unhandled_errors = true;
    return false;
  }

  /**
    Returns true if there were ER_NO_SUCH_/WRONG_MRG_TABLE and there
    were no unhandled errors. false otherwise.
  */
  bool safely_trapped_errors() {
    /*
      Check for m_handled_errors is here for extra safety.
      It can be useful in situation when call to open_table()
      fails because some error which was suppressed by another
      error handler (e.g. in case of MDL deadlock which we
      decided to solve by back-off and retry).
    */
    return (m_handled_errors && (!m_unhandled_errors));
  }

 private:
  bool m_handled_errors;
  bool m_unhandled_errors;
};

/**
  Following are implementation of error handler to convert
  ER_LOCK_DEADLOCK error when executing I_S.TABLES and I_S.FILES system
  view.
*/

Info_schema_error_handler::Info_schema_error_handler(THD *thd,
                                                     const String *schema_name,
                                                     const String *table_name)
    : m_can_deadlock(thd->mdl_context.has_locks()),
      m_schema_name(schema_name),
      m_table_name(table_name),
      m_object_type(Mdl_object_type::TABLE) {}

Info_schema_error_handler::Info_schema_error_handler(
    THD *thd, const String *tablespace_name)
    : m_can_deadlock(thd->mdl_context.has_locks()),
      m_tablespace_name(tablespace_name),
      m_object_type(Mdl_object_type::TABLESPACE) {}

bool Info_schema_error_handler::handle_condition(
    THD *, uint sql_errno, const char *, Sql_condition::enum_severity_level *,
    const char *) {
  if (sql_errno == ER_LOCK_DEADLOCK && m_can_deadlock) {
    // Convert error to ER_WARN_I_S_SKIPPED_TABLE.
    if (m_object_type == Mdl_object_type::TABLE) {
      my_error(ER_WARN_I_S_SKIPPED_TABLE, MYF(0), m_schema_name->ptr(),
               m_table_name->ptr());
    } else  // Convert error to ER_WARN_I_S_SKIPPED_TABLESPACE.
    {
      my_error(ER_I_S_SKIPPED_TABLESPACE, MYF(0), m_tablespace_name->ptr());
    }

    m_error_handled = true;
  }

  return false;
}

bool Foreign_key_error_handler::handle_condition(
    THD *, uint sql_errno, const char *, Sql_condition::enum_severity_level *,
    const char *) {
  const TABLE_SHARE *share = m_table_handler->get_table_share();

  if (sql_errno == ER_NO_REFERENCED_ROW_2) {
    for (TABLE_SHARE_FOREIGN_KEY_INFO *fk = share->foreign_key;
         fk < share->foreign_key + share->foreign_keys; ++fk) {
      TABLE_LIST table(fk->referenced_table_db.str,
                       fk->referenced_table_db.length,
                       fk->referenced_table_name.str,
                       fk->referenced_table_name.length, TL_READ);
      if (check_table_access(m_thd, TABLE_OP_ACLS, &table, true, 1, true)) {
        my_error(ER_NO_REFERENCED_ROW, MYF(0));
        return true;
      }
    }
  } else if (sql_errno == ER_ROW_IS_REFERENCED_2) {
    for (TABLE_SHARE_FOREIGN_KEY_PARENT_INFO *fk_p = share->foreign_key_parent;
         fk_p < share->foreign_key_parent + share->foreign_key_parents;
         ++fk_p) {
      TABLE_LIST table(fk_p->referencing_table_db.str,
                       fk_p->referencing_table_db.length,
                       fk_p->referencing_table_name.str,
                       fk_p->referencing_table_name.length, TL_READ);
      if (check_table_access(m_thd, TABLE_OP_ACLS, &table, true, 1, true)) {
        my_error(ER_ROW_IS_REFERENCED, MYF(0));
        return true;
      }
    }
  }
  return false;
}
