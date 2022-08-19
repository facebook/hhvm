/*  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>
#include <algorithm>
#include <new>

#include "lex_string.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_thread.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/service_parser.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/enum_query_type.h"
#include "sql/error_handler.h"
#include "sql/item.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"  // my_localhost
#include "sql/mysqld_thd_manager.h"
#include "sql/protocol_classic.h"
#include "sql/select_lex_visitor.h"
#include "sql/sql_base.h"  // close_thread_tables
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_digest.h"
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"
#include "sql/system_variables.h"
#include "sql/transaction.h"  // trans_commit_stmt
#include "sql_string.h"
#include "template_utils.h"

/**
  This class implements the parse tree visiting service.
  @see mysql_parser_visit_tree
*/
class Service_visitor : public Select_lex_visitor {
  parse_node_visit_function m_processor;
  uchar *m_arg;

 public:
  /**
    @param processor This function will be called for each literal (Item) in
    the parse tree, along with the Item's type and arg. If the function
    returns non-zero, parse tree traversal will terminate.

    @param arg Will be passed to processor on every call.
  */
  Service_visitor(parse_node_visit_function processor, uchar *arg)
      : m_processor(processor), m_arg(arg) {}

 protected:
  bool visit_item(Item *item) {
    switch (item->type()) {
        // These are all the literals.
      case Item::PARAM_ITEM:
      case Item::STRING_ITEM:
      case Item::INT_ITEM:
      case Item::DECIMAL_ITEM:
      case Item::REAL_ITEM:
      case Item::NULL_ITEM:
      case Item::VARBIN_ITEM:
      case Item::CACHE_ITEM:
        return m_processor(item, m_arg);
      default:
        break;
    }
    return false;
  }
};

/**
  This class implements the framework needed for the callback function that
  handles conditions that may arise during parsing via the parser service.

  @see mysql_parser_visit_tree
*/
class Plugin_error_handler : public Internal_error_handler {
  THD *m_thd;
  const char *m_message;
  const sql_condition_handler_function m_handle_error;
  void *m_state;

 public:
  Plugin_error_handler(THD *thd, sql_condition_handler_function handle_error,
                       void *state)
      : m_thd(thd),
        m_message(nullptr),
        m_handle_error(handle_error),
        m_state(state) {
    if (handle_error != nullptr) thd->push_internal_handler(this);
  }

  virtual bool handle_condition(THD *, uint sql_errno_u, const char *sqlstate,
                                Sql_condition::enum_severity_level *,
                                const char *msg) {
    int sql_errno = static_cast<int>(sql_errno_u);
    if (m_handle_error != nullptr)
      return m_handle_error(sql_errno, sqlstate, msg, m_state) != 0;
    return false;
  }

  const char *get_message() { return m_message; }

  ~Plugin_error_handler() {
    if (m_handle_error != nullptr) m_thd->pop_internal_handler();
  }
};

MYSQL_THD mysql_parser_current_session() { return current_thd; }

MYSQL_THD mysql_parser_open_session() {
  THD *old_thd = current_thd;

  // See create_thd()
  THD *thd = new (std::nothrow) THD;
  if (thd == nullptr) return nullptr;

  thd->security_context()->set_host_ptr(STRING_WITH_LEN(my_localhost));
  thd->lex = new LEX;
  thd->lex->set_current_select(nullptr);

  thd->variables.character_set_client = old_thd->variables.character_set_client;

  return thd;
}

namespace {

struct thread_args {
  thread_args(MYSQL_THD thd, callback_function fun, void *arg)
      : m_thd(thd), m_fun(fun), m_arg(arg) {}
  MYSQL_THD m_thd;
  callback_function m_fun;
  void *m_arg;
};

void *parser_service_start_routine(void *arg) {
  thread_args *tt = pointer_cast<thread_args *>(arg);
  THD *thd = tt->m_thd;
  my_thread_init();

  {
    DBUG_TRACE;

    Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
    thd->thread_stack = reinterpret_cast<char *>(&thd);
    thd->set_new_thread_id();
    mysql_thread_set_psi_id(thd->thread_id());
    thd->store_globals();
    thd->set_time();

    thd_manager->add_thd(thd);
    (tt->m_fun)(tt->m_arg);

    trans_commit_stmt(thd);
    close_thread_tables(thd);
    thd->mdl_context.release_transactional_locks();
    close_mysql_tables(thd);

    thd->release_resources();
    thd->restore_globals();
    thd_manager->remove_thd(thd);

    LEX *lex = thd->lex;
    delete thd;
    delete lex;
    delete tt;
  }
  my_thread_end();
  my_thread_exit(nullptr);
  return nullptr;
}

}  // namespace

void mysql_parser_start_thread(THD *thd, callback_function fun, void *arg,
                               my_thread_handle *thread_handle) {
  my_thread_handle handle;
  my_thread_attr_t attr;
  my_thread_attr_init(&attr);

  thread_args *args = new thread_args(thd, fun, arg);
  mysql_thread_create(key_thread_parser_service, &handle, &attr,
                      parser_service_start_routine, args);
  *thread_handle = handle;
}

void mysql_parser_join_thread(my_thread_handle *thread_id) {
  my_thread_join(thread_id, nullptr);
}

void mysql_parser_set_current_database(MYSQL_THD thd,
                                       const MYSQL_LEX_STRING db) {
  if (db.length == 0) {
    LEX_CSTRING db_const = {nullptr, 0};
    thd->set_db(db_const);
  } else {
    LEX_CSTRING db_const = {db.str, db.length};
    thd->set_db(db_const);
  }
}

int mysql_parser_parse(MYSQL_THD thd, const MYSQL_LEX_STRING query,
                       unsigned char is_prepared,
                       sql_condition_handler_function handle_condition,
                       void *condition_handler_state) {
  /*
    Clean up parse- and item trees in case this function was called before for
    the same THD.
  */
  if (thd->lex->is_lex_started) {
    thd->end_statement();
    thd->cleanup_after_query();
  }

  lex_start(thd);

  if (alloc_query(thd, query.str, query.length))
    return 1;  // Fatal error flag set

  Parser_state parser_state;
  if (parser_state.init(thd, query.str, query.length)) return 1;

  parser_state.m_input.m_compute_digest = true;
  thd->m_digest = &thd->m_digest_state;
  thd->m_digest->reset(thd->m_token_array, max_digest_length);

  if (is_prepared) {
    parser_state.m_lip.stmt_prepare_mode = true;
    parser_state.m_lip.multi_statements = false;
    thd->lex->context_analysis_only |= CONTEXT_ANALYSIS_ONLY_PREPARE;
  }

  Plugin_error_handler error_handler(thd, handle_condition,
                                     condition_handler_state);

  int parse_status = parse_sql(thd, &parser_state, nullptr);

  /*
    Handled conditions are thrown away at this point - they are supposedly
    handled by handle_condition. If there was no handler supplied, the
    diagnostics area is not touched. It will contain any errors thrown by the
    parser.
  */
  if (handle_condition != nullptr) {
    thd->get_stmt_da()->reset_diagnostics_area();
    thd->get_stmt_da()->reset_condition_info(thd);
  }

  return parse_status;
}

int mysql_parser_get_statement_type(MYSQL_THD thd) {
  LEX *lex = thd->lex;
  switch (lex->sql_command) {
    case SQLCOM_SELECT:
      return STATEMENT_TYPE_SELECT;
    case SQLCOM_UPDATE:  // Fall through
    case SQLCOM_UPDATE_MULTI:
      return STATEMENT_TYPE_UPDATE;
    case SQLCOM_INSERT:  // Fall through
    case SQLCOM_INSERT_SELECT:
      return STATEMENT_TYPE_INSERT;
    case SQLCOM_REPLACE:  // Fall through
    case SQLCOM_REPLACE_SELECT:
      return STATEMENT_TYPE_REPLACE;
    case SQLCOM_DELETE:  // Fall through
    case SQLCOM_DELETE_MULTI:
      return STATEMENT_TYPE_DELETE;
    default:
      return STATEMENT_TYPE_OTHER;
  }
}

int mysql_parser_get_statement_digest(MYSQL_THD thd, uchar *digest) {
  static_assert(PARSER_SERVICE_DIGEST_LENGTH == DIGEST_HASH_SIZE,
                "If you change the digest hash, PARSER_SERVICE_DIGEST_LENGTH "
                "needs to adjust");

  if (thd->m_digest == nullptr) return true;
  compute_digest_hash(&thd->m_digest->m_digest_storage, digest);
  return false;
}

int mysql_parser_get_number_params(MYSQL_THD thd) {
  return thd->lex->param_list.elements;
}

int mysql_parser_extract_prepared_params(MYSQL_THD thd, int *positions) {
  LEX *lex = thd->lex;
  List_iterator_fast<Item_param> it(lex->param_list);
  for (uint i = 0; i < lex->param_list.elements; i++)
    positions[i] = it++->pos_in_query;
  return lex->param_list.elements;
}

int mysql_parser_visit_tree(MYSQL_THD thd, parse_node_visit_function processor,
                            unsigned char *arg) {
  Service_visitor visitor(processor, arg);
  return thd->lex->accept(&visitor);
}

MYSQL_LEX_STRING mysql_parser_item_string(MYSQL_ITEM item) {
  String str;
  static_cast<Item *>(item)->print(mysql_parser_current_session(), &str,
                                   QT_ORDINARY);
  MYSQL_LEX_STRING res = {new char[str.length()], 0};
  if (res.str != nullptr) {
    res.length = str.length();
    std::copy(str.ptr(), str.ptr() + str.length(), res.str);
  }
  return res;
}

void mysql_parser_free_string(MYSQL_LEX_STRING string) { delete[] string.str; }

MYSQL_LEX_STRING mysql_parser_get_query(MYSQL_THD thd) {
  MYSQL_LEX_STRING str = {const_cast<char *>(thd->query().str),
                          thd->query().length};
  return str;
}

MYSQL_LEX_STRING mysql_parser_get_normalized_query(MYSQL_THD thd) {
  String normalized_query = thd->normalized_query();
  return normalized_query.lex_string();
}
