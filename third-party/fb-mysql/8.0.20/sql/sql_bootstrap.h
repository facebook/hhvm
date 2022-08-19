/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_BOOTSTRAP_H
#define SQL_BOOTSTRAP_H

#include <stddef.h>
#include <functional>

#include "map_helpers.h"

struct MYSQL_FILE;

/**
  The maximum size of a bootstrap query.
  Increase this size if parsing a longer query during bootstrap is necessary.
  The longest query in use depends on the documentation content,
  see the file fill_help_tables.sql
*/
#define MAX_BOOTSTRAP_QUERY_SIZE 74000
/**
  The maximum size of a bootstrap query, expressed in a single line.
  Do not increase this size, use the multiline syntax instead.
*/
#define MAX_BOOTSTRAP_LINE_SIZE 74000

enum bootstrap_error {
  READ_BOOTSTRAP_SUCCESS = 0,
  READ_BOOTSTRAP_EOF,
  READ_BOOTSTRAP_IO,
  READ_BOOTSTRAP_DELIMITER,
  READ_BOOTSTRAP_SQ_NOT_TERMINATED,
  READ_BOOTSTRAP_DQ_NOT_TERMINATED,
  READ_BOOTSTRAP_COMMENT_NOT_TERMINATED,
  READ_BOOTSTRAP_QUERY_SIZE,
  READ_BOOTSTRAP_ERROR
};

typedef char *(*fgets_fn_t)(char *, size_t, MYSQL_FILE *, int *error);

enum delimiter_state {
  /** Delimiter is ';' */
  DELIMITER_SEMICOLON,
  /** Delimiter is "$$" */
  DELIMITER_DOLLAR_DOLLAR
};

enum code_parsing_state {
  /** Parsing sql code. */
  NORMAL,
  /** Parsing a 'literal' string. */
  IN_SINGLE_QUOTE,
  /** Parsing a "literal" string. */
  IN_DOUBLE_QUOTE,
  /** Parsing a "--" comment. */
  IN_DASH_DASH_COMMENT,
  /** Parsing a '/''*' comment. */
  IN_SLASH_STAR_COMMENT,
  /** Parsing a '#' comment. */
  IN_POUND_COMMENT
};

struct bootstrap_parser_position {
  void init();

  size_t m_line;
  size_t m_column;
};

struct bootstrap_parser_state {
  // This buffer may be rather large,
  // so we allocate it on the heap to save stack space.
  // This struct is also used by the standalone 'comp_sql' tool,
  // so we use plain malloc/free rather than my_() to avoid dependency on mysys.
  unique_ptr_free<char> m_unget_buffer;
  size_t m_unget_buffer_length;

  typedef void (*log_function_t)(const char *message);

  void init(const char *filename);
  void report_error_details(log_function_t log);

  delimiter_state m_delimiter;
  code_parsing_state m_code_state;

  const char *m_filename;
  size_t m_current_line;
  enum bootstrap_error m_last_error;
  int m_io_sub_error;

  bootstrap_parser_position m_last_delimiter;
  bootstrap_parser_position m_last_open_single_quote;
  bootstrap_parser_position m_last_open_double_quote;
  bootstrap_parser_position m_last_open_comment;
  bootstrap_parser_position m_last_query_start;
};

int read_bootstrap_query(char *query, size_t *query_length, MYSQL_FILE *input,
                         fgets_fn_t fgets_fn, bootstrap_parser_state *state);

#endif
