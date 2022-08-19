/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <stddef.h>

#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"

namespace mysys_my_load_path {

TEST(Mysys, MyLoadPath) {
  char dest[FN_REFLEN];

  static const std::string filename = "filename";

  // Path with absolute path component.
  std::string absolute_path_name = FN_LIBCHAR + filename;
  my_load_path(dest, absolute_path_name.c_str(), nullptr);
  EXPECT_STREQ(dest, absolute_path_name.c_str());

  // Path with home directory component.
  dest[0] = '\0';
  std::string home_dir_path_name = FN_HOMELIB + (FN_LIBCHAR + filename);
  my_load_path(dest, home_dir_path_name.c_str(), nullptr);
  EXPECT_STREQ(dest, home_dir_path_name.c_str());

  // Path with current directory component.
  dest[0] = '\0';
  std::string parent_dir_path_name = FN_CURLIB + (FN_LIBCHAR + filename);
  my_load_path(dest, parent_dir_path_name.c_str(), nullptr);
  char temp_buf[256];
  my_getwd(temp_buf, sizeof(temp_buf), MYF(0));
  EXPECT_STREQ(dest, (temp_buf + filename).c_str());

  // Path with prefix component appended.
  dest[0] = '\0';
  std::string prefix_path_name = "/basedir/";
  my_load_path(dest, filename.c_str(), prefix_path_name.c_str());
  EXPECT_STREQ(dest, (prefix_path_name + filename).c_str());

  // Path that has length FN_REFLEN-1
  dest[0] = '\0';
  std::string cur_dir_path_name;
  for (int i = 0; i < (FN_REFLEN - 3); i++) cur_dir_path_name.append("y");
  cur_dir_path_name = FN_CURLIB + (FN_LIBCHAR + cur_dir_path_name);
  my_load_path(dest, cur_dir_path_name.c_str(), nullptr);
  EXPECT_STREQ(dest, cur_dir_path_name.c_str());

  // Path that has length FN_REFLEN.
  dest[0] = '\0';
  cur_dir_path_name.append("y");
  my_load_path(dest, cur_dir_path_name.c_str(), nullptr);
  EXPECT_STREQ(dest, cur_dir_path_name.substr(0, FN_REFLEN - 1).c_str());

  // Path that has length exceeding FN_REFLEN
  dest[0] = '\0';
  cur_dir_path_name.append("y");
  my_load_path(dest, cur_dir_path_name.c_str(), nullptr);
  EXPECT_STREQ(dest, cur_dir_path_name.substr(0, FN_REFLEN - 1).c_str());
}
}  // namespace mysys_my_load_path
