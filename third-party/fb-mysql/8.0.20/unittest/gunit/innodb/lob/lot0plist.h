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
#ifndef _lot0plist_h_
#define _lot0plist_h_

/** The Page List */

#include "lot0types.h"
#include "mach0data.h"
#include "mtr0log.h"
#include "mtr0types.h"

using paddr_t = ulint;

/** The node of page list. */
struct plist_node_t {
  static const ulint OFFSET_PREV = 0; /* 2 bytes */
  static const ulint OFFSET_NEXT = 2; /* 2 bytes */
  static const ulint SIZE = 4;

  plist_node_t() : m_frame(nullptr), m_node(nullptr) {}
  plist_node_t(byte *frame, byte *node) : m_frame(frame), m_node(node) {}

  bool is_before(const plist_node_t &node) const {
    ut_ad(!is_null());
    ut_ad(!node.is_null());
    return (addr() < node.addr());
  }

  void init() {
    ut_ad(m_node != nullptr);
    mlog_write_ulint(m_node + OFFSET_PREV, 0, MLOG_2BYTES);
    mlog_write_ulint(m_node + OFFSET_NEXT, 0, MLOG_2BYTES);
  }

  void set_prev(paddr_t addr) {
    ut_ad(addr < UNIV_PAGE_SIZE);
    mlog_write_ulint(m_node + OFFSET_PREV, addr, MLOG_2BYTES);
  }

  void set_prev_node(plist_node_t &prev) { set_prev(prev.addr()); }

  void set_next(paddr_t addr) {
    ut_ad(!is_null());
    ut_ad(addr < UNIV_PAGE_SIZE);
    mlog_write_ulint(m_node + OFFSET_NEXT, addr, MLOG_2BYTES);
  }

  void set_next_node(const plist_node_t &next) { set_next(next.addr()); }

  paddr_t get_prev() const { return (mach_read_from_2(m_node + OFFSET_PREV)); }

  paddr_t get_next() const { return (mach_read_from_2(m_node + OFFSET_NEXT)); }

  plist_node_t get_next_node() const {
    paddr_t addr = get_next();
    byte *node = nullptr;
    if (addr != 0) {
      node = m_frame + addr;
    }
    return (plist_node_t(m_frame, node));
  }

  plist_node_t get_prev_node() const {
    paddr_t addr = get_prev();
    byte *node = nullptr;
    if (addr != 0) {
      node = m_frame + addr;
    }
    return (plist_node_t(m_frame, node));
  }

  paddr_t addr() const { return (m_node == nullptr) ? 0 : (m_node - m_frame); }

  byte *ptr() const { return (m_node); }

  bool is_null() const { return (m_node == nullptr); }

  std::ostream &print(std::ostream &out) const {
    out << "[plist_node_t: next=" << get_next() << ", prev=" << get_prev()
        << ", this=" << addr() << "]";
    return (out);
  }

  void set_frame(byte *frame) { m_frame = frame; }
  void set_node(byte *node) { m_node = node; }

  byte *m_frame;
  byte *m_node;
};

inline std::ostream &operator<<(std::ostream &out, const plist_node_t &obj) {
  return (obj.print(out));
}

/** The base node of page list. */
struct plist_base_node_t {
  static const ulint OFFSET_LEN = 0;   /* 4 bytes */
  static const ulint OFFSET_FIRST = 4; /* 2 bytes */
  static const ulint OFFSET_LAST = 6;  /* 2 bytes */

  /** The total size (in bytes) of a page list base node. */
  static const ulint SIZE = 8;

  plist_base_node_t(byte *frame, byte *base) : m_frame(frame), m_base(base) {}

  void init() {
    mlog_write_ulint(m_base + OFFSET_LEN, 0, MLOG_4BYTES);
    mlog_write_ulint(m_base + OFFSET_FIRST, 0, MLOG_2BYTES);
    mlog_write_ulint(m_base + OFFSET_LAST, 0, MLOG_2BYTES);
  }

  void remove(plist_node_t &node) {
    plist_node_t prev = node.get_prev_node();
    plist_node_t next = node.get_next_node();

    if (prev.is_null()) {
      set_first(next.addr());
    } else {
      prev.set_next(next.addr());
    }

    if (next.is_null()) {
      set_last(prev.addr());
    } else {
      next.set_prev(prev.addr());
    }

    node.set_next(0);
    node.set_prev(0);

    decr_len();
  }

  void push_front(plist_node_t &node) {
    if (get_len() == 0) {
      add_to_empty(node);
    } else {
      paddr_t cur_addr = node.addr();
      paddr_t first_addr = get_first();
      plist_node_t first_node = get_node(first_addr);
      node.set_next(first_addr);
      node.set_prev(0);
      first_node.set_prev(cur_addr);
      set_first(cur_addr);
      incr_len();
    }
  }

  /** Insert node2 after node1. */
  void insert_after(plist_node_t &node1, plist_node_t &node2) {
    if (node1.is_null()) {
      push_back(node2);
    } else {
      plist_node_t node3 = node1.get_next_node();
      node1.set_next_node(node2);
      node2.set_next_node(node3);

      if (node3.is_null()) {
        set_last(node2.addr());
      } else {
        node3.set_prev_node(node2);
      }

      node2.set_prev_node(node1);

      incr_len();
    }
  }

  /** Insert node2 before node3. */
  void insert_before(plist_node_t &node3, plist_node_t &node2) {
    if (node3.is_null()) {
      push_back(node2);
    } else {
      plist_node_t node1 = node3.get_prev_node();

      if (node1.is_null()) {
        set_first(node2.addr());
      } else {
        node1.set_next_node(node2);
      }

      node2.set_next_node(node3);
      node3.set_prev_node(node2);
      node2.set_prev_node(node1);

      incr_len();
    }
  }

  void add_to_empty(plist_node_t &node) {
    ut_ad(get_len() == 0);

    set_first(node.addr());
    set_last(node.addr());
    incr_len();
  }

  void push_back(plist_node_t &node) {
    if (get_len() == 0) {
      add_to_empty(node);
    } else {
      paddr_t cur_addr = node.addr();
      paddr_t last_addr = get_last();
      plist_node_t last_node = get_node(last_addr);
      node.set_next(0);
      node.set_prev_node(last_node);
      last_node.set_next(cur_addr);
      set_last(cur_addr);
      incr_len();
    }
  }

  bool empty() const { return (get_len() == 0); }

  ulint get_len() const { return (mach_read_from_4(m_base + OFFSET_LEN)); }

  paddr_t get_first() const {
    return (mach_read_from_2(m_base + OFFSET_FIRST));
  }

  plist_node_t get_first_node() const {
    plist_node_t result;
    result.set_frame(m_frame);

    if (!empty()) {
      byte *node = m_frame + get_first();
      result.set_node(node);
    }
    return (result);
  }

  paddr_t get_last() const { return (mach_read_from_2(m_base + OFFSET_LAST)); }

  plist_node_t get_last_node() const {
    plist_node_t result;
    result.set_frame(m_frame);
    if (!empty()) {
      result.set_node(m_frame + get_last());
    }
    return (result);
  }

  void set_len(ulint len) {
    mlog_write_ulint(m_base + OFFSET_LEN, len, MLOG_4BYTES);
  }

  void incr_len() {
    ulint len = mach_read_from_4(m_base + OFFSET_LEN);
    mlog_write_ulint(m_base + OFFSET_LEN, len + 1, MLOG_4BYTES);
  }

  void decr_len() {
    ulint len = mach_read_from_4(m_base + OFFSET_LEN);
    mlog_write_ulint(m_base + OFFSET_LEN, len - 1, MLOG_4BYTES);
  }

  void set_first(paddr_t addr) {
    mlog_write_ulint(m_base + OFFSET_FIRST, addr, MLOG_2BYTES);
  }

  void set_last(paddr_t addr) {
    mlog_write_ulint(m_base + OFFSET_LAST, addr, MLOG_2BYTES);
  }

  plist_node_t get_node(paddr_t addr) {
    byte *node = m_frame + addr;
    return (plist_node_t(m_frame, node));
  }

  paddr_t addr() const { return (m_base - m_frame); }

  std::ostream &print(std::ostream &out) const {
    out << "[plist_base_node_t: len=" << get_len() << ", first=" << get_first()
        << ", last=" << get_last() << ", this=" << addr() << "]";
    return (out);
  }

  std::ostream &print_list(std::ostream &out) const {
    print(out);
    out << std::endl;
    for (plist_node_t cur = get_first_node(); !cur.is_null();
         cur = cur.get_next_node()) {
      out << cur << std::endl;
    }
    return (out);
  }

  byte *m_frame;
  byte *m_base;
};

inline std::ostream &operator<<(std::ostream &out,
                                const plist_base_node_t &obj) {
  return (obj.print(out));
}

#endif  // _lot0plist_h_
