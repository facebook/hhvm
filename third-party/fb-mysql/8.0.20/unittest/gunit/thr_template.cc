/* Copyright (c) 2006, 2018, Oracle and/or its affiliates. All rights reserved.

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

/*
  As this file may be included from inside a namespace, we cannot do
  #includes here. However, you should

    #include "my_systime.h"
    #include "mysql/components/services/my_thread_bits.h"

  outside that namespace.
*/

#include <atomic>
#include <vector>

std::atomic<int32> bad;
my_thread_attr_t thr_attr;

const int THREADS = 30;
const int CYCLES = 3000;

void test_concurrently(const char *test, my_start_routine handler, int n,
                       int m) {
  std::vector<my_thread_handle> t_vec;
  t_vec.resize(n);
  ulonglong now = my_getsystime();
  int num_threads = n;

  my_thread_attr_init(&thr_attr);
  bad = 0;

  for (; n; n--) {
    if (my_thread_create(&t_vec[n - 1], &thr_attr, handler, &m) != 0) {
      ADD_FAILURE() << "Could not create thread";
      abort();
    }
  }

  for (int i = 0; i < num_threads; ++i) {
    my_thread_join(&t_vec[i], nullptr);
  }

  now = my_getsystime() - now;
  EXPECT_FALSE(bad) << "tested " << test << " in " << ((double)now) / 1e7
                    << " secs "
                    << "(" << bad.load() << ")";
}
