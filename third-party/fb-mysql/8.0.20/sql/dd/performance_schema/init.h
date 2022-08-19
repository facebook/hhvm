/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__PERFORMANCE_SCHEMA_INIT_PFS_INCLUDED
#define DD__PERFORMANCE_SCHEMA_INIT_PFS_INCLUDED

class THD;

namespace dd {
class Properties;
enum class enum_dd_init_type;
}  // namespace dd

namespace dd {
namespace performance_schema {

/**
  Create the performance schema.

  @param thd    Thread context.

  @return       Return true upon failure, otherwise false.
*/

bool create_pfs_schema(THD *thd);

/**
  Creates performance schema tables in the Data Dictionary.

  @return       Return true upon failure, otherwise false.
*/

bool init_pfs_tables(enum_dd_init_type int_type);

/**
  Store the property "PS_version" in table's options
*/

void set_PS_version_for_table(dd::Properties *table_options);

}  // namespace performance_schema
}  // namespace dd

#endif
