/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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
  A naive tool to help find code that tries to send messages intended
  for sending to the client to the error log. (The fix for such a
  situation is to find the corresponding message in the server-range
  (index >= 10,000) if it exists, or to create a new message in that
  range if it doesn't.)


  Messages to the client (via diagnostics area: my_error() etc.)

  When new or changed code sends a message to the client, there should
  be a .test case for that code path; if the message is in the wrong
  range (i.e. if it has an index >= 10,000 and therefore is in the range
  allocated to messages to the error log), our asserts should then catch
  it.


  Messages to error-log (LogErr(), LogEvent(), etc.)

  The issues that are intended to be error-logged are sometimes hard
  to raise artificially, and therefore 100 % test coverage of these
  cases can be hard to achieve. This tool is intended to help find
  some of these cases; to that end, it looks for calls to LogErr()
  and LogEvent(), parses out the ER_ symbol, and looks it up in an
  array derived from errmgs-utf8.txt to see whether its index is in
  the server (that is to say, "messages intended for the error-log")
  range, i.e. >= 10,0000. If it's not in the correct range, we print
  a message saying so. Likewise, if an undefined symbol is used, we
  print a message saying so.


  Use

  The tool can accept the path-and-name of a source file on the command
  line, like so: range_check_err sql/mysqld.cc
  If no source file is given on the command line, the tool will instead
  read paths from stdin, one per line:

    find  . -iname "*.cc" -or -iname "*.h"|
      ./runtime_output_directory/range_check_err


  Caveats

  The tool is specific to the tree and version it was built for,
  that is to say, it knows exactly those error-symbols and messages
  which are defined in that tree's share/messages_to_*.txt.  Using
  one build's tool on a different checkout is likely to render
  nonsensical results.

  The tool is in its proof-of-concept stage. Lookups can be sped up.
  The tool will disregard preprocessor directives, and thereby find
  issues in code-paths that do not get built often, which is a feature;
  it will also currently parse calls contained in comments, which is
  likely to be regarded a feature by some, and a bug by others.
  Calls contained in quoted string however are ignored by design.

  The tool currently checks the calls LogErr() and LogEvent().
*/

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "my_inttypes.h"

struct {
  const char *name;        ///< MySQL error symbol ("ER_STARTUP")
  uint mysql_errno;        ///< MySQL error code (consecutive within sections)
  const char *text;        ///< MySQL error message
  const char *odbc_state;  ///< SQL state
  const char *jdbc_state;  ///< JBDC state
  uint error_index;        ///< consecutive. 0 for obsolete.
} typedef server_error;

extern "C" {

server_error errors[] = {
#ifndef IN_DOXYGEN
#include <mysqld_ername.h>

    {nullptr, 0, nullptr, nullptr, nullptr, 0}  // DUMMY ROW
#endif                                          /* IN_DOXYGEN */
};
}

/**
  Find an error-record by it's symbol (e.g. "ER_BAD_TABLE_ERROR")

  @param   symbol     The symbol to look for as a C-string

  @retval  nullptr    No matching record found
  @retval  !=nullptr  The server_error record for the given symbol
*/
server_error *find_error_record_by_symbol(const char *symbol) {
  // naive lookup. optimize later.
  size_t error_record_count = sizeof(errors) / sizeof(server_error) - 1;

  for (size_t index = 0; index < error_record_count; index++) {
    if (0 == strcmp(symbol, errors[index].name)) return &errors[index];
  }

  return nullptr;
}

/**
  Determine whether an error-code is in a reserved range
  (in which case the tool shall not pass judgment on it).

  @param   errcode  Error-code

  @retval  true     The error-code is in a reserved range
  @retval  false    The error-code is not in a reserved range
*/
bool is_reserved_errcode(uint errcode) {
  // x plugin
  if ((errcode >= 5000) && (errcode <= 5999)) return true;
  // reserved (3rd party)
  if ((errcode >= 50000) && (errcode <= 51999)) return true;
  return false;
}

/**
  Determine whether an error-code is valid for use in error-logging.

  @param   errcode  Error-code

  @retval  true     use is legit
  @retval  false    use is a likely to be a bug
*/
bool is_valid_errlog_errcode(uint errcode) {
  return (is_reserved_errcode(errcode) || (errcode >= 10000));
}

/**
  Check a source file.

  @param   file_name  The path-and-name of a source file to verify (C-string)

  @retval  number of detected issues
*/
int check_source(const char *file_name) {
  // read the entirety of the input file (that is, a C++ source file)
  std::ifstream source_file(file_name);
  std::stringstream file_contents;
  file_contents << source_file.rdbuf();
  std::string buff = file_contents.str();

  const char *buff_start = buff.c_str();  ///< start of our text buffer
  const char *curr = buff_start;          ///< current read position

  std::string stmt;        ///< string containing the LogE*() statement
  const char *stmt_start;  ///< start of LogE*() statement in text buffer

  int is_err;     ///< set t if LogErr() (rather than LogEvent())
  int count = 0;  ///< number of detected issues ("wrong" ER_* symbols)

  /*
    Look for LogErr() and LogEvent() statements in the buffer
    containing the source file.
  */
  while ((curr = strstr(curr, "LogE")) != nullptr) {
    if (((0 == (is_err = strncmp(curr, "LogErr(", 7))) ||
         (0 == strncmp(curr, "LogEvent(", 9))) &&
        ((curr == buff_start) || (isspace((int)curr[-1])))) {
      stmt_start = curr;  // remember start of the LogE*() statement

      curr = strchr(curr, '(');  // strcmp() told us this won't fail

      // find the end of the LogE*() statement
      do {
        if (*curr == '\"') {  // on opening quote, skip string
          curr++;

          // skip to end of string, handling \"
          while ((*curr != '\0') && (*curr != '\"')) {
            if ((curr[0] == '\\') && (curr[1] == '\"'))  // \" isn't end-quote
              curr++;
            curr++;
          }
          if (*curr != '\0')  // position after string
            curr++;
        } else  // we're not in a quoted string, proceed with next character
          curr++;
      } while ((*curr != '\0') && (*curr != ';'));  // until end of stmt/buffer

      if (*curr != '\0') {
        /*
          We found something that looks like a LogE*() statement.
          Now check whether it's valid!
        */

        std::string symbol;     ///< ER_FOO-type symbol
        const char *sym_start,  ///< symbol start
            *sym_end;           ///< symbol end

        // copy entire statement to variable and remove line-breaks
        stmt.assign(stmt_start, (size_t)(curr - stmt_start + 1L));
        std::replace(stmt.begin(), stmt.end(), '\n', ' ');

        if (is_err == 0) {  // It's LogErr(), symbol is 2nd argument
          if ((sym_start = strchr(stmt_start, ',')) != nullptr) {
            while ((*sym_start == ',') || (isspace((int)*sym_start)))
              sym_start++;
          }
        } else {  // It's LogEvent(), symbol can appear anywhere
          if ((sym_start = strstr(stmt_start, "(ER_")) != nullptr) sym_start++;
        }

        /*
          The symbol we find was behind statement end, so statement
          did not contain a symbol (and therefore was faulty).
        */
        if (sym_start > curr) sym_start = nullptr;

        if (sym_start != nullptr) {
          // We found a symbol. Find its end!
          sym_end = sym_start;
          while (isupper((int)*sym_end) || isdigit((int)*sym_end) ||
                 (*sym_end == '_'))
            sym_end++;

          // Save symbol in variable.
          symbol.assign(sym_start, (size_t)(sym_end - sym_start));

          /*
            We may not have a symbol here
            - if the macro name was referenced in a comment ("LogErr()")
            - if a variable instead of a constant is used
          */
          if (sym_start != sym_end) {
            // look up the symbol
            server_error *err_record;
            err_record = find_error_record_by_symbol(symbol.c_str());

            if (err_record == nullptr) {
              // symbol unknown, complain!
              count++;
              std::cerr << file_name
                        << ": error logging statement using unknown symbol "
                        << symbol << " found: \"" << stmt << "\"\n";
            } else if (!is_valid_errlog_errcode(err_record->mysql_errno)) {
              // symbol known, but in wrong range; complain!
              count++;
              std::cerr << file_name << ": error logging statement using "
                        << symbol << " (" << err_record->mysql_errno
                        << ") found: \"" << stmt << "\"\n";
            }
          }  // potential symbol non-empty?
        }    // found start of potential symbol?
      }      // not end of buffer?
    } else   // found "LogE" is not actually part of LogErr() or LogEvent() call
      curr++;  // skip bogus LogE
  }            // while())

  return count;  // return number of detected issues
}

/**
  Read paths of source files from stdin, one per line, and verify the files.

  @retval  number of detected issues
*/
int check_stdin(void) {
  int count = 0;

  std::string line;
  while (getline(std::cin, line)) {
    count += check_source(line.c_str());
  }

  return count;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    return check_stdin();
  } else if (argc == 2) {
    return check_source(argv[1]);
  } else {
    std::cerr << argv[0] << " [</path/to/source_file.cc>]"
              << "\n\n"
              << "If no source file is given, we will read file names, "
              << "one per line, from stdin."
              << "\n";
  }

  return 0;
}
