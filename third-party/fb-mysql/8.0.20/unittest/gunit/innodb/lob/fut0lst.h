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
#ifndef _fut0lst_h_
#define _fut0lst_h_

#include "fil0fil.h"
#include "lot0types.h"
#include "mach0data.h"
#include "mtr0log.h"
#include "mtr0types.h"
#include "ut0byte.h"
#include "ut0dbg.h"

/* We define the field offsets of a node for the list */
#define FLST_PREV                                   \
  0 /* 6-byte address of the previous list element; \
    the page part of address is FIL_NULL, if no     \
    previous element */
#define FLST_NEXT                              \
  FIL_ADDR_SIZE /* 6-byte address of the next  \
        list element; the page part of address \
        is FIL_NULL, if no next element */

/* We define the field offsets of a base node for the list */
#define FLST_LEN 0 /* 32-bit list length field */
#define FLST_FIRST                         \
  4 /* 6-byte address of the first element \
    of the list; undefined if empty list */
#define FLST_LAST                              \
  (4 + FIL_ADDR_SIZE) /* 6-byte address of the \
          last element of the list; undefined  \
          if empty list */

/* The physical size of a list base node in bytes */
#define FLST_BASE_NODE_SIZE (4 + 2 * FIL_ADDR_SIZE)

/* The physical size of a list node in bytes */
#define FLST_NODE_SIZE (2 * FIL_ADDR_SIZE)

typedef byte flst_base_node_t;
typedef byte flst_node_t;

void flst_init(flst_base_node_t *base);
void flst_add_last(flst_base_node_t *base, flst_node_t *node);

inline ulint flst_get_len(const flst_base_node_t *base) {
  return (mach_read_from_4(base + FLST_LEN));
}

inline fil_addr_t flst_read_addr(const fil_faddr_t *faddr) {
  fil_addr_t addr;

  addr.page = mach_read_ulint(faddr + FIL_ADDR_PAGE, MLOG_4BYTES);
  addr.boffset = mach_read_ulint(faddr + FIL_ADDR_BYTE, MLOG_2BYTES);
  ut_a(addr.page == FIL_NULL || addr.boffset >= FIL_PAGE_DATA);
  ut_a(ut_align_offset(faddr, UNIV_PAGE_SIZE) >= FIL_PAGE_DATA);
  return (addr);
}

inline fil_addr_t flst_get_prev_addr(const flst_node_t *node) {
  return (flst_read_addr(node + FLST_PREV));
}

inline fil_addr_t flst_get_last(const flst_base_node_t *base) {
  return (flst_read_addr(base + FLST_LAST));
}

inline fil_addr_t flst_get_next_addr(const flst_node_t *node) {
  return (flst_read_addr(node + FLST_NEXT));
}

bool flst_validate(const flst_base_node_t *base);

inline fil_addr_t flst_get_first(const flst_base_node_t *base) {
  return (flst_read_addr(base + FLST_FIRST));
}

void flst_remove(flst_base_node_t *base, flst_node_t *node2);
void flst_add_first(flst_base_node_t *base, flst_node_t *node);

/** Insert node2 after node1 in the list base. */
void flst_insert_after(flst_base_node_t *base, flst_node_t *node1,
                       flst_node_t *node2);
void flst_insert_before(flst_base_node_t *base, flst_node_t *node2,
                        flst_node_t *node3);

void flst_write_addr(fil_faddr_t *faddr, fil_addr_t addr);

/** In-memory representation of flst_base_node_t */
struct flst_bnode_t {
  ulint len;
  fil_addr_t first;
  fil_addr_t last;

  flst_bnode_t(const flst_base_node_t *base)
      : len(flst_get_len(base)),
        first(flst_get_first(base)),
        last(flst_get_last(base)) {}

  void set(const flst_base_node_t *base) {
    len = flst_get_len(base);
    first = flst_get_first(base);
    last = flst_get_last(base);
  }

  std::ostream &print(std::ostream &out) const {
    out << "[flst_base_node_t: len=" << len << ", first=" << first
        << ", last=" << last << "]";
    return (out);
  }
};

inline std::ostream &operator<<(std::ostream &out, const flst_bnode_t &obj) {
  return (obj.print(out));
}

#endif  // _fut0lst_h_
