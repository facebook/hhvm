/*****************************************************************************

Copyright (c) 2016, 2018, Oracle and/or its affiliates. All Rights Reserved.

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
#ifndef _lob0int_h_
#define _lob0int_h_

#include <string.h>

#include "buf0buf.h"
#include "fut0fut.h"
#include "fut0lst.h"
#include "lot0buf.h"
#include "lot0types.h"
#include "trx0types.h"

namespace lob {
const ulint LOB_HDR_PART_LEN = 0;
const ulint LOB_HDR_TRX_ID = 4;
const ulint LOB_HDR_SIZE = 10;
const ulint LOB_PAGE_DATA = FIL_PAGE_DATA + LOB_HDR_SIZE;

/** The offset where the list base node is located.  This is the list
of LOB pages. */
const ulint LOB_INDEX_LIST = LOB_PAGE_DATA;

/** The offset where the list base node is located.  This is the list
of free pages. */
const ulint LOB_INDEX_FREE_NODES = LOB_PAGE_DATA + 16;

const ulint LOB_PAGE_TRAILER_LEN = 8;

/** This page type refers to the first page of an LOB, which contains
half data and half meta data. */
#define FIL_PAGE_TYPE_LOB_BASE 21

struct base_node_page_t;

/** An index entry pointing to an LOB page. */
struct index_entry_t {
  index_entry_t(flst_node_t *node) : m_node(node) {}
  void reset(flst_node_t *node) { m_node = node; }

  /** Index entry offsets within node. */
  static const ulint OFFSET_PREV = 0;
  static const ulint OFFSET_NEXT = 6;

  /** Points to base node of the list of versions. The size of base node is
  16 bytes. */
  static const ulint OFFSET_VERSIONS = 12;
  static const ulint OFFSET_TRXID = 28;
  static const ulint OFFSET_PAGE_NO = 34;
  static const ulint OFFSET_DATA_LEN = 38;

  /** Total length of an index node. */
  static const ulint SIZE = 40;

  /** The versions base node is set to NULL. */
  void set_versions_null() {
    byte *base_node = get_versions_ptr();
    flst_init(base_node);
  }

  /** Can this index entry be purged.
  @param[in] trxid the transaction that is being purged.
  @return true if this entry can be purged, false otherwise. */
  bool can_be_purged(trx_id_t trxid) { return (trxid == get_trx_id()); }

  /* The given entry becomes the old version of the current entry.
  Move the version base node from old entry to current entry.
  @param[in]  entry  the old entry */
  void set_old_version(index_entry_t &entry) {
    flst_node_t *node = entry.get_node_ptr();
    flst_base_node_t *version_list = get_versions_ptr();
    ut_ad(flst_get_len(version_list) == 0);

    entry.move_version_base_node(*this);
    flst_add_first(version_list, node);
  }

  /** The current index entry points to a latest LOB page.  It may or may
  not have older versions.  If older version is there, bring it back to the
  index list from the versions list.  Then remove the current entry from
  the index list.  Move the versions list from current entry to older entry.
  @param[in]  trxid  The transaction identifier.
  @param[in]  first_page  The first lob page containing index list and free
                          list. */
  void make_old_version_current(trx_id_t trxid, base_node_page_t &first_page);

  fil_addr_t purge_version(trx_id_t trxid, flst_base_node_t *ver_list,
                           flst_base_node_t *free_list);

  void add_version(index_entry_t &entry) const {
    flst_node_t *node = entry.get_node_ptr();
    flst_base_node_t *version_list = get_versions_ptr();
    flst_add_first(version_list, node);
  }

  flst_base_node_t *get_versions_list() const { return (get_versions_ptr()); }

  trx_id_t get_trx_id() const {
    byte *ptr = get_trxid_ptr();
    return (mach_read_from_6(ptr));
  }

  void set_trx_id(trx_id_t id) {
    byte *ptr = get_trxid_ptr();
    return (mach_write_to_6(ptr, id));
  }

  void set_page_no(page_no_t num) {
    byte *ptr = get_pageno_ptr();
    return (mach_write_to_4(ptr, num));
  }

  void set_prev_null() { flst_write_addr(m_node + OFFSET_PREV, fil_addr_null); }

  void set_next_null() { flst_write_addr(m_node + OFFSET_NEXT, fil_addr_null); }

  page_no_t get_page_no() {
    byte *ptr = get_pageno_ptr();
    return (mach_read_from_4(ptr));
  }

  void set_data_len(ulint len) {
    byte *ptr = get_datalen_ptr();
    return (mach_write_to_2(ptr, len));
  }

  ulint get_data_len() {
    byte *ptr = get_datalen_ptr();
    return (mach_read_from_2(ptr));
  }

  std::ostream &print(std::ostream &out) {
    out << "[index_entry_t: trxid=" << get_trx_id()
        << ", page_no=" << get_page_no() << ", data_len=" << get_data_len()
        << "]";
    return (out);
  }

  bool is_same(const index_entry_t &that) { return (m_node == that.m_node); }

 private:
  /** Move the version base node from current entry to the given entry.
  @param[in]  entry  The index entry to which the version base node is moved
                     to. */
  void move_version_base_node(index_entry_t &entry);

  /** Purge the current index entry. An index entry points to either a FIRST
  page or DATA page.  That LOB page will be freed if it is DATA page.  A FIRST
  page should not be freed. */
  void purge();

  byte *get_versions_ptr() const { return (m_node + OFFSET_VERSIONS); }
  byte *get_trxid_ptr() const { return (m_node + OFFSET_TRXID); }
  byte *get_pageno_ptr() const { return (m_node + OFFSET_PAGE_NO); }
  byte *get_datalen_ptr() const { return (m_node + OFFSET_DATA_LEN); }
  byte *get_node_ptr() const { return (m_node); }

  byte *m_node;
};

struct page_t {
  page_t() : m_block(nullptr) {}
  page_t(buf_block_t *block) : m_block(block) {}

  trx_id_t get_trx_id() const {
    return (mach_read_from_6(frame() + FIL_PAGE_DATA + LOB_HDR_TRX_ID));
  }

  page_no_t get_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_OFFSET));
  }

  void set_data_len(ulint len) {
    mach_write_to_4(frame() + FIL_PAGE_DATA + LOB_HDR_PART_LEN, len);
  }

  void set_next_page(page_no_t page_no) {
    mach_write_to_4(frame() + FIL_PAGE_NEXT, page_no);
  }

  void set_next_page_null() {
    mach_write_to_4(frame() + FIL_PAGE_NEXT, FIL_NULL);
  }

  page_no_t get_next_page() {
    return (mach_read_from_4(frame() + FIL_PAGE_NEXT));
  }

  ulint get_data_len() {
    return (mach_read_from_4(frame() + FIL_PAGE_DATA + LOB_HDR_PART_LEN));
  }

  byte *frame() const { return (buf_block_get_frame(m_block)); }

  static ulint payload();
  ulint max_space_available();

 protected:
  buf_block_t *m_block;
};

struct data_page_t : public page_t {
  data_page_t() {}
  data_page_t(buf_block_t *block) : page_t(block) {}

  buf_block_t *alloc() {
    ut_ad(m_block == nullptr);
    m_block = btr_page_alloc();
    set_page_type();
    set_next_page_null();
    return (m_block);
  }

  void set_page_type() {
    mach_write_to_2(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_LOB_DATA);
  }

  void set_trx_id(trx_id_t id) {
    mach_write_to_6(frame() + FIL_PAGE_DATA + LOB_HDR_TRX_ID, id);
  }

  static ulint payload() {
    return (UNIV_PAGE_SIZE - LOB_PAGE_DATA - LOB_PAGE_TRAILER_LEN);
  }

  byte *data_begin() const { return (frame() + LOB_PAGE_DATA); }

  buf_block_t *replace(trx_id_t trxid, ulint offset, byte *&ptr, ulint &want);

  ulint read(trx_id_t trxid, ulint offset, byte *ptr, ulint want) {
    byte *start = data_begin();
    start += offset;
    ulint avail_data = get_data_len() - offset;

    ulint copy_len = want < avail_data ? want : avail_data;
    memcpy(ptr, start, copy_len);
    return (copy_len);
  }

  ulint write(trx_id_t trxid, byte *&data, ulint &len) {
    byte *ptr = data_begin();
    ulint written = (len > payload()) ? payload() : len;
    memcpy(ptr, data, written);
    set_data_len(written);

    data += written;
    len -= written;

    return (written);
  }

  ulint append(trx_id_t trxid, byte *&data, ulint &len);

  std::pair<ulint, byte *> insert_middle(trx_id_t trxid, ulint offset,
                                         byte *&data, ulint &len,
                                         buf_block_t *&new_block);

  buf_block_t *remove_middle(trx_id_t trxid, ulint offset, ulint &len);

  ulint max_space_available() const { return (payload()); }
};

struct base_node_page_t : public page_t {
  base_node_page_t() {}
  base_node_page_t(buf_block_t *block) : page_t(block) {}

  buf_block_t *alloc();

  void dealloc() {
    btr_page_free(m_block);
    m_block = nullptr;
  }

  bool empty() const {
    flst_base_node_t *base = index_list();
    return (flst_get_len(base) == 0);
  }

  /** Allocate one index entry. */
  flst_node_t *alloc_index_entry();

  byte *nodes_begin() const {
    return (frame() + LOB_PAGE_DATA + FLST_BASE_NODE_SIZE +
            FLST_BASE_NODE_SIZE);
  }

  static ulint payload() {
    return (UNIV_PAGE_SIZE - LOB_PAGE_DATA - LOB_PAGE_TRAILER_LEN -
            FLST_BASE_NODE_SIZE - FLST_BASE_NODE_SIZE);
  }

  void set_trx_id(trx_id_t id) {
    mach_write_to_6(frame() + FIL_PAGE_DATA + LOB_HDR_TRX_ID, id);
  }

  std::pair<ulint, byte *> insert_middle(trx_id_t trxid, ulint offset,
                                         byte *&data, ulint &len,
                                         buf_block_t *&new_block);

  buf_block_t *remove_middle(trx_id_t trxid, ulint offset, ulint &len);

  /** Write as much as possible of the given data into the page. */
  ulint write(trx_id_t trxid, byte *&data, ulint &len) {
    byte *ptr = data_begin();
    ulint space_available = payload() / 2;
    ulint written = (len > space_available) ? space_available : len;
    memcpy(ptr, data, written);
    set_data_len(written);
    set_trx_id(trxid);

    data += written;
    len -= written;

    return (written);
  }

  buf_block_t *replace(trx_id_t trxid, ulint offset, byte *&ptr, ulint &want);

  ulint read(trx_id_t trxid, ulint offset, byte *ptr, ulint want) {
    byte *start = data_begin();
    start += offset;
    ulint avail_data = get_data_len() - offset;

    ulint copy_len = want < avail_data ? want : avail_data;
    memcpy(ptr, start, copy_len);
    return (copy_len);
  }

  void set_page_type() {
    mach_write_to_2(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_LOB_FIRST);
  }

  flst_base_node_t *index_list() const { return (frame() + LOB_INDEX_LIST); }

  flst_base_node_t *free_list() const {
    return (frame() + LOB_INDEX_FREE_NODES);
  }

  static ulint max_space_available() { return (payload() / 2); }

  /** Get the number of index entries this page can hold.
  @return Number of index entries this page can hold. */
  static ulint node_count() {
    return (max_space_available() / index_entry_t::SIZE);
  }

  std::ostream &print_index_entries(std::ostream &out) const {
    flst_base_node_t *base = index_list();
    fil_addr_t node_loc = flst_get_first(base);

    while (!fil_addr_is_null(node_loc)) {
      flst_node_t *node = fut_get_ptr(node_loc);
      out << (void *)node << std::endl;
      index_entry_t entry(node);
      entry.print(out) << std::endl;
      node_loc = flst_get_next_addr(node);
    }
    return (out);
  }

  byte *data_begin() const {
    ulint space_available = payload() / 2;
    return (frame() + UNIV_PAGE_SIZE - LOB_PAGE_TRAILER_LEN - space_available);
  }

  /** Append data into a LOB first page. */
  ulint append(trx_id_t trxid, byte *&data, ulint &len);
};

struct node_page_t : public page_t {
  node_page_t() {}
  node_page_t(buf_block_t *block) : page_t(block) {}

  buf_block_t *alloc(base_node_page_t &first_page);

  static ulint payload() {
    return (UNIV_PAGE_SIZE - FIL_PAGE_DATA - FIL_PAGE_DATA_END);
  }

  static ulint max_space_available() { return (payload()); }

  /** Get the number of index entries this page can hold.
  @return Number of index entries this page can hold. */
  static ulint node_count() {
    return (max_space_available() / index_entry_t::SIZE);
  }

  void set_page_type() {
    mach_write_to_2(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_LOB_INDEX);
  }

  byte *nodes_begin() const { return (frame() + LOB_PAGE_DATA); }
};

}  // namespace lob
#endif  // _lob0int_h_
