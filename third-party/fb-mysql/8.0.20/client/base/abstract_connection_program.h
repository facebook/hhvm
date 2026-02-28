/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ABSTRACT_CONNECTION_PROGRAM_INCLUDED
#define ABSTRACT_CONNECTION_PROGRAM_INCLUDED

#include <string>

#include "client/base/abstract_program.h"
#include "client/base/composite_options_provider.h"
#include "client/base/i_connection_factory.h"
#include "client/base/i_options_provider.h"
#include "client/base/mysql_connection_options.h"
#include "client/client_priv.h"

namespace Mysql {
namespace Tools {
namespace Base {

/**
  Base class for all programs that use connection to MySQL database server.
 */
class Abstract_connection_program : public Abstract_program,
                                    public I_connection_factory {
 public:
  /**
    Provides new connection to MySQL database server based on option values.
    Implementation of I_connection_factory interface.
   */
  virtual MYSQL *create_connection();

  /**
    Retrieves charset that will be used in new MySQL connections. Can be NULL
    if none was set explicitly.
   */
  CHARSET_INFO *get_current_charset() const;

  /**
    Sets charset that will be used in new MySQL connections.
   */
  void set_current_charset(CHARSET_INFO *charset);

 protected:
  Abstract_connection_program();

 private:
  Options::Mysql_connection_options m_connection_options;
};

}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
