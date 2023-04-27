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

#ifndef PASSWORD_OPTION_INCLUDED
#define PASSWORD_OPTION_INCLUDED

#include <string>

#include "client/base/abstract_string_option.h"
#include "my_getopt.h"
#include "nullable.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  String value option to handle passwords. Removes password from command line
  on UNIX systems to prevent password to be seen when listing processes.
 */
class Password_option : public Abstract_string_option<Password_option> {
 public:
  /**
    Constructs new password option.
    @param value Pointer to string object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Password_option(Nullable<std::string> *value, std::string name,
                  std::string description);

 private:
  void password_callback(char *);
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
