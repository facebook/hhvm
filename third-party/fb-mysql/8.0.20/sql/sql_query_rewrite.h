#ifndef SQL_QUERY_REWRITE_INCLUDED
#define SQL_QUERY_REWRITE_INCLUDED

/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

class Parser_state;
class THD;

/**
  Calls the query rewrite plugins' respective rewrite functions before parsing
  the query.

  @param[in] thd The session sending the query to be rewritten.
*/
void invoke_pre_parse_rewrite_plugins(THD *thd);

/**
  Enables digests in the parser state if any plugin needs it.

  @param thd The session.

  @param ps This parser state will have digests enabled if any plugin
  needs it.

  @note For the time being, only post-parse query rewrite plugins are able to
  request digests. If other plugin types need the same, this function needs to
  be modified.
*/
void enable_digest_if_any_plugin_needs_it(THD *thd, Parser_state *ps);

/**
  Calls query rewrite plugins after parsing the query.

  @param[in] thd The session with the query to be rewritten.
  @param is_prepared True if the query was a prepared statement.
*/
bool invoke_post_parse_rewrite_plugins(THD *thd, bool is_prepared);

#endif /* SQL_QUERY_REWRITE_INCLUDED */
