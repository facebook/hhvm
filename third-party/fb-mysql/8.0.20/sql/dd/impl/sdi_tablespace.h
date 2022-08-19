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

#ifndef DD__SDI_TABLESPACE_INCLUDED
#define DD__SDI_TABLESPACE_INCLUDED

#include "sql/dd/impl/sdi.h"  // dd::Sdi_type
#include "sql/dd/object_id.h"

class THD;
struct handlerton;

namespace dd {
class Tablespace;
namespace cache {
class Dictionary_client;
}
namespace sdi_tablespace {

/**
  @defgroup sdi_tablespace Storage operations for SDIs in tablespaces.
  @ingroup sdi

  Called from functions in sdi.cc if the dd object resides in an SE
  supporting SDI storage in tablespaces.

  @note
  There is no function for dropping the Tablespace
  SDI. Dropping a tablespace implies that all SDIs in it are dropped
  also.

  @{
*/

/**
  Looks up the relevant tablespaces for the table and stores the
  table SDI in each.

  @param thd
  @param hton
  @param sdi
  @param table
  @param schema
 */
bool store_tbl_sdi(THD *thd, handlerton *hton, const Sdi_type &sdi,
                   const Table &table, const dd::Schema &schema);

/**
  Stores the tablespace SDI in the tablespace.

  @param hton
  @param sdi
  @param tablespace
 */
bool store_tsp_sdi(handlerton *hton, const Sdi_type &sdi,
                   const Tablespace &tablespace);

/**
  Looks up the relevant tablespaces for the table and drops the
  table SDI in each.

  @note When the last table in a schema is dropped from a tablespace
  the schema SDI should also be dropped. But leaving them is not a big
  problem as the schema SDIs are small (they only contain the default
  charset for the schema).

  @param thd
  @param hton
  @param table
  @param schema
 */
bool drop_tbl_sdi(THD *thd, const handlerton &hton, const Table &table,
                  const Schema &schema MY_ATTRIBUTE((unused)));

/** @} End of group sdi_tablespace */
}  // namespace sdi_tablespace
}  // namespace dd
#endif  // !DD__SDI_TABLESPACE_INCLUDED
