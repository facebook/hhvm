#ifndef PLUGIN_QUERY_REWRITE_INCLUDED
#define PLUGIN_QUERY_REWRITE_INCLUDED

/* Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.

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

#include "plugin.h"

/**
  @file include/mysql/plugin_query_rewrite.h
  API for the query rewrite plugin types (MYSQL_REWRITE_PRE_PARSE_PLUGIN and
  MYSQL_REWRITE_POST_PARSE_PLUGIN).
*/

/// Must be set by a plugin if the query is rewritten.
#define FLAG_REWRITE_PLUGIN_QUERY_REWRITTEN 1

/// Is set by the server if the query is prepared statement.
#define FLAG_REWRITE_PLUGIN_IS_PREPARED_STATEMENT 2

/// Structure that is passed during each step of a rewriting.
typedef struct Mysql_rewrite_post_parse_param {
  /**
    Indicate the status of the current rewrite.
    @see FLAG_REWRITE_PLUGIN_QUERY_REWRITTEN
    @see FLAG_REWRITE_PLUGIN_IS_PREPARED_STATEMENT
  */
  int flags;

  /// The current session.
  MYSQL_THD thd;

  /// Pointer left to the plugin to store any necessary info as needed.
  void *data;
} Mysql_rewrite_post_parse_param;

struct st_mysql_rewrite_post_parse {
  int interface_version;
  int needs_statement_digest;
  int (*rewrite)(Mysql_rewrite_post_parse_param *param);
};

/// Structure that is passed during each step of a rewriting.
typedef struct Mysql_rewrite_pre_parse_param {
  /**
    Indicate the status of the current rewrite.
    @see FLAG_REWRITE_PLUGIN_QUERY_REWRITTEN
    @see FLAG_REWRITE_PLUGIN_IS_PREPARED_STATEMENT
  */
  int flags;

  /// The current session.
  MYSQL_THD thd;

  /// Pointer left to the plugin to store any necessary info as needed.
  void *data;

  /// The query potentially to be rewritten.
  const char *query;

  /// Length of query potentially to be rewritten.
  size_t query_length;

  /// The rewritten query, if applicable.
  const char *rewritten_query;

  /// Length of the rewritten query, if applicable.
  size_t rewritten_query_length;
} Mysql_rewrite_pre_parse_param;

struct st_mysql_rewrite_pre_parse {
  int interface_version;
  int (*rewrite)(Mysql_rewrite_pre_parse_param *param);
  int (*deinit)(Mysql_rewrite_pre_parse_param *param);
};

#endif /* PLUGIN_QUERY_REWRITE_INCLUDED */
