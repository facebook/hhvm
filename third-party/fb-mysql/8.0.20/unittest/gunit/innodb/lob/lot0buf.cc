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
#include <string.h>
#include <cassert>
#include <iostream>
#include <map>
#include <set>

#include "lot0buf.h"
#include "mach0data.h"
#include "ut0byte.h"
#include "ut0ut.h"

#ifdef UNIV_DEBUG
#define Fname(x) const char *fname = x;
#define LOG(x) \
  { std::cout << fname << ":" << x << std::endl; }
#else
#define Fname(x)
#define LOG(x)
#endif /* UNIV_DEBUG */

const ulint buf_pool_size = GB1;

class buf_pool_t {
 public:
  buf_pool_t() {
    Fname("buf_pool_t::buf_pool_t");
    ulint page_count;
    LOG("Requested buffer pool size = " << MB100);
    ulint mem_size = ut_2pow_round(buf_pool_size, UNIV_PAGE_SIZE);
    LOG("Will be allocating = " << mem_size);
    page_count = mem_size / UNIV_PAGE_SIZE;
    LOG("Number of pages = " << page_count);
    m_frame_array_raw = new (std::nothrow) byte[mem_size];
    m_frames = (byte *)ut_align(m_frame_array_raw, UNIV_PAGE_SIZE);
    LOG("Raw ptr = " << (void *)m_frame_array_raw);
    LOG("Aligned ptr = " << (void *)m_frames);
    if (m_frame_array_raw != m_frames) {
      --page_count;
    }
    m_cur_free_frame = m_frames;
    m_frame_end = m_frames + page_count * UNIV_PAGE_SIZE;
  }

  /** Get the buffer block, given the page number.  It is an error to
  look for a page number that is not there in the buffer pool.
  @param[in]  page_no  the page number to look for in the buffer pool. */
  buf_block_t *get(page_no_t page_no) {
    auto it = m_buf_pool.find(page_no);
    assert(it != m_buf_pool.end());

    /* Free page must not be accessed. */
    ut_ad(m_free_pages.find(it->second) == m_free_pages.end());
    return (it->second);
  }

  /** Allocate a new page. */
  buf_block_t *alloc() {
    if (!m_free_pages.empty()) {
      auto it = m_free_pages.begin();
      buf_block_t *tmp = *it;
      m_free_pages.erase(it);
      return (tmp);
    }
    return (alloc(max_page_no()));
  }

  void reset() {
    m_free_pages.clear();
    m_buf_pool.clear();
    m_cur_free_frame = m_frames;
  }

  void dealloc(buf_block_t *block) { m_free_pages.insert(block); }

 private:
  buf_block_t *alloc(page_no_t page_no) {
    Fname("buf_pool_t::alloc");
    buf_block_t *block = new (std::nothrow) buf_block_t;
    assert(block != nullptr);
    /* Ensure that the page_no is not already allocated. */
    assert(m_buf_pool.find(page_no) == m_buf_pool.end());
    /* Allocate the new page. */
    block->m_frame = alloc_frame();
    assert(block->m_frame != nullptr);
    /* Initialize the page with zeroes */
    memset(block->m_frame, 0x00, UNIV_PAGE_SIZE);
    /* Set the page number */
    block->set_page_no(page_no);
    m_buf_pool.insert(std::pair<page_no_t, buf_block_t *>(page_no, block));
    LOG("block ptr=" << (void *)block);
    LOG("frame ptr=" << (void *)block->m_frame);
    LOG("page_no=" << page_no);
    return (block);
  }

  page_no_t max_page_no() const { return (m_buf_pool.size()); }

  byte *alloc_frame() {
    byte *tmp = m_cur_free_frame;
    ut_a(m_cur_free_frame != m_frame_end);
    m_cur_free_frame += UNIV_PAGE_SIZE;
    return (tmp);
  }

  /** Copy constructor is disabled. */
  buf_pool_t(const buf_pool_t &other);

  /** The map between the page number, and the actual page. */
  std::map<page_no_t, buf_block_t *> m_buf_pool;

  /** One big chunk containing all frames (unaligned). */
  byte *m_frame_array_raw;

  /** First frame in the array, aligned to page size. */
  byte *m_frames;

  /** End of frame in the array, aligned to page size. */
  byte *m_frame_end;

  /** Current free frame */
  byte *m_cur_free_frame;

  /** Free pages. */
  std::set<buf_block_t *> m_free_pages;
};

static buf_pool_t g_buf_pool;

buf_block_t *buf_page_get(page_no_t page_no) {
  return (g_buf_pool.get(page_no));
}

buf_block_t *btr_page_alloc() { return (g_buf_pool.alloc()); }

void btr_page_free(buf_block_t *block) { return (g_buf_pool.dealloc(block)); }

void buf_pool_reset() { g_buf_pool.reset(); }
