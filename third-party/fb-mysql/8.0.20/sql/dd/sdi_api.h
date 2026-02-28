/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SDI_API_INCLUDED
#define DD_SDI_API_INCLUDED

#include <memory>

#include "sql/dd/string_type.h"
#include "sql/dd/types/table.h"

struct MEM_ROOT;
class MDL_request;
class THD;
struct TABLE_LIST;

namespace dd {
namespace sdi {

/**
  State and operations for importing an sdi file into the DD.
 */
class Import_target {
  /** Full path to the sdi file being imported. */
  dd::String_type m_path;

  /** True if path points inside data dir */
  bool m_in_datadir;

  /** Temporary name for sdi files in data dir when import is ongoing */
  dd::String_type m_tmp_sdi_filename;

  /** Table object which the sdi will be deserialized into */
  std::unique_ptr<dd::Table> m_table_object;

  /** Schema name found in sdi */
  dd::String_type m_schema_name_in_sdi;

  /**
    Lower-case representation of table name if
    lower_case_table_names==2, nullptr otherwise.
  */
  std::unique_ptr<dd::String_type> m_lc_tname;

  /**
    Lower-case representation of schema name if
    lower_case_table_names==2, nullptr otherwise.
  */
  std::unique_ptr<dd::String_type> m_lc_sname;

 public:
  /**
    Creates an instance to handle the import of a single sdi file.
    @param path full path to an sdi file to import
    @param in_datadir true if the file is located somewhere under the
           server's data directrory.
   */
  Import_target(String_type &&path, bool in_datadir);

  /**
    Having a unique_ptr as member makes this a move-only type.
   */
  Import_target(Import_target &&) = default;
  Import_target(const Import_target &) = delete;

  /**
    Finish import by removing tmp sdi file when importing from sdi
    file in datadir.

    @retval true if an error occurred
    @retval false otherwise
   */
  bool commit() const;

  /**
    Restore old state by renaming tmp sdi file back to its original
    name when importing from sdi file in datadir.

    @retval true if an error occurred
    @retval false otherwise
   */
  bool rollback() const;

  /**
    Obtains the canonical table name for use with MDL and
    privilege-checking. For lower_case_table_names=0 and 1, this is
    the same as the name of the Table object. For
    lower_case_table_names=2 it is the lower case version of the
    tablename.

    @returns pointer to dd::String_type holding canonical name
   */
  const dd::String_type *can_table_name() const;

  /**
    Obtains the canonical schema name for use with MDL and
    privilege-checking. For lower_case_table_names=0 and 1, this is
    the same as the schema name of the table in the sdi. For
    lower_case_table_names=2 it is the lower case version of the
    schemaname.

    @returns pointer to dd::String_type holding canonical name
   */
  const dd::String_type *can_schema_name() const;

  /**
    Reads the sdi file from disk and dserializes it into a Table
    object and its schema name, but does not store Table object in the
    DD.

    @param thd thread context
    @param shared_buffer pointer to a dd::String_type which is used to
           store the sdi string until it is deserialized.
    @retval true if an error occurred
    @retval false otherwise
   */
  bool load(THD *thd, String_type *shared_buffer);

  /**
    Constructs a TABLE_LIST object with info from this Import_target.
    TABLE_LIST::db and TABLE_LIST::table_name are initialized to the
    canonical (lowercased for lctn==2) representation,
    TABLE_LIST::alias to the native
    table_name, and TABLE_LIST::m_lock_descriptor.type is set to
    TL_IGNORE.
   */
  TABLE_LIST make_table_list() const;

  /**
    Upadate the schema reference in the Table object and store
    it in the DD so that it becomes visible. Precondition: The
    Import_target must be loaded and privileges checked before this
    member function is called.

    @param thd thread handle
    @retval true if an error occurred
    @retval false otherwise
   */
  bool store_in_dd(THD *thd) const;
};

/**
  Check that we have the the necessary privileges to import this
  table.  Precondition: The Import_target must be loaded before this
  member function is called.

  @param thd thread handle
  @param t import target context
  @retval true if an error occurred
  @retval false otherwise
*/
bool check_privileges(THD *thd, const Import_target &t);

/**
  Creates an MDL_request for exclusive MDL on the table being
  imported. Does not actually lock the name, that must be done later
  for all requests to avoid deadlock.

  @param t import target context
  @param mem_root where to allocate the MDL_request
  @return pointer to mem_root allocated MDL_request
*/
MDL_request *mdl_request(const Import_target &t, MEM_ROOT *mem_root);

}  // namespace sdi
}  // namespace dd

#endif /* DD_SDI_API_INCLUDED */
