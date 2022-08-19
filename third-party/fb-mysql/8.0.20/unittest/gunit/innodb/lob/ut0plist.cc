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

#include "lot0buf.h"
#include "lot0plist.h"

void basic_0() {
  buf_block_t *block = btr_page_alloc();
  byte *frame = buf_block_get_frame(block);

  plist_base_node_t base(frame, frame);
  base.init();

  byte *ptr = frame + plist_base_node_t::SIZE;

  for (ulint i = 0; i < 1; ++i) {
    plist_node_t node(frame, ptr);
    base.push_back(node);
    ptr += plist_node_t::SIZE;
  }

  base.print_list(std::cout);
}

void test_00() {
  buf_block_t *block = btr_page_alloc();
  byte *frame = buf_block_get_frame(block);

  plist_base_node_t base(frame, frame);
  base.init();

  byte *ptr = frame + plist_base_node_t::SIZE;

  for (ulint i = 0; i < 5; ++i) {
    plist_node_t node(frame, ptr);
    base.push_back(node);
    ptr += plist_node_t::SIZE;
  }

  base.print_list(std::cout);

  plist_node_t first = base.get_first_node();
  plist_node_t cur = first.get_next_node();
  cur = cur.get_next_node();
  base.remove(cur);
  base.insert_before(first, cur);

  std::cout << "-----" << std::endl;
  base.print_list(std::cout);
}

void test_01() {
  buf_block_t *block = btr_page_alloc();
  byte *frame = buf_block_get_frame(block);

  plist_base_node_t base(frame, frame);
  base.init();

  byte *ptr = frame + plist_base_node_t::SIZE;

  for (ulint i = 0; i < 5; ++i) {
    plist_node_t node(frame, ptr);
    base.push_back(node);
    ptr += plist_node_t::SIZE;
  }

  base.print_list(std::cout);

  plist_node_t first = base.get_first_node();
  plist_node_t last = base.get_last_node();
  plist_node_t cur = first.get_next_node();
  cur = cur.get_next_node();
  base.remove(cur);
  base.insert_before(last, cur);

  std::cout << "-----" << std::endl;
  base.print_list(std::cout);
}

int main() {
  basic_0();
  test_00();
  test_01();
}
