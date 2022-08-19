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

#ifndef DD__SDI_FILE_INCLUDED
#define DD__SDI_FILE_INCLUDED

#include <stddef.h>
#include <utility>

#include "mysql/mysql_lex_string.h"
#include "prealloced_array.h"    // Prealloced_array
#include "sql/dd/impl/sdi.h"     // dd::Sdi_type
#include "sql/dd/object_id.h"    // dd::Object_id
#include "sql/dd/string_type.h"  // dd::String_type

class THD;
struct handlerton;
struct MYSQL_LEX_CSTRING;

namespace dd {
class Entity_object;
class Schema;
class Table;

namespace sdi_file {
/** Number of character (not bytes) of a tablename which will
    contrubute to the sdi file name. The whole name is not needed as
    the Object_id is added so that uniqueness is ensured */
const size_t FILENAME_PREFIX_CHARS = 16;

/** File name extension for sdi files. */
const String_type EXT = ".sdi";

/**
  Formats an sdi filename according to the mysql conventions for an entity
  name and schema name, where the schema may be "".

  @param id object id if dd object
  @param entity_name name (as returned by dd::Entity_object::name())
         of dd obejct.
  @param schema name of schema, or "" for schemaless entites (schemata).
  @retval filename to use for sdi file
 */
String_type sdi_filename(Object_id id, const String_type &entity_name,
                         const String_type &schema);

/**
  Stores sdi for table in a file.

  @param sdi json string to store
  @param table dd object from which sdi was generated
  @param schema object which table belongs to
  @retval true if an error occurs
  @retval false otherwise
*/
bool store_tbl_sdi(const Sdi_type &sdi, const dd::Table &table,
                   const dd::Schema &schema);

/**
  Remove a file name from the file system.

  @param fname file name to remove from file system.
  @retval true if an error occurs
  @retval false otherwise
*/
bool remove(const String_type &fname);

/**
  Removes sdi file for a table.

  @param table dd object for which to remove sdi
  @param schema object which table belongs to
  @retval true if an error occurs
  @retval false otherwise
*/
bool drop_tbl_sdi(const dd::Table &table, const dd::Schema &schema);

/**
  Read an sdi file from disk and store in a buffer.

  @param thd thread handle
  @param fname path to sdi file to load
  @param buf where to store file content
  @retval true if an error occurs
  @retval false otherwise
*/
bool load(THD *thd, const dd::String_type &fname, dd::String_type *buf);

/**
  Instantiation of std::pair to represent the full path to an sdi
  file. Member first is the path, second is true if the path is inside
  datadir, false otherwise.
*/
typedef std::pair<dd::String_type, bool> Path_type;

/**
  Typedef for container type to use as out-parameter when expanding
  sdi file patterns into paths.
*/
typedef Prealloced_array<Path_type, 3> Paths_type;

/**
  Expand an sdi filename pattern into the set of full paths that
  match. The paths and a bool indicating if the path is inside data
  dir is appended to the Paths_type collection provided as argument.

  @param thd thread handle
  @param pattern filenam pattern to expand
  @param paths collection of expanded file paths
  @retval true if an error occurs
  @retval false otherwise
*/
bool expand_pattern(THD *thd, const MYSQL_LEX_STRING &pattern,
                    Paths_type *paths);

/**
  Check that the MYD and MYI files for table exists.

  @param schema_name
  @param table_name
  @retval true if an error occurs
  @retval false otherwise
 */
bool check_data_files_exist(const dd::String_type &schema_name,
                            const dd::String_type &table_name);
}  // namespace sdi_file
}  // namespace dd
#endif  // !DD__SDI_FILE_INCLUDED
