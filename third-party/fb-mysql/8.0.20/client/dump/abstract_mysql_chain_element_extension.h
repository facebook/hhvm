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

#ifndef ABSTRACT_MYSQL_CHAIN_ELEMENT_EXTENSION_INCLUDED
#define ABSTRACT_MYSQL_CHAIN_ELEMENT_EXTENSION_INCLUDED

#include <sys/types.h>
#include <functional>

#include "client/base/message_data.h"
#include "client/dump/abstract_data_object.h"
#include "client/dump/i_chain_element.h"
#include "client/dump/i_connection_provider.h"
#include "client/dump/mysql_chain_element_options.h"
#include "my_inttypes.h"
#include "nullable.h"

#define MYSQL_UNIVERSAL_CLIENT_CHARSET "utf8mb4"
#define MAX_NAME_LEN (64 * 3)

namespace Mysql {
namespace Tools {
namespace Dump {

class Abstract_mysql_chain_element_extension : public virtual I_chain_element {
 protected:
  Abstract_mysql_chain_element_extension(
      I_connection_provider *connection_provider,
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      const Mysql_chain_element_options *options);

  Mysql::Tools::Base::Mysql_query_runner *get_runner() const;

  I_connection_provider *get_connection_provider() const;

  uint64 get_server_version();

  std::string get_server_version_string();

  int compare_no_case_latin_with_db_string(const std::string &latin_name,
                                           const std::string &db_name);

  /**
    Gets CREATE statement for specified object. If object type is database,
    then object_name should be empty.
   */
  Mysql::Nullable<std::string> get_create_statement(
      Mysql::Tools::Base::Mysql_query_runner *runner,
      const std::string &database_name, const std::string &object_name,
      const std::string &object_type, uint field_id = 1);

  /**
    Quotes char string, taking into account compatible mode.
   */
  std::string quote_name(const std::string &name);

  std::string get_quoted_object_full_name(const Abstract_data_object *object);

  std::string get_quoted_object_full_name(const std::string &database_name,
                                          const std::string &object_name);

  const Mysql_chain_element_options *get_mysql_chain_element_options() const;

  CHARSET_INFO *get_charset() const;

 private:
  I_connection_provider *m_connection_provider;
  std::function<bool(const Mysql::Tools::Base::Message_data &)>
      *m_message_handler;
  const Mysql_chain_element_options *m_options;
  CHARSET_INFO *m_charset;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
