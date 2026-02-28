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
#ifndef _fil0fil_h_
#define _fil0fil_h_

#include "fil0types.h"
#include "lot0types.h"
#include "mach0data.h"

/** File space address */
struct fil_addr_t {
  fil_addr_t() : page(FIL_NULL), boffset(0) {}
  fil_addr_t(page_no_t p, ulint off) : page(p), boffset(off) {}

  page_no_t page; /*!< page number within a space */
  ulint boffset;  /*!< byte offset within the page */

  bool is_equal(const fil_addr_t &that) const {
    return ((page == that.page) && (boffset == that.boffset));
  }

  std::ostream &print(std::ostream &out) const {
    out << "[fil_addr_t: page=" << page << ", boffset=" << boffset << "]";
    return (out);
  }
};

inline std::ostream &operator<<(std::ostream &out, const fil_addr_t &obj) {
  return (obj.print(out));
}

typedef byte fil_faddr_t;

#define FIL_ADDR_PAGE 0 /* first in address is the page offset */
#define FIL_ADDR_BYTE 4 /* then comes 2-byte byte offset within page*/
#define FIL_ADDR_SIZE 6 /* address size is 6 bytes */

extern fil_addr_t fil_addr_null;
typedef uint16_t page_type_t;

inline bool fil_addr_is_null(fil_addr_t addr) {
  return (addr.page == FIL_NULL);
}

/** Get the file page type.
@param[in]      page    file page
@return page type */
inline page_type_t fil_page_get_type(const byte *page) {
  return (static_cast<page_type_t>(mach_read_from_2(page + FIL_PAGE_TYPE)));
}

#define FIL_PAGE_TYPE_LOB_INDEX 21
#define FIL_PAGE_TYPE_LOB_DATA 22
#define FIL_PAGE_TYPE_LOB_FIRST 23
#define FIL_PAGE_TYPE_ZLOB_FIRST 24
#define FIL_PAGE_TYPE_ZLOB_DATA 25

/** This page contains an array of z_index_entry_t objects. */
#define FIL_PAGE_TYPE_ZLOB_INDEX 26
#define FIL_PAGE_TYPE_ZLOB_FRAG 27
#define FIL_PAGE_TYPE_ZLOB_FRAG_ENTRY 28

#endif  // _fil0fil_h_
