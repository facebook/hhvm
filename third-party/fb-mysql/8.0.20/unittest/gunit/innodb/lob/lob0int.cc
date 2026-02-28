/*****************************************************************************

Copyright (c) 2016, 2020, Oracle and/or its affiliates. All Rights Reserved.

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

#include "lob0int.h"

#ifdef UNIV_DEBUG
#define Fname(x) const char *fname = x;
#define LOG(x) \
  { std::cout << fname << ":" << x << std::endl; }
#else
#define Fname(x)
#define LOG(x)
#endif /* UNIV_DEBUG */

namespace lob {

static buf_block_t *create_first_page() {
  Fname("lob::create_first_page");

  /* Allocate the page. */
  buf_block_t *block = btr_page_alloc();
  base_node_page_t page(block);

  byte *free_list = page.free_list();
  byte *index_list = page.index_list();
  flst_init(index_list);
  flst_init(free_list);

  ulint node_count = page.node_count();
  LOG("Number of LOB index entries = " << node_count);

  byte *cur = page.nodes_begin();
  for (ulint i = 0; i < node_count; ++i) {
    flst_add_last(free_list, cur);
    cur += index_entry_t::SIZE;
  }
  ut_a(flst_validate(free_list));

  return (block);
}

buf_block_t *base_node_page_t::alloc() {
  ut_ad(m_block == nullptr);
  m_block = create_first_page();
  set_page_type();
  set_next_page_null();
  return (m_block);
}

/** Insert data in the middle of the given LOB page.
@param[in] lob_page  A large object page of type FIL_PAGE_TYPE_LOB_FIRST or
                     FIL_PAGE_TYPE_LOB_DATA.
@param[in] trxid     The transaction identifier.
@param[in] offset    The given data must be inserted at this offset within the
                     given page.
@param[in] data      The new data that needs to be inserted in the middle.
@param[in] len       The length of the new data in bytes.
@param[out] new_block The new version of the page. We do copy-on-write.
@return Saved data and its length. Some data from the current page that needs
        to be inserted at the end of the given new data. */
template <typename T>
std::pair<ulint, byte *> insert_middle_page(T *lob_page, trx_id_t trxid,
                                            ulint offset, byte *&data,
                                            ulint &len,
                                            buf_block_t *&new_block);

/** Insert data in the middle of the given LOB page.
@param[in] trxid     The transaction identifier.
@param[in] offset    The given data must be inserted at this offset within the
                     given page.
@param[in] data      The new data that needs to be inserted in the middle.
@param[in] len       The length of the new data in bytes.
@param[out] new_block The new version of the page. We do copy-on-write.
@return Saved data and its length. Some data from the current page that needs
        to be inserted at the end of the given new data. */
std::pair<ulint, byte *> base_node_page_t::insert_middle(
    trx_id_t trxid, ulint offset, byte *&data, ulint &len,
    buf_block_t *&new_block) {
  ut_ad(trxid >= get_trx_id());

  return (insert_middle_page(this, trxid, offset, data, len, new_block));
}

/** Insert data in the middle of the given LOB page.
@param[in] trxid     The transaction identifier.
@param[in] offset    The given data must be inserted at this offset within the
                     given page.
@param[in] data      The new data that needs to be inserted in the middle.
@param[in] len       The length of the new data in bytes.
@param[out] new_block The new version of the page. We do copy-on-write.
@return Saved data and its length. Some data from the current page that needs
        to be inserted at the end of the given new data. */
std::pair<ulint, byte *> data_page_t::insert_middle(trx_id_t trxid,
                                                    ulint offset, byte *&data,
                                                    ulint &len,
                                                    buf_block_t *&new_block) {
  ut_ad(trxid >= get_trx_id());

  return (insert_middle_page(this, trxid, offset, data, len, new_block));
}

/** Insert data in the middle of an LOB.
@param[in] lob_page  A large object page of type FIL_PAGE_TYPE_LOB_FIRST or
                     FIL_PAGE_TYPE_LOB_DATA.
@param[in] trxid     The transaction identifier.
@param[in] offset    The given data must be inserted at this offset.
@param[in] data      The new data that needs to be inserted in the middle.
@param[in] len       The length of the new data in bytes.
@param[out] new_block The new version of the page. We do copy-on-write.
@return Saved data and its length. Some data from the current page that needs
to be inserted at the end of the given new data. */
template <typename T>
std::pair<ulint, byte *> insert_middle_page(T *lob_page, trx_id_t trxid,
                                            ulint offset, byte *&data,
                                            ulint &len,
                                            buf_block_t *&new_block) {
  Fname("insert_middle_page");
  ut_ad(trxid >= lob_page->get_trx_id());
  std::pair<ulint, byte *> result(0, nullptr);

  ulint data_len = lob_page->get_data_len();
  ulint save_len = data_len - offset;

  /* Don't modify existing page.  Create new page. */
  data_page_t page;
  new_block = page.alloc();
  ulint avail = page.max_space_available();

  byte *old_page_begin = lob_page->data_begin();
  byte *new_page_begin = page.data_begin();

  /* Copy the old data in the beginning. */
  memcpy(new_page_begin, old_page_begin, offset);

  if (data_len + len <= avail) {
    /* Insert will complete within this page. */
    /* Copy new data in middle. */
    memcpy(new_page_begin + offset, data, len);
    /* Copy remaining old data in end. */
    memcpy(new_page_begin + offset + len, old_page_begin + offset, save_len);
    ut_a((data_len + len) == (offset + len + save_len));
    page.set_data_len(data_len + len);
    data += len;
    len = 0;

  } else {
    /* Will span multiple pages. */
    /* Save the existing data that will be over-written. */
    LOG("spanning multiple pages");

    byte *from = lob_page->data_begin() + offset;
    byte *save_mem = new byte[save_len];
    memcpy(save_mem, from, save_len);
    page.set_data_len(offset);

    /* Over-write the existing data with new data. */
    page.append(trxid, data, len);

    /* There maybe some space left. */
    result = std::pair<ulint, byte *>(save_len, save_mem);
  }
  return (result);
}

template <typename PageType>
ulint append_page(PageType *page, trx_id_t trxid, byte *&data, ulint &len) {
  Fname("append_page");

  LOG("Need to append bytes: " << len);

  ulint old_data_len = page->get_data_len();
  byte *ptr = page->data_begin() + old_data_len;
  ulint space_available = page->max_space_available() - old_data_len;

  if (space_available == 0 || len == 0) {
    return (0);
  }

  ulint written = (len > space_available) ? space_available : len;

  memcpy(ptr, data, written);
  page->set_data_len(old_data_len + written);
  page->set_trx_id(trxid);

  data += written;
  len -= written;

  LOG("Written " << written << " bytes into page_no=" << page->get_page_no());
  LOG("remaining=" << len);
  return (written);
}

ulint base_node_page_t::append(trx_id_t trxid, byte *&data, ulint &len) {
  return (append_page(this, trxid, data, len));
}

ulint data_page_t::append(trx_id_t trxid, byte *&data, ulint &len) {
  return (append_page(this, trxid, data, len));
}

template <typename PageType>
buf_block_t *remove_middle_page(PageType *page, trx_id_t trxid, ulint offset,
                                ulint &len) {
  buf_block_t *new_block = nullptr;

  /* Total data available in the given page. */
  ulint data_len = page->get_data_len();

  /* Data that can be removed from the given page. */
  ulint can_be_removed = data_len - offset;

  /* Don't modify existing page.  Create new page. */
  data_page_t new_page;
  new_block = new_page.alloc();
  byte *to = new_page.data_begin();
  byte *from = page->data_begin();

  /* Copy the beginning portion. */
  if (offset > 0) {
    memcpy(to, from, offset);
  }

  if (len < can_be_removed) {
    /* Only a single page modification. */
    to += offset;
    from += (offset + len);
    ulint trail = data_len - offset - len;

    /* Copy the end portion. */
    memcpy(to, from, trail);

    new_page.set_data_len(data_len - len);
    len = 0;
  } else {
    new_page.set_data_len(data_len - can_be_removed);
    len -= can_be_removed;
  }

  return (new_block);
}

buf_block_t *base_node_page_t::remove_middle(trx_id_t trxid, ulint offset,
                                             ulint &len) {
  return (remove_middle_page(this, trxid, offset, len));
}

buf_block_t *data_page_t::remove_middle(trx_id_t trxid, ulint offset,
                                        ulint &len) {
  return (remove_middle_page(this, trxid, offset, len));
}

/** The current index entry points to a latest LOB page.  It may or may
not have older versions.  If older version is there, bring it back to the
index list from the versions list.  Then remove the current entry from
the index list.  Move the versions list from current entry to older entry.
@param[in]  trxid  The transaction identifier.
@param[in]  first_page  The first lob page containing index list and free
                        list. */
void index_entry_t::make_old_version_current(trx_id_t trxid,
                                             base_node_page_t &first_page) {
  flst_base_node_t *base = first_page.index_list();
  flst_base_node_t *free_list = first_page.free_list();
  flst_base_node_t *version_list = get_versions_ptr();

  if (flst_get_len(version_list) > 0) {
    /* Remove the old version from versions list. */
    fil_addr_t old_node_addr = flst_get_first(version_list);
    flst_node_t *old_node = fut_get_ptr(old_node_addr);
    flst_remove(version_list, old_node);

    /* Copy the version base node from current to old entry. */
    index_entry_t old_entry(old_node);
    move_version_base_node(old_entry);

    flst_insert_before(base, old_node, m_node);
  }

  purge_version(trxid, base, free_list);

  ut_ad(flst_validate(base));
}

/** Move the version base node from current entry to the given entry.
@param[in]  old_entry  The index entry to which the version base node is moved
                       to. */
void index_entry_t::move_version_base_node(index_entry_t &old_entry) {
  flst_base_node_t *from_node = get_versions_list();
  flst_base_node_t *to_node = old_entry.get_versions_list();

  memcpy(to_node, from_node, FLST_BASE_NODE_SIZE);
  ut_ad(flst_get_len(from_node) == flst_get_len(to_node));
  flst_init(from_node);
}

/** Purge the current index entry. An index entry points to either a FIRST
page or DATA page.  That LOB page will be freed if it is DATA page.  A FIRST
page should not be freed. */
void index_entry_t::purge() {
  page_no_t page_no = get_page_no();
  buf_block_t *block = buf_page_get(page_no);
  page_type_t type = fil_page_get_type(block->m_frame);
  if (type != FIL_PAGE_TYPE_LOB_FIRST) {
    btr_page_free(block);
  }
  set_prev_null();
  set_next_null();
  set_versions_null();
  set_page_no(0);
  set_trx_id(0);
  set_data_len(0);
}

fil_addr_t index_entry_t::purge_version(trx_id_t trxid, flst_base_node_t *lst,
                                        flst_base_node_t *free_list) {
  /* Save the location of next node. */
  fil_addr_t next_loc = flst_get_next_addr(m_node);

  /* Remove the current node from the list it belongs. */
  flst_remove(lst, m_node);

  /* Purge the current node. */
  purge();

  /* Add the current node to the free list. */
  flst_add_first(free_list, m_node);

  /* Return the next node location. */
  return (next_loc);
}

buf_block_t *node_page_t::alloc(base_node_page_t &first_page) {
  /* Allocate the page. */
  m_block = btr_page_alloc();
  set_page_type();
  set_next_page(first_page.get_next_page());
  first_page.set_next_page(get_page_no());

  /* Use fully for the LOB index contents */
  ulint lob_metadata_len = payload();
  ulint node_count = lob_metadata_len / index_entry_t::SIZE;

  flst_base_node_t *free_list = first_page.free_list();

  byte *cur = nodes_begin();
  for (ulint i = 0; i < node_count; ++i) {
    flst_add_last(free_list, cur);
    cur += index_entry_t::SIZE;
  }
  ut_a(flst_validate(free_list));

  return (m_block);
}

/** Allocate one index entry. */
flst_node_t *base_node_page_t::alloc_index_entry() {
  flst_base_node_t *f_list = free_list();
  fil_addr_t node_addr = flst_get_first(f_list);
  if (fil_addr_is_null(node_addr)) {
    node_page_t node_page;
    node_page.alloc(*this);
    node_addr = flst_get_first(f_list);
  }
  flst_node_t *node = fut_get_ptr(node_addr);
  flst_remove(f_list, node);
  return (node);
}

}  // namespace lob
