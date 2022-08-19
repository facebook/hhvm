/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "dynamic_loader_path_filter_imp.h"

#include <mysql/components/my_service.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/dynamic_loader_scheme_file.h>
#include <mysql_com.h>  // NAME_CHAR_LEN
#include <string>

#include "m_ctype.h"  // system_charset_info
#include "my_io.h"
#include "my_sharedlib.h"
#include "sql/mysqld.h"      // scheme_file_srv
#include "sql/sql_plugin.h"  // opt_plugin_dir

typedef std::string my_string;

bool check_string_char_length(const LEX_CSTRING &str, const char *err_msg,
                              size_t max_char_length, const CHARSET_INFO *cs,
                              bool no_error);

/**
  Checks if path specified to load is contained in plug-in directory and
  change it to absolute one using plug-in directory. Calls wrapped file scheme
  service implementation on calculated absolute URN.

  @param urn URN to file to load components from.
  @param [out] out_data Pointer to pointer to MySQL component data structures
    to set result components data retrieved from specified file.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_dynamic_loader_scheme_file_path_filter_imp::load,
                   (const char *urn, mysql_component_t **out_data)) {
  try {
    my_string path;

    if (check_and_make_absolute_urn(urn, path)) {
      return true;
    }

    return (scheme_file_srv->load(path.c_str(), out_data));
  } catch (...) {
  }
  return true;
}

/**
  Checks if path specified to load is contained in plug-in directory and
  change it to absolute one using plug-in directory. Calls wrapped file scheme
  service implementation on calculated absolute URN.

  @param urn URN to file to unload all components from.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_dynamic_loader_scheme_file_path_filter_imp::unload,
                   (const char *urn)) {
  try {
    my_string path;
    if (check_and_make_absolute_urn(urn, path)) {
      return true;
    }

    return (scheme_file_srv->unload(path.c_str()));
  } catch (...) {
  }
  return true;
}

/**
  Ensure that the dynamic library doesn't have a path.
  This is done to ensure that only approved libraries from the
  plug-in directory are used (to make this even remotely secure).
  Extracts real absolute path to file in plug-in directory.

  @param input_urn URN with path to validate and make absolute.
  @param [out] out_path String to put result URN to.
*/
bool mysql_dynamic_loader_scheme_file_path_filter_imp::
    check_and_make_absolute_urn(const char *input_urn, my_string &out_path) {
  /* Omit scheme prefix to get filename. */
  const char *file = strstr(input_urn, "://");
  if (file == nullptr) {
    return true;
  }
  /* Offset by "://" */
  file += 3;

  size_t plugin_dir_len = strlen(opt_plugin_dir);
  size_t input_path_len = strlen(file);
  LEX_CSTRING dl_cstr = {file, input_path_len};
  if (check_valid_path(file, input_path_len) ||
      check_string_char_length(dl_cstr, "", NAME_CHAR_LEN, system_charset_info,
                               true) ||
      plugin_dir_len + input_path_len + 1 >= FN_REFLEN) {
    return true;
  }
  /* Compile library path */
  my_string path = opt_plugin_dir;
  path += '/';
  path += file;

  char path_buff[FN_REFLEN + 1];
  (void)unpack_filename(path_buff, path.c_str());

  out_path = "file://";
  out_path += path_buff;

  return false;
}
