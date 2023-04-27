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
#include <cassert>
#include <iostream>
#include <map>
#include <memory>

#include "my_config.h"

#include <gtest/gtest.h>

#include "lot0buf.h"
#include "lot0dat.h"
#include "lot0lob.h"
#include "lot0lot.h"

#if 1
#define Fname(x) const char *fname = x;
#define LOG(x) \
  { std::cout << fname << ":" << x << std::endl; }
#else
#define Fname(x)
#define LOG(x)
#endif

void lob_tester_t::print_info() {
  const ulint SIZE = MB10;

  ut_ad(get_page_size() == KB16);

  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 888;

  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);

  lob::print(std::cout, ref);
}

/** Insert 10MB of data and fetch it.  This LOB will have the FIRST page, more
than one DATA page and also more than one INDEX page. This test case assumes
a page size of 16K. */
void lob_tester_t::fetch_full() {
  Fname("test_1");
  const ulint SIZE = MB10;

  ut_ad(get_page_size() == KB16);

  LOG("Inserting LOB of size " << SIZE << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);
  LOG("Inserted lobid=" << lobid);

  trx_id_t trxid = 28;

  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);

  ulint offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, offset, fetch_bytes, buf);
  LOG("Comparing bytes=" << fetch_bytes);
  if (memcmp(lob + offset, buf, fetch_bytes) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }
}

void test_2() {
  Fname("test_2");
  const ulint SIZE = 5000;

  LOG("Inserting LOB of size " << SIZE << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(10, ref, lob, SIZE);

  ulint replace_offset = 5;
  ulint replace_bytes = 10;
  byte *replace_buf = new byte[replace_bytes + 1];
  memset(replace_buf, '\0', replace_bytes + 1);
  strcpy((char *)replace_buf, "1234567890");
  lob::replace(12, ref, replace_offset, replace_bytes, replace_buf);

  ulint offset = 0;
  ulint fetch_bytes = 40;
  byte *buf = new byte[fetch_bytes];
  memset(buf, '\0', fetch_bytes);
  lob::read(11, ref, offset, fetch_bytes - 1, buf);
  LOG("buf=" << buf);
  lob::read(12, ref, offset, fetch_bytes - 1, buf);
  LOG("buf=" << buf);
}

void insert_generic(ulint size) {
  Fname("insert_generic");

  LOG("LOB size=" << size << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, size);

  trx_id_t trxid = 10;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, size);

  /* Fetch the LOB that has been inserted. */
  std::unique_ptr<byte> buf(new byte[size]);
  lob::read(trxid, ref, 0, size, buf.get());

  ASSERT_TRUE(memcmp(buf.get(), lob, size) == 0);

  lob_data::remove_lob(lobid);
  buf_pool_reset();
}

/** An LOB of only one page long.  The LOB has only the first page. Do an
insert_middle() which doesn't spill on other pages. This is done by same
transaction. This test case assumes a page size of 16K. */
void test_3() {
  Fname("test_3");
  const ulint SIZE = 100;

  LOG("Inserting LOB of size " << SIZE << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 10;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);
  LOG("Inserted LOB: " << lob);

  /* Fetch the LOB that has been inserted. */
  ulint offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, offset, fetch_bytes, buf);
  LOG("Fetched LOB: " << buf);

  /* Insert more data in middle of LOB. */
  ulint ins_offset = 20;
  ulint ins_bytes = 10;
  byte *ins_data = new byte[ins_bytes + 1];
  strcpy((char *)ins_data, "1234567890");
  LOG("Inserting data: " << ins_data);
  lob::insert_middle(trxid, ref, ins_offset, ins_data, ins_bytes);

  /* Fetch the LOB that has been inserted. */
  fetch_bytes = SIZE + ins_bytes;
  byte *buf2 = new byte[fetch_bytes + 1];
  memset(buf2, '\0', fetch_bytes + 1);
  lob::read(trxid, ref, offset, fetch_bytes, buf2);
  LOG("Fetched LOB: " << buf2);

  delete[] buf;
  delete[] buf2;
  delete[] ins_data;
}

/** An LOB that will have one FIRST page and one DATA page. Do an
insert_middle() on the second page, which doesn't spill on other pages. This is
done by same transaction. This test case assumes a page size of 16K. */
void lob_tester_t::test_4() {
  Fname("lob_tester_t::test_4");
  const ulint SIZE = KB16;

  LOG("Inserting LOB of size " << SIZE << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 182;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);
  LOG("Inserted lobid=" << lobid);

  /* Fetch the LOB that has been inserted. */
  ulint offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, offset, fetch_bytes, buf);
  if (memcmp(buf, lob, SIZE) == 0) {
    LOG("Fetch PASS");
  } else {
    LOG("Fetch FAIL");
  }

  /* Insert more data in middle of LOB. */
  ulint ins_offset = 11000;
  ulint ins_bytes = 10;
  byte *ins_data = new byte[ins_bytes + 1];
  strcpy((char *)ins_data, "1234567890");
  LOG("Inserting data: " << ins_data);
  lob::insert_middle(trxid, ref, ins_offset, ins_data, ins_bytes);

  /* Fetch the LOB that has been inserted. */
  fetch_bytes = SIZE + ins_bytes;
  byte *buf2 = new byte[fetch_bytes + 1];
  memset(buf2, '\0', fetch_bytes + 1);
  lob::read(trxid, ref, offset, fetch_bytes, buf2);

  LOG("Comparing the initial bytes=" << ins_offset);
  if (memcmp(buf, buf2, ins_offset) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }

  ulint trail = SIZE - ins_offset;
  LOG("Comparing the remaining bytes=" << trail);
  if (memcmp(buf + ins_offset, buf2 + ins_offset + ins_bytes, trail) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }

  if (memcmp(buf2 + ins_offset, ins_data, ins_bytes) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }

  delete[] buf;
  delete[] buf2;
  delete[] ins_data;
}

/** An LOB that will have only one FIRST page. Do an
remove_middle() on the first page, which doesn't spill on other pages. This
test case assumes a page size of 16K. */
void lob_tester_t::remove_middle_1() {
  Fname("lob_tester_t::test_4");
  const ulint SIZE = KB5;

  LOG("Inserting LOB of size " << SIZE << " bytes");
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 200;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);
  LOG("Inserted lobid=" << lobid);

  /* Fetch the LOB that has been inserted. */
  ulint offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, offset, fetch_bytes, buf);
  if (memcmp(buf, lob, SIZE) == 0) {
    LOG("Fetch PASS");
  } else {
    LOG("Fetch FAIL");
  }

  /* Remove 5 bytes data in middle of LOB. */
  ulint rm_offset = 15;
  ulint rm_bytes = 5;
  LOG("Removing " << rm_bytes << " bytes of data");
  lob::remove_middle(trxid, ref, rm_offset, rm_bytes);

  /* Fetch the LOB that has been modified. */
  fetch_bytes = SIZE - rm_bytes;
  byte *buf2 = new byte[fetch_bytes + 1];
  memset(buf2, '\0', fetch_bytes + 1);
  lob::read(trxid, ref, offset, fetch_bytes, buf2);

  LOG("Comparing the initial bytes=" << rm_offset);
  if (memcmp(buf, buf2, rm_offset) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }

  ulint trail = SIZE - rm_offset - rm_bytes;
  LOG("Comparing the remaining bytes=" << trail);
  if (memcmp(buf + rm_offset + rm_bytes, buf2 + rm_offset, trail) == 0) {
    LOG("PASS");
  } else {
    LOG("FAIL");
  }

  delete[] buf;
  delete[] buf2;
}

/** A stress tester for remove_middle(). Fixed removal length, with
a variable offset.  */
void lob_tester_t::remove_middle_stress_1() {
  const ulint SIZE = MB10;
  ulint rm_len = 100;

  for (ulint rm_offset = 0; rm_offset < SIZE; ++rm_offset) {
    std::cerr << "size=" << SIZE << ", len=" << rm_len << ", off=" << rm_offset
              << std::endl;
    remove_middle_gen(SIZE, rm_offset, rm_len);
  }
}

/** A stress tester for replace(). Fixed replace length, with
a variable offset.  */
void lob_tester_t::replace_stress() {
  const ulint SIZE = MB10;
  ulint len = 2000;

  for (ulint offset = 0; offset < SIZE; offset += 200) {
    std::cerr << "size=" << SIZE << ", len=" << len << ", off=" << offset
              << std::endl;
    replace_generic(SIZE, offset, len);
  }
}

/** A generic replace() tester. */
void lob_tester_t::replace_generic(ulint lob_size, ulint offset,
                                   ulint replace_len) {
  Fname("lob_tester_t::replace_generic");

  const ulint SIZE = lob_size;

  LOG("size=" << SIZE << ", len=" << replace_len << ", off=" << offset);

  /* Ensure that the requested replace length is within limits. */
  ulint can_be_replaced = lob_size - offset;
  if (replace_len > can_be_replaced) {
    replace_len = can_be_replaced;
  }

  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);
  trx_id_t trx1 = 100;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trx1, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  std::unique_ptr<byte> buf(new byte[fetch_bytes]);
  lob::read(trx1, ref, fetch_offset, fetch_bytes, buf.get());
  ut_ad(memcmp(buf.get(), lob, SIZE) == 0);

  /* Replace */
  lobid_t lobid2;
  trx_id_t trx2 = 300;
  ulint replace_offset = offset;
  byte *replace_lob = lob_data::generate_lob(&lobid2, '|', replace_len);
  lob::replace(trx2, ref, replace_offset, replace_len, replace_lob);

  /* Fetch the older LOB that has been originally inserted. */
  trx_id_t trx3 = 250;
  std::unique_ptr<byte> buf2(new byte[SIZE]);
  lob::read(trx3, ref, fetch_offset, fetch_bytes, buf2.get());
  ut_ad(memcmp(buf2.get(), lob, SIZE) == 0);

  /* Fetch the newer LOB that has been replaced. */
  trx_id_t trx4 = 350;
  memset(buf2.get(), '\0', SIZE);
  ulint len = lob::read(trx4, ref, fetch_offset, fetch_bytes, buf2.get());

  ut_ad(len == fetch_bytes);

  // Compare the initial bytes.
  ut_ad(memcmp(buf2.get(), lob, replace_offset) == 0);

  // Compare the replaced bytes.
  ut_ad(memcmp(buf2.get() + replace_offset, replace_lob, replace_len) == 0);

  // Compare the trailer
  ulint trailer_len = SIZE - replace_offset - replace_len;
  ut_ad(memcmp(buf2.get() + replace_offset + replace_len,
               lob + replace_offset + replace_len, trailer_len) == 0);

  trx_rollback(trx2, ref);

  /* Fetch the latest LOB. */
  trx_id_t trx5 = 400;
  memset(buf2.get(), '\0', SIZE);
  lob::read(trx5, ref, fetch_offset, fetch_bytes, buf2.get());
  ut_ad(memcmp(buf2.get(), lob, SIZE) == 0);

  lob_data::remove_lob(lobid);
  lob_data::remove_lob(lobid2);
  buf_pool_reset();
}

/** A generic remove_middle() tester. */
void lob_tester_t::remove_middle_gen(ulint lob_size, ulint offset,
                                     ulint rm_len) {
  // Fname("lob_tester_t::remove_middle_gen");
  const ulint SIZE = lob_size;

  /* Ensure that the requested removal length is within limits. */
  ulint can_be_removed = lob_size - offset;
  if (rm_len > can_be_removed) {
    rm_len = can_be_removed;
  }

  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 200;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, fetch_offset, fetch_bytes, buf);
  ut_ad(memcmp(buf, lob, SIZE) == 0);

  trx_id_t rm_trxid = 300;

  ulint rm_offset = offset;
  ulint rm_bytes = rm_len;
  lob::remove_middle(rm_trxid, ref, rm_offset, rm_bytes);

  trx_id_t trx3 = 400;

  /* Fetch the LOB that has been modified. */
  fetch_bytes = SIZE - rm_bytes;
  byte *buf2 = new byte[SIZE];
  memset(buf2, '\0', fetch_bytes + 1);
  ulint len = lob::read(trx3, ref, fetch_offset, fetch_bytes, buf2);

  std::cout << "length=" << len << std::endl;
  /* Comparing the initial bytes. */
  ut_ad(memcmp(buf, buf2, rm_offset) == 0);

  /* Comparing the remaining bytes. */
  ulint trail = SIZE - rm_offset - rm_bytes;
  ut_ad(memcmp(buf + rm_offset + rm_bytes, buf2 + rm_offset, trail) == 0);

  trx_id_t trx4 = 250;

  /* Fetch the older LOB. */
  fetch_offset = 0;
  fetch_bytes = SIZE;
  memset(buf, 0x00, SIZE);
  lob::read(trx4, ref, fetch_offset, fetch_bytes, buf);
  ut_ad(memcmp(buf, lob, SIZE) == 0);

  /* Rollback the remove_middle() operation. */
  trx_rollback(rm_trxid, ref);

  trx_id_t trx5 = 500;

  /* Fetch the latest LOB. */
  fetch_offset = 0;
  fetch_bytes = SIZE;
  memset(buf, 0x00, SIZE);
  len = lob::read(trx5, ref, fetch_offset, fetch_bytes, buf);
  ut_ad(memcmp(buf, lob, SIZE) == 0);

  delete[] buf;
  delete[] buf2;

  lob_data::remove_lob(lobid);
  buf_pool_reset();
}

void lob_tester_t::insert_rollback() {
  const ulint SIZE = MB5;
  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);

  trx_id_t trxid = 300;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trxid, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  byte *buf = new byte[fetch_bytes];
  lob::read(trxid, ref, fetch_offset, fetch_bytes, buf);
  ut_ad(memcmp(buf, lob, SIZE) == 0);

  trx_rollback(trxid, ref);

  ulint len = lob::read(trxid, ref, fetch_offset, fetch_bytes, buf);
  ut_ad(len == 0);

  lob_data::remove_lob(lobid);
  buf_pool_reset();
}

/** A stress tester for insert_middle(). Fixed insert length, with
a variable offset.  */
void lob_tester_t::insert_middle_stress() {
  const ulint size = MB7;
  ulint len = 2000;

  for (ulint offset = 0; offset < size; offset += 200) {
    std::cerr << "size=" << size << ", len=" << len << ", off=" << offset
              << std::endl;
    insert_middle_generic(size, offset, len);
  }
}

/** A generic insert_middle() tester.
@param[in]  lob_size  the size of the LOB.
@param[in]  offset    The offset into LOB where new data inserted.
@param[in]  len       The length of new data inserted.*/
void lob_tester_t::insert_middle_generic(ulint lob_size, ulint offset,
                                         ulint len) {
  const ulint SIZE = lob_size;
  const ulint size2 = lob_size + len;
  ulint ret_len = 0;

  std::unique_ptr<byte> buf2(new byte[size2]);

  lobid_t lobid;
  byte *lob = lob_data::generate_lob(&lobid, SIZE);
  lobid_t lobid2;
  byte *lob2 = lob_data::generate_lob(&lobid2, '|', len);

  trx_id_t trx1 = 100;

  /* Insert the LOB */
  lob::ref_t ref;
  ref.m_id = lobid;
  lob::insert(trx1, ref, lob, SIZE);

  /* Fetch the LOB that has been inserted. */
  ulint fetch_offset = 0;
  ulint fetch_bytes = SIZE;
  std::unique_ptr<byte> buf(new byte[fetch_bytes]);
  lob::read(trx1, ref, fetch_offset, fetch_bytes, buf.get());
  ut_ad(memcmp(buf.get(), lob, SIZE) == 0);

  /* Insert Middle */
  trx_id_t trx2 = 300;
  lob::insert_middle(trx2, ref, offset, lob2, len);

  /* Fetch the older LOB that has been originally inserted. */
  trx_id_t trx3 = 250;
  memset(buf2.get(), '\0', size2);
  ret_len = lob::read(trx3, ref, 0, SIZE, buf2.get());
  ut_ad(memcmp(buf2.get(), lob, SIZE) == 0);

  /* Fetch the newer LOB that has been modified. */
  trx_id_t trx4 = 350;
  memset(buf2.get(), '\0', size2);
  ret_len = lob::read(trx4, ref, 0, size2, buf2.get());

  ut_ad(ret_len == size2);

  // Compare the initial bytes.
  ut_ad(memcmp(buf2.get(), lob, offset) == 0);

  // Compare the replaced bytes.
  ut_ad(memcmp(buf2.get() + offset, lob2, len) == 0);

  // Compare the trailer
  ulint trailer_len = SIZE - offset;
  ut_ad(memcmp(buf2.get() + offset + len, lob + offset, trailer_len) == 0);

  trx_rollback(trx2, ref);

  /* Fetch the latest LOB. */
  trx_id_t trx5 = 400;
  memset(buf2.get(), '\0', size2);
  lob::read(trx5, ref, 0, SIZE, buf2.get());
  ut_ad(memcmp(buf2.get(), lob, SIZE) == 0);

  lob_data::remove_lob(lobid);
  lob_data::remove_lob(lobid2);
  buf_pool_reset();
}

TEST(insert, InsertStressMB1) {
  for (ulint size = 0; size < MB1; size += KB1) {
    insert_generic(size);
  }
}

TEST(insert, InsertStressMB5) {
  for (ulint size = MB1; size < MB5; size += KB2) {
    insert_generic(size);
  }
}

TEST(replace, ReplaceStressMB3) {
  const ulint SIZE = MB3;
  ulint len = 2000;
  lob_tester_t tester;

  for (ulint offset = 0; offset < SIZE; offset += 1000) {
    tester.replace_generic(SIZE, offset, len);
  }
}
