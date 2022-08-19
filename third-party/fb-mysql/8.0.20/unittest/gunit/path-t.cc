/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <my_sys.h>

namespace file_path_ns {
#include "../client/path.cc"

TEST(Client, Path) {
  std::string filename = "filename";
  std::string filename2 = "filename2";
  static const std::string path1 = "/root";
  std::string path2 = "/subdir";

  Path p1(path1);
  Path p2;

  p1.append(path2);
  p1.filename(filename);
  EXPECT_STREQ("/root/subdir/filename", p1.to_str().c_str());
  p1.up();
  EXPECT_STREQ("/root/filename", p1.to_str().c_str());
  p1.filename("");
  EXPECT_STREQ("/root", p1.to_str().c_str());
  p1.up();
  EXPECT_STREQ("", p1.to_str().c_str());
  p1.append("/root/subdir");
  EXPECT_STREQ("/root/subdir", p1.to_str().c_str());
  p1.append("subdir");
  EXPECT_STREQ("/root/subdir/subdir", p1.to_str().c_str());
  p1.append("/subdir/");
  EXPECT_STREQ("/root/subdir/subdir/subdir", p1.to_str().c_str());
  p1.filename(filename);
  EXPECT_STREQ("/root/subdir/subdir/subdir/filename", p1.to_str().c_str());
  p1.filename_append(".exe");
  EXPECT_STREQ("/root/subdir/subdir/subdir/filename.exe", p1.to_str().c_str());
  p1.filename(filename2);
  EXPECT_STREQ("/root/subdir/subdir/subdir/filename2", p1.to_str().c_str());
  p1.up().up();
  EXPECT_STREQ("/root/subdir/filename2", p1.to_str().c_str());

  p1.parent_directory(&p2);
  EXPECT_STREQ("/root", p2.to_str().c_str());
}
}  // namespace file_path_ns
