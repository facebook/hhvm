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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "my_sys.h"

namespace my_b_vprintf_unittest {

void test1(const char *res, const char *fmt, ...) {
  IO_CACHE info;
  va_list args;
  size_t len;
  init_io_cache(&info, -1, 0, WRITE_CACHE, 0, true, MYF(0));
  memset(info.write_buffer, 0, 64); /* RECORD_CACHE_SIZE is 64K */
  va_start(args, fmt);
  len = my_b_vprintf(&info, fmt, args);
  va_end(args);
  EXPECT_EQ(len, strlen(res));
  EXPECT_STREQ((char *)(info.write_buffer), res);
  end_io_cache(&info);
}

TEST(Mysys, Vprintf) {
  test1("Format specifier s works", "Format specifier s %s", "works");
  test1("Format specifier b works", "Format specifier b %.5b", "works!!!");
  test1("Format specifier b with * works", "Format specifier b with * %.*b", 5,
        "workss!!!");
  test1("Format specifier c !", "Format specifier c %c", '!');
  test1("Format specifier d 1", "Format specifier d %d", 1);
  test1("Format specifier u 2", "Format specifier u %u", 2);
  test1("Format specifier ld -3", "Format specifier ld %ld", -3L);
  test1("Format specifier lu 4", "Format specifier lu %lu", 4L);
  test1("Format specifier llu 5", "Format specifier llu %llu", 5LL);

  test1("Width is ignored for strings <x> <y>",
        "Width is ignored for strings <%04s> <%5s>", "x", "y");

  test1("Hello", "Hello");
  test1("Hello int, 1", "Hello int, %d", 1);
  test1("Hello int, -1", "Hello int, %d", -1);
  test1("Hello string 'I am a string'", "Hello string '%s'", "I am a string");
  test1("Hello hack hack 1", "Hello hack hack %d", 1);
  test1("Hello 1 hack 4", "Hello %d hack %d", 1, 4);
  test1("Hello 1 hack hack hack hack hack 4",
        "Hello %d hack hack hack hack hack %d", 1, 4);
  test1("Hello 'hack' hhhhhhhhhhhhhhh", "Hello '%s' hhhhhhhhhhhhhhh", "hack");
  test1("Hello hhhhhhhhh 1 sssssssss", "Hello hhhhhhhhh %d sssssssss", 1);
  test1("Hello 1", "Hello %u", 1);
  test1("Hello 4294967295", "Hello %u", -1);
  test1("Hello TEST", "Hello %05s", "TEST");
}

}  // namespace my_b_vprintf_unittest
