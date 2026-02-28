/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ENUM_QUERY_TYPE_INCLUDED
#define ENUM_QUERY_TYPE_INCLUDED

/**
   Query type constants (usable as bitmap flags).
*/

enum enum_query_type {
  /// Nothing specific, ordinary SQL query.
  QT_ORDINARY = 0,

  /// In utf8.
  QT_TO_SYSTEM_CHARSET = (1 << 0),

  /// Without character set introducers.
  QT_WITHOUT_INTRODUCERS = (1 << 1),

  /**
    Causes string literals to always be printed with character set
    introducers. Takes precedence over QT_WITHOUT_INTRODUCERS.
  */
  QT_FORCE_INTRODUCERS = (1 << 2),

  /// When printing a SELECT, add its number (select_lex->number).
  QT_SHOW_SELECT_NUMBER = (1 << 3),

  /// Don't print a database if it's equal to the connection's database.
  QT_NO_DEFAULT_DB = (1 << 4),

  /// When printing a derived table, don't print its expression, only alias.
  QT_DERIVED_TABLE_ONLY_ALIAS = (1 << 5),

  /// Print in charset of Item::print() argument (typically thd->charset()).
  QT_TO_ARGUMENT_CHARSET = (1 << 6),

  /// Print identifiers without database's name.
  QT_NO_DB = (1 << 7),

  /// Print identifiers without table's name.
  QT_NO_TABLE = (1 << 8),

  /**
    Change all Item_basic_constant to ? (used by query rewrite to compute
    digest.) Un-resolved hints will also be printed in this format.
  */
  QT_NORMALIZED_FORMAT = (1 << 9),

  /**
    If an expression is constant, print the expression, not the value
    it evaluates to. Should be used for error messages, so that they
    don't reveal values.
  */
  QT_NO_DATA_EXPANSION = (1 << 10),

  /**
    Don't print the QB name. Used for the INSERT part of an INSERT...SELECT.
  */
  QT_IGNORE_QB_NAME = (1 << 11),

  /**
    Print only the QB name. Used for the SELECT part of an INSERT...SELECT.
  */
  QT_ONLY_QB_NAME = (1 << 12),

  /**
    When printing subselects, print only the select number
    (like QT_SHOW_SELECT_NUMBER), not the actual subselect text.
   */
  QT_SUBSELECT_AS_ONLY_SELECT_NUMBER = (1 << 13),

};

#endif  // ENUM_QUERY_TYPE_INCLUDED
