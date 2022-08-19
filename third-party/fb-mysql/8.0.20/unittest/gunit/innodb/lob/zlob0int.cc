/*****************************************************************************

Copyright (c) 2016, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

#include <zlib.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>

#include "db0err.h"
#include "fut0fut.h"
#include "lot0lob.h"
#include "zlob0int.h"

namespace zlob {

using lob::ref_t;

/** The input is divided into Z_CHUNK_SIZE -> this is for uncompressed
data. */
const ulint Z_CHUNK_SIZE = KB128;

/** Given the lob:ref_t object get the first page number of zlob. */
static std::map<ref_t, page_no_t> g_zlob;

/** Purge one index entry.
@param[in]  trxid  purging data belonging to trxid.
@param[in,out]  lst  the list from which this entry will be removed.
@param[in,out]  free_list the list to which this entry will be added. */
fil_addr_t z_index_entry_t::purge_version(trx_id_t trxid, z_first_page_t &first,
                                          flst_base_node_t *lst,
                                          flst_base_node_t *free_list) {
  /* Save the location of next node. */
  fil_addr_t next_loc = get_next();

  /* Remove the current node from the list it belongs. */
  remove(lst);

  /* Purge the current node. */
  purge(first);

  /* Add the current node to the free list. */
  push_front(free_list);

  /* Return the next node location. */
  return (next_loc);
}

/** The current index entry points to a latest LOB page.  It may or may
not have older versions.  If older version is there, bring it back to the
index list from the versions list.  Then remove the current entry from
the index list.  Move the versions list from current entry to older entry.
@param[in]  trxid  The transaction identifier.
@param[in]  first  The first lob page containing index list and free list. */
fil_addr_t z_index_entry_t::make_old_version_current(trx_id_t trxid,
                                                     z_first_page_t &first) {
  flst_base_node_t *idx_flst = first.index_list();
  flst_base_node_t *free_list = first.free_list();
  flst_base_node_t *version_list = get_versions_list();

  if (flst_get_len(version_list) > 0) {
    /* Remove the old version from versions list. */
    fil_addr_t old_node_addr = flst_get_first(version_list);
    flst_node_t *old_node = fut_get_ptr(old_node_addr);
    flst_remove(version_list, old_node);

    /* Copy the version base node from current to old entry. */
    z_index_entry_t old_entry(old_node);
    move_version_base_node(old_entry);

    insert_before(idx_flst, old_entry);
  }

  fil_addr_t loc = purge_version(trxid, first, idx_flst, free_list);

  ut_ad(flst_validate(idx_flst));

  return (loc);
}

/** Purge the current index entry. An index entry points to either a FIRST
page or DATA page.  That LOB page will be freed if it is DATA page.  A FIRST
page should not be freed. */
void z_index_entry_t::purge(z_first_page_t &first_arg) {
  set_data_len(0);

  while (true) {
    page_no_t page_no = get_z_page_no();
    if (page_no == FIL_NULL) {
      break;
    }
    buf_block_t *block = buf_page_get(page_no);
    page_type_t type = fil_page_get_type(block->m_frame);
    page_no_t next = block->get_next_page();
    set_z_page_no(next);

    switch (type) {
      case FIL_PAGE_TYPE_LOB_FIRST: {
        z_first_page_t first(block);
        first.set_data_len(0);
        first.set_trx_id(0);
      } break;
      case FIL_PAGE_TYPE_ZLOB_DATA:
        btr_page_free(block);
        break;
      case FIL_PAGE_TYPE_ZLOB_FRAG: {
        z_frag_page_t frag_page(block);
        frag_id_t fid = get_z_frag_id();
        frag_page.dealloc_fragment(fid);
        if (frag_page.get_n_frags() == 0) {
          frag_page.dealloc(first_arg);
        }
      } break;
      default:
        ut_ad(0);
    }

    if (type == FIL_PAGE_TYPE_ZLOB_FRAG) {
      break;
    }
  }

  init();
}

std::ostream &z_index_entry_t::print(std::ostream &out) const {
  out << "[z_index_entry_t: m_node=" << (void *)m_node
      << ", prev=" << get_prev() << ", next=" << get_next()
      << ", versions=" << flst_bnode_t(get_versions_list())
      << ", trx_id=" << get_trx_id() << ", z_page_no=" << get_z_page_no()
      << ", z_frag_id=" << get_z_frag_id() << ", data_len=" << get_data_len()
      << ", zdata_len=" << get_zdata_len() << "]";
  print_pages(out);
  return (out);
}

std::ostream &z_index_entry_t::print_pages(std::ostream &out) const {
  page_no_t page_no = get_z_page_no();

  out << "[PAGES: ";
  while (page_no != FIL_NULL) {
    buf_block_t *block = buf_page_get(page_no);
    ulint type = block->get_page_type();
    out << "[page_no=" << page_no << ", type=" << block->get_page_type_str()
        << "]";
    page_no = block->get_next_page();
    if (type == FIL_PAGE_TYPE_ZLOB_FRAG) {
      /* Reached the fragment page. Stop. */
      break;
    }
  }
  out << "]";
  return (out);
}

std::ostream &z_frag_entry_t::print(std::ostream &out) const {
  out << "[z_frag_entry_t: prev=" << get_prev() << ", next=" << get_next()
      << ", page_no=" << get_page_no() << ", n_frags=" << get_n_frags()
      << ", used_len=" << get_used_len()
      << ", total_free_len=" << get_total_free_len()
      << ", big_free_len=" << get_big_free_len() << "]";
  return (out);
}

void z_frag_entry_t::purge(flst_base_node_t *used_lst,
                           flst_base_node_t *free_lst) {
  remove(used_lst);
  init();
  push_front(free_lst);
}

void z_frag_entry_t::update(const z_frag_page_t &frag_page) {
  set_page_no(frag_page.get_page_no());
  set_n_frags(frag_page.get_n_frags());
  set_used_len(frag_page.get_total_stored_data());
  set_total_free_len(frag_page.get_total_free_len());
  set_big_free_len(frag_page.get_big_free_len());
}

/** Print the index entries. */
std::ostream &z_first_page_t::print_index_entries(std::ostream &out) const {
  flst_base_node_t *flst = index_list();
  fil_addr_t node_loc = flst_get_first(flst);

  out << "Index Entries: " << flst_bnode_t(flst) << std::endl;

  while (!fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    z_index_entry_t entry(node);
    out << entry << std::endl;
    node_loc = entry.get_next();
  }

  return (out);
}

/** Print the frag entries. */
std::ostream &z_first_page_t::print_frag_entries(std::ostream &out) const {
  flst_base_node_t *flst = frag_list();
  fil_addr_t node_loc = flst_get_first(flst);

  out << "Frag Entries: " << flst_bnode_t(flst) << std::endl;

  while (!fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    z_frag_entry_t entry(node);
    out << entry << std::endl;
    node_loc = entry.get_next();
  }

  return (out);
}

/** Allocate one index entry.  If there is no free index entry, allocate
an index page (a page full of z_index_entry_t objects) and service the
request.
@return the allocated index entry. */
z_index_entry_t z_first_page_t::alloc_index_entry() {
  flst_base_node_t *free_lst = free_list();
  fil_addr_t first_loc = flst_get_first(free_lst);
  if (fil_addr_is_null(first_loc)) {
    z_index_page_t page;
    page.alloc(*this);
    first_loc = flst_get_first(free_lst);
  }
  ut_ad(!fil_addr_is_null(first_loc));
  flst_node_t *first_ptr = fut_get_ptr(first_loc);
  z_index_entry_t entry(first_ptr);
  entry.remove(free_lst);
  return (entry);
}

/** Allocate one frag page entry.  If there is no free frag entry, allocate
an frag node page (a page full of z_frag_entry_t objects) and service the
request.
@return the allocated frag entry. */
z_frag_entry_t z_first_page_t::alloc_frag_entry() {
  flst_base_node_t *free_lst = free_frag_list();
  flst_base_node_t *used_lst = frag_list();
  fil_addr_t first_loc = flst_get_first(free_lst);
  if (fil_addr_is_null(first_loc)) {
    z_frag_node_page_t page;
    page.alloc(*this);
    first_loc = flst_get_first(free_lst);
  }
  ut_ad(!fil_addr_is_null(first_loc));
  flst_node_t *first_ptr = fut_get_ptr(first_loc);
  z_frag_entry_t entry(first_ptr);
  entry.remove(free_lst);
  entry.push_front(used_lst);
  return (entry);
}

/** Find a fragment page, that has atleast len free space. */
z_frag_entry_t z_first_page_t::find_frag_page(ulint len) {
  z_frag_entry_t entry;

  /* Make sure that there will be some extra space for page directory
  entry and meta data.  Adding a margin to provide for this. */
  const ulint look_size = len + frag_node_t::OFFSET_DATA + 10;

  flst_base_node_t *frag_lst = frag_list();
  /* Iterate through the list of frag entries in the page. */
  fil_addr_t loc = flst_get_first(frag_lst);
  while (!fil_addr_is_null(loc)) {
    flst_node_t *node = fut_get_ptr(loc);
    entry.reset(node);

    if (entry.get_big_free_len() >= look_size) {
      break;
    }

    loc = flst_get_next_addr(node);
  }

  if (fil_addr_is_null(loc)) {
    /* Need to allocate a new fragment page. */
    z_frag_page_t frag_page;
    frag_page.alloc();

    entry = alloc_frag_entry();
    entry.set_page_no(frag_page.get_page_no());
    entry.update(frag_page);
    frag_page.set_frag_entry(entry.get_self_addr());
  }

  return (entry);
}

/** Print the page. */
std::ostream &z_first_page_t::print(std::ostream &out) const {
  print_index_entries(out);
  print_frag_entries(out);
  return (out);
}

/** Free all the z_frag_page_t pages. All the z_frag_page_t pages are
singly linked to each other.  The head of the list is maintained in the
first page. */
void z_first_page_t::free_all_frag_node_pages() {
  while (true) {
    page_no_t page_no = get_frag_node_page_no();
    if (page_no == FIL_NULL) {
      break;
    }
    z_frag_node_page_t frag_node_page;
    frag_node_page.load(page_no);
    page_no_t next_page = frag_node_page.get_next_page_no();
    set_frag_node_page_no(next_page);
    frag_node_page.dealloc();
  }
}

/** Free all the index pages. */
void z_first_page_t::free_all_index_pages() {
  while (true) {
    page_no_t page_no = get_index_page_no();
    if (page_no == FIL_NULL) {
      break;
    }
    z_index_page_t index_page;
    index_page.load(page_no);
    page_no_t next_page = index_page.get_next_page_no();
    set_index_page_no(next_page);
    index_page.dealloc();
  }
}

std::pair<ulint, ulint> z_insert_strm(z_first_page_t &first, trx_id_t trxid,
                                      byte *blob, ulint len) {
  ulint remain = len;
  page_no_t start_page_no = FIL_NULL;
  frag_id_t frag_id = FRAG_ID_NULL;
  page_no_t prev_page_no;
  byte *lob_ptr = blob;

  /* If the first page is empty, then make use of it. */
  if (first.get_data_len() == 0) {
    /* First page is unused. Use it. */
    byte *ptr = first.begin_data_ptr();
    ulint size = first.payload();

    ulint to_copy = remain > size ? size : remain;
    memcpy(ptr, lob_ptr, to_copy);
    remain -= to_copy;
    lob_ptr += to_copy;

    start_page_no = first.get_page_no();
    prev_page_no = start_page_no;

    first.set_data_len(to_copy);
    first.set_trx_id(trxid);

  } else if (remain >= z_frag_page_t::max_payload()) {
    /* Data cannot fit into a fragment page. Allocate a data page. */

    z_data_page_t data_page;
    data_page.alloc();

    byte *ptr = data_page.begin_data_ptr();
    ulint size = data_page.payload();
    ulint to_copy = remain > size ? size : remain;
    memcpy(ptr, lob_ptr, to_copy);
    remain -= to_copy;
    lob_ptr += to_copy;

    start_page_no = data_page.get_page_no();
    prev_page_no = start_page_no;

    data_page.set_data_len(to_copy);
    data_page.set_trx_id(trxid);

  } else {
    /* Data can fit into a fragment page. */
    z_frag_entry_t frag_entry = first.find_frag_page(remain);
    z_frag_page_t frag_page;
    frag_page.load(frag_entry.get_page_no());
    frag_id = frag_page.alloc_fragment(remain);
    frag_node_t node = frag_page.get_frag_node(frag_id);
    byte *ptr = node.frag_begin();

    ut_ad(remain == node.payload());
    memcpy(ptr, lob_ptr, remain);
    remain = 0;
    lob_ptr += remain;
    start_page_no = frag_page.get_page_no();

    /* Update the frag entry. */
    frag_entry.update(frag_page);

    return (std::pair<ulint, ulint>(start_page_no, frag_id));
  }

  while (remain > 0 && remain >= z_frag_page_t::max_payload()) {
    z_data_page_t data_page;
    data_page.alloc();

    byte *ptr = data_page.begin_data_ptr();
    ulint size = data_page.payload();
    ulint to_copy = remain > size ? size : remain;
    memcpy(ptr, lob_ptr, to_copy);
    remain -= to_copy;
    lob_ptr += to_copy;

    data_page.set_data_len(to_copy);
    data_page.set_trx_id(trxid);

    /* Get the previous page and update its next page. */
    buf_block_t *block = buf_page_get(prev_page_no);
    block->set_next_page_no(data_page.get_page_no());

    prev_page_no = data_page.get_page_no();
  }

  if (remain > 0) {
    ut_ad(remain < z_frag_page_t::max_payload());
    ut_ad(frag_id == FRAG_ID_NULL);
    z_frag_page_t frag_page;
    z_frag_entry_t frag_entry;

    frag_entry = first.find_frag_page(remain);
    ut_ad(frag_entry.get_big_free_len() >= remain);

    frag_page.load(frag_entry.get_page_no());
    ut_ad(frag_page.get_big_free_len() >= remain);

    frag_id = frag_page.alloc_fragment(remain);
    ut_ad(frag_id != FRAG_ID_NULL);

    frag_node_t node = frag_page.get_frag_node(frag_id);
    byte *ptr = node.frag_begin();

    ulint pl = node.payload();
    ut_ad(remain == node.payload());
    ut_ad(remain == pl);
    memcpy(ptr, lob_ptr, remain);
    remain = 0;
    lob_ptr += remain;

    /* Update the frag entry. */
    frag_entry.update(frag_page);

    /* Get the previous page and update its next page. */
    buf_block_t *block = buf_page_get(prev_page_no);
    block->set_next_page_no(frag_page.get_page_no());
  }

  return (std::pair<ulint, ulint>(start_page_no, frag_id));
}

/** Insert one chunk of input.  The maximum size of a chunk is Z_CHUNK_SIZE.
@param[in]  blob  the uncompressed LOB.
@param[out] out_entry the newly inserted index entry. can be NULL.
*/
dberr_t z_insert_chunk(z_first_page_t &first, trx_id_t trxid, ref_t ref,
                       byte *blob, ulint len, z_index_entry_t *out_entry) {
  ut_ad(len <= Z_CHUNK_SIZE);
  z_stream strm;

  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;

  // int ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  int ret = deflateInit(&strm, Z_NO_COMPRESSION);

  ut_a(ret == Z_OK);

  strm.avail_in = len;
  strm.next_in = blob;

  /* It is possible that the compressed stream is actually bigger.  So
  making use of this call to find it out for sure. */
  const ulint max_buf = deflateBound(&strm, len);

  /** @todo We should use mem_heap here. */
  std::unique_ptr<byte> tmpbuf(new byte[max_buf]);
  strm.avail_out = max_buf;
  strm.next_out = tmpbuf.get();

  ret = deflate(&strm, Z_FINISH);
  ut_a(ret == Z_STREAM_END);
  ut_a(strm.avail_in == 0);
  ut_a(strm.total_out == (max_buf - strm.avail_out));

  std::pair<ulint, ulint> result =
      z_insert_strm(first, trxid, tmpbuf.get(), strm.total_out);

  z_index_entry_t entry = first.alloc_index_entry();

  entry.set_trx_id(trxid);
  entry.set_z_page_no(result.first);
  entry.set_z_frag_id(result.second);
  entry.set_data_len(len);
  entry.set_zdata_len(strm.total_out);

  deflateEnd(&strm);

  if (out_entry != nullptr) {
    out_entry->reset(entry);
  }

  return (DB_SUCCESS);
}

dberr_t z_insert(trx_id_t trxid, ref_t ref, byte *blob, ulint len) {
  ulint remain = len;
  byte *ptr = blob;
  dberr_t err(DB_SUCCESS);

  z_first_page_t first;
  first.alloc();
  flst_base_node_t *idx_list = first.index_list();

  while (remain > 0) {
    z_index_entry_t entry;
    ulint size = (remain >= Z_CHUNK_SIZE) ? Z_CHUNK_SIZE : remain;
    err = z_insert_chunk(first, trxid, ref, ptr, size, &entry);
    ptr += size;
    remain -= size;

    entry.push_back(idx_list);
  }

  g_zlob.insert(std::pair<ref_t, page_no_t>(ref, first.get_page_no()));
  return (err);
}

/** Read one zlib stream completed, given its index entry.
@param[in]  entry  the index entry.
@param[in,out]  zbuf   the output buffer
@param[in]  zbuf_size  the size of the output buffer.
@return the size of the zlib stream.*/
ulint z_read_strm(z_index_entry_t &entry, byte *zbuf, ulint zbuf_size) {
  page_no_t page_no = entry.get_z_page_no();
  byte *ptr = zbuf;
  ulint remain = zbuf_size;

  while (remain > 0 && page_no != FIL_NULL) {
    buf_block_t *block = buf_page_get(page_no);
    ulint ptype = block->get_page_type();
    byte *data = nullptr;
    ulint data_size = 0;
    if (ptype == FIL_PAGE_TYPE_ZLOB_FRAG) {
      frag_id_t fid = entry.get_z_frag_id();
      z_frag_page_t frag_page(block);
      frag_node_t node = frag_page.get_frag_node(fid);
      data = node.data_begin();
      data_size = node.payload();
    } else if (ptype == FIL_PAGE_TYPE_ZLOB_FIRST) {
      z_first_page_t first(block);
      data = first.begin_data_ptr();
      data_size = first.get_data_len();
    } else {
      ut_a(ptype == FIL_PAGE_TYPE_ZLOB_DATA);
      z_data_page_t data_page(block);
      data = data_page.begin_data_ptr();
      data_size = data_page.get_data_len();
      ut_a(data_size <= data_page.payload());
    }
    memcpy(ptr, data, data_size);
    ptr += data_size;
    ut_a(data_size <= remain);
    remain -= data_size;
    page_no = block->get_next_page();
  }

  ulint zbytes = (zbuf_size - remain);
  return (zbytes);
}

/** Read one data chunk associated with one index entry.
@param[in]  trxid  the transaction doing the read.
@param[in]  entry  the index entry
@param[in]  offset  the offset from which to read the chunk.
@param[in,out]  len  the length of the output buffer. This length can
                     be greater than the chunk size.
@param[in,out]  buf  the output buffer.
*/
ulint z_read_chunk(trx_id_t trxid, z_index_entry_t &entry, ulint offset,
                   ulint &len, byte *&buf) {
  ut_ad(entry.can_see(trxid));

  if (entry.get_z_page_no() == FIL_NULL || entry.get_data_len() == 0) {
    return (0);
  }

  ulint zbuf_size = entry.get_zdata_len();

  /** @todo This might need to be converted to a mem_heap call. */
  std::unique_ptr<byte> zbuf(new byte[zbuf_size]);

  ulint zbytes = z_read_strm(entry, zbuf.get(), zbuf_size);
  ut_a(zbytes == zbuf_size);

  z_stream strm;
  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;

  int ret = inflateInit(&strm);
  ut_a(ret == Z_OK);

  strm.avail_in = zbytes;
  strm.next_in = zbuf.get();

  ulint to_copy = 0;
  if (offset == 0 && len >= entry.get_data_len()) {
    /* The full chunk is needed for output. */
    // strm.avail_out = entry.get_data_len();
    strm.avail_out = len;
    strm.next_out = buf;

    ret = inflate(&strm, Z_FINISH);
    ut_a(ret == Z_STREAM_END);

    to_copy = strm.total_out;
  } else {
    /* Only part of the chunk is needed. */
    byte ubuf[Z_CHUNK_SIZE];
    strm.avail_out = Z_CHUNK_SIZE;
    strm.next_out = ubuf;

    ret = inflate(&strm, Z_FINISH);
    ut_a(ret == Z_STREAM_END);

    ulint chunk_size = strm.total_out;
    ut_a(chunk_size == entry.get_data_len());
    ut_a(offset < chunk_size);

    byte *ptr = ubuf + offset;
    ulint remain = chunk_size - offset;
    to_copy = len > remain ? remain : len;
    memcpy(buf, ptr, to_copy);
  }

  len -= to_copy;
  buf += to_copy;
  inflateEnd(&strm);
  return (to_copy);
}

/** Fetch a compressed large object (ZLOB) from the system.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset read the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be fetched.
@param[out] buf  the output buffer (owned by caller) of minimum len bytes.
@return the amount of data (in bytes) that was actually read. */
ulint z_read(trx_id_t trxid, ref_t ref, ulint offset, ulint len, byte *buf) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return (0);
  }

  /* The current entry - it is the latest version. */
  z_index_entry_t cur_entry;

  /* The older version of the current entry. */
  z_index_entry_t old_version;

  page_no_t first_page_no = it->second;

  z_first_page_t first;
  first.load(first_page_no);

  flst_base_node_t *flst = first.index_list();
  fil_addr_t node_loc = flst_get_first(flst);

  /* The skip loop. */
  ulint skipped = 0;
  while (!fil_addr_is_null(node_loc)) {
    ulint will_skip = 0;
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    if (cur_entry.can_see(trxid)) {
      /* Can read the entry. */
      ulint data_len = cur_entry.get_data_len();
      will_skip = skipped + data_len;
    } else {
      /* Cannot read the entry. Look at older versions. */
      flst_base_node_t *ver_lst = cur_entry.get_versions_list();
      fil_addr_t old_node_loc = flst_get_first(ver_lst);
      while (!fil_addr_is_null(old_node_loc)) {
        flst_node_t *old_node = fut_get_ptr(old_node_loc);
        old_version.reset(old_node);
        if (old_version.can_see(trxid)) {
          /* Can read the entry. */
          ulint data_len = old_version.get_data_len();
          will_skip = skipped + data_len;
          break;
        }
        old_node_loc = old_version.get_next();
        old_version.reset(nullptr);
      }
    }

    if (will_skip >= offset) {
      /* Reached the entry containing the offset. */
      break;
    }

    skipped = will_skip;
    node_loc = cur_entry.get_next();
    cur_entry.reset(nullptr);
  }  // skip loop end

  ut_ad(skipped <= offset);

  if (old_version.is_null() && cur_entry.is_null()) {
    return (0);
  }

  ulint yet_to_skip = offset - skipped;

  ut_ad(yet_to_skip < Z_CHUNK_SIZE);

  byte *ptr = buf;
  ulint remain = len;

  if (old_version.is_null()) {
    /* We need to read the latest version entry. */
    if (cur_entry.can_see(trxid)) {
      z_read_chunk(trxid, cur_entry, yet_to_skip, remain, ptr);
    }
  } else {
    /* We need to read the old version entry. */
    if (old_version.can_see(trxid)) {
      z_read_chunk(trxid, old_version, yet_to_skip, remain, ptr);
    }
  }

  node_loc = cur_entry.get_next();

  while (remain > 0 && !fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    if (cur_entry.can_see(trxid)) {
      /* Can read the entry. */
      z_read_chunk(trxid, cur_entry, 0, remain, ptr);

    } else {
      /* Cannot read the entry. Look at older versions. */
      flst_base_node_t *ver_lst = cur_entry.get_versions_list();
      fil_addr_t old_node_loc = flst_get_first(ver_lst);
      while (!fil_addr_is_null(old_node_loc)) {
        flst_node_t *old_node = fut_get_ptr(old_node_loc);
        old_version.reset(old_node);
        if (old_version.can_see(trxid)) {
          /* Can read the entry. */
          z_read_chunk(trxid, old_version, 0, remain, ptr);
          break;
        }
        old_node_loc = old_version.get_next();
        old_version.reset(nullptr);
      }
    }

    node_loc = cur_entry.get_next();
    cur_entry.reset(nullptr);
  }

  return (len - remain);
}

dberr_t z_print_info(lob::ref_t ref, std::ostream &out) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return (DB_FAIL);
  }

  out << "Found zlob: " << ref << ", first=" << it->second << std::endl;

  z_first_page_t first;
  first.load(it->second);

  first.print(out);

  return (DB_SUCCESS);
}

/** Replace a large object (LOB) with the given new data.
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset replace the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be replaced.
@param[in] buf  the buffer (owned by caller) with new data (len bytes).
@return the actual number of bytes replaced. */
ulint z_replace(trx_id_t trxid, lob::ref_t ref, ulint offset, ulint len,
                byte *buf) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return (0);
  }

  /* The current entry - it is the latest version. */
  z_index_entry_t cur_entry;

  page_no_t first_page_no = it->second;

  z_first_page_t first;
  first.load(first_page_no);

  flst_base_node_t *flst = first.index_list();
  fil_addr_t node_loc = flst_get_first(flst);

  /* The skip loop. */
  ulint skipped = 0;
  while (!fil_addr_is_null(node_loc)) {
    ulint will_skip = 0;
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    if (cur_entry.can_see(trxid)) {
      /* Can read the entry. */
      ulint data_len = cur_entry.get_data_len();
      will_skip = skipped + data_len;
    } else {
      /* Cannot modify this lob. */
      return (0);
    }

    if (will_skip >= offset) {
      /* Reached the entry containing the offset. */
      break;
    }

    skipped = will_skip;
    node_loc = cur_entry.get_next();
    cur_entry.reset(nullptr);
  }  // skip loop end

  ut_ad(skipped <= offset);

  if (cur_entry.is_null()) {
    return (0);
  }

  /* The cur_entry points to the chunk that needs to be partially replaced. */
  byte chunk[Z_CHUNK_SIZE];
  ulint yet_to_skip = offset - skipped;
  ut_ad(yet_to_skip < Z_CHUNK_SIZE);

  byte *ptr = chunk;
  ulint replace_len = len; /* bytes remaining to be replaced. */
  ulint read_length = cur_entry.get_data_len();
  ulint length_1 = z_read_chunk(trxid, cur_entry, 0, read_length, ptr);
  ut_ad(length_1 == cur_entry.get_data_len());
  ut_ad(yet_to_skip <= length_1);

  byte *to_ptr = chunk + yet_to_skip;
  byte *from_ptr = buf;
  ulint remain = length_1 - yet_to_skip;
  ulint can_be_replaced = remain > replace_len ? replace_len : remain;
  memcpy(to_ptr, from_ptr, can_be_replaced);
  from_ptr += can_be_replaced;
  replace_len -= can_be_replaced;

  /* chunk now has the new data to be inserted. */
  z_index_entry_t new_entry;
  z_insert_chunk(first, trxid, ref, chunk, length_1, &new_entry);
  cur_entry.insert_after(flst, new_entry);
  cur_entry.remove(flst);
  new_entry.set_old_version(cur_entry);

  node_loc = new_entry.get_next();
  new_entry.reset(nullptr);
  cur_entry.reset(nullptr);

  /* Replace remaining. */
  while (replace_len > 0 && !fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node); /* the old entry. */

    if (!cur_entry.can_see(trxid)) {
      /** @todo  Rollback the operation that has been done so far. */
      return (0);
    }

    ulint size = cur_entry.get_data_len();

    if (replace_len < size) {
      /* Only partial chunk is to be replaced. Read old data. */
      ulint read_len = size;
      ptr = chunk;
      ulint len1 = z_read_chunk(trxid, cur_entry, 0, read_len, ptr);
      ut_ad(len1 == cur_entry.get_data_len());
      ut_ad(read_len == 0);
      ut_ad(len1 == size);
      memcpy(chunk, from_ptr, replace_len);

      /* Chunk now contains new data to be inserted. */
      /** @todo if there was error, rollback must happen. */
      z_insert_chunk(first, trxid, ref, chunk, len1, &new_entry);
      cur_entry.insert_after(flst, new_entry);
      cur_entry.remove(flst);
      new_entry.set_old_version(cur_entry);

      /* Replace Completed. */
      replace_len = 0;
      break;

    } else {
      /* Full chunk is to be replaced. No need to read old data. */
      /** @todo if there was error, rollback must happen. */
      z_insert_chunk(first, trxid, ref, from_ptr, size, &new_entry);

      ut_ad(new_entry.get_trx_id() == trxid);

      from_ptr += size;
      ut_a(size <= replace_len);
      replace_len -= size;

      fil_addr_t next_loc1 = cur_entry.get_next();
      cur_entry.insert_after(flst, new_entry);
      cur_entry.remove(flst);

      fil_addr_t next_loc2 = new_entry.get_next();
      ut_ad(next_loc1.page == next_loc2.page &&
            next_loc1.boffset == next_loc2.boffset);

      new_entry.set_old_version(cur_entry);
      node_loc = new_entry.get_next();
    }

    new_entry.reset(nullptr);
    cur_entry.reset(nullptr);
  }

  const ulint actual_replace = len - replace_len;

  return (actual_replace);
}

/** Insert data into the middle of an LOB */
ulint z_insert_middle(trx_id_t trxid, lob::ref_t ref, ulint offset, byte *data,
                      ulint len) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return (0);
  }

  /* The current entry - it is the latest version. */
  z_index_entry_t cur_entry;

  page_no_t first_page_no = it->second;

  z_first_page_t first;
  first.load(first_page_no);

  flst_base_node_t *flst = first.index_list();
  fil_addr_t node_loc = flst_get_first(flst);

  /* The skip loop. */
  ulint skipped = 0;
  while (!fil_addr_is_null(node_loc)) {
    ulint will_skip = skipped;
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    if (cur_entry.can_see(trxid)) {
      /* Can read the entry. */
      ulint data_len = cur_entry.get_data_len();
      will_skip = skipped + data_len;
    } else {
      /* Cannot modify this lob. */
      return (0);
    }

    /** @todo this should be > and not >= */
    if (will_skip >= offset) {
      /* Reached the entry containing the offset. */
      break;
    }

    skipped = will_skip;
    node_loc = cur_entry.get_next();
    cur_entry.reset(nullptr);
  }  // skip loop end

  ut_ad(skipped <= offset);

  if (cur_entry.is_null()) {
    return (0);
  }

  byte chunk[Z_CHUNK_SIZE];
  byte saved[Z_CHUNK_SIZE];
  ulint saved_len = 0;
  byte *insert_ptr = data;
  ulint insert_len = len;
  bool saved_data = false;
  bool insert_before = false;

  if (skipped < offset) {
    /* Inserting in the middle of the chunk. */
    byte *ptr = chunk;
    ulint read_len = cur_entry.get_data_len();
    ulint len1 = z_read_chunk(trxid, cur_entry, 0, read_len, ptr);
    ut_ad(read_len == 0);
    ut_ad(len1 == cur_entry.get_data_len());

    ptr = chunk;
    ulint yet_to_skip = offset - skipped;
    saved_len = len1 - yet_to_skip;
    memcpy(saved, ptr + yet_to_skip, saved_len);
    saved_data = true;

    ulint can_insert = Z_CHUNK_SIZE - yet_to_skip;
    if (insert_len < can_insert) {
      /* If the requested length to be inserted is smaller. */
      can_insert = insert_len;
    }
    memcpy(ptr + yet_to_skip, insert_ptr, can_insert);
    insert_ptr += can_insert;
    ut_ad(insert_len >= can_insert);
    insert_len -= can_insert;

    ulint chunk_size = yet_to_skip + can_insert;
    z_index_entry_t entry;
    z_insert_chunk(first, trxid, ref, ptr, chunk_size, &entry);
    cur_entry.insert_after(flst, entry);
    cur_entry.remove(flst);
    entry.set_old_version(cur_entry);

    cur_entry.reset(entry);
  } else {
    ut_ad(skipped == offset);
    insert_before = true;
  }

  while (insert_len > 0) {
    z_index_entry_t entry;
    ulint size = (insert_len >= Z_CHUNK_SIZE) ? Z_CHUNK_SIZE : insert_len;

    if (size < Z_CHUNK_SIZE && saved_data) {
      break;
    }

    z_insert_chunk(first, trxid, ref, insert_ptr, size, &entry);
    insert_ptr += size;

    ut_ad(insert_len >= size);
    insert_len -= size;

    if (insert_before) {
      cur_entry.insert_before(flst, entry);
    } else {
      cur_entry.insert_after(flst, entry);
      cur_entry.reset(entry);
    }
  }

  if (saved_data) {
    ut_ad(insert_len < Z_CHUNK_SIZE);
    byte *ptr = chunk;
    memcpy(ptr, insert_ptr, insert_len);
    ulint remain_space = Z_CHUNK_SIZE - insert_len;
    ulint to_copy = saved_len > remain_space ? remain_space : saved_len;

    byte *saved_ptr = saved;
    memcpy(ptr + insert_len, saved_ptr, to_copy);

    z_index_entry_t entry;
    z_insert_chunk(first, trxid, ref, ptr, insert_len + to_copy, &entry);

    cur_entry.insert_after(flst, entry);
    cur_entry.reset(entry);

    if (to_copy < saved_len) {
      ulint saved_remain = saved_len - to_copy;
      z_insert_chunk(first, trxid, ref, saved_ptr + to_copy, saved_remain,
                     &entry);

      cur_entry.insert_after(flst, entry);
      cur_entry.reset(entry);
    }
  }

  return (DB_SUCCESS);
}

/** Delete a portion of the given large object (LOB)
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset remove the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be removed.
@return actual number of bytes removed. */
ulint z_remove_middle(trx_id_t trxid, lob::ref_t ref, ulint offset, ulint len) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return (0);
  }

  /* The current entry - it is the latest version. */
  z_index_entry_t cur_entry;

  page_no_t first_page_no = it->second;

  z_first_page_t first;
  first.load(first_page_no);

  flst_base_node_t *flst = first.index_list();
  fil_addr_t node_loc = flst_get_first(flst);

  /* The skip loop. */
  ulint skipped = 0;
  while (!fil_addr_is_null(node_loc)) {
    ulint will_skip = skipped;
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    if (cur_entry.can_see(trxid)) {
      /* Can read the entry. */
      ulint data_len = cur_entry.get_data_len();
      will_skip = skipped + data_len;
    } else {
      /* Cannot modify this lob. */
      return (0);
    }

    if (will_skip > offset) {
      /* Reached the entry containing the offset. */
      break;
    }

    skipped = will_skip;
    node_loc = cur_entry.get_next();
    cur_entry.reset(nullptr);
  }  // skip loop end

  ut_ad(skipped <= offset);

  if (cur_entry.is_null()) {
    return (0);
  }

  ulint remove_len = len;

  if (skipped < offset) {
    /* Partial data of current chunk needs to be removed. */
    byte chunk[Z_CHUNK_SIZE];
    byte *ptr = chunk;
    ulint read_len = cur_entry.get_data_len();
    ulint len1 = z_read_chunk(trxid, cur_entry, 0, read_len, ptr);
    ut_ad(read_len == 0);
    ut_ad(len1 == cur_entry.get_data_len());

    ulint yet_to_skip = offset - skipped;
    ut_ad(len1 > yet_to_skip);

    ulint can_remove = len1 - yet_to_skip;

    ulint to_remove = can_remove < remove_len ? can_remove : remove_len;

    z_index_entry_t new_entry;

    ptr = chunk;
    byte *to = ptr + yet_to_skip;
    byte *from = ptr + yet_to_skip + to_remove;
    ulint n = len1 - yet_to_skip - to_remove;
    memmove(to, from, n);

    z_insert_chunk(first, trxid, ref, chunk, (len1 - to_remove), &new_entry);
    cur_entry.insert_after(flst, new_entry);
    cur_entry.remove(flst);
    new_entry.set_old_version(cur_entry);

    remove_len -= to_remove;
    node_loc = new_entry.get_next();
  }

  while (remove_len > 0 && !fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    ulint read_len = cur_entry.get_data_len();

    if (remove_len < read_len) {
      break;
    }

    /* Full chunk is to be removed. So just add a dummy entry. */
    z_index_entry_t new_entry = first.alloc_index_entry();
    new_entry.init();
    new_entry.set_trx_id(trxid);
    cur_entry.insert_after(flst, new_entry);
    cur_entry.remove(flst);
    new_entry.set_old_version(cur_entry);

    node_loc = new_entry.get_next();

    remove_len -= read_len;
  }

  if (fil_addr_is_null(node_loc)) {
    return (len - remove_len);
  }

  if (remove_len > 0) {
    /* Partial data of current chunk needs to be removed. */
    byte chunk[Z_CHUNK_SIZE];
    byte *ptr = chunk;
    ulint read_len = cur_entry.get_data_len();
    ulint len1 = z_read_chunk(trxid, cur_entry, 0, read_len, ptr);
    ut_ad(read_len == 0);
    ut_ad(len1 == cur_entry.get_data_len());

    ulint can_remove = len1;
    ulint to_remove = remove_len > can_remove ? can_remove : remove_len;

    z_index_entry_t new_entry;

    ptr = &chunk[to_remove];
    z_insert_chunk(first, trxid, ref, ptr, (len1 - to_remove), &new_entry);
    cur_entry.insert_after(flst, new_entry);
    cur_entry.remove(flst);
    new_entry.set_old_version(cur_entry);

    remove_len -= to_remove;
  }

  return (len - remove_len);
}

void z_purge(trx_id_t trxid, lob::ref_t ref) {
  auto it = g_zlob.find(ref);

  if (it == g_zlob.end()) {
    return;
  }

  /* The current entry - it is the latest version. */
  z_index_entry_t cur_entry;

  page_no_t first_page_no = it->second;

  z_first_page_t first;
  first.load(first_page_no);

  flst_base_node_t *flst = first.index_list();
  flst_base_node_t *free_list = first.free_list();
  fil_addr_t node_loc = flst_get_first(flst);

  while (!fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    cur_entry.reset(node);

    flst_base_node_t *vers = cur_entry.get_versions_list();
    fil_addr_t ver_loc = flst_get_first(vers);

    /* Scan the older versions. */
    while (!fil_addr_is_null(ver_loc)) {
      flst_node_t *ver_node = fut_get_ptr(ver_loc);
      z_index_entry_t vers_entry(ver_node);
      if (vers_entry.can_be_purged(trxid)) {
        ver_loc = vers_entry.purge_version(trxid, first, vers, free_list);
      } else {
        ver_loc = vers_entry.get_next();
      }
    }

    /* Now process the current entry. */
    if (cur_entry.can_be_purged(trxid)) {
      node_loc = cur_entry.make_old_version_current(trxid, first);
    } else {
      node_loc = cur_entry.get_next();
    }

    cur_entry.reset(nullptr);
  }

  if (first.empty()) {
    first.free_all_frag_node_pages();
    first.free_all_index_pages();
    first.dealloc();
  }
}

/** Allocate the fragment page.
@return the allocated buffer block. */
buf_block_t *z_frag_page_t::alloc() {
  m_block = btr_page_alloc();

  /* Set page type to FIL_PAGE_TYPE_ZLOB_FRAG. */
  set_page_type();
  set_page_next(FIL_NULL);

  set_frag_entry_null();

  /* Initialize the frag free list. */
  plist_base_node_t fl = free_list();
  fl.init();

  /* Initialize the used frag list. */
  plist_base_node_t frag_lst = frag_list();
  frag_lst.init();

  byte *f = frame();

  /* Add the available space as free frag to free list. */
  frag_node_t frag(f, f + OFFSET_FRAGS_BEGIN, payload());
  fl.push_front(frag.m_node);
  frag.set_frag_id_null();

  return (m_block);
}

z_frag_entry_t z_frag_page_t::get_frag_entry() {
  fil_addr_t node_loc = get_frag_entry_addr();
  flst_node_t *node = fut_get_ptr(node_loc);
  z_frag_entry_t entry(node);
  ut_ad(entry.get_page_no() == get_page_no());
  return (entry);
}

void z_frag_page_t::dealloc(z_first_page_t &first) {
  ut_ad(get_n_frags() == 0);
  z_frag_entry_t entry = get_frag_entry();
  entry.purge(first.frag_list(), first.free_frag_list());
  btr_page_free(m_block);
  m_block = nullptr;
}

std::ostream &z_frag_page_t::print_frags_in_order(std::ostream &out) const {
  if (m_block == nullptr) {
    return (out);
  }
  plist_base_node_t free_lst = free_list();
  plist_base_node_t frag_lst = frag_list();

  out << "[Free List: " << free_lst << "]" << std::endl;
  out << "[Frag List: " << frag_lst << "]" << std::endl;

  frag_node_t cur_free = free_lst.get_first_node();
  frag_node_t cur_frag = frag_lst.get_first_node();

  while (!cur_free.is_null() && !cur_frag.is_null()) {
    if (cur_free.is_before(cur_frag)) {
      out << "F: " << cur_free << std::endl;
      cur_free = cur_free.get_next_node();
    } else {
      out << "U: " << cur_frag << std::endl;
      cur_frag = cur_frag.get_next_node();
    }
  }

  if (cur_free.is_null()) {
    while (!cur_frag.is_null()) {
      out << "U: " << cur_frag << std::endl;
      cur_frag = cur_frag.get_next_node();
    }
  }

  if (cur_frag.is_null()) {
    while (!cur_free.is_null()) {
      out << "F: " << cur_free << std::endl;
      cur_free = cur_free.get_next_node();
    }
  }

  return (out);
}

/** Get the total amount of stored data in this page. */
ulint z_frag_page_t::get_total_stored_data() const {
  ulint len = 0;

  ut_ad(m_block != nullptr);

  plist_base_node_t frag_lst = frag_list();

  for (plist_node_t cur = frag_lst.get_first_node(); !cur.is_null();
       cur = cur.get_next_node()) {
    frag_node_t frag(cur);
    len += frag.payload();
  }

  return (len);
}

/** Get the total cumulative free space in this page. */
ulint z_frag_page_t::get_total_free_len() const {
  ulint len = 0;

  ut_ad(m_block != nullptr);

  plist_base_node_t free_lst = free_list();
  for (plist_node_t cur = free_lst.get_first_node(); !cur.is_null();
       cur = cur.get_next_node()) {
    frag_node_t frag(cur);
    len += frag.payload();
  }
  return (len);
}

/** Get the big free space in this page. */
ulint z_frag_page_t::get_big_free_len() const {
  ulint big = 0;

  ut_ad(m_block != nullptr);

  plist_base_node_t free_lst = free_list();
  for (plist_node_t cur = free_lst.get_first_node(); !cur.is_null();
       cur = cur.get_next_node()) {
    frag_node_t frag(cur);
    ulint payload = frag.payload();
    if (payload > big) {
      big = payload;
    }
  }

  return (big);
}

/** Deallocate all the free slots from the end of the page directory. */
void z_frag_page_t::dealloc_frag_id() {
  plist_base_node_t free_lst = free_list();
  plist_node_t last = free_lst.get_last_node();
  frag_node_t frag(last);
  /* The last free fragment must be adjacent to the directory.
  Then only it can take space from one slot. */
  if (frag.end_ptr() != slots_end_ptr()) {
    return;
  }

  ulint frag_id = get_n_dir_entries() - 1;
  paddr_t addr = frag_id_to_addr(frag_id);
  while (addr == 0) {
    frag.incr_length_by_2();
    decr_n_dir_entries();
    if (frag_id == 0) {
      break;
    }
    frag_id--;
    addr = frag_id_to_addr(frag_id);
  }
}
}  // namespace zlob
