/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/mysqltest/regular_expressions.h"

#include "m_ctype.h"
#include "my_compiler.h"

extern void die(const char *fmt, ...) MY_ATTRIBUTE((format(printf, 1, 2)))
    MY_ATTRIBUTE((noreturn));

/*
  Filter for queries that can be run using the
  MySQL Prepared Statements C API.
*/
static const char *const ps_re_str =
    "^("
    "[[:space:]]*REPLACE[[:space:]]|"
    "[[:space:]]*INSERT[[:space:]]|"
    "[[:space:]]*UPDATE[[:space:]]|"
    "[[:space:]]*DELETE[[:space:]]|"
    "[[:space:]]*SELECT[[:space:]]|"
    "[[:space:]]*CREATE[[:space:]]+DATABASE[[:space:]]|"
    "[[:space:]]*CREATE[[:space:]]+INDEX[[:space:]]|"
    "[[:space:]]*CREATE[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*CREATE[[:space:]]+USER[[:space:]]|"
    "[[:space:]]*CREATE[[:space:]]+TEMPORARY[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+DATABASE[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+INDEX[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+USER[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+VIEW[[:space:]]|"
    "[[:space:]]*DROP[[:space:]]+TEMPORARY[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*ALTER[[:space:]]+USER[[:space:]]|"
    "[[:space:]]*RENAME[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*RENAME[[:space:]]+USER[[:space:]]|"
    "[[:space:]]*TRUNCATE[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*ANALYZE[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*CHECKSUM[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*CHECKSUM[[:space:]]+TABLES[[:space:]]|"
    "[[:space:]]*OPTIMIZE[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*REPAIR[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*GRANT[[:space:]]|"
    "[[:space:]]*KILL[[:space:]]|"
    "[[:space:]]*REVOKE[[:space:]]+ALL[[:space:]]+PRIVILEGES[[:space:]]|"
    "[[:space:]]*DO[[:space:]]|"
    "[[:space:]]*CALL[[:space:]]|"
    "[[:space:]]*COMMIT[[:space:]]|"
    "[[:space:]]*SET[[:space:]]+OPTION[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+CREATE[[:space:]]+TABLE[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+CREATE[[:space:]]+PROCEDURE[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+CREATE[[:space:]]+FUNCTION[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+CREATE[[:space:]]+VIEW[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+CREATE[[:space:]]+EVENT[[:space:]]|"
    "[[:space:]]*INSTALL[[:space:]]+PLUGIN[[:space:]]|"
    "[[:space:]]*UNINSTALL[[:space:]]+PLUGIN[[:space:]]|"
    "[[:space:]]*RESET[[:space:]]+MASTER[[:space:]]|"
    "[[:space:]]*RESET[[:space:]]+SLAVE[[:space:]]|"
    "[[:space:]]*RESET[[:space:]]+QUERY[[:space:]]+CACHE[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+BINLOG[[:space:]]+EVENTS[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+MASTER[[:space:]]+LOGS[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+MASTER[[:space:]]+STATUS[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+BINARY[[:space:]]+LOGS[[:space:]]|"
    "[[:space:]]*SHOW[[:space:]]+SLAVE[[:space:]]+STATUS[[:space:]]|"
    "[[:space:]]*SLAVE[[:space:]]+START[[:space:]]|"
    "[[:space:]]*SLAVE[[:space:]]+STOP[[:space:]]|"
    "[[:space:]]*DELETE[[:space:]]+MULTI[[:space:]]|"
    "[[:space:]]*UPDATE[[:space:]]+MULTI[[:space:]]|"
    "[[:space:]]*INSERT[[:space:]]+SELECT[[:space:]])";

/*
  Filter for queries that can be run using the
  Stored procedures.
*/
static const char *const sp_re_str = ps_re_str;

/*
  Filter for queries that can be run as views.
*/
static const char *const view_re_str =
    "^("
    "[[:space:]]*SELECT[[:space:]])";

const char *const opt_trace_re_str =
    "^("
    "[[:space:]]*INSERT[[:space:]]|"
    "[[:space:]]*UPDATE[[:space:]]|"
    "[[:space:]]*DELETE[[:space:]]|"
    "[[:space:]]*EXPLAIN[[:space:]]|"
    "[[:space:]]*SELECT[[:space:]])";

/* Filter for queries that can be converted to EXPLAIN. */
static const char *const explain_re_str =
    "^("
    "[[:space:]]*(SELECT|DELETE|UPDATE|INSERT|REPLACE)[[:space:]])";

/* Precompiled regular expressions. */
std::regex ps_re(ps_re_str,
                 std::regex_constants::nosubs | std::regex_constants::icase);
std::regex sp_re(sp_re_str,
                 std::regex_constants::nosubs | std::regex_constants::icase);
std::regex view_re(view_re_str,
                   std::regex_constants::nosubs | std::regex_constants::icase);
std::regex opt_trace_re(opt_trace_re_str, std::regex_constants::nosubs |
                                              std::regex_constants::icase);
std::regex explain_re(explain_re_str, std::regex_constants::nosubs |
                                          std::regex_constants::icase);

extern CHARSET_INFO *charset_info;

/**
  Execute all substitutions on val.

  @param[in]      val  Pointer to the character string to be used as
                       input for the regex replace operation.
  @param[in,out]  r    Pointer to the st_replace_regex structure which
                       holds arguments and information for the command.
  @param[in,out]  len  Pointer to variable holding length of input string.

  @retval True  If substituition was made.
  @retval False If no substituition was made.

  @note
  r->buf points at the resulting buffer with all substitutions done.
  len points at length of resulting buffer.
  r->even_buf and r->odd_buf might have been reallocated.
  r->even_buf_len and r->odd_buf_len might have been changed.

  @todo
  At some point figure out if there is a way to do everything in one pass.
*/
int multi_reg_replace(struct st_replace_regex *r, char *val, size_t *len) {
  size_t i;
  char *in_buf, *out_buf;
  int *buf_len_p;

  in_buf = val;
  out_buf = r->even_buf;
  buf_len_p = &r->even_buf_len;
  r->buf = nullptr;

  /*
    For each substitution, perform replacement only if the input buffer
    is not empty.
  */
  if (*len > 0) {
    for (i = 0; i < r->regex_arr.size(); i++) {
      try {
        struct st_regex re(r->regex_arr[i]);
        char *save_out_buf = out_buf;

        std::regex rpat(re.pattern, (re.icase == 0)
                                        ? std::regex_constants::ECMAScript
                                        : std::regex_constants::icase);

        std::string sin = std::string(in_buf, *len);
        std::string sout;

        /*
          We use iterators instead of using the input buffer directly as
          it may include the null character (0x00) and characters succeeding
          them will be ignored unless we specify the start and end positions
          of the input string explicitly.
        */
        std::regex_replace(std::back_inserter(sout), sin.begin(), sin.end(),
                           rpat, re.replace, std::regex_constants::format_sed);

        /*
          If some replacement is performed, write the replaced string into the
          output buffer.
        */
        if (sout.compare(sin) != 0) {
          *len = sout.length();
          if (*len >= (uint)*buf_len_p) {
            uint need_buf_len = (*len) + 1;
            out_buf = (char *)my_realloc(PSI_NOT_INSTRUMENTED, out_buf,
                                         need_buf_len, MYF(MY_WME + MY_FAE));
            *buf_len_p = need_buf_len;
          }

          // Copy result to output buffer.
          strncpy(out_buf, sout.c_str(), *len + 1);

          // If the buffer has been reallocated, make adjustements
          if (save_out_buf != out_buf) {
            if (save_out_buf == r->even_buf)
              r->even_buf = out_buf;
            else
              r->odd_buf = out_buf;
          }

          r->buf = out_buf;
          if (in_buf == val) in_buf = r->odd_buf;

          std::swap(in_buf, out_buf);

          buf_len_p =
              (out_buf == r->even_buf) ? &r->even_buf_len : &r->odd_buf_len;
        }
      } catch (const std::regex_error &e) {
        die("Error in replace_regex for `/%s/%s/` : %s",
            (r->regex_arr[i]).pattern, (r->regex_arr[i]).replace, e.what());
      }
    }
  }

  return (r->buf == nullptr);
}

/**
  Function to check if a protocol's regular expression matches the query
  string.

  @param re  Pointer to a precompiled regular expression.
  @param str Pointer to character string in which the pattern needs to be
             searched.

  @retval 1 If the pattern is found.
  @retval 0 If the pattern is not found.
*/
int search_protocol_re(std::regex *re, const char *str) {
  while (my_isspace(charset_info, *str)) str++;
  if (str[0] == '/' && str[1] == '*') {
    const char *comm_end = strstr(str, "*/");
    if (!comm_end) die("Statement is unterminated comment");
    str = comm_end + 2;
  }

  // Check if statement matches the pattern string
  if (std::regex_search(str, *re, std::regex_constants::match_continuous)) {
    /*
      Simulate the "[^;]*$" check which follows the SQL prefix
      in the regex used to filter statements to be run with ps/
      sp protocol as using it directly in the regex is currently
      not possible due to an issue in the standard regex library.
    */
    if ((re == &ps_re || re == &sp_re) && strchr(str, ';') != nullptr) return 0;

    // Match found
    return 1;
  } else {
    return 0;
  }
}
