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
#ifndef _lot0buf_h_
#define _lot0buf_h_

#include "buf0types.h"
#include "fil0fil.h"
#include "lot0types.h"
#include "mach0data.h"

struct buf_block_t {
  byte *m_frame;

  void set_next_page(page_no_t num) {
    mach_write_to_4(m_frame + FIL_PAGE_NEXT, num);
  }

  page_no_t get_next_page() {
    return (mach_read_from_4(m_frame + FIL_PAGE_NEXT));
  }

  void set_page_no(page_no_t num) {
    mach_write_to_4(m_frame + FIL_PAGE_OFFSET, num);
  }

  void set_next_page_no(page_no_t num) {
    mach_write_to_4(m_frame + FIL_PAGE_NEXT, num);
  }

  page_no_t get_page_no() {
    return (mach_read_from_4(m_frame + FIL_PAGE_OFFSET));
  }

  ulint get_page_type() { return (mach_read_from_2(m_frame + FIL_PAGE_TYPE)); }

  const char *get_page_type_str() {
    ulint type = get_page_type();
    switch (type) {
      case FIL_PAGE_TYPE_LOB_INDEX:
        return ("FIL_PAGE_TYPE_LOB_INDEX");
      case FIL_PAGE_TYPE_LOB_DATA:
        return ("FIL_PAGE_TYPE_LOB_DATA");
      case FIL_PAGE_TYPE_LOB_FIRST:
        return ("FIL_PAGE_TYPE_LOB_FIRST");
      case FIL_PAGE_TYPE_ZLOB_FIRST:
        return ("FIL_PAGE_TYPE_ZLOB_FIRST");
      case FIL_PAGE_TYPE_ZLOB_DATA:
        return ("FIL_PAGE_TYPE_ZLOB_DATA");
      case FIL_PAGE_TYPE_ZLOB_INDEX:
        return ("FIL_PAGE_TYPE_ZLOB_INDEX");
      case FIL_PAGE_TYPE_ZLOB_FRAG:
        return ("FIL_PAGE_TYPE_ZLOB_FRAG");
      case FIL_PAGE_TYPE_ZLOB_FRAG_ENTRY:
        return ("FIL_PAGE_TYPE_ZLOB_FRAG_ENTRY");
      default:
        return ("UNKNOWN");
    }
  }
};

/** Get the specified page number.
@param[in] page_no the requested page number. */
buf_block_t *buf_page_get(page_no_t page_no);

/** Allocate a new page.
@return the newly allocated buffer block. */
buf_block_t *btr_page_alloc();

void btr_page_free(buf_block_t *block);

inline buf_frame_t *buf_block_get_frame(const buf_block_t *block) {
  return (block->m_frame);
}

void buf_pool_reset();

#endif  // _lot0buf_h_
