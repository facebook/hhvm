/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__SDI_INCLUDED
#define DD__SDI_INCLUDED

#include "my_compiler.h"
#include "sql/dd/string_type.h"  // dd::String_type

class THD;
struct handlerton;

/**
  @file
  @ingroup sdi

  Exposes SDI-related functionality to the rest of the dictionary code.
*/

namespace dd {

class Schema;
class Table;
class Tablespace;
class View;

typedef String_type Sdi_type;

/**
  The version of the current SDI Json format.

  This version number is stored inside SDIs. This number is bumpred only when
  there is a change in how the data dictionary information is converted to json
  (e.g. adding a forgotten member). It does not need to be bumped when the data
  dictionary schema structure is changed, as this is covered by the DD_VERSION
  variable.

  The SDI version number is the MySQL server version number
  of the first MySQL server version that published a given SDI Json format.
  The format is Mmmdd with M=Major, m=minor, d=dot, so that MySQL 8.0.4 is
  encoded as 80004. This is the same version numbering scheme as the
  information schema and performance schema are using.

  The next change to the SDI Json format will be associated with the next
  available MySQL server version number.

  Historical version number published in the data dictionary, (note that 1 is a
  legacy number from before SDI version numbers were mapped to server versions):


  1: Published in 8.0.15
  ----------------------------------------------------------------------------
  Initial version.


  80016: Published in 8.0.16
  ----------------------------------------------------------------------------
  Changes from version 1:

  - Bug#29210646: DD::INDEX_IMPL::M_HIDDEN NOT INCLUDED IN SDI


  80019: Current
  ----------------------------------------------------------------------------
  Changes from version 80016:

  - Bug#30326020: SUBPARTITIONING NOT REFLECTED IN SDI


  800XX: Next SDI version number after the previous is public. The next
         server version > current SDI version where a change to the SDI
         JSON format is made.
  ----------------------------------------------------------------------------
  Changes from current version:

  - No changes, this version number is not active yet.

  If a new SDI version is published in a MRU, it will not
  be possible to import this version into previous MRUs within the same GA.
*/
constexpr const std::uint64_t SDI_VERSION = 80019;

/**
  @defgroup serialize_api (De)serialize api functions.
  @ingroup sdi

  Functions for serializing (with complete header) and deserializing
  the dd object which supports this.
  @{
 */

/**
  Serialize a Schema object.

  @param schema dobject which will be serialized

  @return sdi (as json string).

*/
Sdi_type serialize(const Schema &schema);

/**
  Serialize a Table object.

  @param thd
  @param table object which will be serialized
  @param schema_name
  @return sdi (as json string).

*/
Sdi_type serialize(THD *thd, const Table &table,
                   const String_type &schema_name);

/**
  Serialize a Tablespace object.

  @param tablespace object which will be serialized

  @return sdi (as json string).

*/

Sdi_type serialize(const Tablespace &tablespace);

/**
  Deserialize a dd::Schema object.

  Populates the dd::Schema object provided with data from sdi string.
  Note! Additional objects are dynamically allocated and added to the
  top-level Schema object, which assumes ownership.

  @param thd thread context
  @param sdi  serialized representation of schema (as a json string)
  @param schema empty top-level object

  @return error status
    @retval false if successful
    @retval true otherwise

*/

bool deserialize(THD *thd, const Sdi_type &sdi, Schema *schema);

/**
  Deserialize a dd::Table object.

  Populates the dd::Table object provided with data from sdi string.
  Note! Additional objects are dynamically allocated and added to the
  top-level Schema object, which assumes ownership.

  @param thd thread context
  @param sdi  serialized representation of schema (as a json string)
  @param table empty top-level object
  @param deser_schema_name name of schema containing the table

  @return error status
    @retval false if successful
    @retval true otherwise

*/

bool deserialize(THD *thd, const Sdi_type &sdi, Table *table,
                 String_type *deser_schema_name = nullptr);

/**
  Deserialize a dd::Tablespace object.

  Populates the dd::Tablespace object provided with data from sdi string.
  Note! Additional objects are dynamically allocated and added to the
  top-level Tablespace object, which assumes ownership.

  @param thd thread context
  @param sdi  serialized representation of schema (as a json string)
  @param tablespace empty top-level object

  @return error status
    @retval false if successful
    @retval true otherwise

*/

bool deserialize(THD *thd, const Sdi_type &sdi, Tablespace *tablespace);

/** @} End of group serialize_api */

namespace sdi {

/**
  @defgroup sdi_storage_ops Storage operations on SDIs.
  @ingroup sdi

  Functions for storing and dropping (deleting) SDIs. To be used from
  dd::cache::Storage_adapter and dd::cache::Dictionary_client to store
  and remove SDIs as DD objects are added, modified or deleted.
  @{
*/

/**
  Generic noop for all types that don't have a specific overload. No
  SDIs are written for these types.

  @param thd
  @param ddo
  @return error status
    @retval false always
 */

template <class DDT>
inline bool store(THD *thd MY_ATTRIBUTE((unused)),
                  const DDT *ddo MY_ATTRIBUTE((unused))) {
  return false;
}

/**
  Stores the SDI for a table.

  Serializes the table, and then forwards to SE through handlerton
  api, or falls back to storing the sdi string in an .SDI file in the
  default case. The schema object is serialized and stored
  if the schema's SDI file does not exist, or if is missing from the
  tablespace used to store the table.

  @param thd
  @param t Table object.

  @return error status
    @retval false on success
    @retval true otherwise
*/

bool store(THD *thd, const Table *t);

/**
  Stores the SDI for a table space.

  Serializes the table space object, and then forwards to SE through
  handlerton api, or falls back to storing the sdi string in an .SDI
  file in the default case.

  @param thd
  @param ts     Tablespace object.

  @return error status
    @retval false on success
    @retval true otherwise
*/

bool store(THD *thd, const Tablespace *ts);

/**
  Generic noop for all types that don't have a specific overload. No
  SDIs are removed for these types.

  @param thd
  @return error status
    @retval false always
 */

template <class DDT>
inline bool drop(THD *thd MY_ATTRIBUTE((unused)), const DDT *) {
  return false;
}

/**
  Remove SDI for a table.

  Forwards to SE through handlerton api, which will remove from
  tablespace, or falls back to deleting the .SDI file in the default
  case.

  @param thd
  @param t Table object.

  @return error status
    @retval false on success
    @retval true otherwise
*/

bool drop(THD *thd, const Table *t);

// Note that there is NO drop() overload for Tablespace. Dropping a
// tablespace implies that all sdis in it are dropped also.

/**
  Hook for SDI cleanup after updating DD object. Generic noop for all
  types that don't have a specific overload.

  @param thd
  @param old_ddo
  @param new_ddo
  @return error status
    @retval false always
 */

template <class DDT>
inline bool drop_after_update(THD *thd MY_ATTRIBUTE((unused)),
                              const DDT *old_ddo MY_ATTRIBUTE((unused)),
                              const DDT *new_ddo MY_ATTRIBUTE((unused))) {
  return false;
}

/**
  Table cleanup hook. When a Dictionary_client issues a store which is
  performed as an update in the DD a new table SDI file will be
  stored. If SDI is stored in a file and the update modifies the name
  of the table it is necessary to remove the old SDI file after the
  new one has been written successfully. If the file names are the
  same the file is updated in place, potentially leaving it corrupted
  if something goes wrong. If the SDI is stored in a tablespace it
  will use the same key even if the names change and the update will
  transactional so then this hook does nothing.

  @param thd
  @param old_t old Schema object
  @param new_t new Schema object

  @return error status
    @retval false on success
    @retval true otherwise
*/

bool drop_after_update(THD *thd, const Table *old_t, const Table *new_t);

/** @} End of group sdi_storage_ops */
}  // namespace sdi
}  // namespace dd

#endif /* DD__SDI_INCLUDED */
