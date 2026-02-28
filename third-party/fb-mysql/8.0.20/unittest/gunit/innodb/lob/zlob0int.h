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
#ifndef _zlob0int_h_
#define _zlob0int_h_

/** Internal header file for compressed lob (zlob). */

#include <string.h>
#include "db0err.h"
#include "fil0fil.h"
#include "fil0types.h"
#include "fut0lst.h"
#include "lot0buf.h"
#include "lot0lob.h"
#include "lot0plist.h"
#include "lot0types.h"
#include "page0page.h"

namespace zlob {

struct z_first_page_t;
struct z_frag_page_t;

/** An index entry pointing to one zlib stream. */
struct z_index_entry_t {
  /** Offset with index entry pointing to the prev index entry. */
  static const ulint OFFSET_PREV = 0;

  /** Offset with index entry pointing to the next index entry. */
  static const ulint OFFSET_NEXT = OFFSET_PREV + FIL_ADDR_SIZE;

  /** Offset within index entry pointing to base node of list of versions.*/
  static const ulint OFFSET_VERSIONS = OFFSET_NEXT + FIL_ADDR_SIZE;

  /** Offset within index entry pointing to trxid.*/
  static const ulint OFFSET_TRXID = OFFSET_VERSIONS + FLST_BASE_NODE_SIZE;

  /** Offset within index entry pointing to page number where zlib stream
  starts. This could be a data page or a fragment page. */
  static const ulint OFFSET_Z_PAGE_NO = OFFSET_TRXID + 6;

  /** Offset within index entry pointing to location of zlib stream.*/
  static const ulint OFFSET_Z_FRAG_ID = OFFSET_Z_PAGE_NO + 4;

  /** Offset within index entry pointing to uncompressed data len (bytes).*/
  static const ulint OFFSET_DATA_LEN = OFFSET_Z_FRAG_ID + 2;

  /** Offset within index entry pointing to compressed data len (bytes).*/
  static const ulint OFFSET_ZDATA_LEN = OFFSET_DATA_LEN + 4;

  /** Total size of one index entry. */
  static const ulint SIZE = OFFSET_ZDATA_LEN + 4;

  /** Constructor. */
  z_index_entry_t() : m_node(nullptr) {}
  z_index_entry_t(flst_node_t *node) : m_node(node) {}

  /** Point to another index entry.
  @param[in]  node  point to this file list node. */
  void reset(flst_node_t *node) { m_node = node; }

  /** Point to another index entry. */
  void reset(const z_index_entry_t &entry) { m_node = entry.m_node; }

  /** Initialize an index entry to some sane value. */
  void init() {
    set_prev_null();
    set_next_null();
    set_versions_null();
    set_trx_id(0);
    set_z_page_no(FIL_NULL);
    set_z_frag_id(FRAG_ID_NULL);
    set_data_len(0);
    set_zdata_len(0);
  }

  /** Can this index entry be purged.
  @param[in] trxid the transaction that is being purged.
  @return true if this entry can be purged, false otherwise. */
  bool can_be_purged(trx_id_t trxid) { return (trxid == get_trx_id()); }

  /** Purge one index entry.
  @param[in]  trxid  purging data belonging to trxid.
  @param[in,out]  lst  the list from which this entry will be removed.
  @param[in,out]  free_list the list to which this entry will be added. */
  fil_addr_t purge_version(trx_id_t trxid, z_first_page_t &first,
                           flst_base_node_t *lst, flst_base_node_t *free_list);

  /** Purge the current index entry. An index entry points to either a FIRST
  page or DATA page.  That LOB page will be freed if it is DATA page.  A FIRST
  page should not be freed. */
  void purge(z_first_page_t &first);

  /** Remove this node from the given list.
  @param[in]  bnode  the base node of the list from which to remove current
                     node. */
  void remove(flst_base_node_t *bnode) { flst_remove(bnode, m_node); }

  void insert_after(flst_base_node_t *base, z_index_entry_t &entry) {
    flst_insert_after(base, m_node, entry.get_node());
  }

  void insert_before(flst_base_node_t *base, z_index_entry_t &entry) {
    flst_insert_before(base, entry.get_node(), m_node);
  }

  /** Add this node as the last node in the given list.
  @param[in]  bnode  the base node of the file list. */
  void push_back(flst_base_node_t *bnode) { flst_add_last(bnode, m_node); }

  /** Add this node as the last node in the given list.
  @param[in]  bnode  the base node of the file list. */
  void push_front(flst_base_node_t *bnode) { flst_add_first(bnode, m_node); }

  /** Set the previous index entry as null. */
  void set_prev_null() { flst_write_addr(m_node + OFFSET_PREV, fil_addr_null); }

  /** Get the location of previous index entry. */
  fil_addr_t get_prev() const { return (flst_read_addr(m_node + OFFSET_PREV)); }

  /** Get the location of next index entry. */
  fil_addr_t get_next() const { return (flst_read_addr(m_node + OFFSET_NEXT)); }

  /** Set the next index entry as null. */
  void set_next_null() { flst_write_addr(m_node + OFFSET_NEXT, fil_addr_null); }

  /** Set the versions list as null. */
  void set_versions_null() {
    flst_base_node_t *bnode = get_versions_list();
    flst_init(bnode);
  }

  /** Get the base node of the list of versions. */
  flst_base_node_t *get_versions_list() const {
    return (m_node + OFFSET_VERSIONS);
  }

  trx_id_t get_trx_id() const {
    return (mach_read_from_6(m_node + OFFSET_TRXID));
  }

  /** Check to see if the given transaction can see this entry.
  @param[in]  trxid  the transaction wanting to read the entry.
  @return true if the transaction can see the entry, false otherwise.
  @todo This check is for the prototype.  When porting to InnoDB this
  will take into account the read view.  Fix it. */
  bool can_see(trx_id_t trxid) const {
    trx_id_t entry_trxid = get_trx_id();
    return (entry_trxid <= trxid);
  }

  void set_trx_id(trx_id_t id) {
    return (mach_write_to_6(m_node + OFFSET_TRXID, id));
  }

  page_no_t get_z_page_no() const {
    return (mach_read_from_4(m_node + OFFSET_Z_PAGE_NO));
  }

  void set_z_page_no(page_no_t page_no) {
    mlog_write_ulint(m_node + OFFSET_Z_PAGE_NO, page_no, MLOG_4BYTES);
  }

  page_no_t get_z_frag_id() const {
    return (mach_read_from_2(m_node + OFFSET_Z_FRAG_ID));
  }

  void set_z_frag_id(frag_id_t id) {
    mlog_write_ulint(m_node + OFFSET_Z_FRAG_ID, id, MLOG_2BYTES);
  }

  /** Get the uncompressed data length in bytes. */
  ulint get_data_len() const {
    return (mach_read_from_4(m_node + OFFSET_DATA_LEN));
  }

  /** Set the uncompressed data length in bytes.
  @param[in]  len  the uncompressed data length in bytes */
  void set_data_len(ulint len) {
    mlog_write_ulint(m_node + OFFSET_DATA_LEN, len, MLOG_4BYTES);
  }

  /** Get the compressed data length in bytes. */
  ulint get_zdata_len() const {
    return (mach_read_from_4(m_node + OFFSET_ZDATA_LEN));
  }

  /** Set the compressed data length in bytes.
  @param[in]  len  the compressed data length in bytes */
  void set_zdata_len(ulint len) {
    mlog_write_ulint(m_node + OFFSET_ZDATA_LEN, len, MLOG_4BYTES);
  }

  /* The given entry becomes the old version of the current entry.
  Move the version base node from old entry to current entry.
  @param[in]  entry  the old entry */
  void set_old_version(z_index_entry_t &entry) {
    flst_base_node_t *version_list = get_versions_list();
    ut_ad(flst_get_len(version_list) == 0);

    entry.move_version_base_node(*this);
    entry.push_front(get_versions_list());
  }

  /** The current index entry points to a latest LOB page.  It may or may
  not have older versions.  If older version is there, bring it back to the
  index list from the versions list.  Then remove the current entry from
  the index list.  Move the versions list from current entry to older entry.
  @param[in]  trxid  The transaction identifier.
  @param[in]  first  The first lob page containing index list and free
                          list. */
  fil_addr_t make_old_version_current(trx_id_t trxid, z_first_page_t &first);

  flst_node_t *get_node() { return (m_node); }
  bool is_null() const { return (m_node == nullptr); }

  std::ostream &print(std::ostream &out) const;
  std::ostream &print_pages(std::ostream &out) const;

 private:
  /** Move the version base node from current entry to the given entry.
  @param[in]  entry  The index entry to which the version base node is moved
                     to. */
  void move_version_base_node(z_index_entry_t &entry) {
    flst_base_node_t *from_node = get_versions_list();
    flst_base_node_t *to_node = entry.get_versions_list();

    memcpy(to_node, from_node, FLST_BASE_NODE_SIZE);
    ut_ad(flst_get_len(from_node) == flst_get_len(to_node));
    fil_addr_t addr1 = flst_get_first(from_node);
    fil_addr_t addr2 = flst_get_first(to_node);
    ut_ad(addr1.is_equal(addr2));
    addr1 = flst_get_last(from_node);
    addr2 = flst_get_last(to_node);
    ut_ad(addr1.is_equal(addr2));
    flst_init(from_node);
  }

  flst_node_t *m_node;
};

inline std::ostream &operator<<(std::ostream &out, const z_index_entry_t &obj) {
  return (obj.print(out));
}

/** An entry representing one fragment page. */
struct z_frag_entry_t {
 public:
  /** Offset withing frag entry pointing to prev frag entry. */
  static const ulint OFFSET_PREV = 0;

  /** Offset withing frag entry pointing to next frag entry. */
  static const ulint OFFSET_NEXT = OFFSET_PREV + FIL_ADDR_SIZE;

  /** Offset withing frag entry holding the page number of frag page. */
  static const ulint OFFSET_PAGE_NO = OFFSET_NEXT + FIL_ADDR_SIZE;

  /** Number of used fragments. */
  static const ulint OFFSET_N_FRAGS = OFFSET_PAGE_NO + 4;

  /** Used space in bytes. */
  static const ulint OFFSET_USED_LEN = OFFSET_N_FRAGS + 2;

  /** Total free space in bytes. */
  static const ulint OFFSET_TOTAL_FREE_LEN = OFFSET_USED_LEN + 2;

  /** The biggest free frag space in bytes. */
  static const ulint OFFSET_BIG_FREE_LEN = OFFSET_TOTAL_FREE_LEN + 2;

  /** Total size of one frag entry. */
  static const ulint SIZE = OFFSET_BIG_FREE_LEN + 2;

  /** Constructor. */
  z_frag_entry_t(flst_node_t *node) : m_node(node) {}

  /** Constructor. */
  z_frag_entry_t() : m_node(nullptr) {}

  /** Initialize */
  void init() {
    set_prev_null();
    set_next_null();
    set_page_no(FIL_NULL);
    set_n_frags(0);
    set_used_len(0);
    set_total_free_len(0);
    set_big_free_len(0);
  }

  fil_addr_t get_self_addr() const {
    page_t *frame = page_align(m_node);
    page_no_t page_no = mach_read_from_4(frame + FIL_PAGE_OFFSET);
    ulint offset = m_node - frame;
    ut_ad(offset < UNIV_PAGE_SIZE);
    return (fil_addr_t(page_no, offset));
  }

  void update(const z_frag_page_t &frag_page);

  /** Remove this node from the given list.
  @param[in]  bnode  the base node of the list from which to remove current
                     node. */
  void remove(flst_base_node_t *bnode) { flst_remove(bnode, m_node); }

  /** Add this node as the last node in the given list.
  @param[in]  bnode  the base node of the file list. */
  void push_back(flst_base_node_t *bnode) { flst_add_last(bnode, m_node); }

  /** Add this node as the first node in the given list.
  @param[in]  bnode  the base node of the file list. */
  void push_front(flst_base_node_t *bnode) { flst_add_first(bnode, m_node); }

  /** Point to another frag entry.
  @param[in]  node  point to this file list node. */
  void reset(flst_node_t *node) { m_node = node; }

  /** Set the previous frag entry as null. */
  void set_prev_null() { flst_write_addr(m_node + OFFSET_PREV, fil_addr_null); }

  /** Set the previous frag entry as null. */
  void set_prev(const fil_addr_t &addr) {
    flst_write_addr(m_node + OFFSET_PREV, addr);
  }

  /** Get the location of previous frag entry. */
  fil_addr_t get_prev() const { return (flst_read_addr(m_node + OFFSET_PREV)); }

  /** Set the next frag entry as null. */
  void set_next_null() { flst_write_addr(m_node + OFFSET_NEXT, fil_addr_null); }

  /** Set the next frag entry. */
  void set_next_null(const fil_addr_t &addr) {
    flst_write_addr(m_node + OFFSET_NEXT, addr);
  }

  /** Get the location of next frag entry. */
  fil_addr_t get_next() const { return (flst_read_addr(m_node + OFFSET_NEXT)); }

  /** Get the frag page number. */
  page_no_t get_page_no() const {
    return (mach_read_from_4(m_node + OFFSET_PAGE_NO));
  }

  /** Set the frag page number. */
  void set_page_no(page_no_t page_no) const {
    mlog_write_ulint(m_node + OFFSET_PAGE_NO, page_no, MLOG_4BYTES);
  }

  /** Get the frag page number. */
  ulint get_n_frags() const {
    return (mach_read_from_2(m_node + OFFSET_N_FRAGS));
  }

  /** Set the frag page number. */
  void set_n_frags(ulint frags) const {
    mlog_write_ulint(m_node + OFFSET_N_FRAGS, frags, MLOG_2BYTES);
  }

  /** Get the used bytes. */
  ulint get_used_len() const {
    return (mach_read_from_2(m_node + OFFSET_USED_LEN));
  }

  /** Set the used bytes. */
  void set_used_len(ulint used) const {
    mlog_write_ulint(m_node + OFFSET_USED_LEN, used, MLOG_2BYTES);
  }

  /** Get the total cumulative free bytes. */
  ulint get_total_free_len() const {
    return (mach_read_from_2(m_node + OFFSET_TOTAL_FREE_LEN));
  }

  /** Get the biggest free frag bytes. */
  ulint get_big_free_len() const {
    return (mach_read_from_2(m_node + OFFSET_BIG_FREE_LEN));
  }

  /** Set the total free bytes. */
  void set_total_free_len(ulint n) {
    mlog_write_ulint(m_node + OFFSET_TOTAL_FREE_LEN, n, MLOG_2BYTES);
  }

  /** Set the big free frag bytes. */
  void set_big_free_len(ulint n) {
    mlog_write_ulint(m_node + OFFSET_BIG_FREE_LEN, n, MLOG_2BYTES);
  }

  void purge(flst_base_node_t *used_lst, flst_base_node_t *free_lst);

  std::ostream &print(std::ostream &out) const;

 private:
  flst_node_t *m_node;
};

inline std::ostream &operator<<(std::ostream &out, const z_frag_entry_t &obj) {
  return (obj.print(out));
}

/** The first page of an zlob. */
struct z_first_page_t {
  /* The length of compressed data stored in this page. */
  static const ulint OFFSET_DATA_LEN = FIL_PAGE_DATA;

  /* The transaction that created data in the data portion. */
  static const ulint OFFSET_TRX_ID = FIL_PAGE_DATA + 4;

  /** The next index page. */
  static const ulint OFFSET_INDEX_PAGE_NO = OFFSET_TRX_ID + 6;

  /** The next frag nodes page. */
  static const ulint OFFSET_FRAG_NODES_PAGE_NO = OFFSET_INDEX_PAGE_NO + 4;

  /* List of free index entries. */
  static const ulint OFFSET_FREE_LIST = OFFSET_FRAG_NODES_PAGE_NO + 4;

  /* List of index entries. */
  static const ulint OFFSET_INDEX_LIST = OFFSET_FREE_LIST + FLST_BASE_NODE_SIZE;

  /* List of free frag entries. */
  static const ulint OFFSET_FREE_FRAG_LIST =
      OFFSET_INDEX_LIST + FLST_BASE_NODE_SIZE;

  /* List of frag entries. */
  static const ulint OFFSET_FRAG_LIST =
      OFFSET_FREE_FRAG_LIST + FLST_BASE_NODE_SIZE;

  /* Begin of index entries. */
  static const ulint OFFSET_INDEX_BEGIN =
      OFFSET_FRAG_LIST + FLST_BASE_NODE_SIZE;

  /** Given the page size, what is the number of index entries the first page
  can contain. */
  static ulint get_n_index_entries() {
    const ulint page_size = UNIV_PAGE_SIZE;
    switch (page_size) {
      case KB16:
        /* For a page size of 16KB, there are 100 index entries in the first
        page of the zlob. */
        return (100);
        break;
      default:
        ut_error;
    }
  }

  /** Given the page size, what is the number of frag entries the first page
  can contain. */
  static ulint get_n_frag_entries() {
    const ulint page_size = UNIV_PAGE_SIZE;
    switch (page_size) {
      case KB16:
        /* For a page size of 16KB, there are 200 frag entries in the first
        page of the zlob. */
        return (200);
        break;
      default:
        ut_error;
    }
  }

  static ulint size_of_index_entries() {
    return (z_index_entry_t::SIZE * get_n_index_entries());
  }

  static ulint size_of_frag_entries() {
    return (z_frag_entry_t::SIZE * get_n_frag_entries());
  }

  static ulint begin_frag_entries() {
    return (OFFSET_INDEX_BEGIN + size_of_index_entries());
  }

  static ulint begin_data() {
    return (begin_frag_entries() + size_of_frag_entries());
  }

  bool empty() const {
    flst_base_node_t *flst = index_list();
    return (flst_get_len(flst) == 0);
  }

  byte *begin_data_ptr() const { return (frame() + begin_data()); }

  /** Amount of zlob data that can be stored in first page (in bytes). */
  static ulint payload() {
    return (UNIV_PAGE_SIZE - begin_data() - FIL_PAGE_DATA_END);
  }

  z_first_page_t() : m_block(nullptr) {}
  z_first_page_t(buf_block_t *block) : m_block(block) {}

  buf_block_t *alloc() {
    m_block = btr_page_alloc();
    init();
    return (m_block);
  }

  buf_block_t *load(page_no_t page_no) {
    m_block = buf_page_get(page_no);
    return (m_block);
  }

  void dealloc() {
    ut_ad(empty());
    btr_page_free(m_block);
    m_block = nullptr;
  }

  void init() {
    set_page_type();
    set_data_len(0);
    set_trx_id(0);
    flst_base_node_t *flst = free_list();
    flst_init(flst);
    flst_base_node_t *ilst = index_list();
    flst_init(ilst);
    flst_base_node_t *free_frag_lst = free_frag_list();
    flst_init(free_frag_lst);
    flst_base_node_t *frag_lst = frag_list();
    flst_init(frag_lst);
    init_index_entries();
    init_frag_entries();
  }

  /** Get the amount of zlob data stored in this page. */
  ulint get_data_len() const {
    return (mach_read_from_4(frame() + OFFSET_DATA_LEN));
  }

  /** Get the page number. */
  ulint get_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_OFFSET));
  }

  fil_addr_t get_self_addr() const {
    page_no_t page_no = get_page_no();
    ulint offset = begin_data();
    return (fil_addr_t(page_no, offset));
  }

  /** All the index pages are singled linked with each other, and the first
  page contains the link to one index page.
  @param[in]  page_no  the page number of an index page. */
  void set_index_page_no(page_no_t page_no) {
    mlog_write_ulint(frame() + OFFSET_INDEX_PAGE_NO, page_no, MLOG_4BYTES);
  }

  /** All the index pages are singled linked with each other, and the first
  page contains the link to one index page. Get that index page number.
  @return the index page number. */
  page_no_t get_index_page_no() const {
    return (mach_read_from_4(frame() + OFFSET_INDEX_PAGE_NO));
  }

  /** All the frag node pages are singled linked with each other, and the
  first page contains the link to one frag node page.
  @param[in]  page_no  the page number of an frag node page. */
  void set_frag_node_page_no(page_no_t page_no) {
    mlog_write_ulint(frame() + OFFSET_FRAG_NODES_PAGE_NO, page_no, MLOG_4BYTES);
  }

  /** Free all the z_frag_page_t pages. All the z_frag_page_t pages are
  singly linked to each other.  The head of the list is maintained in the
  first page. */
  void free_all_frag_node_pages();

  /** Free all the index pages. */
  void free_all_index_pages();

  /** All the frag node pages are singled linked with each other, and the
  first page contains the link to one frag node page. Get that frag node
  page number.
  @return the index page number. */
  page_no_t get_frag_node_page_no() {
    return (mach_read_from_4(frame() + OFFSET_FRAG_NODES_PAGE_NO));
  }

  void set_page_type() {
    mlog_write_ulint(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_ZLOB_FIRST,
                     MLOG_2BYTES);
  }

  void set_data_len(ulint len) {
    mlog_write_ulint(frame() + OFFSET_DATA_LEN, len, MLOG_4BYTES);
  }

  void set_trx_id(trx_id_t tid) {
    mach_write_to_6(frame() + OFFSET_TRX_ID, tid);
  }

  flst_base_node_t *free_list() const { return (frame() + OFFSET_FREE_LIST); }

  flst_base_node_t *index_list() const { return (frame() + OFFSET_INDEX_LIST); }

  flst_base_node_t *free_frag_list() const {
    return (frame() + OFFSET_FREE_FRAG_LIST);
  }

  flst_base_node_t *frag_list() const { return (frame() + OFFSET_FRAG_LIST); }

  void init_frag_entries() {
    flst_base_node_t *free_frag_lst = free_frag_list();
    ulint n = get_n_frag_entries();
    for (ulint i = 0; i < n; ++i) {
      flst_node_t *ptr = frame() + begin_frag_entries();
      ptr += (i * z_frag_entry_t::SIZE);
      z_frag_entry_t frag_entry(ptr);
      frag_entry.init();
      frag_entry.push_back(free_frag_lst);
    }
  }

  void init_index_entries() {
    flst_base_node_t *flst = free_list();
    ulint n = get_n_index_entries();
    for (ulint i = 0; i < n; ++i) {
      flst_node_t *ptr = frame() + OFFSET_INDEX_BEGIN;
      ptr += (i * z_index_entry_t::SIZE);
      z_index_entry_t entry(ptr);
      entry.init();
      entry.push_back(flst);
    }
  }

  /** Find a fragment page, that has atleast len free space. */
  z_frag_entry_t find_frag_page(ulint len);

  /** Allocate one index entry.  If there is no free index entry, allocate
  an index page (a page full of z_index_entry_t objects) and service the
  request.
  @return the allocated index entry. */
  z_index_entry_t alloc_index_entry();

  /** Allocate one frag page entry.  If there is no free frag entry, allocate
  an frag node page (a page full of z_frag_entry_t objects) and service the
  request.
  @return the allocated frag entry. */
  z_frag_entry_t alloc_frag_entry();

  /** Print the index entries. */
  std::ostream &print_index_entries(std::ostream &out) const;

  /** Print the index entries. */
  std::ostream &print_frag_entries(std::ostream &out) const;

  /** Print the page. */
  std::ostream &print(std::ostream &out) const;

  byte *frame() const { return (buf_block_get_frame(m_block)); }

 private:
  buf_block_t *m_block;
};  // struct z_first_page_t

inline std::ostream &operator<<(std::ostream &out, const z_first_page_t &obj) {
  return (obj.print(out));
}

/** An index page containing an array of z_index_entry_t objects. */
struct z_index_page_t {
  /** Set the correct page type. */
  void set_page_type() {
    mlog_write_ulint(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_ZLOB_INDEX,
                     MLOG_2BYTES);
  }

  /** Set the next page number. */
  void set_next_page_no(page_no_t page_no) {
    mlog_write_ulint(frame() + FIL_PAGE_NEXT, page_no, MLOG_4BYTES);
  }

  /** Get the page number. */
  ulint get_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_OFFSET));
  }

  /** Get the next page number. */
  ulint get_next_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_NEXT));
  }

  buf_block_t *alloc(z_first_page_t &first) {
    m_block = btr_page_alloc();
    set_page_type();
    flst_base_node_t *free_lst = first.free_list();
    init(free_lst);

    /* Link the allocated index page to the first page. */
    page_no_t page_no = first.get_index_page_no();
    set_next_page_no(page_no);
    first.set_index_page_no(get_page_no());
    return (m_block);
  }

  buf_block_t *load(page_no_t page_no) {
    m_block = buf_page_get(page_no);
    return (m_block);
  }

  void dealloc() {
    btr_page_free(m_block);
    m_block = nullptr;
  }

  void init(flst_base_node_t *free_lst) {
    ulint n = get_n_index_entries();
    for (ulint i = 0; i < n; ++i) {
      byte *ptr = frame() + FIL_PAGE_DATA;
      ptr += (i * z_index_entry_t::SIZE);
      z_index_entry_t entry(ptr);
      entry.init();
      entry.push_back(free_lst);
    }
  }

  ulint payload() const {
    return (UNIV_PAGE_SIZE - FIL_PAGE_DATA_END - FIL_PAGE_DATA);
  }

  ulint get_n_index_entries() const {
    return (payload() / z_index_entry_t::SIZE);
  }

  byte *frame() const { return (buf_block_get_frame(m_block)); }

  buf_block_t *m_block;
};

/** The data page holding the zlob. */
struct z_data_page_t {
  /* The length of compressed data stored in this page. */
  static const ulint OFFSET_DATA_LEN = FIL_PAGE_DATA;

  /* The transaction that created this page. */
  static const ulint OFFSET_TRX_ID = OFFSET_DATA_LEN + 4;

  /* The data stored in this page begins at this offset. */
  static const ulint OFFSET_DATA_BEGIN = OFFSET_TRX_ID + 6;

  static ulint payload() {
    return (UNIV_PAGE_SIZE - OFFSET_DATA_BEGIN - FIL_PAGE_DATA_END);
  }

  z_data_page_t() : m_block(nullptr) {}
  z_data_page_t(buf_block_t *block) : m_block(block) {}

  buf_block_t *alloc() {
    m_block = btr_page_alloc();
    init();
    return (m_block);
  }

  /** Set the correct page type. */
  void set_page_type() {
    mlog_write_ulint(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_ZLOB_DATA,
                     MLOG_2BYTES);
  }

  /** Set the next page. */
  void set_next_page(page_no_t page_no) {
    mlog_write_ulint(frame() + FIL_PAGE_NEXT, page_no, MLOG_4BYTES);
  }

  void init() {
    set_page_type();
    set_next_page(FIL_NULL);
    set_data_len(0);
    set_trx_id(0);
  }

  byte *begin_data_ptr() const { return (frame() + OFFSET_DATA_BEGIN); }

  void set_data_len(ulint len) {
    mlog_write_ulint(frame() + OFFSET_DATA_LEN, len, MLOG_4BYTES);
  }

  ulint get_data_len() const {
    return (mach_read_from_4(frame() + OFFSET_DATA_LEN));
  }

  void set_trx_id(trx_id_t tid) {
    mach_write_to_6(frame() + OFFSET_TRX_ID, tid);
  }

  /** Get the page number. */
  page_no_t get_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_OFFSET));
  }

  fil_addr_t get_self_addr() const {
    page_no_t page_no = get_page_no();
    return (fil_addr_t(page_no, OFFSET_DATA_BEGIN));
  }

  byte *frame() const { return (buf_block_get_frame(m_block)); }

  buf_block_t *m_block;
};

/** A frag nodes page containing an array of z_frag_entry_t objects. */
struct z_frag_node_page_t {
  /** Set the correct page type. */
  void set_page_type() {
    mlog_write_ulint(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_ZLOB_FRAG_ENTRY,
                     MLOG_2BYTES);
  }

  /** Set the next page number. */
  void set_next_page_no(page_no_t page_no) {
    mlog_write_ulint(frame() + FIL_PAGE_NEXT, page_no, MLOG_4BYTES);
  }

  /** Get the page number. */
  ulint get_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_OFFSET));
  }

  /** Get the next page number. */
  ulint get_next_page_no() const {
    return (mach_read_from_4(frame() + FIL_PAGE_NEXT));
  }

  buf_block_t *alloc(z_first_page_t &first) {
    m_block = btr_page_alloc();
    set_page_type();
    flst_base_node_t *free_lst = first.free_frag_list();
    init(free_lst);

    /* Link the allocated index page to the first page. */
    page_no_t page_no = first.get_frag_node_page_no();
    set_next_page_no(page_no);
    first.set_frag_node_page_no(get_page_no());
    return (m_block);
  }

  void dealloc() {
    btr_page_free(m_block);
    m_block = nullptr;
  }

  buf_block_t *load(page_no_t page_no) {
    m_block = buf_page_get(page_no);
    return (m_block);
  }

  void init(flst_base_node_t *free_lst) {
    ulint n = get_n_frag_entries();
    for (ulint i = 0; i < n; ++i) {
      byte *ptr = frame() + FIL_PAGE_DATA;
      ptr += (i * z_frag_entry_t::SIZE);
      z_frag_entry_t entry(ptr);
      entry.init();
      entry.push_back(free_lst);
    }
  }

  ulint payload() const {
    return (UNIV_PAGE_SIZE - FIL_PAGE_DATA_END - FIL_PAGE_DATA);
  }

  ulint get_n_frag_entries() const {
    return (payload() / z_frag_entry_t::SIZE);
  }

  byte *frame() const { return (buf_block_get_frame(m_block)); }

  buf_block_t *m_block;
};  // struct z_frag_node_page_t

/** Insert a large object (LOB) into the system.
@param[in,out]  ref  the LOB reference.
@param[in]  blob  the large object.
@param[in]  len  the length of the large object.*/
dberr_t z_insert(trx_id_t trxid, lob::ref_t ref, byte *blob, ulint len);

/** Print information about the given compressed lob. */
dberr_t z_print_info(lob::ref_t ref, std::ostream &out);

/** Fetch a compressed large object (ZLOB) from the system.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset read the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be fetched.
@param[out] buf  the output buffer (owned by caller) of minimum len bytes.
@return the amount of data (in bytes) that was actually read. */
ulint z_read(trx_id_t trxid, lob::ref_t ref, ulint offset, ulint len,
             byte *buf);

/** Replace a large object (LOB) with the given new data.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset replace the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be replaced.
@param[in] buf  the buffer (owned by caller) with new data (len bytes).
@return the actual number of bytes replaced. */
ulint z_replace(trx_id_t trxid, lob::ref_t ref, ulint offset, ulint len,
                byte *buf);

/** Insert data into the middle of an LOB */
ulint z_insert_middle(trx_id_t trxid, lob::ref_t ref, ulint offset, byte *data,
                      ulint len);

/** Delete a portion of the given large object (LOB)
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset remove the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be removed.
@return actual number of bytes removed. */
ulint z_remove_middle(trx_id_t trxid, lob::ref_t ref, ulint offset, ulint len);

void z_purge(trx_id_t trxid, lob::ref_t ref);

struct frag_node_t {
  static const ulint OFFSET_LEN = plist_node_t::SIZE;
  static const ulint OFFSET_FRAG_ID = OFFSET_LEN + 2;
  static const ulint OFFSET_DATA = OFFSET_FRAG_ID + 2;

  frag_node_t(flst_node_t *node) : m_node(page_align(node), node) {}

  frag_node_t() : m_node() {}
  frag_node_t(const plist_node_t &node) : m_node(node) {}

  frag_node_t(byte *frame, byte *ptr) : m_node(frame, ptr) {}

  frag_node_t(byte *frame, byte *ptr, ulint len) : m_node(frame, ptr) {
    mlog_write_ulint(m_node.ptr() + OFFSET_LEN, len, MLOG_2BYTES);
  }

  byte *frag_begin() const { return (m_node.ptr() + OFFSET_DATA); }
  byte *data_begin() const { return (m_node.ptr() + OFFSET_DATA); }

  void set_total_len(ulint len) {
    mlog_write_ulint(m_node.ptr() + OFFSET_LEN, len, MLOG_2BYTES);
  }

  /** Increment the total length of this fragment by 2 bytes. */
  void incr_length_by_2() {
    ulint len = mach_read_from_2(m_node.ptr() + OFFSET_LEN);
    mlog_write_ulint(m_node.ptr() + OFFSET_LEN, len + 2, MLOG_2BYTES);
  }

  /** Decrement the total length of this fragment by 2 bytes. */
  void decr_length_by_2() {
    ulint len = mach_read_from_2(m_node.ptr() + OFFSET_LEN);
    mlog_write_ulint(m_node.ptr() + OFFSET_LEN, len - 2, MLOG_2BYTES);
  }

  bool is_before(const frag_node_t &frag) const {
    return (m_node.is_before(frag.m_node));
  }

  void set_frag_id_null() {
    mlog_write_ulint(m_node.ptr() + OFFSET_FRAG_ID, FRAG_ID_NULL, MLOG_2BYTES);
  }

  void set_frag_id(ulint id) {
    mlog_write_ulint(m_node.ptr() + OFFSET_FRAG_ID, id, MLOG_2BYTES);
  }

  ulint get_frag_id() const {
    return (mach_read_from_2(m_node.ptr() + OFFSET_FRAG_ID));
  }

  /** Get the space available in this fragment for storing data. */
  ulint payload() const { return (get_total_len() - OFFSET_DATA); }

  /** Get the total length of this fragment, including its metadata. */
  ulint get_total_len() const {
    return (mach_read_from_2(m_node.ptr() + OFFSET_LEN));
  }

  /** Get the offset of the current fragment within page.
  @return the offset of the current fragment within. */
  paddr_t addr() const { return m_node.addr(); }

  /** Gets the pointer to the beginning of the current fragment.  Note that
  the beginning of the fragment contains meta data.
  @return pointer to the beginning of the current fragment. */
  byte *ptr() const {
    ut_ad(!m_node.is_null());
    return (m_node.ptr());
  }

  /** Gets the pointer just after the current fragment.  The pointer returned
  does not belong to this fragment.  This is used to check adjacency.
  @return pointer to the end of the current fragment. */
  byte *end_ptr() const {
    ut_ad(!m_node.is_null());
    return (ptr() + get_total_len());
  }

  byte *frame() const { return (m_node.m_frame); }

  std::ostream &print(std::ostream &out) const {
    if (!m_node.is_null()) {
      ulint len = get_total_len();
      out << "[frag_node_t: " << m_node << ", len=" << len << "/" << payload()
          << ", frag_id=" << get_frag_id() << "]";
    } else {
      out << "[frag_node_t: null, len=0]";
    }
    return (out);
  }

  frag_node_t get_next_frag() {
    ut_ad(!is_null());
    plist_node_t next = m_node.get_next_node();
    return (frag_node_t(next));
  }

  frag_node_t get_next_node() { return (get_next_frag()); }
  frag_node_t get_prev_node() { return (get_prev_frag()); }

  frag_node_t get_prev_frag() {
    ut_ad(!is_null());
    plist_node_t prev = m_node.get_prev_node();
    return (frag_node_t(prev));
  }

  bool merge(frag_node_t &next) {
    byte *p1 = ptr();
    ulint len1 = get_total_len();
    byte *p2 = next.ptr();
    ulint len2 = next.get_total_len();

    if (p2 == (p1 + len1)) {
      set_total_len(len1 + len2);
      return (true);
    }
    return (false);
  }

  bool is_null() const { return (m_node.is_null()); }

  plist_node_t m_node;
};

inline std::ostream &operator<<(std::ostream &out, const frag_node_t &obj) {
  return (obj.print(out));
}

/** The fragment page.  This page will contain fragments from different
zlib streams. */
struct z_frag_page_t {
  /** The location of z_frag_entry_t for this page. */
  static const ulint OFFSET_FRAG_ENTRY = FIL_PAGE_DATA;

  /** The offset within page where the free space list begins. */
  static const ulint OFFSET_FREE_LIST = OFFSET_FRAG_ENTRY + FIL_ADDR_SIZE;

  /** The offset within page where the fragment list begins. */
  static const ulint OFFSET_FRAGS_LIST =
      OFFSET_FREE_LIST + plist_base_node_t::SIZE;

  /** The offset within page where the fragments can occupy . */
  static const ulint OFFSET_FRAGS_BEGIN =
      OFFSET_FRAGS_LIST + plist_base_node_t::SIZE;

  /** Offset of number of page directory entries (from end) */
  static const ulint OFFSET_PAGE_DIR_ENTRY_COUNT = FIL_PAGE_DATA_END + 2;

  /** Offset of first page directory entry (from end) */
  static const ulint OFFSET_PAGE_DIR_ENTRY_FIRST =
      OFFSET_PAGE_DIR_ENTRY_COUNT + 2;

  static const ulint SIZE_OF_PAGE_DIR_ENTRY = 2; /* bytes */

  z_frag_page_t() : m_block(nullptr) {}
  z_frag_page_t(buf_block_t *block) : m_block(block) {}

  z_frag_entry_t get_frag_entry();

  void update_frag_entry() {
    z_frag_entry_t entry = get_frag_entry();
    entry.update(*this);
  }

  void set_frag_entry(const fil_addr_t &addr) const {
    ut_ad(addr.boffset < UNIV_PAGE_SIZE);
    return (flst_write_addr(frame() + OFFSET_FRAG_ENTRY, addr));
  }

  void set_frag_entry_null() const {
    return (flst_write_addr(frame() + OFFSET_FRAG_ENTRY, fil_addr_null));
  }

  ulint get_n_dir_entries() const {
    byte *ptr = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_COUNT;
    return (mach_read_from_2(ptr));
  }

  void set_n_dir_entries(ulint n) const {
    byte *ptr = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_COUNT;
    mlog_write_ulint(ptr, n, MLOG_2BYTES);
  }

  bool is_border_frag(const frag_node_t &node) const {
    return (slots_end_ptr() == node.end_ptr());
  }

  byte *slots_end_ptr() const {
    ulint n = get_n_dir_entries();
    byte *first = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_COUNT;
    byte *ptr = first - (n * SIZE_OF_PAGE_DIR_ENTRY);
    return (ptr);
  }

  paddr_t frag_id_to_addr(ulint frag_id) const {
    byte *first = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_FIRST;
    byte *ptr = first - (frag_id * SIZE_OF_PAGE_DIR_ENTRY);
    return (mach_read_from_2(ptr));
  }

  ulint get_nth_dir_entry(ulint frag_id) {
    byte *first = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_FIRST;
    byte *ptr = first - (frag_id * SIZE_OF_PAGE_DIR_ENTRY);
    return (mach_read_from_2(ptr));
  }

  void set_nth_dir_entry(ulint frag_id, paddr_t val) {
    byte *first = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_FIRST;
    byte *ptr = first - (frag_id * SIZE_OF_PAGE_DIR_ENTRY);
    mlog_write_ulint(ptr, val, MLOG_2BYTES);
  }

  ulint init_last_dir_entry() {
    ulint n = get_n_dir_entries();
    set_nth_dir_entry(n - 1, 0);
    return (n - 1);
  }

  void incr_n_dir_entries() const {
    byte *ptr = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_COUNT;
    ulint n = mach_read_from_2(ptr);
    ut_a(n < FRAG_ID_NULL);
    mlog_write_ulint(ptr, n + 1, MLOG_2BYTES);
  }

  void decr_n_dir_entries() const {
    byte *ptr = frame() + UNIV_PAGE_SIZE - OFFSET_PAGE_DIR_ENTRY_COUNT;
    ulint n = mach_read_from_2(ptr);
    ut_a(n > 0);
    mlog_write_ulint(ptr, n - 1, MLOG_2BYTES);
  }

  ulint space_used_by_dir() const {
    ulint n = get_n_dir_entries();
    return (n * SIZE_OF_PAGE_DIR_ENTRY);
  }

  ulint locate_free_slot() {
    ulint n = get_n_dir_entries();
    for (ulint frag_id = 0; frag_id < n; frag_id++) {
      ulint paddr = get_nth_dir_entry(frag_id);
      if (paddr == 0) {
        return (frag_id);
      }
    }

    return (FRAG_ID_NULL);
  }

  /** Allocate a fragment id.
  @return On success, return fragment id.
  @return On failure, return FRAG_ID_NULL. */
  ulint alloc_frag_id() {
    ulint id = locate_free_slot();
    if (id == FRAG_ID_NULL) {
      return (alloc_dir_entry());
    }
    return (id);
  }

  std::ostream &print_frag_id(std::ostream &out) {
    ulint n = get_n_dir_entries();
    out << "FRAG IDS: " << std::endl;
    for (ulint frag_id = 0; frag_id < n; frag_id++) {
      out << "id=" << frag_id << ", addr=" << frag_id_to_addr(frag_id)
          << std::endl;
    }
    return (out);
  }

  /** Grow the frag directory by one entry.
  @return the fragment identifier that was newly added. */
  ulint alloc_dir_entry() {
    plist_base_node_t free_lst = free_list();
    plist_node_t last = free_lst.get_last_node();
    frag_node_t frag(last);
    ulint len = frag.payload();

    /* The last free fragment must be adjacent to the directory.
    Then only it can give space to one slot. */
    if (frag.end_ptr() != slots_end_ptr()) {
      return (FRAG_ID_NULL);
    }

    if (len <= SIZE_OF_PAGE_DIR_ENTRY) {
      return (FRAG_ID_NULL);
    }

    incr_n_dir_entries();
    frag.decr_length_by_2();
    return (init_last_dir_entry());
  }

  /** Set the next page. */
  void set_page_next(page_no_t page_no) {
    mlog_write_ulint(frame() + FIL_PAGE_NEXT, page_no, MLOG_4BYTES);
  }

  /** Allocate the fragment page.
  @return the allocated buffer block. */
  buf_block_t *alloc();

  /** Free the fragment page along with its entry. */
  void dealloc(z_first_page_t &first);

  buf_block_t *load(page_no_t page_no) {
    m_block = buf_page_get(page_no);
    return (m_block);
  }

  void merge_free_frags() {
    plist_base_node_t free_lst = free_list();
    frag_node_t frag(free_lst.get_first_node());
    frag_node_t next = frag.get_next_frag();

    while (!next.is_null() && frag.merge(next)) {
      free_lst.remove(next.m_node);
      next = frag.get_next_frag();
    }
  }

  void merge_free_frags(frag_node_t frag) {
    ut_ad(!frag.is_null());
    plist_base_node_t free_lst = free_list();
    frag_node_t next = frag.get_next_frag();

    while (!next.is_null() && frag.merge(next)) {
      free_lst.remove(next.m_node);
      next = frag.get_next_frag();
    }
  }

  void insert_into_free_list(frag_node_t &frag) {
    ut_ad(frag.get_frag_id() == FRAG_ID_NULL);

    plist_base_node_t free_lst = free_list();

    plist_node_t node = free_lst.get_first_node();
    plist_node_t prev_node;

    while (!node.is_null()) {
      if (frag.addr() < node.addr()) {
        break;
      }
      prev_node = node;
      node = node.get_next_node();
    }

    free_lst.insert_before(node, frag.m_node);
    if (prev_node.is_null()) {
      merge_free_frags();
    } else {
      merge_free_frags(prev_node);
    }
  }

  void insert_into_frag_list(frag_node_t &frag) {
    plist_base_node_t frag_lst = frag_list();
    plist_node_t node = frag_lst.get_first_node();

    while (!node.is_null()) {
      ut_ad(frag.addr() != node.addr());
      if (frag.addr() < node.addr()) {
        break;
      }
      node = node.get_next_node();
    }

    frag_lst.insert_before(node, frag.m_node);
  }

  /** Split one free fragment into two.
   @param[in]  free_frag  the free fragment that will be split.
   @param[in]  size  the payload size in bytes. */
  void split_free_frag(frag_node_t &free_frag, ulint size) {
    ut_ad(size < free_frag.payload());
    const ulint old_total_len = free_frag.get_total_len();
    plist_base_node_t free_lst = free_list();

    byte *p1 = free_frag.ptr();
    byte *p2 = p1 + frag_node_t::OFFSET_DATA + size;
    ulint remain = free_frag.get_total_len() - frag_node_t::OFFSET_DATA - size;
    ut_a(remain >= frag_node_t::OFFSET_DATA);
    free_frag.set_total_len(frag_node_t::OFFSET_DATA + size);
    frag_node_t frag2(frame(), p2, remain);
    frag2.set_total_len(remain);
    frag2.set_frag_id_null();
    free_lst.insert_after(free_frag.m_node, frag2.m_node);

    ut_a(free_frag.get_total_len() + frag2.get_total_len() == old_total_len);
  }

  frag_node_t get_frag_node(frag_id_t id) const {
    paddr_t off = frag_id_to_addr(id);
    byte *f = frame();
    return (frag_node_t(f, f + off));
  }

  void dealloc_fragment(ulint frag_id) {
    paddr_t off = frag_id_to_addr(frag_id);
    byte *f = frame();
    frag_node_t frag(f, f + off);
    dealloc_fragment(frag);
    dealloc_frag_id(frag_id);

    /* Update the index entry. */
    update_frag_entry();
  }

  /** Allocate a fragment with the given payload.
  @return the frag_id of the allocated fragment. */
  frag_id_t alloc_fragment(ulint size) {
    ulint frag_id = alloc_frag_id();

    if (frag_id == FRAG_ID_NULL) {
      return (FRAG_ID_NULL);
    }

    plist_base_node_t free_lst = free_list();

    for (plist_node_t cur = free_lst.get_first_node(); !cur.is_null();
         cur = cur.get_next_node()) {
      frag_node_t frag(cur);

      if (frag.payload() == size) {
        // this is the requested fragment.
        free_lst.remove(cur);
        insert_into_frag_list(frag);

        frag.set_frag_id(frag_id);
        set_nth_dir_entry(frag_id, frag.addr());
        return (frag_id);
      } else if (frag.payload() >= (size + frag_node_t::OFFSET_DATA + 2)) {
        // break the current fragment into two.
        split_free_frag(frag, size);
        free_lst.remove(frag.m_node);
        insert_into_frag_list(frag);
        frag.set_frag_id(frag_id);
        set_nth_dir_entry(frag_id, frag.addr());
        return (frag_id);
      }
    }
    return (FRAG_ID_NULL);
  }

  plist_base_node_t free_list() const {
    byte *f = frame();
    return (plist_base_node_t(f, f + OFFSET_FREE_LIST));
  }

  plist_base_node_t frag_list() const {
    byte *f = frame();
    return (plist_base_node_t(f, f + OFFSET_FRAGS_LIST));
  }

  void set_page_type() {
    mach_write_to_2(frame() + FIL_PAGE_TYPE, FIL_PAGE_TYPE_ZLOB_FRAG);
  }

  ulint get_page_type() const {
    return (mach_read_from_2(frame() + FIL_PAGE_TYPE));
  }

  const char *get_page_type_str() const {
    ulint type = get_page_type();
    ut_a(type == FIL_PAGE_TYPE_ZLOB_FRAG);
    return ("FIL_PAGE_TYPE_ZLOB_FRAG");
  }

  static ulint payload() {
    return (UNIV_PAGE_SIZE - OFFSET_FRAGS_BEGIN - OFFSET_PAGE_DIR_ENTRY_COUNT);
  }

  /** The maximum amount of data that can be stored in a fragment page. */
  static ulint max_payload() {
    return (UNIV_PAGE_SIZE - OFFSET_FRAGS_BEGIN - OFFSET_PAGE_DIR_ENTRY_COUNT -
            SIZE_OF_PAGE_DIR_ENTRY - frag_node_t::OFFSET_DATA);
  }

  frag_node_t find_frag(ulint size) {
    plist_base_node_t free_lst = free_list();
    for (plist_node_t cur = free_lst.get_first_node(); !cur.is_null();
         cur = cur.get_next_node()) {
      frag_node_t frag(cur);
      if (frag.payload() >= size) {
        return (frag);
      }
    }
    return (frag_node_t());
  }

  /** Get the frag page number. */
  page_no_t get_page_no() const { return (m_block->get_page_no()); }

  byte *frame() const { return (buf_block_get_frame(m_block)); }

  std::ostream &print(std::ostream &out) const {
    print_free_list(out);
    print_frag_list(out);
    print_frags_in_order(out);
    print_page_dir(out);
    return (out);
  }

  /** Get the total amount of stored data in this page. */
  ulint get_total_stored_data() const;

  /** Get the total cumulative free space in this page. */
  ulint get_total_free_len() const;

  /** Get the big free space in this page. */
  ulint get_big_free_len() const;

  /** Get the number of fragments in this frag page. */
  ulint get_n_frags() const {
    plist_base_node_t frag_lst = frag_list();
    return (frag_lst.get_len());
  }

  std::ostream &print_frags_in_order(std::ostream &out) const;

  std::ostream &print_free_list(std::ostream &out) const {
    if (m_block == nullptr) {
      return (out);
    }
    plist_base_node_t free_lst = free_list();
    out << "[Free List: " << free_lst << "]" << std::endl;
    for (plist_node_t cur = free_lst.get_first_node(); !cur.is_null();
         cur = cur.get_next_node()) {
      frag_node_t frag(cur);
      out << frag << std::endl;
    }
    return (out);
  }

  std::ostream &print_frag_list(std::ostream &out) const {
    if (m_block == nullptr) {
      return (out);
    }
    plist_base_node_t frag_lst = frag_list();
    out << "[Frag List: " << frag_lst << "]" << std::endl;
    for (plist_node_t cur = frag_lst.get_first_node(); !cur.is_null();
         cur = cur.get_next_node()) {
      frag_node_t frag(cur);
      out << frag << std::endl;
    }
    return (out);
  }

  std::ostream &print_page_dir(std::ostream &out) const {
    if (m_block == nullptr) {
      return (out);
    }

    ulint n = get_n_dir_entries();

    for (ulint frag_id = 0; frag_id < n; ++frag_id) {
      paddr_t off = frag_id_to_addr(frag_id);
      out << "[frag_id=" << frag_id << ", addr=" << off << "]" << std::endl;
    }

    return (out);
  }

 private:
  fil_addr_t get_frag_entry_addr() const {
    return (flst_read_addr(frame() + OFFSET_FRAG_ENTRY));
  }

  void dealloc_fragment(frag_node_t frag) {
    plist_base_node_t frag_lst = frag_list();
    frag_lst.remove(frag.m_node);
    frag.set_frag_id_null();
    insert_into_free_list(frag);
  }

  /** Deallocate all the free slots from the end of the page directory. */
  void dealloc_frag_id();

  /** Deallocate the given fragment id.
  @param[in] frag_id The fragment that needs to be deallocated. */
  void dealloc_frag_id(ulint frag_id) {
    set_nth_dir_entry(frag_id, 0);
    dealloc_frag_id();
  }

  buf_block_t *m_block;
};

}  // namespace zlob

#endif  // _zlob0int_h_
