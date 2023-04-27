#ifndef MYSQL_SERVICE_PARSER_INCLUDED
#define MYSQL_SERVICE_PARSER_INCLUDED
/*  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <mysql/mysql_lex_string.h>

#ifndef MYSQL_ABI_CHECK
#include <stdlib.h>
#endif

class THD;
class Item;
#define MYSQL_THD THD *
typedef Item *MYSQL_ITEM;

/**
  @file include/mysql/service_parser.h

  Plugin service that provides access to the parser and some operations on the
  parse tree.
*/

/* See DIGEST_HASH_SIZE in sql/sql_digest.h */
#define PARSER_SERVICE_DIGEST_LENGTH 16

#define STATEMENT_TYPE_SELECT 1
#define STATEMENT_TYPE_UPDATE 2
#define STATEMENT_TYPE_INSERT 3
#define STATEMENT_TYPE_DELETE 4
#define STATEMENT_TYPE_REPLACE 5
#define STATEMENT_TYPE_OTHER 6

typedef int (*parse_node_visit_function)(MYSQL_ITEM item, unsigned char *arg);

typedef int (*sql_condition_handler_function)(int sql_errno,
                                              const char *sqlstate,
                                              const char *msg, void *state);

struct my_thread_handle;

typedef MYSQL_THD (*mysql_current_session_t)();

typedef MYSQL_THD (*mysql_open_session_t)();

typedef void (*mysql_start_thread_t)(MYSQL_THD thd,
                                     void *(*callback_fun)(void *), void *arg,
                                     struct my_thread_handle *thread_handle);

typedef void (*mysql_join_thread_t)(struct my_thread_handle *thread_handle);

typedef void (*mysql_set_current_database_t)(MYSQL_THD thd,
                                             const MYSQL_LEX_STRING db);

/**
  Parses the query.

  @param thd The session in which to parse.

  @param query The query to parse.

  @param is_prepared If non-zero, the query will be parsed as a prepared
  statement and won't throw errors when the query string contains '?'.

  @param handle_condition Callback function that is called if a condition is
  raised during the preparation, parsing or cleanup after parsing. If this
  argument is non-NULL, the diagnostics area will be cleared before this
  function returns.

  @param condition_handler_state Will be passed to handle_condition when
  called. Otherwise ignored.

  @retval 0 Success.
  @retval 1 Parse error.
*/
typedef int (*mysql_parse_t)(MYSQL_THD thd, const MYSQL_LEX_STRING query,
                             unsigned char is_prepared,
                             sql_condition_handler_function handle_condition,
                             void *condition_handler_state);

typedef int (*mysql_get_statement_type_t)(MYSQL_THD thd);

/**
  Returns the digest of the last parsed statement in the session.

  @param thd The session in which the statement was parsed.

  @param [out] digest An area of at least size PARSER_SERVICE_DIGEST_LENGTH,
  where the digest is written.

  @retval 0 Success.
  @retval 1 Parse error.
*/
typedef int (*mysql_get_statement_digest_t)(MYSQL_THD thd,
                                            unsigned char *digest);

/**
  Returns the number of parameters ('?') in the parsed query.
  This works only if the last query was parsed as a prepared statement.

  @param thd The session in which the query was parsed.

  @return The number of parameter markers.
*/
typedef int (*mysql_get_number_params_t)(MYSQL_THD thd);

/**
  Stores in 'positions' the positions in the last parsed query of each
  parameter marker('?'). Positions must be an already allocated array of at
  least mysql_parser_service_st::mysql_get_number_params() size. This works
  only if the last query was parsed as a prepared statement.

  @param thd The session in which the query was parsed.

  @param positions An already allocated array of at least
  mysql_parser_service_st::mysql_get_number_params() size.

  @return The number of parameter markers and hence number of written
  positions.
*/
typedef int (*mysql_extract_prepared_params_t)(MYSQL_THD thd, int *positions);

/**
  Walks the tree depth first and applies a user defined function on each
  literal.

  @param thd The session in which the query was parsed.

  @param processor Will be called for each literal in the parse tree.

  @param arg Will be passed as argument to each call to 'processor'.
*/
typedef int (*mysql_visit_tree_t)(MYSQL_THD thd,
                                  parse_node_visit_function processor,
                                  unsigned char *arg);

/**
  Renders the MYSQL_ITEM as a string and returns a reference in the form of
  a MYSQL_LEX_STRING. The string buffer is allocated by the server and must
  be freed by mysql_free_string().

  @param item The literal to print.

  @return The result of printing the literal.

  @see mysql_parser_service_st::mysql_free_string().
*/
typedef MYSQL_LEX_STRING (*mysql_item_string_t)(MYSQL_ITEM item);

/**
  Frees a string buffer allocated by the server.

  @param string The string whose buffer will be freed.
*/
typedef void (*mysql_free_string_t)(MYSQL_LEX_STRING string);

/**
  Returns the current query string. This string is managed by the server and
  should @b not be freed by a plugin.

  @param thd The session in which the query was submitted.

  @return The query string.
*/
typedef MYSQL_LEX_STRING (*mysql_get_query_t)(MYSQL_THD thd);

/**
  Returns the current query in normalized form. This string is managed by
  the server and should @b not be freed by a plugin.

  @param thd The session in which the query was submitted.

  @return The query string normalized.
*/
typedef MYSQL_LEX_STRING (*mysql_get_normalized_query_t)(MYSQL_THD thd);

extern "C" struct mysql_parser_service_st {
  mysql_current_session_t mysql_current_session;
  mysql_open_session_t mysql_open_session;
  mysql_start_thread_t mysql_start_thread;
  mysql_join_thread_t mysql_join_thread;
  mysql_set_current_database_t mysql_set_current_database;
  mysql_parse_t mysql_parse;
  mysql_get_statement_type_t mysql_get_statement_type;
  mysql_get_statement_digest_t mysql_get_statement_digest;
  mysql_get_number_params_t mysql_get_number_params;
  mysql_extract_prepared_params_t mysql_extract_prepared_params;
  mysql_visit_tree_t mysql_visit_tree;
  mysql_item_string_t mysql_item_string;
  mysql_free_string_t mysql_free_string;
  mysql_get_query_t mysql_get_query;
  mysql_get_normalized_query_t mysql_get_normalized_query;
} * mysql_parser_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define mysql_parser_current_session() \
  mysql_parser_service->mysql_current_session()

#define mysql_parser_open_session() mysql_parser_service->mysql_open_session()

#define mysql_parser_start_thread(thd, func, arg, thread_handle) \
  mysql_parser_service->mysql_start_thread(thd, func, arg, thread_handle)

#define mysql_parser_join_thread(thread_handle) \
  mysql_parser_service->mysql_join_thread(thread_handle)

#define mysql_parser_set_current_database(thd, db) \
  mysql_parser_service->mysql_set_current_database(thd, db)

#define mysql_parser_parse(thd, query, is_prepared, condition_handler, \
                           condition_handler_state)                    \
  mysql_parser_service->mysql_parse(                                   \
      thd, query, is_prepared, condition_handler, condition_handler_state)

#define mysql_parser_get_statement_type(thd) \
  mysql_parser_service->mysql_get_statement_type(thd)

#define mysql_parser_get_statement_digest(thd, digest) \
  mysql_parser_service->mysql_get_statement_digest(thd, digest)

#define mysql_parser_get_number_params(thd) \
  mysql_parser_service->mysql_get_number_params(thd)

#define mysql_parser_extract_prepared_params(thd, positions) \
  mysql_parser_service->mysql_extract_prepared_params(thd, positions)

#define mysql_parser_visit_tree(thd, processor, arg) \
  mysql_parser_service->mysql_visit_tree(thd, processor, arg)

#define mysql_parser_item_string(item) \
  mysql_parser_service->mysql_item_string(item)

#define mysql_parser_free_string(string) \
  mysql_parser_service->mysql_free_string(string)

#define mysql_parser_get_query(thd) mysql_parser_service->mysql_get_query(thd)

#define mysql_parser_get_normalized_query(thd) \
  mysql_parser_service->mysql_get_normalized_query(thd)

#else
typedef void *(*callback_function)(void *);
MYSQL_THD mysql_parser_current_session();
MYSQL_THD mysql_parser_open_session();
void mysql_parser_start_thread(MYSQL_THD thd, callback_function fun, void *arg,
                               struct my_thread_handle *thread_handle);
void mysql_parser_join_thread(struct my_thread_handle *thread_handle);
void mysql_parser_set_current_database(MYSQL_THD thd,
                                       const MYSQL_LEX_STRING db);
int mysql_parser_parse(MYSQL_THD thd, const MYSQL_LEX_STRING query,
                       unsigned char is_prepared,
                       sql_condition_handler_function handle_condition,
                       void *condition_handler_state);
int mysql_parser_get_statement_type(MYSQL_THD thd);
int mysql_parser_get_statement_digest(MYSQL_THD thd, unsigned char *digest);
int mysql_parser_get_number_params(MYSQL_THD thd);
int mysql_parser_extract_prepared_params(MYSQL_THD thd, int *positions);
int mysql_parser_visit_tree(MYSQL_THD thd, parse_node_visit_function processor,
                            unsigned char *arg);
MYSQL_LEX_STRING mysql_parser_item_string(MYSQL_ITEM item);
void mysql_parser_free_string(MYSQL_LEX_STRING string);
MYSQL_LEX_STRING mysql_parser_get_query(MYSQL_THD thd);
MYSQL_LEX_STRING mysql_parser_get_normalized_query(MYSQL_THD thd);

#endif /* MYSQL_DYNAMIC_PLUGIN */

#endif /* MYSQL_SERVICE_PARSER_INCLUDED */
