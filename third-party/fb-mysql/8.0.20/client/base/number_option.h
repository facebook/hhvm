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

#ifndef NUMBER_OPTION_INCLUDED
#define NUMBER_OPTION_INCLUDED

#include <string>

#include "client/base/abstract_integer_number_option.h"

ulonglong getopt_double2ulonglong(double v);

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Template class for all number options.
 */
template <typename T_value>
class Number_option
    : public Abstract_integer_number_option<Number_option<T_value>, T_value> {
 private:
  /**
    This class cannot be instanced. It is only as template for specialized
    implementations.
   */
  Number_option();
};

template <typename T_value>
Number_option<T_value>::Number_option() {}

/**
  32-bit signed number option.
 */
template <>
class Number_option<int32>
    : public Abstract_integer_number_option<Number_option<int32>, int32> {
 public:
  /**
    Constructs new 32-bit signed number option.
    @param value Pointer to int32 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option(int32 *value, std::string name, std::string description)
      : Abstract_integer_number_option<Number_option<int32>, int32>(
            value, GET_INT32, name, description) {}
};

/**
  32-bit unsigned number option.
 */
template <>
class Number_option<uint32>
    : public Abstract_integer_number_option<Number_option<uint32>, uint32> {
 public:
  /**
    Constructs new 32-bit unsigned number option.
    @param value Pointer to uint32 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option(uint32 *value, std::string name, std::string description)
      : Abstract_integer_number_option<Number_option<uint32>, uint32>(
            value, GET_UINT32, name, description) {}
};

/**
  64-bit signed number option.
 */
template <>
class Number_option<int64>
    : public Abstract_integer_number_option<Number_option<int64>, int64> {
 public:
  /**
    Constructs new 64-bit signed number option.
    @param value Pointer to int64 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option(int64 *value, std::string name, std::string description)
      : Abstract_integer_number_option<Number_option<int64>, int64>(
            value, GET_LL, name, description) {}
};

/**
  64-bit unsigned number option.
 */
template <>
class Number_option<uint64>
    : public Abstract_integer_number_option<Number_option<uint64>, uint64> {
 public:
  /**
    Constructs new 64-bit unsigned number option.
    @param value Pointer to uint64 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option(uint64 *value, std::string name, std::string description)
      : Abstract_integer_number_option<Number_option<uint64>, uint64>(
            value, GET_ULL, name, description) {}
};

/**
  Double precision floating-point number option.
 */
template <>
class Number_option<double>
    : public Abstract_number_option<Number_option<double>, double> {
 public:
  /**
    Constructs new floating-point number option.
    @param value Pointer to double object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option(double *value, std::string name, std::string description)
      : Abstract_number_option<Number_option<double>, double>(
            value, GET_DOUBLE, name, description,
            getopt_double2ulonglong((double)*value)) {}

  /**
    Sets minimum value boundary for option value. Smaller values passed as
    option value will be changed to this minimum value.
   */
  virtual Number_option<double> *set_minimum_value(double minimum) {
    this->m_option_structure.min_value = getopt_double2ulonglong(minimum);
    return this;
  }

  /**
    Sets maximum value boundary for option value. Greater values passed as
    option value will be changed to this maximum value.
   */
  virtual Number_option<double> *set_maximum_value(double maximum) {
    this->m_option_structure.max_value = getopt_double2ulonglong(maximum);
    return this;
  }
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
