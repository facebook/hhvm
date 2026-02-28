/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ABSTRACT_STRING_OPTION_INCLUDED
#define ABSTRACT_STRING_OPTION_INCLUDED

#include <functional>
#include <string>

#include "client/base/abstract_value_option.h"
#include "my_getopt.h"
#include "nullable.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Abstract option to handle options accepting string value argument.
 */
template <typename T_type>
class Abstract_string_option : public Abstract_value_option<T_type> {
 public:
  /**
    Sets value for this option. If it is specified before handling commandline
    options then supplied value is used as default value of this option.
   */
  T_type *set_value(std::string value);

 protected:
  /**
    Constructs new string option.
    @param value Pointer to string object to receive option value.
    @param var_type my_getopt internal option type.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Abstract_string_option(Nullable<std::string> *value, ulong var_type,
                         std::string name, std::string description);

  Nullable<std::string> *m_destination_value;

 private:
  void string_callback(char *argument);

  const char *m_original_value;
};

template <typename T_type>
Abstract_string_option<T_type>::Abstract_string_option(
    Nullable<std::string> *value, ulong var_type, std::string name,
    std::string description)
    : Abstract_value_option<T_type>(&this->m_original_value, var_type, name,
                                    description, (uint64)NULL),
      m_destination_value(value) {
  *value = Nullable<std::string>();
  this->m_original_value = nullptr;

  this->add_callback(new std::function<void(char *)>(
      std::bind(&Abstract_string_option<T_type>::string_callback, this,
                std::placeholders::_1)));
}

template <typename T_type>
T_type *Abstract_string_option<T_type>::set_value(std::string value) {
  *this->m_destination_value = Nullable<std::string>(value);
  this->m_original_value = this->m_destination_value->value().c_str();
  this->m_option_structure.def_value =
      (uint64)this->m_destination_value->value().c_str();
  return (T_type *)this;
}

template <typename T_type>
void Abstract_string_option<T_type>::string_callback(char *argument) {
  if (argument != nullptr) {
    // Copy argument value from char* to destination string.
    *this->m_destination_value = Nullable<std::string>(this->m_original_value);
  } else {
    // There is no argument supplied, we shouldn't change default value.
  }
}

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
