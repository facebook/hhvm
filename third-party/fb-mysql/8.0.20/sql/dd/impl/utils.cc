/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/utils.h"

#include <string>

#include "lex_string.h"
#include "m_ctype.h"  // CHARSET_INFO
#include "m_string.h"
#include "my_time.h"  // TIME_to_ulonglong_datetime
#include "mysql/components/services/log_builtins.h"  // LogErr
#include "mysql_com.h"                               // NAME_LEN
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"     // DD_bootstrap_ctx
#include "sql/dd/properties.h"                       // dd::Properties
#include "sql/sql_base.h"                            // close_thread_tables
#include "sql/sql_prepare.h"                         // Ed_connection
#include "sql/stateless_allocator.h"
#include "sql/strfunc.h"
#include "sql/transaction.h"  // trans_rollback

namespace dd {
// Execute a single SQL query.
bool execute_query(THD *thd, const dd::String_type &q_buf) {
  Ed_connection con(thd);
  LEX_STRING str;
  lex_string_strmake(thd->mem_root, &str, q_buf.c_str(), q_buf.length());
  if (con.execute_direct(str)) {
    // Report error to log file during bootstrap.
    if (dd::bootstrap::DD_bootstrap_ctx::instance().get_stage() <
        dd::bootstrap::Stage::FINISHED) {
      LogErr(ERROR_LEVEL, ER_DD_INITIALIZE_SQL_ERROR, q_buf.c_str(),
             con.get_last_errno(), con.get_last_error());
    }
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////

void escape(String_type *sp, const String_type &src) {
  const String_type::const_iterator src_end = src.end();
  for (String_type::const_iterator s = src.begin(); s != src_end; ++s) {
    if (*s == '\\' || *s == '=' || *s == ';') {
      sp->append(1, '\\');
    }
    sp->append(1, *s);
  }
}

///////////////////////////////////////////////////////////////////////////

bool unescape(String_type &dest) {
  String_type tmp_dest;
  for (String_type::iterator d = dest.begin(); d != dest.end(); d++) {
    if (*d == '\\') {
      // An escape character preceeding end is an error, it must be succeeded
      // by an escapable character.
      if ((d + 1) != dest.end() &&
          (*(d + 1) == '\\' || *(d + 1) == '=' || *(d + 1) == ';'))
        d++;
      else
        return true;
    }
    tmp_dest.push_back(*d);
  }
  dest = tmp_dest;

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool eat_to(String_type::const_iterator &it, String_type::const_iterator end,
            char c) {
  // Verify valid stop characters
  if (c != '=' && c != ';') return true;

  // Loop until end of string or stop character
  while (it != end && *it != c) {
    // Unexpected unescaped stop character is an error
    if ((*it == '=' && c == ';') || (*it == ';' && c == '=')) return true;

    // The escape character must be succeeded by an escapable character
    if (*it == '\\') {
      it++;
      // Hitting end here is an error, we must have an escapable character
      if (it == end || (*it != '\\' && *it != '=' && *it != ';')) return true;
    }

    // Advance iterator, also after finding an escapable character
    it++;
  }

  // Hitting end searching for ';' is ok; if searching for '=', it is not
  return (it == end && c == '=');
}

///////////////////////////////////////////////////////////////////////////

bool eat_str(String_type &dest, String_type::const_iterator &it,
             String_type::const_iterator end, char c) {
  // Save starting point for later copying
  String_type::const_iterator start = it;

  // Find the first unescaped occurrence of c, or the end
  if (eat_to(it, end, c)) return true;

  // Create destination string up to, but not including c
  dest = String_type(start, it);

  // Remove escape characters
  if (unescape(dest)) return true;

  // Make iterator point to character after c or at the end of the string
  if (it != end) it++;

  return false;
}

///////////////////////////////////////////////////////////////////////////

bool eat_pairs(String_type::const_iterator &it, String_type::const_iterator end,
               dd::Properties *props) {
  String_type key("");
  String_type val("");

  if (it == end) return false;

  if (eat_str(key, it, end, '=') || eat_str(val, it, end, ';')) return true;

  // Empty keys are rejected, empty values are ok
  if (key == "") return true;

  // Silently skip invalid keys.
  if (props->valid_key(key)) props->set(key, val);

  return eat_pairs(it, end, props);
}

///////////////////////////////////////////////////////////////////////////

ulonglong my_time_t_to_ull_datetime(my_time_t seconds_since_epoch) {
  MYSQL_TIME curtime;
  my_tz_OFFSET0->gmt_sec_to_TIME(&curtime, seconds_since_epoch);
  return TIME_to_ulonglong_datetime(curtime);
}

///////////////////////////////////////////////////////////////////////////

bool is_string_in_lowercase(const String_type &str, const CHARSET_INFO *cs) {
  char lowercase_str_buf[NAME_LEN + 1];
  my_stpcpy(lowercase_str_buf, str.c_str());
  my_casedn_str(cs, lowercase_str_buf);
  return (memcmp(str.c_str(), lowercase_str_buf, str.length()) == 0);
}

///////////////////////////////////////////////////////////////////////////
// Helper function to do rollback or commit, depending on error.
bool end_transaction(THD *thd, bool error) {
  if (error) {
    // Rollback the statement before we can rollback the real transaction.
    trans_rollback_stmt(thd);
    trans_rollback(thd);
  } else if (trans_commit_stmt(thd) || trans_commit(thd)) {
    error = true;
    trans_rollback(thd);
  }

  // Close tables etc. and release MDL locks, regardless of error.
  close_thread_tables(thd);
  thd->mdl_context.release_transactional_locks();
  return error;
}

}  // namespace dd
