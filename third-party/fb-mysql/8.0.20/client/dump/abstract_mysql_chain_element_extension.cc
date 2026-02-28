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

#include "client/dump/abstract_mysql_chain_element_extension.h"

#include <stddef.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <sstream>

#include "m_ctype.h"

using namespace Mysql::Tools::Dump;

Abstract_mysql_chain_element_extension::Abstract_mysql_chain_element_extension(
    I_connection_provider *connection_provider,
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler,
    const Mysql_chain_element_options *options)
    : m_connection_provider(connection_provider),
      m_message_handler(message_handler),
      m_options(options),
      m_charset(options->get_program()->get_current_charset() != nullptr
                    ? options->get_program()->get_current_charset()
                    : get_charset_by_csname(MYSQL_UNIVERSAL_CLIENT_CHARSET,
                                            MY_CS_PRIMARY, MYF(MY_WME))) {}

Mysql::Tools::Base::Mysql_query_runner *
Abstract_mysql_chain_element_extension::get_runner() const {
  return m_connection_provider->get_runner(m_message_handler);
}

I_connection_provider *
Abstract_mysql_chain_element_extension::get_connection_provider() const {
  return m_connection_provider;
}

uint64 Abstract_mysql_chain_element_extension::get_server_version() {
  Mysql::Tools::Base::Mysql_query_runner *runner = this->get_runner();
  if (!runner) return 0;

  ulong version = mysql_get_server_version(runner->get_low_level_connection());
  delete runner;
  return version;
}

std::string
Abstract_mysql_chain_element_extension::get_server_version_string() {
  uint64 version = this->get_server_version();
  std::ostringstream result;
  result << ((version / (100 * 100)) % 100) << "." << ((version / (100)) % 100)
         << "." << ((version / (1)) % 100);
  return result.str();
}

int Abstract_mysql_chain_element_extension::
    compare_no_case_latin_with_db_string(const std::string &latin_name,
                                         const std::string &db_name) {
  return my_strcasecmp(&my_charset_latin1, latin_name.c_str(), db_name.c_str());
}

Mysql::Nullable<std::string>
Abstract_mysql_chain_element_extension::get_create_statement(
    Mysql::Tools::Base::Mysql_query_runner *runner,
    const std::string &database_name, const std::string &object_name,
    const std::string &object_type, uint) {
  std::vector<const Mysql::Tools::Base::Mysql_query_runner::Row *> result;

  runner->run_query_store(
      "SHOW CREATE " + object_type + " " +
          this->get_quoted_object_full_name(database_name, object_name),
      &result);

  Mysql::Nullable<std::string> res;
  if (result.size() > 0) {
    if (object_type == "FUNCTION" || object_type == "PROCEDURE" ||
        object_type == "TRIGGER")
      res = (*result[0])[2];
    else if (object_type == "EVENT")
      res = (*result[0])[3];
    else
      res = (*result[0])[1];
    /*
     TODO: SHOW CREATE will not include schema name except for views
           and triggers. Once fix is done from server below hack will
           be removed.
    */
    if (database_name.size() > 0) {
      std::string obj_name_without_quote =
          this->quote_name(database_name) + "." + object_name;
      std::string obj_name_with_quote =
          this->quote_name(database_name) + "." + this->quote_name(object_name);
      size_t pos1 = res.value().find(obj_name_without_quote);
      size_t pos2 = res.value().find(obj_name_with_quote);

      // if object name does not have db name then include it
      if (pos1 == std::string::npos && pos2 == std::string::npos) {
        size_t pos = res.value().find(object_type);
        if (pos != std::string::npos) {
          pos = pos + object_type.size() + 1;
          res = res.value().substr(0, pos) + this->quote_name(database_name) +
                "." + res.value().substr(pos);
        }
      }
    }
  }
  Mysql::Tools::Base::Mysql_query_runner::cleanup_result(&result);
  return res;
}

std::string Abstract_mysql_chain_element_extension::quote_name(
    const std::string &name) {
  char buff[MAX_NAME_LEN * 2 + 3] = {0};
  const char *name_str = name.c_str();
  Mysql::Tools::Base::Mysql_query_runner *runner = this->get_runner();
  if (!runner) return name;

  buff[0] = '`';
  int len = mysql_real_escape_string_quote(runner->get_low_level_connection(),
                                           buff + 1, name_str,
                                           (ulong)name.size(), '`');
  buff[len + 1] = '`';
  delete runner;
  return std::string(buff);
}

std::string Abstract_mysql_chain_element_extension::get_quoted_object_full_name(
    const Abstract_data_object *object) {
  return this->get_quoted_object_full_name(object->get_schema(),
                                           object->get_name());
}

std::string Abstract_mysql_chain_element_extension::get_quoted_object_full_name(
    const std::string &database_name, const std::string &object_name) {
  if (database_name != "")
    return this->quote_name(database_name) + "." +
           this->quote_name(object_name);
  return this->quote_name(object_name);
}

const Mysql_chain_element_options *
Abstract_mysql_chain_element_extension::get_mysql_chain_element_options()
    const {
  return m_options;
}

CHARSET_INFO *Abstract_mysql_chain_element_extension::get_charset() const {
  return m_charset;
}
