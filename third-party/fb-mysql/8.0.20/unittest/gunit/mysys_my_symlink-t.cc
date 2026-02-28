/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include "m_string.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"

namespace mysys_my_symlink {

// For simplicity, we skip this test on Windows.
#if !defined(_WIN32)
TEST(Mysys, MysysMySymlink) {
  char filename[FN_REFLEN];
  int fd = create_temp_file(filename, nullptr, "gunit_mysys_symlink",
                            O_CREAT | O_WRONLY, KEEP_FILE, MYF(MY_WME));
  EXPECT_GT(fd, 0);

  char linkname[FN_REFLEN];
  char *name_end = my_stpcpy(linkname, filename);
  (*name_end++) = 'S';
  *name_end = 0;
  int ret = my_symlink(filename, linkname, MYF(MY_WME));
  EXPECT_EQ(0, ret);

  char resolvedname[FN_REFLEN];
  ret = my_realpath(resolvedname, linkname, MYF(MY_WME));
  EXPECT_EQ(0, ret);

  // In case filename is also based on a symbolic link, like
  // for for example on Mac:  /var -> /private/var
  char resolved_filename[FN_REFLEN];
  ret = my_realpath(resolved_filename, filename, MYF(MY_WME));
  EXPECT_EQ(0, ret);

  EXPECT_STREQ(resolvedname, resolved_filename);

  ret = my_close(fd, MYF(MY_WME));
  EXPECT_EQ(0, ret);

  ret = my_delete_with_symlink(linkname, MYF(MY_WME));
  EXPECT_EQ(0, ret);
}
#endif
}  // namespace mysys_my_symlink
