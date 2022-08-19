/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_ONE_ROW_RESULTSET_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_ONE_ROW_RESULTSET_H_

#include <gmock/gmock.h>
#include <string>

#include "plugin/x/src/mysql_variables.h"
#include "plugin/x/src/xpl_resultset.h"

namespace xpl {
namespace test {

class One_row_resultset : public xpl::Collect_resultset {
 public:
  using Resultset = Buffering_command_delegate::Resultset;
  using Row_data = Callback_command_delegate::Row_data;
  using Field_value = Callback_command_delegate::Field_value;
  using Field_types = Callback_command_delegate::Field_types;

  struct Init {
    Init(const int v)  // NOLINT(runtime/explicit)
        : field(static_cast<longlong>(v)) {}
    Init(const bool v)  // NOLINT(runtime/explicit)
        : field(static_cast<longlong>(v)) {}
    Init(const char *v)  // NOLINT(runtime/explicit)
        : field(v, strlen(v)) {}

    const Field_value field;
  };

 public:
  One_row_resultset(std::initializer_list<Init> values) {
    auto &callbacks = get_callbacks();
    Row_data row;
    Resultset resultset;
    Field_types types;

    for (const auto &v : values) {
      row.fields.push_back(ngs::allocate_object<Field_value>(v.field));

      types.push_back(
          {v.field.is_string ? MYSQL_TYPE_STRING : MYSQL_TYPE_LONGLONG, 0});
    }

    resultset.push_back(row);
    callbacks.set_resultset(resultset);
    callbacks.set_field_types(types);
  }
};

ACTION_P(SetUpResultset, init_data) {
  static_cast<xpl::Collect_resultset &>(*arg2) = init_data;
}

}  // namespace test
}  // namespace xpl

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_ONE_ROW_RESULTSET_H_
