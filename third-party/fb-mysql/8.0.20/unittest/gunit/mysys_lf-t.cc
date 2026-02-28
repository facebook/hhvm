/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  Unit tests for lock-free algorithms of mysys
*/

#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "lf.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/my_thread_bits.h"

namespace mysys_lf_unittest {

#include "unittest/gunit/thr_template.cc"

std::atomic<int32> inserts{0};
std::atomic<int32> N{0};
LF_ALLOCATOR lf_allocator;
LF_HASH lf_hash;

int with_my_thread_init = 0;

/*
  pin allocator - alloc and release an element in a loop
*/
extern "C" void *test_lf_pinbox(void *arg) {
  int m = *(int *)arg;
  LF_PINS *pins;

  if (with_my_thread_init) my_thread_init();

  pins = lf_pinbox_get_pins(&lf_allocator.pinbox);

  for (; m; m--) {
    lf_pinbox_put_pins(pins);
    pins = lf_pinbox_get_pins(&lf_allocator.pinbox);
  }
  lf_pinbox_put_pins(pins);

  if (with_my_thread_init) my_thread_end();

  return nullptr;
}

/*
  thread local data area, allocated using lf_alloc.
  union is required to enforce the minimum required element size (sizeof(ptr))
*/
union TLA {
  std::atomic<int32> data;
  void *not_used;
};

// SUPPRESS_UBSAN: integer overflow when generating random data.
extern "C" void *test_lf_alloc(void *arg) SUPPRESS_UBSAN;
extern "C" void *test_lf_alloc(void *arg) {
  int m = (*(int *)arg) / 2;
  int32 x, y = 0;
  LF_PINS *pins;

  if (with_my_thread_init) my_thread_init();

  pins = lf_pinbox_get_pins(&lf_allocator.pinbox);

  for (x = ((int)(intptr)(&m)); m; m--) {
    TLA *node1, *node2;
    x = (x * m + 0x87654321) & INT_MAX32;
    node1 = (TLA *)lf_alloc_new(pins);
    node1->data = x;
    y += node1->data;
    node1->data = 0;
    node2 = (TLA *)lf_alloc_new(pins);
    node2->data = x;
    y -= node2->data;
    node2->data = 0;
    lf_pinbox_free(pins, node1);
    lf_pinbox_free(pins, node2);
  }
  lf_pinbox_put_pins(pins);

  bad += y;

  if (--N == 0) {
#ifdef MY_LF_EXTRA_DEBUG
    bad |= lf_allocator.mallocs - lf_alloc_pool_count(&lf_allocator);
#endif
  }

  if (with_my_thread_init) my_thread_end();
  return nullptr;
}

const int N_TLH = 1000;
extern "C" void *test_lf_hash(void *arg) {
  int m = (*(int *)arg) / (2 * N_TLH);
  int32 x, y, z, sum = 0, ins = 0;
  LF_PINS *pins;

  if (with_my_thread_init) my_thread_init();

  pins = lf_hash_get_pins(&lf_hash);

  for (x = ((int)(intptr)(&m)); m; m--) {
    int i;
    y = x;
    for (i = 0; i < N_TLH; i++) {
      x = (x * (m + i) + 0x87654321) & INT_MAX32;
      z = (x < 0) ? -x : x;
      if (lf_hash_insert(&lf_hash, pins, &z)) {
        sum += z;
        ins++;
      }
    }
    for (i = 0; i < N_TLH; i++) {
      y = (y * (m + i) + 0x87654321) & INT_MAX32;
      z = (y < 0) ? -y : y;
      if (lf_hash_delete(&lf_hash, pins, (uchar *)&z, sizeof(z))) sum -= z;
    }
  }
  lf_hash_put_pins(pins);

  bad += sum;
  inserts += ins;

  if (--N == 0) {
    bad |= lf_hash.count;
  }

  if (with_my_thread_init) my_thread_end();
  return nullptr;
}

void do_tests() {
  lf_alloc_init(&lf_allocator, sizeof(TLA), offsetof(TLA, not_used));
  lf_hash_init(&lf_hash, sizeof(int), LF_HASH_UNIQUE, 0, sizeof(int), nullptr,
               &my_charset_bin);

  with_my_thread_init = 1;
  test_concurrently("lf_pinbox (with my_thread_init)", test_lf_pinbox,
                    N = THREADS, CYCLES);
  test_concurrently("lf_alloc (with my_thread_init)", test_lf_alloc,
                    N = THREADS, CYCLES);
  test_concurrently("lf_hash (with my_thread_init)", test_lf_hash, N = THREADS,
                    CYCLES / 10);

  with_my_thread_init = 0;
  test_concurrently("lf_pinbox (without my_thread_init)", test_lf_pinbox,
                    N = THREADS, CYCLES);
  test_concurrently("lf_alloc (without my_thread_init)", test_lf_alloc,
                    N = THREADS, CYCLES);
  test_concurrently("lf_hash (without my_thread_init)", test_lf_hash,
                    N = THREADS, CYCLES / 10);

  lf_hash_destroy(&lf_hash);
  lf_alloc_destroy(&lf_allocator);
}

TEST(Mysys, LockFree) {
  my_thread_attr_init(&thr_attr);
  my_thread_attr_setdetachstate(&thr_attr, MY_THREAD_CREATE_DETACHED);

  do_tests();

  my_thread_attr_destroy(&thr_attr);
}

extern "C" {

static uint test_hash(const LF_HASH *, const uchar *key, size_t length) {
  if (length < sizeof(uint32))
    return 0;
  else {
    /* We use ulongget() to avoid potential problems with alignment. */
    return ulongget(key);
  }
}

static int test_match(const uchar *arg) {
  /*
    Unlike keys passed to hash function memory passed to match
    functions are always correctly aligned.
  */
  return *reinterpret_cast<const uint32 *>(arg) & 0x100;
}

} /* extern "C" */

/**
  Glass box test for lf_hash_random_match() function.
*/
TEST(Mysys, LFHashRandomMatch) {
  LF_HASH hash;
  LF_PINS *pins;
  uint32 val, *fnd, *null_fnd = nullptr;
  int rc;

  lf_hash_init2(&hash, sizeof(uint32), LF_HASH_UNIQUE, 0, sizeof(int), nullptr,
                &my_charset_bin, &test_hash, nullptr, nullptr, nullptr);
  /* Right after initialization hash is expected to be empty */
  EXPECT_EQ(0, hash.count.load());

  pins = lf_hash_get_pins(&hash);

  /* We should not be able to find anything in empty hash. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 0));
  EXPECT_EQ(null_fnd, fnd);
  lf_hash_search_unpin(pins);

  /* Insert a few non-matching values. */
  for (val = 0; val < 4; ++val) {
    rc = lf_hash_insert(&hash, pins, &val);
    EXPECT_EQ(0, rc);
  }
  EXPECT_EQ(4, hash.count.load());

  /* Search still should return nothing. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 0));
  EXPECT_EQ(null_fnd, fnd);
  lf_hash_search_unpin(pins);

  /* Even if we start from different bucket/hash value. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 3));
  EXPECT_EQ(null_fnd, fnd);
  lf_hash_search_unpin(pins);

  /* Insert matching record which also will be last in split-ordered list. */
  val = 3 + 0x100;
  rc = lf_hash_insert(&hash, pins, &val);
  EXPECT_EQ(0, rc);

  /*
    We should be able to find this record when we start searching
    from bucket #0.
  */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 0));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /*
    Also when searching from buckets #2 and #5, which don't have dummy nodes and
    lists associated yet.
  */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 2));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);
  fnd = static_cast<uint *>(lf_hash_random_match(&hash, pins, &test_match, 5));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /* Naturally, it should also be reachable from its native bucket #3. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 3));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /*
    If random value provided identifies element which should reside later
    than the matching element in split-ordered list it should NOT be normally
    reachable even if random value corresponds to the same bucket (i.e. #3).
    This allows to avoid bias towards elements at the start of the bucket list.

    But in this particular case it will be reachable because we have only one
    matching element in the hash and we restart our search from the head of
    split-ordered list if no matching elements were found at the tail.
    Such restart is necessary to avoid bias against elements at the start of
    the split-ordered list.
  */
  fnd = static_cast<uint32 *>(
      lf_hash_random_match(&hash, pins, &test_match, 3 + 0x10 + 0x100));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /* Let us add non-matching record to the end of the split-ordered list. */
  val = 7;
  rc = lf_hash_insert(&hash, pins, &val);
  EXPECT_EQ(0, rc);

  /*
    Our matching record will be reachable from its bucket #7 as well,
    since our search wraps around from the tail of the list to its head.
  */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 7));
  EXPECT_NE(null_fnd, fnd);
  val = 3 + 0x100;
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /* Now let us add one more matching record to bucket #4. */
  val = 4 + 0x100;
  rc = lf_hash_insert(&hash, pins, &val);
  EXPECT_EQ(0, rc);

  /* This record should be reachable from buckets #0 and #4. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 0));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 4));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /* But not from bucket #3. We will find another record instead. */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 3));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_NE(val, *fnd);
  lf_hash_search_unpin(pins);

  /*
    Thanks to search wrapping around from tail to the head of the list,
    our record will be reachable from bucket #7.
  */
  fnd =
      static_cast<uint32 *>(lf_hash_random_match(&hash, pins, &test_match, 7));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_EQ(val, *fnd);
  lf_hash_search_unpin(pins);

  /*
    To avoid bias towards elements at the start of the bucket list this record
    should not be reachable even from its native bucket #4, if random number
    identifies higher element. Instead we should get another matching element.
  */
  fnd = static_cast<uint32 *>(
      lf_hash_random_match(&hash, pins, &test_match, 4 + 0x10 + 0x100));
  EXPECT_NE(null_fnd, fnd);
  EXPECT_NE(val, *fnd);
  lf_hash_search_unpin(pins);

  lf_hash_put_pins(pins);
  lf_hash_destroy(&hash);
}
}  // namespace mysys_lf_unittest
