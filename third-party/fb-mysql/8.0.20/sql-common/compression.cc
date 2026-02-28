/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "compression.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"

const char *mysql_compression_lib_names[COMPRESSION_ALGORITHM_COUNT_MAX] = {
    COMPRESSION_ALGORITHM_ZLIB, COMPRESSION_ALGORITHM_ZSTD,
    COMPRESSION_ALGORITHM_ZSTD_STREAM, COMPRESSION_ALGORITHM_LZ4F_STREAM,
    nullptr};

/**
  This function is used to validate compression algorithm specified as part
  of change master to statement.

  @param name   compression algorithm name. Name can be either zlib,zstd or
                empty string.

  @retval  an enum to represents what algorithm is specified in case it is
           a valid algorithm else return INVALID.
*/
enum_compression_algorithm get_compression_algorithm(std::string name) {
  if (name.empty() || name.c_str() == nullptr)
    return enum_compression_algorithm::MYSQL_INVALID;
  if (!my_strcasecmp(&my_charset_latin1, name.c_str(),
                     COMPRESSION_ALGORITHM_ZLIB))
    return enum_compression_algorithm::MYSQL_ZLIB;
  else if (!my_strcasecmp(&my_charset_latin1, name.c_str(),
                          COMPRESSION_ALGORITHM_ZSTD))
    return enum_compression_algorithm::MYSQL_ZSTD;
  else if (!my_strcasecmp(&my_charset_latin1, name.c_str(),
                          COMPRESSION_ALGORITHM_ZSTD_STREAM))
    return enum_compression_algorithm::MYSQL_ZSTD_STREAM;
  else if (!my_strcasecmp(&my_charset_latin1, name.c_str(),
                          COMPRESSION_ALGORITHM_LZ4F_STREAM))
    return enum_compression_algorithm::MYSQL_LZ4F_STREAM;
  else if (!my_strcasecmp(&my_charset_latin1, name.c_str(),
                          COMPRESSION_ALGORITHM_UNCOMPRESSED))
    return enum_compression_algorithm::MYSQL_UNCOMPRESSED;
  return enum_compression_algorithm::MYSQL_INVALID;
}

/**
  This function is used to parse comma separated list of compression algorithm
  names and return a list containing every algorithm name.

  @param       name    comma separated list of compression algorithm names
  @param[out]  list    list containing algorithm names

  @retval void
*/
void parse_compression_algorithms_list(std::string name,
                                       std::vector<std::string> &list) {
  std::string token;
  std::stringstream str(name);
  while (getline(str, token, ',')) list.push_back(token);
}

/**
  This function is used to validate compression level for zstd compression

  @param level  compression level to be validated against compression name

  @retval false if level is not valid.
  @retval true if level is valid.
*/
bool is_zstd_compression_level_valid(uint level) {
  return (level >= 1 && level <= 22);
}

/**
  This function is used to validate compression algorithm names and maximum
  names is not more than 3

  @param     algorithm_names   list of compression algorithm names.
  @param     channel_name      Replication channel name.
  @param     ignore_errors     If set to false, report errors to the client,
                               otherwise do not report errors"

  @retval 0  success
  @retval 1  error or warnings
*/
bool validate_compression_attributes(std::string algorithm_names,
                                     std::string channel_name,
                                     bool ignore_errors) {
  DBUG_TRACE;
  DBUG_ASSERT(algorithm_names.length() <
              COMPRESSION_ALGORITHM_NAME_BUFFER_SIZE);
  std::vector<std::string> algorithm_name_list;

  parse_compression_algorithms_list(algorithm_names, algorithm_name_list);
  unsigned int total_names = algorithm_name_list.size();

  if (total_names > COMPRESSION_ALGORITHM_COUNT_MAX) {
    if (!ignore_errors)
      my_error(ER_CHANGE_MASTER_WRONG_COMPRESSION_ALGORITHM_LIST_CLIENT, MYF(0),
               algorithm_names.c_str(), channel_name.c_str());
    return true;
  }
  /* validate compression algorithm names */
  auto name_it = algorithm_name_list.begin();
  enum_compression_algorithm method = enum_compression_algorithm::MYSQL_INVALID;
  while (name_it != algorithm_name_list.end()) {
    std::string algorithm_name = *name_it;
    /* validate algorithm name */
    method = get_compression_algorithm(algorithm_name);
    if (method == enum_compression_algorithm::MYSQL_INVALID) {
      if (!ignore_errors)
        my_error(ER_CHANGE_MASTER_WRONG_COMPRESSION_ALGORITHM_CLIENT, MYF(0),
                 algorithm_name.c_str(), channel_name.c_str());
      return true;
    }
    name_it++;
  }
  return false;
}
