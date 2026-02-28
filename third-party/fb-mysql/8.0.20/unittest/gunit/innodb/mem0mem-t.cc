/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* See http://code.google.com/p/googletest/wiki/Primer */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>

#include "sql/handler.h"
#include "storage/innobase/include/mem0mem.h"
#include "storage/innobase/include/os0event.h"
#include "storage/innobase/include/srv0conc.h"
#include "storage/innobase/include/srv0srv.h"
#include "storage/innobase/include/univ.i"

namespace innodb_mem0mem_unittest {

class mem0mem : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    srv_max_n_threads = srv_sync_array_size + 1;
    os_event_global_init();
    sync_check_init(srv_max_n_threads);
  }
  static void TearDownTestCase() {
    sync_check_close();
    os_event_global_destroy();
  }
};

/* test mem_heap_is_top() */
TEST_F(mem0mem, memheapistop) {
  mem_heap_t *heap;
  const char *str = "aabbccddeeff";
  size_t str_len = strlen(str);
  char *str_in_heap;
  void *dummy;

#define INITIAL_HEAP_SIZE 512

  heap = mem_heap_create(INITIAL_HEAP_SIZE);

  str_in_heap = mem_heap_strdup(heap, str);

  EXPECT_TRUE(mem_heap_is_top(heap, str_in_heap, str_len + 1));

  /* Check with a random pointer to make sure that mem_heap_is_top()
  does not return true unconditionally. */
  EXPECT_FALSE(mem_heap_is_top(heap, "foo", 4));

  /* Allocate another chunk and check that our string is not at the
  top anymore. */
  dummy = mem_heap_alloc(heap, 32);
  ut_a(dummy != nullptr);
  EXPECT_FALSE(mem_heap_is_top(heap, str_in_heap, str_len + 1));

  /* Cause the heap to allocate a second block and retest. */
  dummy = mem_heap_alloc(heap, INITIAL_HEAP_SIZE + 1);
  str_in_heap = mem_heap_strdup(heap, str);
  EXPECT_TRUE(mem_heap_is_top(heap, str_in_heap, str_len + 1));

  /* Allocate another chunk, free it, and then confirm that our string
  is still the topmost element. */
  const ulint x = 64;
  dummy = mem_heap_alloc(heap, x);
  EXPECT_FALSE(mem_heap_is_top(heap, str_in_heap, str_len + 1));
  mem_heap_free_top(heap, x);
  EXPECT_TRUE(mem_heap_is_top(heap, str_in_heap, str_len + 1));

  mem_heap_free(heap);
}

/* test mem_heap_replace() */
TEST_F(mem0mem, memheapreplace) {
  mem_heap_t *heap;
  void *p1;
  const ulint p1_size = 16;
  void *p2;
  const ulint p2_size = 32;
  void *p3;
  const ulint p3_size = 64;
  void *p4;
  const ulint p4_size = 128;
  void *p5;
  const ulint p5_size = 256;

  heap = mem_heap_create(1024);

  p1 = mem_heap_alloc(heap, p1_size);
  p2 = mem_heap_alloc(heap, p2_size);
  p3 = mem_heap_replace(heap, p1, p1_size, p3_size);

  EXPECT_NE(p2, p3);
  EXPECT_TRUE(mem_heap_is_top(heap, p3, p3_size));

  p4 = mem_heap_replace(heap, p3, p3_size, p4_size);

  EXPECT_EQ(p3, p4);
  EXPECT_TRUE(mem_heap_is_top(heap, p4, p4_size));

  p5 = mem_heap_replace(heap, p4, p4_size - 5, p5_size);

#ifdef UNIV_MEM_DEBUG
  /* In UNIV_MEM_DEBUG we need to specify the correct size of the
  old top in order for it to get replaced. */
  EXPECT_NE(p4, p5);
#else  /* UNIV_MEM_DEBUG */
  EXPECT_EQ(p4, p5);
#endif /* UNIV_MEM_DEBUG */
  EXPECT_TRUE(mem_heap_is_top(heap, p5, p5_size));

  mem_heap_free(heap);
}

}  // namespace innodb_mem0mem_unittest
