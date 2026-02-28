/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_FORMATTER_OPTIONS_INCLUDED
#define SQL_FORMATTER_OPTIONS_INCLUDED

#include <stddef.h>

#include "client/base/abstract_options_provider.h"
#include "client/dump/mysql_chain_element_options.h"
#include "template_utils.h"
#include "typelib.h"

namespace Mysql {
namespace Tools {
namespace Dump {

enum class enum_gtid_purged_mode : unsigned long {
  GTID_PURGED_OFF = 0,
  GTID_PURGED_AUTO = 1,
  GTID_PURGED_ON = 2
};

class Sql_formatter_options
    : public Mysql::Tools::Base::Options::Abstract_options_provider {
 public:
  Sql_formatter_options(
      const Mysql_chain_element_options *mysql_chain_element_options);

  void create_options();

  bool m_add_locks;
  bool m_charsets_consistent;
  bool m_deffer_table_indexes;
  bool m_drop_database;
  bool m_drop_table;
  bool m_drop_user;
  bool m_dump_column_names;
  bool m_hex_blob;
  bool m_insert_type_replace;
  bool m_insert_type_ignore;
  bool m_suppress_create_table;
  bool m_suppress_create_database;
  bool m_timezone_consistent;
  bool m_skip_definer;
  bool m_innodb_stats_tables_included;
  bool m_column_statistics;
  enum_gtid_purged_mode m_gtid_purged;
  const Mysql_chain_element_options *m_mysql_chain_element_options;

  const TYPELIB *get_gtid_purged_mode_typelib() {
    static const char *gtid_purged_mode_names[4] = {"OFF", "AUTO", "ON", NullS};
    TYPELIB static gtid_purged_mode_typelib = {
        array_elements(gtid_purged_mode_names) - 1, "", gtid_purged_mode_names,
        nullptr};
    return &gtid_purged_mode_typelib;
  }
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
