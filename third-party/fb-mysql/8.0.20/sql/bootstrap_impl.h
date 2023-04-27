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

#ifndef BOOTSTRAP_IMPL_H
#define BOOTSTRAP_IMPL_H 1
#include <string>

#include "sql/sql_bootstrap.h"  // MAX_BOOTSTRAP_QUERY_SIZE

namespace bootstrap {

/** Abstract interface to reading bootstrap commands */
class Command_iterator {
 public:
  typedef void (*log_function_t)(const char *message);

  /**
    start processing the iterator
    @retval false Success
    @retval true failure
  */
  virtual bool begin(void) { return false; }

  /**
    Get the next query string.

    @param[out] query return the query
    @return one of the READ_BOOTSTRAP
  */
  virtual int next(std::string &query) = 0;

  virtual void report_error_details(log_function_t log) = 0;

  /** End processing the iterator */
  virtual void end(void) {}

 protected:
  Command_iterator() {}
  ~Command_iterator() {}
};

/** File bootstrap command reader */
class File_command_iterator : public Command_iterator {
 public:
  File_command_iterator(const char *file_name, MYSQL_FILE *input,
                        fgets_fn_t fgets_fn)
      : m_input(input), m_fgets_fn(fgets_fn) {
    m_parser_state.init(file_name);
  }
  virtual ~File_command_iterator();

  int next(std::string &query) override;

  void report_error_details(log_function_t log) override;

 protected:
  bootstrap_parser_state m_parser_state;
  MYSQL_FILE *m_input;
  fgets_fn_t m_fgets_fn;
};

}  // namespace bootstrap

#endif /* BOOTSTRAP_IMPL_H */
