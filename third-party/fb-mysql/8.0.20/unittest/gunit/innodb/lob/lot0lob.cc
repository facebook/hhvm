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
#include <string.h>
#include <cassert>
#include <map>
#include <stack>

#include "db0err.h"
#include "fut0fut.h"
#include "fut0lst.h"
#include "lob0int.h"
#include "lot0buf.h"
#include "lot0lob.h"
#include "lot0types.h"

#ifdef UNIV_DEBUG
#define Fname(x) const char *fname = x;
#define LOG(x) \
  { std::cout << fname << ":" << x << std::endl; }
#else
#define Fname(x)
#define LOG(x)
#endif

namespace lob {

/** An uncompressed LOB will be treated as a small LOB if the number of
pages it occupies is less than or equal to SMALL_THRESHOLD_PAGE_COUNT. */
// const ulint SMALL_THRESHOLD_PAGE_COUNT = 10;

static std::map<ref_t, buf_block_t *> g_lob;

/** Create LOB data page */
static buf_block_t *create_data_page();

/** Insert a large object (LOB) into the system.
@param[in,out]  ref  the LOB reference.
@param[in]  blob  the large object.
@param[in]  len  the length (in bytes) of the large object.*/
dberr_t insert(trx_id_t trxid, ref_t ref, byte *blob, ulint len) {
  Fname("lob::insert");

  dberr_t ret = DB_SUCCESS;
  ulint total_written = 0;
  byte *ptr = blob;

  LOG("LOB length = " << len);
  base_node_page_t page;
  buf_block_t *base = page.alloc();

  flst_base_node_t *index_list = page.index_list();

  ulint to_write = page.write(trxid, ptr, len);
  total_written += to_write;
  ulint remaining = len;
  LOG("Remaining = " << remaining);

  {
    flst_node_t *node = page.alloc_index_entry();
    index_entry_t entry(node);
    entry.set_versions_null();
    entry.set_trx_id(trxid);
    entry.set_page_no(page.get_page_no());
    entry.set_data_len(to_write);
    flst_add_last(index_list, node);

    page.set_trx_id(trxid);
    page.set_data_len(to_write);
  }

  while (remaining > 0) {
    LOG("Allocate a new LOB page");
    data_page_t data_page;
    data_page.alloc();

    LOG("Copy data into the new LOB page");
    to_write = data_page.write(trxid, ptr, remaining);
    total_written += to_write;
    data_page.set_trx_id(trxid);

    /* Allocate a new index entry */
    flst_node_t *node = page.alloc_index_entry();

    LOG("Update the node data");
    index_entry_t entry(node);
    entry.set_versions_null();
    entry.set_trx_id(trxid);
    entry.set_page_no(data_page.get_page_no());
    entry.set_data_len(to_write);

    LOG("Append the node data into the LOB list");
    flst_add_last(index_list, node);
    ut_a(flst_validate(index_list));
  }
  g_lob.insert(std::pair<ref_t, buf_block_t *>(ref, base));

#ifdef UNIV_DEBUG
  page.print_index_entries(std::cout);
#endif /* UNIV_DEBUG */

  return (ret);
}

ulint read(trx_id_t trxid, ref_t ref, ulint offset, ulint len, byte *buf) {
  Fname("lob::read");
  ulint total_read = 0;
  ulint actual_read = 0;

  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return (0);
  }

  buf_block_t *first = it->second;
  base_node_page_t first_page(first);

  flst_base_node_t *base_node = first_page.index_list();
  fil_addr_t node_loc = flst_get_first(base_node);
  flst_node_t *node = nullptr;

  ulint skipped = 0;

  /* The skip loop. */
  while (!fil_addr_is_null(node_loc)) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /** @todo Check if the reading trx can see the entry. */

    /* Get the amount of data */
    ulint data_len = entry.get_data_len();

    ulint will_skip = skipped + data_len;

    if (will_skip >= offset) {
      /* Reached the page containing the offset. */
      break;
    }

    node_loc = flst_get_next_addr(node);
    skipped += data_len;
  }

  ulint page_offset = offset - skipped;
  ulint want = len;
  byte *ptr = buf;

  while (!fil_addr_is_null(node_loc) && want > 0) {
    LOG("want=" << want);
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    if (entry.get_trx_id() > trxid) {
      flst_base_node_t *versions = entry.get_versions_list();
      fil_addr_t node_versions = flst_get_first(versions);

      while (!fil_addr_is_null(node_versions)) {
        flst_node_t *node_old_version = fut_get_ptr(node_versions);
        index_entry_t old_version_entry(node_old_version);

        if (old_version_entry.get_trx_id() <= trxid) {
          /* The current trx can see this entry. */
          entry.reset(node_old_version);
          break;
        }
        node_versions = flst_get_next_addr(node_old_version);
      }

      if (fil_addr_is_null(node_versions)) {
        node_loc = flst_get_next_addr(node);
        continue;
      }
    }

    /* Get the page number */
    page_no_t page_no = entry.get_page_no();

    LOG("page_no=" << page_no);
    /* need data in this page. */
    buf_block_t *block = buf_page_get(page_no);
    page_type_t type = fil_page_get_type(block->m_frame);

    if (type == FIL_PAGE_TYPE_LOB_FIRST) {
      base_node_page_t page(block);
      actual_read = page.read(trxid, page_offset, ptr, want);
      ptr += actual_read;
      want -= actual_read;
    } else if (type == FIL_PAGE_TYPE_LOB_DATA) {
      data_page_t page(block);
      actual_read = page.read(trxid, page_offset, ptr, want);
      ptr += actual_read;
      want -= actual_read;
    } else {
      ut_error;
    }
    total_read += actual_read;
    page_offset = 0;

    node_loc = flst_get_next_addr(node);
  }

  LOG("Total bytes read=" << total_read << ", requested=" << len);
  return (total_read);
}

buf_block_t *create_data_page() {
  /* Allocate the page. */
  data_page_t page;
  buf_block_t *block = page.alloc();
  return (block);
}

dberr_t replace(trx_id_t trxid, ref_t ref, ulint offset, ulint len, byte *buf) {
  Fname("lob::replace");

  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return (DB_FAIL);
  }

  buf_block_t *first = it->second;
  base_node_page_t first_page(first);

  flst_base_node_t *base_node = first_page.index_list();
  fil_addr_t node_loc = flst_get_first(base_node);
  flst_node_t *node = nullptr;

  ulint skipped = 0;

  /* The skip loop. */
  while (!fil_addr_is_null(node_loc)) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /* Get the amount of data */
    ulint data_len = entry.get_data_len();

    ulint will_skip = skipped + data_len;
    if (will_skip >= offset) {
      /* Reached the page containing the offset. */
      break;
    }
    node_loc = flst_get_next_addr(node);
    skipped += data_len;
  }

  ulint page_offset = offset - skipped;
  ulint want = len; /* want to be replaced. */
  byte *ptr = buf;

  while (!fil_addr_is_null(node_loc) && want > 0) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    if (entry.get_trx_id() > trxid) {
      /* Replace allowed only on the latest version of LOB. */
      std::cout << "FAILED" << std::endl;
      return (DB_FAIL);
    }

    /* Get the page number */
    page_no_t page_no = entry.get_page_no();

    /* need data in this page. */
    buf_block_t *block = buf_page_get(page_no);
    page_type_t type = fil_page_get_type(block->m_frame);

    buf_block_t *new_block = nullptr;
    if (type == FIL_PAGE_TYPE_LOB_FIRST) {
      base_node_page_t page(block);
      new_block = page.replace(trxid, page_offset, ptr, want);

    } else if (type == FIL_PAGE_TYPE_LOB_DATA) {
      data_page_t page(block);
      new_block = page.replace(trxid, page_offset, ptr, want);
    } else {
      ut_error;
    }

    ut_ad(new_block != nullptr);

    data_page_t new_page(new_block);

    /* Allocate a new index entry */
    flst_node_t *new_node = first_page.alloc_index_entry();

    index_entry_t new_entry(new_node);
    new_entry.set_versions_null();
    new_entry.set_trx_id(trxid);
    new_entry.set_page_no(new_page.get_page_no());
    new_entry.set_data_len(new_page.get_data_len());

    flst_insert_after(base_node, node, new_node);
    flst_remove(base_node, node);
    new_entry.set_old_version(entry);

    page_offset = 0;
    node_loc = flst_get_next_addr(new_node);
  }
  LOG("Total bytes replaced=" << (len - want) << ", requested=" << len);
  return (DB_SUCCESS);
}

template <typename PageType>
buf_block_t *replace_page(PageType *page, trx_id_t trxid, ulint offset,
                          byte *&ptr, ulint &want) {
  buf_block_t *new_block = nullptr;

  /** Allocate a new data page. */
  data_page_t new_page;
  new_block = new_page.alloc();

  byte *new_ptr = new_page.data_begin();
  byte *old_ptr = page->data_begin();

  new_page.set_trx_id(trxid);
  new_page.set_data_len(page->get_data_len());

  /** Copy contents from old page to new page. */
  memcpy(new_ptr, old_ptr, offset);

  /** Copy the new data to new page. */
  ulint data_avail = page->get_data_len() - offset;
  ulint data_to_copy = want > data_avail ? data_avail : want;
  memcpy(new_ptr + offset, ptr, data_to_copy);

  /** Copy contents from old page to new page. */
  if (want < data_avail) {
    memcpy(new_ptr + offset + data_to_copy, old_ptr + offset + data_to_copy,
           (data_avail - want));
  }

  ptr += data_to_copy;
  want -= data_to_copy;

  return (new_block);
}

/** Insert extra data in the middle of an existing LOB.  Will increase the
size of LOB. */
ulint insert_middle(trx_id_t trxid, ref_t ref, ulint offset, byte *&data,
                    ulint &len) {
  Fname("lob::insert_middle");

  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return (DB_FAIL);
  }

  buf_block_t *first = it->second;
  base_node_page_t first_page(first);
  flst_base_node_t *base_node = first_page.index_list();
  fil_addr_t node_loc = flst_get_first(base_node);
  flst_node_t *node = nullptr;

  ulint skipped = 0;

  /* The skip loop. */
  while (!fil_addr_is_null(node_loc)) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /* Get the amount of data */
    ulint data_len = entry.get_data_len();

    ulint will_skip = skipped + data_len;
    if (will_skip >= offset) {
      /* Reached the page containing the offset. */
      break;
    }
    node_loc = flst_get_next_addr(node);
    skipped += data_len;
  }

  ulint page_offset = offset - skipped;
  ulint want = len;
  byte *ptr = data;

  LOG("want=" << want);
  node = fut_get_ptr(node_loc);
  index_entry_t entry(node);

  if (trxid < entry.get_trx_id()) {
    return (DB_FAIL);
  }

  /* Get the page number */
  page_no_t page_no = entry.get_page_no();

  /* Need to insert into this page. */
  LOG("inserting into page_no=" << page_no);
  buf_block_t *block = buf_page_get(page_no);
  page_type_t type = fil_page_get_type(block->m_frame);

  std::pair<ulint, byte *> saved_data;

  buf_block_t *new_block = nullptr;
  if (type == FIL_PAGE_TYPE_LOB_FIRST) {
    base_node_page_t page(block);
    saved_data = page.insert_middle(trxid, page_offset, ptr, want, new_block);

  } else if (type == FIL_PAGE_TYPE_LOB_DATA) {
    data_page_t page(block);
    saved_data = page.insert_middle(trxid, page_offset, ptr, want, new_block);
  } else {
    ut_error;
  }

  flst_node_t *prev_node = nullptr;
  ut_ad(new_block != nullptr);

  data_page_t new_data_page(new_block);

  /* Allocate a new index entry */
  flst_node_t *new_node = first_page.alloc_index_entry();

  /* Fill the new index entry. */
  index_entry_t new_index_entry(new_node);
  new_index_entry.set_versions_null();
  new_index_entry.set_page_no(new_data_page.get_page_no());
  new_index_entry.set_data_len(new_data_page.get_data_len());
  new_index_entry.set_trx_id(trxid);

  /* Insert the new node, just after the old node. */
  flst_insert_after(base_node, node, new_node);

  /* Remove old version from index list. */
  flst_remove(base_node, node);

  /* Make the existing entry an old version of new entry. */
  new_index_entry.set_old_version(entry);

  prev_node = new_node;

  LOG("want=" << want);

  while (want > 0) {
    /* There is more data to be inserted. Do them in new pages. */
    new_block = create_data_page();
    data_page_t new_page(new_block);

    /* Allocate a new index entry */
    new_node = first_page.alloc_index_entry();

    /* Fill the new index entry. */
    index_entry_t new_entry(new_node);
    new_entry.set_versions_null();
    new_entry.set_page_no(new_page.get_page_no());
    new_page.write(trxid, ptr, want);
    new_entry.set_data_len(new_page.get_data_len());
    new_entry.set_trx_id(trxid);

    flst_insert_after(base_node, prev_node, new_node);
    prev_node = new_node;
  }

  /* Insert the saved data now. */
  want = saved_data.first;
  ptr = saved_data.second;

  LOG("For saved data, want=" << want);
  if (new_block != nullptr && want > 0) {
    data_page_t new_page(new_block);
    new_page.append(trxid, ptr, want);

    /* Update its index entry. */
    index_entry_t new_entry(new_node);

    ut_ad(new_block->get_page_no() == new_entry.get_page_no());
    new_entry.set_data_len(new_page.get_data_len());
    new_entry.set_trx_id(trxid);
  }

  while (want > 0) {
    /* There is more data to be inserted. Do them in new pages. */
    buf_block_t *block2 = create_data_page();
    data_page_t new_page(block2);

    /* Allocate a new index entry */
    flst_node_t *node2 = first_page.alloc_index_entry();

    /* Fill the new index entry. */
    index_entry_t new_entry(node2);
    new_entry.set_versions_null();
    new_entry.set_page_no(new_page.get_page_no());
    new_page.write(trxid, ptr, want);
    new_entry.set_data_len(new_page.get_data_len());
    new_entry.set_trx_id(trxid);

    flst_insert_after(base_node, prev_node, node2);
    prev_node = node2;
  }

  page_offset = 0;

  // first_page.print_index_entries(std::cout);
  return (DB_SUCCESS);
}

buf_block_t *base_node_page_t::replace(trx_id_t trxid, ulint offset, byte *&ptr,
                                       ulint &want) {
  return (replace_page(this, trxid, offset, ptr, want));
}

buf_block_t *data_page_t::replace(trx_id_t trxid, ulint offset, byte *&ptr,
                                  ulint &want) {
  return (replace_page(this, trxid, offset, ptr, want));
}

/** Delete a portion of the given large object (LOB)
@param[in] trxid the transaction that is doing the read.
@param[in] ref the LOB reference identifying the LOB.
@param[in] offset remove the LOB from the given offset.
@param[in] len   the length of LOB data that needs to be removed. */
dberr_t remove_middle(trx_id_t trxid, ref_t ref, ulint offset, ulint len) {
  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return (DB_FAIL);
  }

  buf_block_t *first = it->second;
  base_node_page_t first_page(first);
  flst_base_node_t *base_node = first_page.index_list();
  fil_addr_t node_loc = flst_get_first(base_node);
  flst_node_t *node = nullptr;

  ulint skipped = 0;

  /* The skip loop. */
  while (!fil_addr_is_null(node_loc)) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /* Get the amount of data */
    ulint data_len = entry.get_data_len();

    ulint will_skip = skipped + data_len;
    if (will_skip >= offset) {
      /* Reached the page containing the offset. */
      break;
    }
    node_loc = flst_get_next_addr(node);
    skipped += data_len;
  }

  ulint page_offset = offset - skipped;
  ulint want = len;

  while (!fil_addr_is_null(node_loc)) {
    node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    if (trxid < entry.get_trx_id()) {
      return (DB_FAIL);
    }

    /* Get the page number. */
    page_no_t page_no = entry.get_page_no();

    flst_node_t *new_node = nullptr;

    /* Need to remove data starting from this page. */
    buf_block_t *block = buf_page_get(page_no);
    page_type_t type = fil_page_get_type(block->m_frame);

    buf_block_t *new_block = nullptr;
    if (type == FIL_PAGE_TYPE_LOB_FIRST) {
      base_node_page_t page(block);
      new_block = page.remove_middle(trxid, page_offset, want);

    } else if (type == FIL_PAGE_TYPE_LOB_DATA) {
      data_page_t page(block);
      new_block = page.remove_middle(trxid, page_offset, want);
    } else {
      ut_error;
    }

    ut_ad(new_block != nullptr);

    data_page_t new_page(new_block);

    /* Allocate a new index entry */
    new_node = first_page.alloc_index_entry();

    /* Fill the new index entry. */
    index_entry_t new_entry(new_node);
    new_entry.set_versions_null();
    new_entry.set_page_no(new_page.get_page_no());
    new_entry.set_data_len(new_page.get_data_len());
    new_entry.set_trx_id(trxid);
    new_entry.set_prev_null();
    new_entry.set_next_null();

    /* Insert the new node, just after the old node. */
    ut_a(flst_validate(base_node));
    flst_insert_after(base_node, node, new_node);
    ut_a(flst_validate(base_node));

    /* Remove old version from index list. */
    flst_remove(base_node, node);

    /* Make the existing entry an old version of new entry. */
    ut_ad(!new_entry.is_same(entry));
    new_entry.set_old_version(entry);

    page_offset = 0;

    if (want == 0) {
      break;
    }

    node_loc = flst_get_next_addr(new_node);
  }
  return (DB_SUCCESS);
}

void trx_purge(trx_id_t trxid, ref_t ref) {
  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return;
  }

  buf_block_t *first = it->second;
  base_node_page_t page(first);
  flst_base_node_t *base = page.index_list();
  flst_base_node_t *free_list = page.free_list();
  fil_addr_t node_loc = flst_get_first(base);

  while (!fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /* Process the version list. */
    flst_base_node_t *vers = entry.get_versions_list();
    fil_addr_t ver_loc = flst_get_first(vers);

    while (!fil_addr_is_null(ver_loc)) {
      flst_node_t *ver_node = fut_get_ptr(ver_loc);
      index_entry_t vers_entry(ver_node);
      if (vers_entry.can_be_purged(trxid)) {
        ver_loc = vers_entry.purge_version(trxid, vers, free_list);
      } else {
        ver_loc = flst_get_next_addr(ver_node);
      }
    }

    /* Note the next entry to process */
    node_loc = flst_get_next_addr(node);

    /* Now process the current entry. */
    if (entry.can_be_purged(trxid)) {
      entry.make_old_version_current(trxid, page);
    }
  }

  if (page.empty()) {
    page_no_t next_page = page.get_next_page();
    while (next_page != FIL_NULL) {
      buf_block_t *next_block = buf_page_get(next_page);
      next_page = next_block->get_next_page();
      page.set_next_page(next_page);
      btr_page_free(next_block);
    }
    page.dealloc();
  }
}

/** Remove/Delete/Destory the given LOB.
@param[in]  trxid  The transaction identifier.
@param[in]  ref  The LOB reference.*/
void remove(trx_id_t trxid, ref_t ref) {
  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return;
  }

  buf_block_t *first = it->second;
  base_node_page_t page(first);
  flst_base_node_t *base = page.index_list();
  flst_base_node_t *free_list = page.free_list();
  fil_addr_t node_loc = flst_get_first(base);

  while (!fil_addr_is_null(node_loc)) {
    flst_node_t *node = fut_get_ptr(node_loc);
    index_entry_t entry(node);

    /* Process the version list. */
    flst_base_node_t *vers = entry.get_versions_list();
    fil_addr_t ver_loc = flst_get_first(vers);

    while (!fil_addr_is_null(ver_loc)) {
      flst_node_t *ver_node = fut_get_ptr(ver_loc);
      index_entry_t vers_entry(ver_node);
      vers_entry.purge_version(trxid, vers, free_list);
      ver_loc = flst_get_first(ver_node);
    }

    /* Now process the current entry. */
    entry.purge_version(trxid, base, free_list);

    node_loc = flst_get_first(base);
  }

  ut_ad(page.empty());

  page_no_t next_page = page.get_next_page();
  while (next_page != FIL_NULL) {
    buf_block_t *next_block = buf_page_get(next_page);
    next_page = next_block->get_next_page();
    page.set_next_page(next_page);
    btr_page_free(next_block);
  }
  page.dealloc();
}

void print(std::ostream &out, ref_t ref) {
  auto it = g_lob.find(ref);

  if (it == g_lob.end()) {
    return;
  }

  buf_block_t *first = it->second;
  base_node_page_t page(first);

  out << "Number of index entries in first page: " << page.node_count()
      << std::endl;
  out << "Amount of data in first page: " << page.max_space_available()
      << std::endl;
  out << "Total size of LOB: "
      << page.max_space_available() + UNIV_PAGE_SIZE * (page.node_count() - 1)
      << std::endl;
  out << "Number of index entries in index page: " << node_page_t::node_count()
      << std::endl;
  out << "Total size of LOB: " << UNIV_PAGE_SIZE * node_page_t::node_count()
      << std::endl;
}

}  // namespace lob
