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

#include "sql/sql_bootstrap.h"

#include <ctype.h>
#include <string.h>

#include "map_helpers.h"

#include "m_string.h"
#include "my_dbug.h"

void bootstrap_parser_position::init() {
  m_line = 0;
  m_column = 0;
}

void bootstrap_parser_state::init(const char *filename) {
  m_unget_buffer.reset(static_cast<char *>(malloc(MAX_BOOTSTRAP_LINE_SIZE)));
  m_delimiter = DELIMITER_SEMICOLON;
  m_code_state = NORMAL;
  m_filename = filename;
  m_current_line = 0;
  m_last_error = READ_BOOTSTRAP_SUCCESS;
  m_io_sub_error = 0;
  m_last_delimiter.init();
  m_last_open_single_quote.init();
  m_last_open_double_quote.init();
  m_last_open_comment.init();
  m_last_query_start.init();
  m_unget_buffer_length = 0;
}

void bootstrap_parser_state::report_error_details(log_function_t log) {
  char buffer[1024];

  switch (m_last_error) {
    case READ_BOOTSTRAP_IO:
      snprintf(buffer, sizeof(buffer),
               "Input Output error while reading file %s, line %zu, I/O error "
               "code %d\n",
               m_filename, m_current_line, m_io_sub_error);
      break;
    case READ_BOOTSTRAP_DELIMITER:
      snprintf(buffer, sizeof(buffer),
               "Unsupported DELIMITER, file %s, line %zu\n", m_filename,
               m_current_line);
      break;
    case READ_BOOTSTRAP_SQ_NOT_TERMINATED:
      snprintf(buffer, sizeof(buffer),
               "End of file %s at line %zu, while inside a single quote string "
               "started at %zu:%zu\n",
               m_filename, m_current_line, m_last_open_single_quote.m_line,
               m_last_open_single_quote.m_column);
      break;
    case READ_BOOTSTRAP_DQ_NOT_TERMINATED:
      snprintf(buffer, sizeof(buffer),
               "End of file %s at line %zu, while inside a double quote string "
               "started at %zu:%zu\n",
               m_filename, m_current_line, m_last_open_double_quote.m_line,
               m_last_open_double_quote.m_column);
      break;
    case READ_BOOTSTRAP_COMMENT_NOT_TERMINATED:
      snprintf(buffer, sizeof(buffer),
               "End of file %s at line %zu, while inside a comment started at "
               "%zu:%zu\n",
               m_filename, m_current_line, m_last_open_comment.m_line,
               m_last_open_comment.m_column);
      break;
    case READ_BOOTSTRAP_QUERY_SIZE:
      snprintf(buffer, sizeof(buffer),
               "Max query size reached at file %s, line %zu, query started at "
               "%zu:%zu\n",
               m_filename, m_current_line, m_last_query_start.m_line,
               m_last_query_start.m_column);
      break;
    case READ_BOOTSTRAP_ERROR:
    default:
      snprintf(buffer, sizeof(buffer),
               "Unknown parsing error at file %s, line %zu\n", m_filename,
               m_current_line);
      break;
  }

  log(buffer);
}

int read_bootstrap_query(char *query, size_t *query_length, MYSQL_FILE *input,
                         fgets_fn_t fgets_fn, bootstrap_parser_state *state) {
  /* Allow for up to 3 extra characters in lookup. */
  unique_ptr_free<char> line_buffer(
      static_cast<char *>(malloc(MAX_BOOTSTRAP_LINE_SIZE + 3)));
  char *line;
  size_t len;
  size_t query_len = 0;
  int fgets_error = 0;

  state->m_last_query_start.m_line = state->m_current_line;
  state->m_last_query_start.m_column = 1;

  for (;;) {
    if (state->m_unget_buffer_length == 0) {
      /* Read a line from the init file. */
      line = (*fgets_fn)(line_buffer.get(), MAX_BOOTSTRAP_LINE_SIZE, input,
                         &fgets_error);
    } else {
      /* Read from a previous line, partially consumed. */
      memcpy(line_buffer.get(), state->m_unget_buffer.get(),
             state->m_unget_buffer_length);
      line = line_buffer.get();
      state->m_unget_buffer_length = 0;
    }

    if (fgets_error != 0) {
      state->m_last_error = READ_BOOTSTRAP_IO;
      state->m_io_sub_error = fgets_error;
      return READ_BOOTSTRAP_IO;
    }

    if (line == nullptr) {
      if (query_len == 0) {
        state->m_last_error = READ_BOOTSTRAP_EOF;
        return READ_BOOTSTRAP_EOF;
      }

      switch (state->m_code_state) {
        case NORMAL:
          /*
            The last line is terminated by EOF".
            Return the query found.
          */
          query[query_len] = '\0';
          *query_length = query_len;
          state->m_last_error = READ_BOOTSTRAP_SUCCESS;
          return READ_BOOTSTRAP_SUCCESS;
        case IN_SINGLE_QUOTE:
          state->m_last_error = READ_BOOTSTRAP_SQ_NOT_TERMINATED;
          return READ_BOOTSTRAP_SQ_NOT_TERMINATED;
        case IN_DOUBLE_QUOTE:
          state->m_last_error = READ_BOOTSTRAP_DQ_NOT_TERMINATED;
          return READ_BOOTSTRAP_DQ_NOT_TERMINATED;
        case IN_SLASH_STAR_COMMENT:
          state->m_last_error = READ_BOOTSTRAP_COMMENT_NOT_TERMINATED;
          return READ_BOOTSTRAP_COMMENT_NOT_TERMINATED;
        case IN_DASH_DASH_COMMENT:
        case IN_POUND_COMMENT:
        default:
          DBUG_ASSERT(false);
          state->m_last_error = READ_BOOTSTRAP_ERROR;
          return READ_BOOTSTRAP_ERROR;
      };
    }

    len = strlen(line);

    /*
      Cleanly end the string, so we don't have to test len > x
      all the time before reading line[x], line[x+1], line[x+2],
      in the code below.
    */
    line_buffer.get()[len] = '\0';
    line_buffer.get()[len + 1] = '\0';
    line_buffer.get()[len + 2] = '\0';

    size_t lead_whitespace_len = 0;

    /*
      Do not be confused by:
      - leading whitespace in multi line single quoted strings
      - leading whitespace in multi line double quoted strings
      - leading whitespace in multi line comments
    */
    if (state->m_code_state == NORMAL) {
      /* Find lead white space */
      while (isspace(line[lead_whitespace_len]) && lead_whitespace_len < len) {
        lead_whitespace_len++;
      }

      /* Skip blank lines */
      if (lead_whitespace_len == len) {
        continue;
      }
    }

    /*
      Do not be confused by:
      - DELIMITER in multi line single quoted strings
      - DELIMITER in multi line double quoted strings
      - DELIMITER in multi line comments
    */
    if (state->m_code_state == NORMAL) {
      if (native_strncasecmp(line + lead_whitespace_len, "DELIMITER ;", 11) ==
          0) {
        state->m_current_line++;
        state->m_delimiter = DELIMITER_SEMICOLON;
        state->m_last_delimiter.m_line = state->m_current_line;
        state->m_last_delimiter.m_column = lead_whitespace_len + 1;
        continue;
      }

      if (native_strncasecmp(line + lead_whitespace_len, "DELIMITER $$", 12) ==
          0) {
        state->m_current_line++;
        state->m_delimiter = DELIMITER_DOLLAR_DOLLAR;
        state->m_last_delimiter.m_line = state->m_current_line;
        state->m_last_delimiter.m_column = lead_whitespace_len + 1;
        continue;
      }

      if (native_strncasecmp(line + lead_whitespace_len, "DELIMITER", 9) == 0) {
        state->m_current_line++;
        /* This is not a generic client, use either ';' or '$$' as delimiter. */
        state->m_last_error = READ_BOOTSTRAP_DELIMITER;
        DBUG_ASSERT(false);
        return READ_BOOTSTRAP_DELIMITER;
      }
    }

    bool found_delimiter = false;
    size_t after_delimiter_index = 0;
    size_t remaining_line_index = 0;

    for (size_t i = 0; (i < len) && !found_delimiter; i++) {
      switch (state->m_code_state) {
        case NORMAL:
          if (line[i] == '\'') {
            state->m_code_state = IN_SINGLE_QUOTE;
            state->m_last_open_single_quote.m_line = state->m_current_line + 1;
            state->m_last_open_single_quote.m_column = i + 1;
          } else if (line[i] == '\"') {
            state->m_code_state = IN_DOUBLE_QUOTE;
            state->m_last_open_double_quote.m_line = state->m_current_line + 1;
            state->m_last_open_double_quote.m_column = i + 1;
          } else if (line[i] == '#') {
            state->m_code_state = IN_POUND_COMMENT;
            state->m_last_open_comment.m_line = state->m_current_line + 1;
            state->m_last_open_comment.m_column = i + 1;
          } else if (line[i] == '/') {
            if (line[i + 1] == '*') {
              state->m_code_state = IN_SLASH_STAR_COMMENT;
              state->m_last_open_comment.m_line = state->m_current_line + 1;
              state->m_last_open_comment.m_column = i + 1;
              i++;
            }
          } else if (line[i] == '-') {
            if ((line[i + 1] == '-') && isspace(line[i + 2])) {
              state->m_code_state = IN_DASH_DASH_COMMENT;
              state->m_last_open_comment.m_line = state->m_current_line + 1;
              state->m_last_open_comment.m_column = i + 1;
              i += 2;
            }
          } else if ((line[i] == ';') &&
                     (state->m_delimiter == DELIMITER_SEMICOLON)) {
            after_delimiter_index = i + 1;
            remaining_line_index = i + 1;
            found_delimiter = true;
          } else if ((line[i] == '$') && (line[i + 1] == '$') &&
                     (state->m_delimiter == DELIMITER_DOLLAR_DOLLAR)) {
            line[i] = '\0';
            after_delimiter_index = i;
            remaining_line_index = i + 2;
            found_delimiter = true;
          }
          break;
        case IN_SINGLE_QUOTE:
          if (line[i] == '\\') {
            if (line[i + 1] == '\\') {
              /* ' slash: \\ in string ' */
              i++;
            } else if (line[i + 1] == '\'') {
              /* ' single quote: \' in string ' */
              i++;
            }
          } else if (line[i] == '\'') {
            if (line[i + 1] == '\'') {
              /* ' single quote: '' in string ' */
              i++;
            } else {
              state->m_code_state = NORMAL;
            }
          }
          break;
        case IN_DOUBLE_QUOTE:
          if (line[i] == '\\') {
            if (line[i + 1] == '\\') {
              /* " slash: \\ in string " */
              i++;
            } else if (line[i + 1] == '\"') {
              /* " double quote: \" in string " */
              i++;
            }
          } else if (line[i] == '\"') {
            if (line[i + 1] == '\"') {
              /* " double quote: "" in string " */
              i++;
            } else {
              state->m_code_state = NORMAL;
            }
          }
          break;
        case IN_POUND_COMMENT:
          break;
        case IN_SLASH_STAR_COMMENT:
          if ((line[i] == '*') && (line[i + 1] == '/')) {
            i++;
            state->m_code_state = NORMAL;
          }
          break;
        case IN_DASH_DASH_COMMENT:
          break;
      }
    }

    if ((state->m_code_state == IN_POUND_COMMENT) ||
        (state->m_code_state == IN_DASH_DASH_COMMENT)) {
      /* Since we processed a full line, the comment ends. */
      state->m_code_state = NORMAL;
      DBUG_ASSERT(!found_delimiter);
    }

    /* Append the current line to a multi line query. If the new line will make
       the query too long, preserve the partial line to provide context for the
       error message.
    */
    if (query_len + len + 1 >= MAX_BOOTSTRAP_QUERY_SIZE) {
      size_t new_len = MAX_BOOTSTRAP_QUERY_SIZE - query_len - 1;
      if ((new_len > 0) && (query_len < MAX_BOOTSTRAP_QUERY_SIZE)) {
        memcpy(query + query_len, line, new_len);
        query_len += new_len;
      }
      query[query_len] = '\0';
      *query_length = query_len;
      state->m_last_error = READ_BOOTSTRAP_QUERY_SIZE;
      return READ_BOOTSTRAP_QUERY_SIZE;
    }

    state->m_current_line++;

    if (!found_delimiter) {
      /* Append the full current line to the query */
      memcpy(query + query_len, line, len);
      query_len += len;
    } else {
      /*
        Append the partial current line to the query,
        up to the delimiter.
      */
      size_t partial_len = after_delimiter_index;
      memcpy(query + query_len, line, partial_len);
      query_len += partial_len;

      /* Scan for whitespace after the delimiter */
      while (isspace(line[remaining_line_index]) &&
             (remaining_line_index < len)) {
        remaining_line_index++;
      }

      if (remaining_line_index < len) {
        const char *remaining_line = &line[remaining_line_index];
        size_t remaining_len = len - remaining_line_index;
        DBUG_ASSERT(remaining_len + 1 < MAX_BOOTSTRAP_LINE_SIZE);
        /* Unput a partial line, including a terminating '\0' */
        memcpy(state->m_unget_buffer.get(), remaining_line, remaining_len + 1);
        state->m_unget_buffer_length = remaining_len;
        state->m_current_line--;
      }

      /* Return the query found. */
      query[query_len] = '\0';
      *query_length = query_len;
      state->m_last_error = READ_BOOTSTRAP_SUCCESS;
      return READ_BOOTSTRAP_SUCCESS;
    }
  }
}
