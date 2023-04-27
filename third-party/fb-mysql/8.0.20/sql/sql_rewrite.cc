/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @brief  In here, we rewrite queries.

  For now, this is only used to obfuscate passwords before we log a
  statement.  If we ever get other clients for rewriting, we should
  introduce a rewrite_flags to determine what kind of rewriting
  (password obfuscation etc.) is desired by the client.

  Some items in the server can self-print anyway, but many can't.

  For instance, you'll see a re-synthesized SELECT in EXPLAIN EXTENDED,
  but you won't get a resynthized query in EXPLAIN EXTENDED if you
  were explaining an UPDATE.

  The following does not claim to be able to re-synthesize every
  statement, but attempts to ultimately be able to resynthesize
  all statements that have need of rewriting.

  Stored procedures may also rewrite their statements (to show the actual
  values of their variables etc.). There is currently no scenario where
  a statement can be eligible for both rewrites (see sp_instr.cc).
  Special consideration will need to be taken if this is intenionally
  changed at a later date.  (There is an ASSERT() in place that will
  hopefully catch unintentional changes.)

  Finally, sp_* have code to print a stored program for use by
  SHOW PROCEDURE CODE / SHOW FUNCTION CODE.

  Thus, regular query parsing comes through here for logging.
  So does prepared statement logging.
  Stored instructions of the sp_instr_stmt type (which should
  be the only ones to contain passwords, and therefore at this
  time be eligible for rewriting) go through the regular parsing
  facilities and therefore also come through here for logging
  (other sp_instr_* types don't).

  Finally, as rewriting goes, by default we replace the password with a literal
  \<secret\>, with *no* quotation marks so the statement would fail if the
  user were to cut & paste it without filling in the real password. This
  default behavior is ON for rewriting to the textual logs. For instance :
  General, slow query and audit log. Rewriters also have a provision to
  replace the password with its hash where we have the latter. (so they
  could be replayed, IDENTIFIED WITH \<plugin_name\> AS \<hash\>);
  This hash is needed while writing the statements for binlog.
*/

#include "sql/sql_rewrite.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <memory>
#include <set>
#include <string>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // GRANT_ACL
#include "sql/handler.h"
#include "sql/log_event.h"  // append_query_string
#include "sql/rpl_slave.h"  // SLAVE_SQL, SLAVE_IO
#include "sql/set_var.h"
#include "sql/sql_admin.h"  // Sql_cmd_clone
#include "sql/sql_class.h"  // THD
#include "sql/sql_connect.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // get_current_user
#include "sql/sql_servers.h"
#include "sql/sql_show.h"  // append_identifier
#include "sql/table.h"
#include "sql_string.h"  // String
#include "violite.h"

#ifndef DBUG_OFF
#define HASH_STRING_WITH_QUOTE \
  "$5$BVZy9O>'a+2MH]_?$fpWyabcdiHjfCVqId/quykZzjaA7adpkcen/uiQrtmOK4p4"
#endif

namespace {
/**
  Append a comma to given string if item wasn't the first to be added.

  @param[in,out]  str    The string to (maybe) append to.
  @param[in,out]  comma  If true, there are already items in the list.
                         Always true afterwards.
*/
void comma_maybe(String *str, bool *comma) {
  if (*comma)
    str->append(STRING_WITH_LEN(", "));
  else
    *comma = true;
}

/**
  Append a key/value pair to a string, with an optional preceeding comma.
  For numeric values.

  @param[in,out]   str                  The string to append to
  @param           comma                Prepend a comma?
  @param           txt                  C-string, must end in a space
  @param           len                  strlen(txt)
  @param           val                  numeric value
  @param           cond                 only append if this evaluates to true

  @retval          false if any subsequent key/value pair would be the first
*/

bool append_int(String *str, bool comma, const char *txt, size_t len, long val,
                int cond) {
  if (cond) {
    String numbuf(42);
    comma_maybe(str, &comma);
    str->append(txt, len);
    str->append(STRING_WITH_LEN(" "));
    numbuf.set((longlong)val, &my_charset_bin);
    str->append(numbuf);
    return true;
  }
  return comma;
}

/**
  Append a key/value pair to a string if the value is non-NULL,
  with an optional preceding comma.

  @param[in,out]   str                  The string to append to
  @param           comma                Prepend a comma?
  @param           key                  C-string: the key, must be non-NULL
  @param           val                  C-string: the value

  @retval          false if any subsequent key/value pair would be the first
*/

bool append_str(String *str, bool comma, const char *key, const char *val) {
  if (val) {
    comma_maybe(str, &comma);
    str->append(key);
    str->append(STRING_WITH_LEN(" '"));
    str->append(val);
    str->append(STRING_WITH_LEN("'"));
    return true;
  }
  return comma;
}
/**
  Append the authorization id for the user

  @param [in]       thd     The THD to find the SQL mode
  @param [in]       user    LEX User to retrieve the plugin string
  @param [in]       comma   Separator to be prefixed before adding user info
  @param [in, out]  str     The string in which authID is suffixed
*/
void append_auth_id(const THD *thd, const LEX_USER *user, bool comma,
                    String *str) {
  DBUG_ASSERT(thd);
  String from_user(user->user.str, user->user.length, system_charset_info);
  String from_host(user->host.str, user->host.length, system_charset_info);
  if (comma) str->append(',');
  append_query_string(thd, system_charset_info, &from_user, str);
  str->append(STRING_WITH_LEN("@"));
  append_query_string(thd, system_charset_info, &from_host, str);
}
/**
  Used with List<>::sort for alphabetic sorting of LEX_USER records
  using user,host as keys.

  @param l1 A LEX_USER element
  @param l2 A LEX_USER element

  @retval 1 if n1 &gt; n2
  @retval 0 if n1 &lt;= n2
*/
int lex_user_comp(LEX_USER *l1, LEX_USER *l2) {
  size_t length = std::min(l1->user.length, l2->user.length);
  int key = memcmp(l1->user.str, l2->user.str, length);
  if (key == 0 && l1->user.length == l2->user.length) {
    length = std::min(l1->host.length, l2->host.length);
    key = memcmp(l1->host.str, l2->host.str, length);
    if (key == 0 && l1->host.length == l2->host.length) return 0;
  }
  if (key == 0)
    return (l1->user.length > l2->user.length ? 1 : 0);
  else
    return (key > 0 ? 1 : 0);
}
/**
  Util method which does the real rewrite of the SQL statment.
  If a Rewriter is available for the specified SQL command then
  the rewritten query will be stored in the String rlb; otherwise,
  the string will just be cleared.

  @param         thd     The THD to rewrite for.
  @param         type    Purpose of rewriting the query
  @param         params  Wrapper object of parameters in case needed by a SQL
                         rewriter.
  @param[in,out] rlb     Buffer to return the rewritten query in.
                         Will be empty if no rewriting happened.
  @retval        true    If the Query is re-written.
  @retval        false   Otherwise
*/
bool rewrite_query(THD *thd, Consumer_type type, Rewrite_params *params,
                   String &rlb) {
  DBUG_TRACE;
  std::unique_ptr<I_rewriter> rw = nullptr;
  bool rewrite = false;

  rlb.length(0);

  switch (thd->lex->sql_command) {
    case SQLCOM_GRANT:
      rw.reset(new Rewriter_grant(thd, type, params));
      break;
    case SQLCOM_SET_PASSWORD:
      rw.reset(new Rewriter_set_password(thd, type, params));
      break;
    case SQLCOM_SET_OPTION:
      rw.reset(new Rewriter_set(thd, type));
      break;
    case SQLCOM_CREATE_USER:
      rw.reset(new Rewriter_create_user(thd, type));
      break;
    case SQLCOM_ALTER_USER:
      rw.reset(new Rewriter_alter_user(thd, type));
      break;
    case SQLCOM_SHOW_CREATE_USER:
      rw.reset(new Rewriter_show_create_user(thd, type, params));
      break;
    case SQLCOM_CHANGE_MASTER:
      rw.reset(new Rewriter_change_master(thd, type));
      break;
    case SQLCOM_SLAVE_START:
      rw.reset(new Rewriter_slave_start(thd, type));
      break;
    case SQLCOM_CREATE_SERVER:
      rw.reset(new Rewriter_create_server(thd, type));
      break;
    case SQLCOM_ALTER_SERVER:
      rw.reset(new Rewriter_alter_server(thd, type));
      break;

    /*
      PREPARE stmt FROM <string> is rewritten so that <string> is
      not logged.  The statement in <string> will in turn be logged
      by the prepare and the execute functions in sql_prepare.cc.
      They do call rewrite so they can safely log the statement,
      but when they call us, it'll be with sql_command set to reflect
      the statement in question, not SQLCOM_PREPARE or SQLCOM_EXECUTE.
      Therefore, there is no SQLCOM_EXECUTE case here, and all
      SQLCOM_PREPARE does is remove <string>; the "other half",
      i.e. printing what string we prepare from happens when the
      prepare function calls the logger (and comes by here with
      sql_command set to the command being prepared).
    */
    case SQLCOM_PREPARE:
      rw.reset(new Rewriter_prepare(thd, type));
      break;
    case SQLCOM_CLONE: {
      rw.reset(new Rewriter_clone(thd, type));
      break;
    }
    default: /* unhandled query types are legal. */
      break;
  }
  if (rw) rewrite = rw->rewrite(rlb);

  return rewrite;
}
}  // anonymous namespace

/**
  Provides the default interface to rewrite the SQL statements to
  to obfuscate passwords.

  The query aimed to be rewritten in the usual log files
  (i.e. General, slow query and audit log) uses default value of
  type which is Consumer_type::LOG

   Side-effects:

   - thd->m_rewritten_query will contain a rewritten query,
     or be cleared if no rewriting took place.
     LOCK_thd_query will be temporarily acquired to make that change.

   @note Keep in mind that these side-effects will only happen when
         calling this top-level function, but not when calling
         individual sub-functions directly!

  @param thd        The THD to rewrite for.
  @param type       Purpose of rewriting the query
                     Consumer_type::LOG
                      To rewrite the query either for general, slow query
                      and audit log.
                     Consumer_type::BINLOG
                      To rewrite the query for binlogs.
                     Consumer_type::CONSOLE
                      To rewrite the query for standard output.
  @param params     Wrapper object of parameters in case needed by a SQL
                      rewriter.
*/
void mysql_rewrite_query(THD *thd, Consumer_type type /*= Consumer_type::LOG */,
                         Rewrite_params *params /*= nullptr*/) {
  String rlb;

  DBUG_TRACE;
  DBUG_ASSERT(thd);

  // We should not come through here twice for the same query.
  DBUG_ASSERT(thd->rewritten_query().length() == 0);

  if (thd->lex->contains_plaintext_password) {
    rewrite_query(thd, type, params, rlb);
    if (rlb.length() > 0) thd->swap_rewritten_query(rlb);
    // The previous rewritten query is in rlb now, which now goes out of scope.
  }
}
/**
  Provides the default interface to rewrite the ACL query.

  @param thd        The THD to rewrite for.
  @param rlb        Buffer to return rewritten query in (if any) if
                    do_ps_instrument is false.
  @param type       Purpose of rewriting the query
                     Consumer_type::LOG
                      To rewrite the query either for general, slow query
                      and audit log.
                     Consumer_type::BINLOG
                      To rewrite the query for binlogs.
                     Consumer_type::CONSOLE
                      To rewrite the query for standard output.
  @param params     Wrapper object of parameters in case needed by a SQL
                      rewriter.
  @param do_ps_instrument flag to indicate if the query has to be instrumented
                          in the PSI. Default value is true.
                          If instrumented, the previous
*/
void mysql_rewrite_acl_query(THD *thd, String &rlb, Consumer_type type,
                             Rewrite_params *params /* = nullptr */,
                             bool do_ps_instrument /* = true */) {
  if (rewrite_query(thd, type, params, rlb) && (rlb.length() > 0) &&
      do_ps_instrument) {
    thd->swap_rewritten_query(rlb);
    thd->set_query_for_display(thd->rewritten_query().ptr(),
                               thd->rewritten_query().length());
    /*
      rlb now contains the previous rewritten query.
      We clear it here both to save memory and to prevent possible confusion.
    */
    rlb.mem_free();
  }
}

I_rewriter::I_rewriter(THD *thd, Consumer_type type)
    : m_thd(thd), m_consumer_type(type) {
  DBUG_ASSERT(thd);
}

I_rewriter::~I_rewriter() {}
/**
  Reset the previous consumer type.

  @param [in] type  new consumer type for which query is to be rewritten
*/
void I_rewriter::set_consumer_type(Consumer_type type) {
  m_consumer_type = type;
}
/**
  Return the current consumer type set in the object

  @retval  Consumer type set currently.
*/
Consumer_type I_rewriter::consumer_type() { return m_consumer_type; }
/* Constructor */
Rewriter_user::Rewriter_user(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}
/**
  Appends the essential clauses for SHOW CREATE|CREATE|ALTER USER statements
  in the buffer rlb.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        false   Since it does the partial query rewrite.
                         Must be called through derived classes rewrite().
*/
bool Rewriter_user::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;
  rewrite_users(lex, &rlb);
  rewrite_default_roles(lex, &rlb);
  rewrite_ssl_properties(lex, &rlb);
  rewrite_user_resources(lex, &rlb);
  rewrite_password_expired(lex, &rlb);
  rewrite_account_lock(lex, &rlb);
  rewrite_password_history(lex, &rlb);
  rewrite_password_reuse(lex, &rlb);
  rewrite_password_require_current(lex, &rlb);
  rewrite_account_lock_state(lex, &rlb);
  return false;
}
/**
  Append the literal \<secret\> in place of password to the output string

  @param [in, out]  str     The string in which literal value is suffixed
*/
void Rewriter_user::append_literal_secret(String *str) const {
  str->append(STRING_WITH_LEN("<secret>"));
}
/**
  Append the password hash to the output string

  @param [in]       user    LEX_USER to fetch the auth string of it.
  @param [in, out]  str     The string in which hash value is suffixed
*/

void Rewriter_user::append_auth_str(LEX_USER *user, String *str) const {
  String from_auth(user->auth.str, user->auth.length, system_charset_info);
  append_query_string(m_thd, system_charset_info, &from_auth, str);
}
/**
  Append the SSL clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_ssl_properties(const LEX *lex, String *str) const {
  if (lex->ssl_type != SSL_TYPE_NOT_SPECIFIED) {
    str->append(STRING_WITH_LEN(" REQUIRE"));
    switch (lex->ssl_type) {
      case SSL_TYPE_SPECIFIED:
        if (lex->x509_subject) {
          str->append(STRING_WITH_LEN(" SUBJECT '"));
          str->append(lex->x509_subject);
          str->append(STRING_WITH_LEN("'"));
        }
        if (lex->x509_issuer) {
          str->append(STRING_WITH_LEN(" ISSUER '"));
          str->append(lex->x509_issuer);
          str->append(STRING_WITH_LEN("'"));
        }
        if (lex->ssl_cipher) {
          str->append(STRING_WITH_LEN(" CIPHER '"));
          str->append(lex->ssl_cipher);
          str->append(STRING_WITH_LEN("'"));
        }
        break;
      case SSL_TYPE_X509:
        str->append(STRING_WITH_LEN(" X509"));
        break;
      case SSL_TYPE_ANY:
        str->append(STRING_WITH_LEN(" SSL"));
        break;
      case SSL_TYPE_NONE:
        str->append(STRING_WITH_LEN(" NONE"));
        break;
      default:
        DBUG_ASSERT(false);
        break;
    }
  }
}
/**
  Append the user resource clauses for users

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_user_resources(const LEX *lex, String *str) const {
  if (lex->mqh.specified_limits) {
    str->append(" WITH");
    append_int(str, false, STRING_WITH_LEN(" MAX_QUERIES_PER_HOUR"),
               lex->mqh.questions,
               lex->mqh.specified_limits & USER_RESOURCES::QUERIES_PER_HOUR);

    append_int(str, false, STRING_WITH_LEN(" MAX_UPDATES_PER_HOUR"),
               lex->mqh.updates,
               lex->mqh.specified_limits & USER_RESOURCES::UPDATES_PER_HOUR);

    append_int(
        str, false, STRING_WITH_LEN(" MAX_CONNECTIONS_PER_HOUR"),
        lex->mqh.conn_per_hour,
        lex->mqh.specified_limits & USER_RESOURCES::CONNECTIONS_PER_HOUR);

    append_int(str, false, STRING_WITH_LEN(" MAX_USER_CONNECTIONS"),
               lex->mqh.user_conn,
               lex->mqh.specified_limits & USER_RESOURCES::USER_CONNECTIONS);
  }
}
/**
  Append the ACCOUNT LOCK clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_account_lock(const LEX *lex, String *str) const {
  if (lex->alter_password.update_account_locked_column) {
    if (lex->alter_password.account_locked) {
      str->append(STRING_WITH_LEN(" ACCOUNT LOCK"));
    } else {
      str->append(STRING_WITH_LEN(" ACCOUNT UNLOCK"));
    }
  }
}
/**
  Append the PASSWORD EXPIRE clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_password_expired(const LEX *lex,
                                             String *str) const {
  if (lex->alter_password.update_password_expired_fields) {
    if (lex->alter_password.update_password_expired_column) {
      str->append(STRING_WITH_LEN(" PASSWORD EXPIRE"));
    } else if (lex->alter_password.expire_after_days) {
      append_int(str, false, STRING_WITH_LEN(" PASSWORD EXPIRE INTERVAL"),
                 lex->alter_password.expire_after_days, true);
      str->append(STRING_WITH_LEN(" DAY"));
    } else if (lex->alter_password.use_default_password_lifetime) {
      str->append(STRING_WITH_LEN(" PASSWORD EXPIRE DEFAULT"));
    } else {
      str->append(STRING_WITH_LEN(" PASSWORD EXPIRE NEVER"));
    }
  }
}
/**
  Append the PASSWORD REQUIRE CURRENT clause for users
  @param [in]       lex    LEX struct to know if the clause was specified
  @param [in, out]  str    The string in which the clause is suffixed
*/
void Rewriter_user::rewrite_password_require_current(LEX *lex,
                                                     String *str) const {
  switch (lex->alter_password.update_password_require_current) {
    case Lex_acl_attrib_udyn::YES:
      str->append(STRING_WITH_LEN(" PASSWORD REQUIRE CURRENT"));
      break;
    case Lex_acl_attrib_udyn::DEFAULT:
      str->append(STRING_WITH_LEN(" PASSWORD REQUIRE CURRENT DEFAULT"));
      break;
    case Lex_acl_attrib_udyn::NO:
      str->append(STRING_WITH_LEN(" PASSWORD REQUIRE CURRENT OPTIONAL"));
      break;
    case Lex_acl_attrib_udyn::UNCHANGED:
      // Do nothing
      break;
    default:
      DBUG_ASSERT(false);
  }
}

/**
  Append the account lock state

  Append FAILED_LOGIN_ATTEMPTS/PASSWORD_LOCK_TIME if account auto-lock
  is active.

  @param [in]       lex     LEX to retrieve data from
  @param [in, out]  str     The string in which plugin info is suffixed
*/
void Rewriter_user::rewrite_account_lock_state(LEX *lex, String *str) const {
  if (lex->alter_password.update_failed_login_attempts) {
    append_int(str, false, STRING_WITH_LEN(" FAILED_LOGIN_ATTEMPTS"),
               lex->alter_password.failed_login_attempts, true);
  }
  if (lex->alter_password.update_password_lock_time) {
    if (lex->alter_password.password_lock_time >= 0)
      append_int(str, false, STRING_WITH_LEN(" PASSWORD_LOCK_TIME"),
                 lex->alter_password.password_lock_time, true);
    else
      str->append(STRING_WITH_LEN(" PASSWORD_LOCK_TIME UNBOUNDED"));
  }
}

/**
  Append the authentication plugin name for the user

  @param [in]       user    LEX User to retrieve the plugin string
  @param [in, out]  str     The string in which plugin info is suffixed
*/
void Rewriter_user::append_plugin_name(const LEX_USER *user,
                                       String *str) const {
  str->append(STRING_WITH_LEN(" WITH "));
  if (user->plugin.length > 0) {
    String from_plugin(user->plugin.str, user->plugin.length,
                       system_charset_info);
    append_query_string(m_thd, system_charset_info, &from_plugin, str);
  } else {
    std::string def_plugin_name = get_default_autnetication_plugin_name();
    String default_plugin(def_plugin_name.c_str(), def_plugin_name.length(),
                          system_charset_info);
    append_query_string(m_thd, system_charset_info, &default_plugin, str);
  }
}
/**
   The default implementation is to append the PASSWORD HISTORY clause iff it
   is specified. Though concrete classes may add their own implementation.

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_password_history(const LEX *lex,
                                             String *str) const {
  if (lex->alter_password.use_default_password_history) {
    str->append(STRING_WITH_LEN(" PASSWORD HISTORY DEFAULT"));
  } else {
    append_int(str, false, STRING_WITH_LEN(" PASSWORD HISTORY"),
               lex->alter_password.password_history_length, true);
  }
}
/**
  The default implementation is to append the PASSWORD REUSE clause iff it is
  specified. Though concrete classes may add their own implementation.

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_password_reuse(const LEX *lex, String *str) const {
  if (lex->alter_password.use_default_password_reuse_interval) {
    str->append(STRING_WITH_LEN(" PASSWORD REUSE INTERVAL DEFAULT"));
  } else {
    append_int(str, false, STRING_WITH_LEN(" PASSWORD REUSE INTERVAL"),
               lex->alter_password.password_reuse_interval, true);
    str->append(STRING_WITH_LEN(" DAY"));
  }
}
/**
  Fetch the users from user_list in LEX struct and append them to the String.

  @param [in]       lex
  @param [in, out]  str
*/
void Rewriter_user::rewrite_users(LEX *lex, String *str) const {
  bool comma = false;
  LEX_USER *user_name, *tmp_user_name;
  List_iterator<LEX_USER> user_list(lex->users_list);
  while ((tmp_user_name = user_list++)) {
    if ((user_name = get_current_user(m_thd, tmp_user_name))) {
      append_user_auth_info(user_name, comma, str);
      comma = true;
    }
  }
}

/**
  Append the DEFAULT ROLE clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_user::rewrite_default_roles(const LEX *lex, String *str) const {
  bool comma = false;
  if (lex->default_roles && lex->default_roles->elements > 0) {
    str->append(" DEFAULT ROLE ");
    lex->default_roles->sort(&lex_user_comp);
    List_iterator<LEX_USER> role_it(*(lex->default_roles));
    LEX_USER *role;
    while ((role = role_it++)) {
      if (comma) str->append(',');
      str->append(create_authid_str_from(role).c_str());
      comma = true;
    }
  }
}

Rewriter_create_user::Rewriter_create_user(THD *thd, Consumer_type type)
    : Rewriter_user(thd, type) {}

/**
  Rewrite the query for the CREATE USER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    The query was rewritten.
*/
bool Rewriter_create_user::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;
  rlb.append("CREATE USER ");

  if (lex->create_info->options & HA_LEX_CREATE_IF_NOT_EXISTS)
    rlb.append("IF NOT EXISTS ");

  parent::rewrite(rlb);
  return true;
}
/**
  Append the authID, plugin and auth str of the user to output string :
   - If the corresponding clause is specified.
   - Always add the plugin info for the target type BINLOG
   - Add the literal \<secret\> in place of plain text password
     for the target type LOG

  @param  [in]      user    Lex user to fetch the info
  @param  [in]      comma   separator to be prefixed while appending user info
  @param  [in, out] str     String to which user auth info is suffixed.
*/
void Rewriter_create_user::append_user_auth_info(LEX_USER *user, bool comma,
                                                 String *str) const {
  append_auth_id(m_thd, user, comma, str);
  if (user->uses_identified_by_clause || user->uses_identified_with_clause ||
      user->uses_authentication_string_clause ||
      m_consumer_type == Consumer_type::BINLOG) {
    str->append(STRING_WITH_LEN(" IDENTIFIED"));
    if (user->uses_identified_with_clause ||
        m_consumer_type == Consumer_type::BINLOG)
      append_plugin_name(user, str);

    if (user->uses_identified_by_clause &&
        m_consumer_type == Consumer_type::TEXTLOG) {
      str->append(STRING_WITH_LEN(" BY "));
      append_literal_secret(str);
    } else if (user->auth.length > 0) {
      str->append(STRING_WITH_LEN(" AS "));
      append_auth_str(user, str);
    }
  }
}
/**
  Append the PASSWORD HISTORY clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_create_user::rewrite_password_history(const LEX *lex,
                                                    String *str) const {
  if (lex->alter_password.update_password_history) {
    parent::rewrite_password_history(lex, str);
  }
}
/**
  Append the PASSWORD REUSE clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_create_user::rewrite_password_reuse(const LEX *lex,
                                                  String *str) const {
  if (lex->alter_password.update_password_reuse_interval) {
    parent::rewrite_password_reuse(lex, str);
  }
}
Rewriter_alter_user::Rewriter_alter_user(THD *thd, Consumer_type type)
    : Rewriter_user(thd, type) {}

/**
  Rewrite the query for the ALTER USER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    The query was rewritten.
*/
bool Rewriter_alter_user::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;
  rlb.append("ALTER USER ");

  if (lex->drop_if_exists) rlb.append("IF EXISTS ");

  parent::rewrite(rlb);
  return true;
}
/**
  Append the authID, plugin and auth str of the user to output string :
   - If the corresponding clause is specified.
   - Always add the plugin info for the target type BINLOG
   - Add the literal \<secret\> in place of plain text password
     for the target type LOG

  @param  [in]      user    Lex user to fetch the info
  @param  [in]      comma   separator to be prefixed while appending user info
  @param  [in, out] str     String to which user auth info is suffixed.
*/
void Rewriter_alter_user::append_user_auth_info(LEX_USER *user, bool comma,
                                                String *str) const {
  append_auth_id(m_thd, user, comma, str);
  if (user->uses_identified_by_clause || user->uses_identified_with_clause ||
      user->uses_authentication_string_clause) {
    str->append(STRING_WITH_LEN(" IDENTIFIED"));
    if (user->uses_identified_with_clause ||
        m_consumer_type == Consumer_type::BINLOG)
      append_plugin_name(user, str);

    if (user->uses_identified_by_clause &&
        m_consumer_type == Consumer_type::TEXTLOG) {
      str->append(STRING_WITH_LEN(" BY "));
      append_literal_secret(str);
      /* Add the literal value in place of current password in the textlogs. */
      if (user->uses_replace_clause) {
        str->append(STRING_WITH_LEN(" REPLACE <secret>"));
      }
    } else if (user->auth.length > 0) {
      str->append(STRING_WITH_LEN(" AS "));
      append_auth_str(user, str);
    }

    if (user->retain_current_password) {
      str->append(STRING_WITH_LEN(" RETAIN CURRENT PASSWORD"));
    }
  }

  if (user->discard_old_password) {
    str->append(STRING_WITH_LEN(" DISCARD OLD PASSWORD"));
  }
}
/**
  Append the PASSWORD HISTORY clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_alter_user::rewrite_password_history(const LEX *lex,
                                                   String *str) const {
  if (lex->alter_password.update_password_history) {
    parent::rewrite_password_history(lex, str);
  }
}
/**
  Append the PASSWORD REUSE clause for users iff it is specified

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_alter_user::rewrite_password_reuse(const LEX *lex,
                                                 String *str) const {
  if (lex->alter_password.update_password_reuse_interval) {
    parent::rewrite_password_reuse(lex, str);
  }
}
Rewriter_show_create_user::Rewriter_show_create_user(THD *thd,
                                                     Consumer_type type,
                                                     Rewrite_params *params)
    : Rewriter_user(thd, type),
      show_params_(dynamic_cast<Show_user_params *>(params)) {}

/**
  Rewrite the query for the SHOW CREATE USER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    The query was rewritten.
*/
bool Rewriter_show_create_user::rewrite(String &rlb) const {
  rlb.append("CREATE USER ");
  parent::rewrite(rlb);
  return true;
}

/**
  A special rewriter override to make SHOW CREATE USER convert the string
  to hex if print_identified_with_as hex is on

  @param [in]       user    LEX_USER to fetch the auth string of it.
  @param [in, out]  str     The string in which hash value is suffixed

  @sa Rewriter_user::append_auth_str
*/
void Rewriter_show_create_user::append_auth_str(LEX_USER *user,
                                                String *str) const {
  String from_auth(user->auth.str, user->auth.length, system_charset_info);

  if (show_params_ && show_params_->print_identified_with_as_hex_ &&
      user->auth.length) {
    for (const char *c = user->auth.str;
         static_cast<size_t>(c - user->auth.str) < user->auth.length; c++) {
      if (!my_isgraph(system_charset_info, *c)) {
        from_auth.alloc(user->auth.length * 2 + 3);
        str_to_hex(from_auth.c_ptr_quick(), user->auth.str, user->auth.length);
        from_auth.length(user->auth.length * 2 + 2);
        str->append(from_auth);

        return;
      }
    }
  }
  append_query_string(m_thd, system_charset_info, &from_auth, str);
}
/**
  Append the PASSWORD HISTORY clause for users

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_show_create_user::rewrite_password_history(const LEX *lex,
                                                         String *str) const {
  parent::rewrite_password_history(lex, str);
}
/**
  Append the PASSWORD REUSE clause for users

  @param [in]       lex     LEX struct to check if clause is specified
  @param [in, out]  str     The string in which clause is suffixed
*/
void Rewriter_show_create_user::rewrite_password_reuse(const LEX *lex,
                                                       String *str) const {
  parent::rewrite_password_reuse(lex, str);
}
/**
  Append the authID, plugin name and suth str user to output string

  @param  [in]      user    Lex user to fetch the info
  @param  [in]      comma   separator to be prefixed while appending user info
  @param  [in, out] str     String to which user auth info is suffixed.
*/
void Rewriter_show_create_user::append_user_auth_info(LEX_USER *user,
                                                      bool comma,
                                                      String *str) const {
  append_auth_id(m_thd, user, comma, str);
  DBUG_ASSERT(m_thd->lex->contains_plaintext_password == false);
  str->append(STRING_WITH_LEN(" IDENTIFIED"));
  append_plugin_name(user, str);
  if (user->auth.length > 0) {
    str->append(STRING_WITH_LEN(" AS "));
    if (show_params_ && show_params_->hide_password_hash) {
      append_literal_secret(str);
    } else {
      append_auth_str(user, str);
    }
  }
}

Rewriter_set::Rewriter_set(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}

/**
  Rewrite the query for the SET statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_set::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;
  List_iterator_fast<set_var_base> it(lex->var_list);
  set_var_base *var;
  bool comma = false;

  rlb.append(STRING_WITH_LEN("SET "));

  while ((var = it++)) {
    comma_maybe(&rlb, &comma);
    var->print(m_thd, &rlb);
  }
  return true;
}

Rewriter_set_password::Rewriter_set_password(THD *thd, Consumer_type type,
                                             Rewrite_params *params)
    : Rewriter_set(thd, type) {
  User_params *param = dynamic_cast<User_params *>(params);
  if (param) m_users = param->users;
}

/**
  Rewrite the query for the SET PASSWORD statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_set_password::rewrite(String &rlb) const {
  bool ret_val = false;
  if (m_consumer_type == Consumer_type::BINLOG) {
    if (m_users == nullptr || m_users->size() == 0) return ret_val;

    /* SET PASSWORD should always have one user */
    DBUG_ASSERT(m_users->size() == 1);
    bool set_temp_string = false;
    /*
      Setting this flag will generate the password hash string which
      contains a single quote.
    */
    DBUG_EXECUTE_IF("force_hash_string_with_quote", set_temp_string = true;);
    LEX_USER *user = *(m_users->begin());
    String current_user(user->user.str, user->user.length, system_charset_info);
    String current_host(user->host.str, user->host.length, system_charset_info);
    String auth_str;
    if (set_temp_string) {
#ifndef DBUG_OFF
      auth_str = String(HASH_STRING_WITH_QUOTE, strlen(HASH_STRING_WITH_QUOTE),
                        system_charset_info);
#endif
    } else {
      auth_str = String(user->auth.str, user->auth.length, system_charset_info);
    }
    /*
      Construct :
      ALTER USER '<user>'@'<host>' IDENTIFIED WITH '<plugin>' AS '<HASH>'
    */
    rlb.append(STRING_WITH_LEN("ALTER USER "));
    append_query_string(m_thd, system_charset_info, &current_user, &rlb);
    rlb.append(STRING_WITH_LEN("@"));
    append_query_string(m_thd, system_charset_info, &current_host, &rlb);
    rlb.append(STRING_WITH_LEN(" IDENTIFIED WITH '"));
    rlb.append(user->plugin.str);
    rlb.append(STRING_WITH_LEN("' AS "));
    append_query_string(m_thd, system_charset_info, &auth_str, &rlb);
    if (user->retain_current_password)
      rlb.append(STRING_WITH_LEN(" RETAIN CURRENT PASSWORD"));
    ret_val = true;
  } else {
    ret_val = parent::rewrite(rlb);
  }

  return ret_val;
}

Rewriter_grant::Rewriter_grant(THD *thd, Consumer_type type,
                               Rewrite_params *params)
    : I_rewriter(thd, type) {
  grant_params = dynamic_cast<Grant_params *>(params);
}

/**
  Rewrite the query for the GRANT statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
*/
bool Rewriter_grant::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;

  TABLE_LIST *first_table = lex->select_lex->table_list.first;
  bool proxy_grant = lex->type == TYPE_ENUM_PROXY;
  String cols(1024);
  int c;

  auto append_authids = [&rlb](THD *thd, List_iterator<LEX_USER> &list) {
    LEX_USER *user_name, *tmp_user_name;
    bool comma = false;
    while ((tmp_user_name = list++)) {
      if ((user_name = get_current_user(thd, tmp_user_name))) {
        append_auth_id(thd, user_name, comma, &rlb);
        comma = true;
      }
    }
  };

  rlb.append(STRING_WITH_LEN("GRANT "));

  if (proxy_grant)
    rlb.append(STRING_WITH_LEN("PROXY"));
  else if (lex->all_privileges)
    rlb.append(STRING_WITH_LEN("ALL PRIVILEGES"));
  else if (lex->grant_privilege)
    rlb.append(STRING_WITH_LEN("GRANT OPTION"));
  else {
    bool comma = false;
    ulong priv;

    for (c = 0, priv = SELECT_ACL; priv <= GLOBAL_ACLS; c++, priv <<= 1) {
      if (priv == GRANT_ACL) continue;

      bool comma_inner = false;

      if (lex->columns.elements)  // show columns, if any
      {
        class LEX_COLUMN *column;
        List_iterator<LEX_COLUMN> column_iter(lex->columns);

        cols.length(0);
        cols.append(STRING_WITH_LEN(" ("));

        /*
          If the statement was GRANT SELECT(f2), INSERT(f3), UPDATE(f1,f3, f2),
          our list cols will contain the order f2, f3, f1, and thus that's
          the order we'll recreate the privilege: UPDATE (f2, f3, f1)
        */

        while ((column = column_iter++)) {
          if (column->rights & priv) {
            comma_maybe(&cols, &comma_inner);
            append_identifier(m_thd, &cols, column->column.ptr(),
                              column->column.length());
          }
        }
        cols.append(STRING_WITH_LEN(")"));
      }

      if (comma_inner || (lex->grant & priv))  // show privilege name
      {
        comma_maybe(&rlb, &comma);
        rlb.append(global_acls_vector[c].c_str(),
                   global_acls_vector[c].length());
        if (!(lex->grant & priv))  // general outranks specific
          rlb.append(cols);
      }
    }
    /* List extended global privilege IDs */
    if (!first_table && !lex->current_select()->db) {
      List_iterator<LEX_CSTRING> it(lex->dynamic_privileges);
      LEX_CSTRING *privilege;
      while ((privilege = it++)) {
        comma_maybe(&rlb, &comma);
        rlb.append(privilege->str, privilege->length);
      }
    }
    if (!comma)  // no privs, default to USAGE
      rlb.append(STRING_WITH_LEN("USAGE"));
  }

  rlb.append(STRING_WITH_LEN(" ON "));
  switch (lex->type) {
    case TYPE_ENUM_PROCEDURE:
      rlb.append(STRING_WITH_LEN("PROCEDURE "));
      break;
    case TYPE_ENUM_FUNCTION:
      rlb.append(STRING_WITH_LEN("FUNCTION "));
      break;
    default:
      break;
  }

  LEX_USER *user_name, *tmp_user_name;
  List_iterator<LEX_USER> user_list(lex->users_list);

  if (proxy_grant) {
    bool comma = false;
    tmp_user_name = user_list++;
    user_name = get_current_user(m_thd, tmp_user_name);
    if (user_name) append_auth_id(m_thd, user_name, comma, &rlb);
  } else if (first_table) {
    if (first_table->is_view()) {
      append_identifier(m_thd, &rlb, first_table->view_db.str,
                        first_table->view_db.length);
      rlb.append(STRING_WITH_LEN("."));
      append_identifier(m_thd, &rlb, first_table->view_name.str,
                        first_table->view_name.length);
    } else {
      append_identifier(m_thd, &rlb, first_table->db, strlen(first_table->db));
      rlb.append(STRING_WITH_LEN("."));
      append_identifier(m_thd, &rlb, first_table->table_name,
                        strlen(first_table->table_name));
    }
  } else {
    if (lex->current_select()->db)
      append_identifier(m_thd, &rlb, lex->current_select()->db,
                        strlen(lex->current_select()->db));
    else
      rlb.append("*");
    rlb.append(STRING_WITH_LEN(".*"));
  }

  rlb.append(STRING_WITH_LEN(" TO "));

  append_authids(m_thd, user_list);
  if (lex->grant & GRANT_ACL) {
    rlb.append(STRING_WITH_LEN(" WITH GRANT OPTION"));
  }

  /*
    AS ... clause is added in following cases
    1. User has explicitly executed GRANT ... AS ...
       In this case we write it as it.
    2. --partial_revokes is ON and we are rewritting
       GRANT for binary log.
  */
  if (grant_params != nullptr) {
    LEX_GRANT_AS *grant_as = grant_params->grant_as_info;
    if (grant_params->grant_as_provided ||
        (m_consumer_type == Consumer_type::BINLOG && grant_as &&
         grant_as->grant_as_used)) {
      if ((user_name = get_current_user(m_thd, grant_as->user))) {
        rlb.append(STRING_WITH_LEN(" AS "));
        append_auth_id(m_thd, user_name, false, &rlb);
        rlb.append(STRING_WITH_LEN(" WITH ROLE "));
        switch (grant_as->role_type) {
          case role_enum::ROLE_DEFAULT:
            rlb.append(STRING_WITH_LEN("DEFAULT"));
            break;
          case role_enum::ROLE_ALL:
            rlb.append(STRING_WITH_LEN("ALL"));
            if (grant_as->role_list) {
              rlb.append(STRING_WITH_LEN(" EXCEPT "));
              List_iterator<LEX_USER> role_list(*grant_as->role_list);
              append_authids(m_thd, role_list);
            }
            break;
          case role_enum::ROLE_NAME: {
            List_iterator<LEX_USER> role_list(*grant_as->role_list);
            append_authids(m_thd, role_list);
          } break;
          case role_enum::ROLE_NONE:
            rlb.append(STRING_WITH_LEN("NONE"));
            break;
          default:
            DBUG_ASSERT(false);
            rlb.append(STRING_WITH_LEN("NONE"));
            break;
        }
      }
    }
  }
  return true;
}

Rewriter_change_master::Rewriter_change_master(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}

/**
  Rewrite the query for the CHANGE MASTER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_change_master::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;
  rlb.append(STRING_WITH_LEN("CHANGE MASTER TO "));
  bool comma = false;
  comma = append_str(&rlb, comma, "MASTER_BIND =", lex->mi.bind_addr);
  comma = append_str(&rlb, comma, "MASTER_HOST =", lex->mi.host);
  comma = append_str(&rlb, comma, "MASTER_USER =", lex->mi.user);

  if (lex->mi.password) {
    comma_maybe(&rlb, &comma);
    rlb.append(STRING_WITH_LEN("MASTER_PASSWORD = <secret>"));
  }
  comma = append_int(&rlb, comma, STRING_WITH_LEN("MASTER_PORT ="),
                     lex->mi.port, lex->mi.port > 0);
  // condition as per rpl_slave.cc
  comma = append_int(&rlb, comma, STRING_WITH_LEN("MASTER_CONNECT_RETRY ="),
                     lex->mi.connect_retry, lex->mi.connect_retry > 0);
  comma = append_int(
      &rlb, comma, STRING_WITH_LEN("MASTER_RETRY_COUNT ="), lex->mi.retry_count,
      lex->mi.retry_count_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED);
  // MASTER_DELAY 0..MASTER_DELAY_MAX; -1 == unspecified
  comma = append_int(&rlb, comma, STRING_WITH_LEN("MASTER_DELAY ="),
                     lex->mi.sql_delay, lex->mi.sql_delay >= 0);

  if (lex->mi.heartbeat_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED) {
    comma_maybe(&rlb, &comma);
    rlb.append(STRING_WITH_LEN("MASTER_HEARTBEAT_PERIOD = "));
    if (lex->mi.heartbeat_opt == LEX_MASTER_INFO::LEX_MI_DISABLE)
      rlb.append(STRING_WITH_LEN("0"));
    else {
      char buf[64];
      snprintf(buf, 64, "%f", lex->mi.heartbeat_period);
      rlb.append(buf);
    }
  }

  // log file (slave I/O thread)
  comma = append_str(&rlb, comma, "MASTER_LOG_FILE =", lex->mi.log_file_name);
  // MASTER_LOG_POS is >= BIN_LOG_HEADER_SIZE; 0 == unspecified in stmt.
  comma = append_int(&rlb, comma, STRING_WITH_LEN("MASTER_LOG_POS ="),
                     lex->mi.pos, lex->mi.pos != 0);
  comma = append_int(
      &rlb, comma, STRING_WITH_LEN("MASTER_AUTO_POSITION ="),
      (lex->mi.auto_position == LEX_MASTER_INFO::LEX_MI_ENABLE) ? 1 : 0,
      lex->mi.auto_position != LEX_MASTER_INFO::LEX_MI_UNCHANGED);

  // log file (slave SQL thread)
  comma = append_str(&rlb, comma, "RELAY_LOG_FILE =", lex->mi.relay_log_name);
  // RELAY_LOG_POS is >= BIN_LOG_HEADER_SIZE; 0 == unspecified in stmt.
  comma = append_int(&rlb, comma, STRING_WITH_LEN("RELAY_LOG_POS ="),
                     lex->mi.relay_log_pos, lex->mi.relay_log_pos != 0);

  // SSL
  comma = append_int(&rlb, comma, STRING_WITH_LEN("MASTER_SSL ="),
                     lex->mi.ssl == LEX_MASTER_INFO::LEX_MI_ENABLE ? 1 : 0,
                     lex->mi.ssl != LEX_MASTER_INFO::LEX_MI_UNCHANGED);
  comma = append_str(&rlb, comma, "MASTER_SSL_CA =", lex->mi.ssl_ca);
  comma = append_str(&rlb, comma, "MASTER_SSL_CAPATH =", lex->mi.ssl_capath);
  comma = append_str(&rlb, comma, "MASTER_SSL_CERT =", lex->mi.ssl_cert);
  comma = append_str(&rlb, comma, "MASTER_SSL_CRL =", lex->mi.ssl_crl);
  comma = append_str(&rlb, comma, "MASTER_SSL_CRLPATH =", lex->mi.ssl_crlpath);
  comma = append_str(&rlb, comma, "MASTER_SSL_KEY =", lex->mi.ssl_key);
  comma = append_str(&rlb, comma, "MASTER_SSL_CIPHER =", lex->mi.ssl_cipher);
  comma = append_int(
      &rlb, comma, STRING_WITH_LEN("MASTER_SSL_VERIFY_SERVER_CERT ="),
      (lex->mi.ssl_verify_server_cert == LEX_MASTER_INFO::LEX_MI_ENABLE) ? 1
                                                                         : 0,
      lex->mi.ssl_verify_server_cert != LEX_MASTER_INFO::LEX_MI_UNCHANGED);

  comma = append_str(&rlb, comma, "MASTER_TLS_VERSION =", lex->mi.tls_version);
  if (LEX_MASTER_INFO::SPECIFIED_NULL == lex->mi.tls_ciphersuites) {
    comma_maybe(&rlb, &comma);
    rlb.append(STRING_WITH_LEN("MASTER_TLS_CIPHERSUITES = NULL"));
  } else if (LEX_MASTER_INFO::SPECIFIED_STRING == lex->mi.tls_ciphersuites) {
    comma = append_str(&rlb, comma, "MASTER_TLS_CIPHERSUITES =",
                       lex->mi.tls_ciphersuites_string);
  }

  // Public key
  comma = append_str(&rlb, comma,
                     "MASTER_PUBLIC_KEY_PATH =", lex->mi.public_key_path);
  comma = append_int(
      &rlb, comma, STRING_WITH_LEN("GET_MASTER_PUBLIC_KEY ="),
      (lex->mi.get_public_key == LEX_MASTER_INFO::LEX_MI_ENABLE) ? 1 : 0,
      lex->mi.get_public_key != LEX_MASTER_INFO::LEX_MI_UNCHANGED);

  // IGNORE_SERVER_IDS
  if (lex->mi.repl_ignore_server_ids_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED) {
    bool comma_list = false;

    comma_maybe(&rlb, &comma);
    rlb.append(STRING_WITH_LEN("IGNORE_SERVER_IDS = ( "));

    for (size_t i = 0; i < lex->mi.repl_ignore_server_ids.size(); i++) {
      ulong s_id = lex->mi.repl_ignore_server_ids[i];
      comma_maybe(&rlb, &comma_list);
      rlb.append_ulonglong(s_id);
    }
    rlb.append(STRING_WITH_LEN(" )"));
  }
  if (lex->mi.compression_algorithm)
    comma = append_str(&rlb, comma, "MASTER_COMPRESSION_ALGORITHMS = ",
                       lex->mi.compression_algorithm);
  comma = append_int(
      &rlb, comma, STRING_WITH_LEN("MASTER_ZSTD_COMPRESSION_LEVEL = "),
      lex->mi.zstd_compression_level, lex->mi.zstd_compression_level != 0);

  /* channel options -- no preceding comma here! */
  if (lex->mi.for_channel)
    append_str(&rlb, false, " FOR CHANNEL", lex->mi.channel);
  return true;
}

Rewriter_slave_start::Rewriter_slave_start(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}

/**
  Rewrite the query for the SLAVE START statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    The query was rewritten.
*/
bool Rewriter_slave_start::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;

  rlb.append(STRING_WITH_LEN("START SLAVE"));

  /* thread_types */

  if (lex->slave_thd_opt & SLAVE_IO) rlb.append(STRING_WITH_LEN(" IO_THREAD"));

  if (lex->slave_thd_opt & SLAVE_IO && lex->slave_thd_opt & SLAVE_SQL)
    rlb.append(STRING_WITH_LEN(","));

  if (lex->slave_thd_opt & SLAVE_SQL)
    rlb.append(STRING_WITH_LEN(" SQL_THREAD"));

  /* UNTIL options */

  // GTID
  if (lex->mi.gtid) {
    rlb.append((lex->mi.gtid_until_condition ==
                LEX_MASTER_INFO::UNTIL_SQL_BEFORE_GTIDS)
                   ? " UNTIL SQL_BEFORE_GTIDS"
                   : " UNTIL SQL_AFTER_GTIDS");
    append_str(&rlb, false, " =", lex->mi.gtid);
  }

  // SQL_AFTER_MTS_GAPS
  else if (lex->mi.until_after_gaps) {
    rlb.append(STRING_WITH_LEN(" UNTIL SQL_AFTER_MTS_GAPS"));
  }

  // MASTER_LOG_FILE/POS
  else if (lex->mi.log_file_name) {
    append_str(&rlb, false, " UNTIL MASTER_LOG_FILE =", lex->mi.log_file_name);
    append_int(&rlb, true, STRING_WITH_LEN("MASTER_LOG_POS ="), lex->mi.pos,
               lex->mi.pos > 0);
  }

  // RELAY_LOG_FILE/POS
  else if (lex->mi.relay_log_name) {
    append_str(&rlb, false, " UNTIL RELAY_LOG_FILE =", lex->mi.relay_log_name);
    append_int(&rlb, true, STRING_WITH_LEN("RELAY_LOG_POS ="),
               lex->mi.relay_log_pos, lex->mi.relay_log_pos > 0);
  }

  /* connection options */
  append_str(&rlb, false, " USER =", lex->slave_connection.user);

  if (lex->slave_connection.password)
    rlb.append(STRING_WITH_LEN(" PASSWORD = <secret>"));

  append_str(&rlb, false, " DEFAULT_AUTH =", lex->slave_connection.plugin_auth);
  append_str(&rlb, false, " PLUGIN_DIR =", lex->slave_connection.plugin_dir);

  /* channel options */
  if (lex->mi.for_channel)
    append_str(&rlb, false, " FOR CHANNEL", lex->mi.channel);
  return true;
}

Rewriter_server_option::Rewriter_server_option(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}
Rewriter_create_server::Rewriter_create_server(THD *thd, Consumer_type type)
    : Rewriter_server_option(thd, type) {}
/**
  Append the SERVER OPTIONS clause

  @param [in]     lex Lex structure
  @param [in,out] str A String object to append the rewritten query in.
*/
void Rewriter_server_option::mysql_rewrite_server_options(const LEX *lex,
                                                          String *str) const {
  str->append(STRING_WITH_LEN(" OPTIONS ( "));
  str->append(STRING_WITH_LEN("PASSWORD <secret>"));
  append_str(str, true, "USER", lex->server_options.get_username());
  append_str(str, true, "HOST", lex->server_options.get_host());
  append_str(str, true, "DATABASE", lex->server_options.get_db());
  append_str(str, true, "OWNER", lex->server_options.get_owner());
  append_str(str, true, "SOCKET", lex->server_options.get_socket());
  append_int(str, true, STRING_WITH_LEN("PORT"), lex->server_options.get_port(),
             lex->server_options.get_port() != Server_options::PORT_NOT_SET);
  str->append(STRING_WITH_LEN(" )"));
}
/**
  Rewrite the query for the CREATE SERVER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_create_server::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;

  if (!lex->server_options.get_password()) return false;

  rlb.append(STRING_WITH_LEN("CREATE SERVER "));
  rlb.append(lex->server_options.m_server_name.str
                 ? lex->server_options.m_server_name.str
                 : "");
  rlb.append(STRING_WITH_LEN(" FOREIGN DATA WRAPPER '"));
  rlb.append(lex->server_options.get_scheme() ? lex->server_options.get_scheme()
                                              : "");
  rlb.append(STRING_WITH_LEN("'"));
  parent::mysql_rewrite_server_options(lex, &rlb);

  return true;
}

Rewriter_alter_server::Rewriter_alter_server(THD *thd, Consumer_type type)
    : Rewriter_server_option(thd, type) {}

/**
  Rewrite the query for the ALTER SERVER statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_alter_server::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;

  if (!lex->server_options.get_password()) return false;

  rlb.append(STRING_WITH_LEN("ALTER SERVER "));
  rlb.append(lex->server_options.m_server_name.str
                 ? lex->server_options.m_server_name.str
                 : "");
  parent::mysql_rewrite_server_options(lex, &rlb);

  return true;
}

Rewriter_prepare::Rewriter_prepare(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}

/**
  Rewrite the query for the PREPARE statement.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_prepare::rewrite(String &rlb) const {
  LEX *lex = m_thd->lex;

  if (lex->prepared_stmt_code_is_varref) return false;

  rlb.append(STRING_WITH_LEN("PREPARE "));
  rlb.append(lex->prepared_stmt_name.str, lex->prepared_stmt_name.length);
  rlb.append(STRING_WITH_LEN(" FROM ..."));
  return true;
}

Rewriter_clone::Rewriter_clone(THD *thd, Consumer_type type)
    : I_rewriter(thd, type) {}

/**
  Rewrite the query for the CLONE statement to hide password.

  @param[in,out] rlb     Buffer to return the rewritten query in.

  @retval        true    the query was rewritten
  @retval        false   otherwise
*/
bool Rewriter_clone::rewrite(String &rlb) const {
  auto clone_cmd = dynamic_cast<Sql_cmd_clone *>(m_thd->lex->m_sql_cmd);
  return (clone_cmd->rewrite(m_thd, rlb));
}
