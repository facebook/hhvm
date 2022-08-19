/*****************************************************************************

Copyright (c) 2016, 2017, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

#include "my_config.h"

#include <memory>

#include <gtest/gtest.h>

#include "db0err.h"
#include "lot0dat.h"
#include "lot0lob.h"
#include "zlob0int.h"

#define Fname(x) const char *fname = x;
#define LOG(x) \
  { std::cout << fname << ":" << x << std::endl; }

using namespace zlob;

void index_entry_test_00() {
  std::unique_ptr<byte> ptr(new byte[300]);
  z_index_entry_t ie(ptr.get());
  ie.init();
  std::cout << ie << std::endl;
  std::cout << "Size of one z_index_entry_t: " << ie.SIZE << std::endl;
}

void frag_entry_test_00() {
  std::unique_ptr<byte> ptr(new byte[300]);
  z_frag_entry_t fe(ptr.get());
  fe.init();
  std::cout << fe << std::endl;
  std::cout << "Size of one z_frag_entry_t: " << fe.SIZE << std::endl;
}

void z_first_page_basic_test() {
  z_first_page_t first;
  std::cout << "Size of index entries=" << first.size_of_index_entries()
            << std::endl;
  std::cout << "Size of frag entries=" << first.size_of_frag_entries()
            << std::endl;
  std::cout << "Payload=" << first.payload() << std::endl;
  first.alloc();

  std::cout << first << std::endl;
}

void basic_insert_test(ulint size) {
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, size);

  trx_id_t trxid = 28;

  lob::ref_t ref;
  ref.m_id = lobid;
  dberr_t err = zlob::z_insert(trxid, ref, lob, size);
  ASSERT_TRUE(err == DB_SUCCESS);

  std::unique_ptr<byte> buf(new byte[size]);
  ulint n = zlob::z_read(trxid, ref, 0, size, buf.get());
  ASSERT_EQ(n, size);
  ASSERT_TRUE(memcmp(lob, buf.get(), size) == 0);

  lob_data::remove_lob(lobid);
  buf_pool_reset();
}

void basic_insert_test_2() {
  lobid_t lobid;
  std::pair<byte *, ulint> result = lob_data::get_lob(&lobid, "earth215kb.jpg");

  byte *lob = result.first;
  ulint size = result.second;

  std::cout << "lobid=" << lobid << ", size=" << size << std::endl;

  trx_id_t trxid = 30;

  lob::ref_t ref;
  ref.m_id = lobid;
  dberr_t err = zlob::z_insert(trxid, ref, lob, size);
  ut_a(err == DB_SUCCESS);

  zlob::z_print_info(ref, std::cout);

  lob_data::remove_lob(lobid);
}

void basic_insert_read_test() {
  lobid_t lobid;
  std::pair<byte *, ulint> result =
      lob_data::get_lob(&lobid, "/home/innodb/x.avi");

  byte *lob = result.first;
  ulint size = result.second;

  std::cout << "lobid=" << lobid << ", size=" << size << std::endl;

  trx_id_t trxid = 50;

  lob::ref_t ref;
  ref.m_id = lobid;
  dberr_t err = zlob::z_insert(trxid, ref, lob, size);
  ut_a(err == DB_SUCCESS);

  zlob::z_print_info(ref, std::cout);

  std::unique_ptr<byte> buf(new byte[size]);

  ulint n = zlob::z_read(trxid, ref, 0, size, buf.get());
  ut_a(n == size);
  ut_a(memcmp(lob, buf.get(), size) == 0);
  lob_data::remove_lob(lobid);
}

void z_replace_generic(ulint size, ulint offset, ulint replace_len) {
  Fname("z_replace_generic");

  LOG("size=" << size << ", offset=" << offset << ", len=" << replace_len);

  const ulint SIZE = size;
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  /* Ensure that the requested replace length is within limits. */
  ulint can_be_replaced = SIZE - offset;

  /* alen is for actual replace length. */
  ulint alen = replace_len;
  if (replace_len > can_be_replaced) {
    alen = can_be_replaced;
  }

  trx_id_t trx1 = 100;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  zlob::z_insert(trx1, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  std::unique_ptr<byte> buf(new byte[fetch_bytes]);
  zlob::z_read(trx1, ref, fetch_offset, fetch_bytes, buf.get());
  ut_ad(memcmp(buf.get(), lob, SIZE) == 0);

  /* Replace */
  lobid_t lobid2;
  trx_id_t trx2 = 300;
  ulint replace_offset = offset;
  byte *replace_lob = lob_data::generate_lob(&lobid2, '|', replace_len);
  ulint l1 =
      zlob::z_replace(trx2, ref, replace_offset, replace_len, replace_lob);

  if (l1 != alen) {
    ut_ad(0);
  }

  ASSERT_TRUE(l1 == alen);

  /* Fetch the older LOB that has been originally inserted. */
  trx_id_t trx3 = 250;
  std::unique_ptr<byte> buf2(new byte[SIZE]);
  zlob::z_read(trx3, ref, fetch_offset, fetch_bytes, buf2.get());

  if (memcmp(buf2.get(), lob, SIZE) != 0) {
    ulint i = 0;

    for (i = 0; i < SIZE; ++i) {
      if (lob[i] != buf2.get()[i]) {
        break;
      }
    }

    std::cout << i << std::endl;
    std::cout << replace_lob << std::endl;
    std::cout << lob << std::endl;
    std::cout << buf2.get() << std::endl;
  }

  ut_ad(memcmp(buf2.get(), lob, SIZE) == 0);

  /* Fetch the newer LOB that has been replaced. */
  trx_id_t trx4 = 350;
  memset(buf2.get(), '\0', SIZE);
  ulint len = zlob::z_read(trx4, ref, fetch_offset, fetch_bytes, buf2.get());

  ut_ad(len == fetch_bytes);

  // Compare the initial bytes.
  ut_ad(memcmp(buf2.get(), lob, replace_offset) == 0);

  // Compare the replaced bytes.
  ut_ad(memcmp(buf2.get() + replace_offset, replace_lob, alen) == 0);

  // Compare the trailer
  ulint trailer_len = SIZE - replace_offset - alen;
  ut_ad(memcmp(buf2.get() + replace_offset + alen, lob + replace_offset + alen,
               trailer_len) == 0);

  lob_data::remove_lob(lobid);
  lob_data::remove_lob(lobid2);
  buf_pool_reset();
}

void z_insert_middle_generic(ulint size, ulint offset, ulint insert_len) {
  Fname("z_insert_middle_generic");
  const ulint SIZE = size;
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  LOG("lob_size=" << size << ", offset=" << offset << ", len=" << insert_len);

  trx_id_t trx1 = 100;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  zlob::z_insert(trx1, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  std::unique_ptr<byte> buf(new byte[fetch_bytes]);
  zlob::z_read(trx1, ref, fetch_offset, fetch_bytes, buf.get());

  ASSERT_TRUE(memcmp(buf.get(), lob, SIZE) == 0);

  /* Insert middle */
  lobid_t lobid2;
  trx_id_t trx2 = 300;
  ulint insert_offset = offset;
  byte *insert_lob = lob_data::generate_lob(&lobid2, '|', insert_len);
  zlob::z_insert_middle(trx2, ref, insert_offset, insert_lob, insert_len);

  /* Fetch the older LOB that has been originally inserted. */
  trx_id_t trx3 = 250;
  std::unique_ptr<byte> buf2(new byte[SIZE]);
  zlob::z_read(trx3, ref, fetch_offset, fetch_bytes, buf2.get());

  ASSERT_TRUE(memcmp(buf2.get(), lob, SIZE) == 0);

  /* Fetch the newer LOB that has been enlarged. */
  trx_id_t trx4 = 350;
  ulint new_size = SIZE + insert_len;
  std::unique_ptr<byte> buf3(new byte[new_size]);
  memset(buf3.get(), '\0', new_size);
  ulint len = zlob::z_read(trx4, ref, 0, new_size, buf3.get());
  ASSERT_TRUE(len == new_size);

  // Compare the initial bytes.
  ASSERT_TRUE(memcmp(buf3.get(), lob, offset) == 0);

  // Compare the inserted bytes.
  if (memcmp(buf3.get() + offset, insert_lob, insert_len) != 0) {
    std::cout << buf3.get() << std::endl;
  }

  ASSERT_TRUE(memcmp(buf3.get() + offset, insert_lob, insert_len) == 0);

  // Compare the trailer
  ulint trailer_len = SIZE - offset;
  ASSERT_TRUE(
      memcmp(buf3.get() + offset + insert_len, lob + offset, trailer_len) == 0);

  lob_data::remove_lob(lobid);
  lob_data::remove_lob(lobid2);
  buf_pool_reset();
}

void z_remove_middle_generic(ulint size, ulint offset, ulint remove_len) {
  Fname("z_remove_middle_generic");

  LOG("size=" << size << ", offset=" << offset << ", len=" << remove_len);
  const ulint SIZE = size;
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trx1 = 100;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  zlob::z_insert(trx1, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  std::unique_ptr<byte> buf(new byte[fetch_bytes]);
  zlob::z_read(trx1, ref, fetch_offset, fetch_bytes, buf.get());

  ASSERT_TRUE(memcmp(buf.get(), lob, SIZE) == 0);

  /* Remove middle */
  trx_id_t trx2 = 300;
  zlob::z_remove_middle(trx2, ref, offset, remove_len);

  /* Fetch the older LOB that has been originally inserted. */
  trx_id_t trx3 = 250;
  std::unique_ptr<byte> buf2(new byte[SIZE]);
  zlob::z_read(trx3, ref, fetch_offset, fetch_bytes, buf2.get());

  if (memcmp(buf2.get(), lob, SIZE) != 0) {
    std::cout << lob << std::endl;
    std::cout << buf2.get() << std::endl;
  }
  ASSERT_TRUE(memcmp(buf2.get(), lob, SIZE) == 0);

  /* Fetch the newer LOB that has been shortened. */
  trx_id_t trx4 = 350;

  /* If there is request to remove more than available data, adjust it
  properly. */
  ulint can_delete = (SIZE - offset);
  ulint actually_deleted = remove_len > can_delete ? can_delete : remove_len;
  ulint new_size = SIZE - actually_deleted;
  std::unique_ptr<byte> buf3(new byte[new_size]);
  memset(buf3.get(), '\0', new_size);
  ulint len = zlob::z_read(trx4, ref, 0, new_size, buf3.get());
  ASSERT_EQ(len, new_size);

  ASSERT_TRUE(memcmp(buf3.get(), lob, offset) == 0);

  // Compare the trailer
  ulint trailer_len = SIZE - offset - actually_deleted;
  if (memcmp(buf3.get() + offset, lob + offset + actually_deleted,
             trailer_len) != 0) {
    std::cout << buf3.get() << std::endl;
    std::cout << lob << std::endl;
    ut_ad(0);
  }
  ASSERT_TRUE(memcmp(buf3.get() + offset, lob + offset + actually_deleted,
                     trailer_len) == 0);

  zlob::z_purge(300, ref);

  trx_id_t trx5 = 500;

  /* Fetch the LOB that has been inserted. */
  memset(buf.get(), 0x00, SIZE);
  zlob::z_read(trx5, ref, 0, SIZE, buf.get());
  ASSERT_TRUE(memcmp(buf.get(), lob, SIZE) == 0);

  lob_data::remove_lob(lobid);
  buf_pool_reset();
}

TEST(z_insert, StressUptoMB1) {
  for (ulint size = 0; size < MB1; size += KB1) {
    basic_insert_test(size);
  }
}

TEST(z_insert, StressUptoMB100) {
  for (ulint size = 0; size < MB100; size += MB10) {
    basic_insert_test(size);
  }
}

TEST(z_insert_middle, InsertFront) {
  const ulint offset = 0;
  ulint lob_size;
  ulint insert_length;

  lob_size = KB1;
  insert_length = MB1;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = 10;
  insert_length = 10;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = 10;
  insert_length = MB5;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = MB10;
  insert_length = MB10;
  z_insert_middle_generic(lob_size, offset, insert_length);
}

TEST(z_insert_middle, Append) {
  ulint offset = 0;
  ulint lob_size;
  ulint insert_length;

  lob_size = KB1;
  offset = lob_size;
  insert_length = MB1;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = 10;
  offset = lob_size;
  insert_length = 10;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = 10;
  offset = lob_size;
  insert_length = MB5;
  z_insert_middle_generic(lob_size, offset, insert_length);

  lob_size = MB10;
  offset = lob_size;
  insert_length = MB10;
  z_insert_middle_generic(lob_size, offset, insert_length);
}

TEST(z_insert_middle, StressUptoKB300) {
  const ulint lob_size = MB1;
  const ulint insert_length = KB128;

  for (ulint offset = 1; offset <= lob_size; offset += KB1) {
    z_insert_middle_generic(lob_size, offset, insert_length);
  }
}

TEST(z_remove_middle, reallymiddle) { z_remove_middle_generic(MB5, MB1, MB1); }

TEST(z_remove_middle, PopFront) {
  const ulint offset = 0;
  ulint lob_size = MB5;
  ulint remove_len = MB1;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB4;
  remove_len = MB2;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB3;
  remove_len = MB3;
  z_remove_middle_generic(lob_size, offset, remove_len);
}

TEST(z_remove_middle, PopBack) {
  ulint lob_size = MB5;
  ulint remove_len = MB1;
  ulint offset = (lob_size - remove_len);
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB4;
  offset = (lob_size - remove_len);
  remove_len = MB2;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB3;
  offset = (lob_size - remove_len);
  remove_len = MB3;
  z_remove_middle_generic(lob_size, offset, remove_len);
}

TEST(z_remove_middle, InvalidOffset) {
  ulint lob_size = MB5;
  ulint remove_len = MB1;
  ulint offset = lob_size;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB4;
  offset = lob_size;
  remove_len = MB2;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB3;
  offset = lob_size;
  remove_len = MB3;
  z_remove_middle_generic(lob_size, offset, remove_len);
}

TEST(z_remove_middle, FullRemoval) {
  const ulint offset = 0;

  ulint lob_size = MB5;
  ulint remove_len = MB5;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB10;
  remove_len = MB10;
  z_remove_middle_generic(lob_size, offset, remove_len);

  lob_size = MB1;
  remove_len = MB1;
  z_remove_middle_generic(lob_size, offset, remove_len);
}

TEST(z_remove_middle, StressUpto300KB) {
  const ulint lob_size = KB300;
  const ulint remove_len = KB1;

  for (ulint offset = 1; offset <= lob_size; offset += KB1) {
    z_remove_middle_generic(lob_size, offset, remove_len);
  }
}

TEST(z_replace, ReplaceBegin) {
  ulint size;
  ulint offset;
  ulint replace_len;

  size = MB10;
  offset = 0;
  replace_len = MB5;
  ;
  z_replace_generic(size, offset, replace_len);
}

TEST(z_replace, ReplaceStressKB500) {
  ulint lob_size = KB500;
  ulint replace_len = 1000;

  for (ulint offset = 0; offset <= lob_size; offset += 500) {
    z_replace_generic(lob_size, offset, replace_len);
  }
}
