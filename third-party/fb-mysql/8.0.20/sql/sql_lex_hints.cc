/*
   Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

/* A lexical scanner for optimizer hints pseudo-commentary syntax */

#include "sql/sql_lex_hints.h"

#include <limits.h>

#include "my_dbug.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/lex_token.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_yacc.h"
#include "sql/system_variables.h"

class PT_hint_list;

/**
  Consrtuctor.

  @param thd_arg          The thread handler.
  @param lineno_arg       The starting line number of a hint string in a query.
  @param buf              The rest of a query buffer with hints at the start.
  @param len              The length of the buf.
  @param digest_state_arg The digest buffer to output scanned token data.
*/
Hint_scanner::Hint_scanner(THD *thd_arg, size_t lineno_arg, const char *buf,
                           size_t len, sql_digest_state *digest_state_arg)
    : thd(thd_arg),
      cs(thd->charset()),
      is_ansi_quotes(thd->variables.sql_mode & MODE_ANSI_QUOTES),
      backslash_escapes(!(thd->variables.sql_mode & MODE_NO_BACKSLASH_ESCAPES)),
      lineno(lineno_arg),
      char_classes(cs->state_maps->hint_map),
      input_buf(buf),
      input_buf_end(input_buf + len),
      ptr(input_buf + 3),  // skip "/*+"
      prev_token(0),
      digest_state(digest_state_arg),
      raw_yytext(ptr),
      yytext(ptr),
      yyleng(0),
      has_hints(false) {}

void HINT_PARSER_error(THD *thd MY_ATTRIBUTE((unused)), Hint_scanner *scanner,
                       PT_hint_list **, const char *msg) {
  if (strcmp(msg, "syntax error") == 0)
    msg = ER_THD(thd, ER_WARN_OPTIMIZER_HINT_SYNTAX_ERROR);
  scanner->syntax_warning(msg);
}

/**
  @brief Push a warning message into MySQL error stack with line
  and position information.

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the error stack, which is normally produced only if
  a parse error is discovered internally by the Bison generated
  parser.
*/

void Hint_scanner::syntax_warning(const char *msg) const {
  /* Push an error into the error stack */
  ErrConvString err(raw_yytext, input_buf_end - raw_yytext,
                    thd->variables.character_set_client);

  push_warning_printf(thd, Sql_condition::SL_WARNING, ER_PARSE_ERROR,
                      ER_THD(thd, ER_PARSE_ERROR), msg, err.ptr(),
                      static_cast<int>(lineno));
}

/**
  Add hint tokens to main lexer's digest calculation buffer.
*/
void Hint_scanner::add_hint_token_digest() {
  if (digest_state == nullptr) return;  // cant add: digest buffer is full

  if (prev_token == 0 || prev_token == HINT_ERROR) return;  // nothing to add

  if (prev_token == HINT_CLOSE) {
    if (has_hints) add_digest(TOK_HINT_COMMENT_CLOSE);
    return;
  }
  if (!has_hints) {  // the 1st hint in the comment
    add_digest(TOK_HINT_COMMENT_OPEN);
    has_hints = true;
  }

  switch (prev_token) {
    case HINT_ARG_NUMBER:
      add_digest(NUM);
      break;
    case HINT_ARG_IDENT:
      add_digest((peek_class() == HINT_CHR_AT) ? TOK_IDENT_AT : IDENT);
      break;
    case HINT_ARG_QB_NAME:
      add_digest('@');
      add_digest(IDENT);
      break;
    case HINT_ARG_TEXT:
      add_digest(TEXT_STRING);
      break;
    case HINT_IDENT_OR_NUMBER_WITH_SCALE:
      add_digest(NUM);
      break;
    default:
      if (prev_token <= UCHAR_MAX)  // Single-char token.
        add_digest(prev_token);
      else  // keyword
      {
        /* Make sure this is a known hint keyword: */
        switch (prev_token) {
          case BKA_HINT:
          case BNL_HINT:
          case DUPSWEEDOUT_HINT:
          case FIRSTMATCH_HINT:
          case INTOEXISTS_HINT:
          case LOOSESCAN_HINT:
          case MATERIALIZATION_HINT:
          case MAX_EXECUTION_TIME_HINT:
          case MRR_HINT:
          case NO_BKA_HINT:
          case NO_BNL_HINT:
          case NO_ICP_HINT:
          case NO_MRR_HINT:
          case NO_RANGE_OPTIMIZATION_HINT:
          case NO_SEMIJOIN_HINT:
          case QB_NAME_HINT:
          case SEMIJOIN_HINT:
          case SET_VAR_HINT:
          case SUBQUERY_HINT:
          case DERIVED_MERGE_HINT:
          case NO_DERIVED_MERGE_HINT:
          case JOIN_PREFIX_HINT:
          case JOIN_SUFFIX_HINT:
          case JOIN_ORDER_HINT:
          case JOIN_FIXED_ORDER_HINT:
          case INDEX_MERGE_HINT:
          case NO_INDEX_MERGE_HINT:
          case RESOURCE_GROUP_HINT:
          case SKIP_SCAN_HINT:
          case NO_SKIP_SCAN_HINT:
          case GROUP_BY_LIS_HINT:
          case NO_GROUP_BY_LIS_HINT:
          case HASH_JOIN_HINT:
          case NO_HASH_JOIN_HINT:
          case INDEX_HINT:
          case NO_INDEX_HINT:
          case JOIN_INDEX_HINT:
          case NO_JOIN_INDEX_HINT:
          case GROUP_INDEX_HINT:
          case NO_GROUP_INDEX_HINT:
          case ORDER_INDEX_HINT:
          case NO_ORDER_INDEX_HINT:
            break;
          default:
            DBUG_ASSERT(false);
        }
        add_digest(prev_token);
      }
  }
}
