/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/base/password_option.h"

#include <stddef.h>
#include <functional>

#include "client/client_priv.h"

using namespace Mysql::Tools::Base::Options;
using Mysql::Nullable;
using std::string;
using std::placeholders::_1;

Password_option::Password_option(Nullable<string> *value, string name,
                                 string description)
    : Abstract_string_option<Password_option>(value, GET_PASSWORD, name,
                                              description) {
  this->value_optional()->add_callback(new std::function<void(char *)>(
      std::bind(&Password_option::password_callback, this, _1)));
}

void Password_option::password_callback(char *argument) {
  if (argument == ::disabled_my_option) {
    // This prevents ::disabled_my_option being overriden later in this
    // function.
    argument = const_cast<char *>("");
  }

  if (argument != nullptr) {
    /*
     Destroy argument value, this modifies part of argv passed to main
     routine. This makes command line on linux changed, so no user can see
     password shortly after program starts. This works for example for
     /proc/<pid>/cmdline file and ps tool.
     */
    for (char *pos = argument; *pos != 0; pos++) {
      *pos = '*';
    }

    /*
     This cuts argument length to hide password length on linux commandline
     showing tools.
     */
    if (*argument) argument[1] = 0;
  } else {
    char *password = ::get_tty_password(nullptr);
    *this->m_destination_value = Nullable<string>(password);
    my_free(password);
  }
}
