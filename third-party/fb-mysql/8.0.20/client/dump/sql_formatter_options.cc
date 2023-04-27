/*
  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client/dump/sql_formatter_options.h"

using namespace Mysql::Tools::Dump;

void Sql_formatter_options::create_options() {
  this->create_new_option(
      &m_add_locks, "add-locks",
      "Wrap data inserts on table with write lock on that table in output. "
      "This doesn't work with parallelism.");
  this->create_new_option(&m_drop_database, "add-drop-database",
                          "Add a DROP DATABASE before each CREATE DATABASE.");
  this->create_new_option(&m_drop_table, "add-drop-table",
                          "Add a DROP TABLE before each CREATE TABLE.");
  this->create_new_option(&m_drop_user, "add-drop-user",
                          "Add a DROP USER before each CREATE USER.");
  this->create_new_option(
      &m_dump_column_names, "complete-insert",
      "Use complete insert statements, include column names.");
  this->create_new_option(
          &m_deffer_table_indexes, "defer-table-indexes",
          "Defer addition of indexes of table to be added after all rows are "
          "dumped.")
      ->set_value(true);
  this->create_new_option(
      &m_insert_type_replace, "replace",
      "Use REPLACE INTO for dumped rows instead of INSERT INTO.");
  this->create_new_option(
      &m_insert_type_ignore, "insert-ignore",
      "Use INSERT IGNORE INTO for dumped rows instead of INSERT INTO.");
  this->create_new_option(&m_suppress_create_table, "no-create-info",
                          "Suppress CREATE TABLE statements.")
      ->set_short_character('t');
  this->create_new_option(&m_suppress_create_database, "no-create-db",
                          "Suppress CREATE DATABASE statements.");
  this->create_new_option(
      &m_hex_blob, "hex-blob",
      "Dump binary strings (in fields of type BINARY, VARBINARY, BLOB, ...) "
      "in hexadecimal format.");
  this->create_new_option(
          &m_timezone_consistent, "tz-utc",
          "SET TIME_ZONE='+00:00' at top of dump to allow dumping of TIMESTAMP "
          "data when a server has data in different time zones or data is "
          "being "
          "moved between servers with different time zones.")
      ->set_value(true);
  this->create_new_option(&m_charsets_consistent, "set-charset",
                          "Add 'SET NAMES default_character_set' to the output "
                          "to keep charsets "
                          "consistent.")
      ->set_value(true);
  this->create_new_option(
      &m_skip_definer, "skip-definer",
      "Skip DEFINER and SQL SECURITY clauses for Views and Stored Routines.");
  this->create_new_enum_option(
          &m_gtid_purged, get_gtid_purged_mode_typelib(), "set-gtid-purged",
          "Add 'SET @@GLOBAL.GTID_PURGED' to the output. Possible values for "
          "this option are ON, OFF and AUTO. If ON is used and GTIDs "
          "are not enabled on the server, an error is generated. If OFF is "
          "used, this option does nothing. If AUTO is used and GTIDs are "
          "enabled "
          "on the server, 'SET @@GLOBAL.GTID_PURGED' is added to the output. "
          "If GTIDs are disabled, AUTO does nothing. If no value is supplied "
          "then the default (AUTO) value will be considered.")
      ->set_value(enum_gtid_purged_mode::GTID_PURGED_AUTO);
  this->create_new_option(
          &m_column_statistics, "column-statistics",
          "Add an ANALYZE TABLE statement to regenerate any existing column "
          "statistics.")
      ->set_value(false);
}

Sql_formatter_options::Sql_formatter_options(
    const Mysql_chain_element_options *mysql_chain_element_options)
    : m_innodb_stats_tables_included(false),
      m_gtid_purged(enum_gtid_purged_mode::GTID_PURGED_AUTO),
      m_mysql_chain_element_options(mysql_chain_element_options) {}
