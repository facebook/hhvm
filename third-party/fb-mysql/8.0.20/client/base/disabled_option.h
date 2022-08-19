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

#ifndef DISABLED_OPTION_INCLUDED
#define DISABLED_OPTION_INCLUDED

#include "client/base/abstract_option.h"
#include "my_getopt.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Disabled option. This option is used to mark existance of options
  inavailable due to distribution configuration.
 */
class Disabled_option : public Abstract_option<Disabled_option> {
 public:
  /**
    Constructs new disabled option.
    @param name Name of option. It is used in command-line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Disabled_option(std::string name, std::string description);
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
