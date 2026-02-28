/* Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "include/template_utils.h"

namespace template_utils_unittest {

class Base {
 public:
  int id() const { return 1; }

  // Needed to make compiler understand that it's a polymorphic class.
  virtual ~Base() {}

  // To silence -Wdeprecated-copy.
  Base() {}
  Base(const Base &) = default;
};

class Descendent : public Base {
 public:
  int id() const { return 2; }
};

TEST(TemplateUtilsTest, DownCastReference) {
  Descendent descendent;
  Base &baseref = descendent;
  auto descendentref = down_cast<Descendent &>(baseref);

  EXPECT_EQ(1, baseref.id());
  EXPECT_EQ(2, descendentref.id());
}

TEST(TemplateUtilsTest, DownCastRvalueReference) {
  Descendent descendent;
  Base &&baseref = Descendent();
  auto descendentref = down_cast<Descendent &&>(baseref);

  EXPECT_EQ(1, baseref.id());
  EXPECT_EQ(2, descendentref.id());
}

TEST(TemplateUtilsTest, DownCastPointer) {
  Descendent descendent;
  Base *baseref = &descendent;
  auto descendentref = down_cast<Descendent *>(baseref);

  EXPECT_EQ(1, baseref->id());
  EXPECT_EQ(2, descendentref->id());
}

}  // namespace template_utils_unittest
