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

#include <list>
#include <vector>

#include "zlob0int.h"

using namespace zlob;

void line() { std::cout << " - - - - - - - - - - - " << std::endl; }

void basic_0() {
  zlob::z_frag_page_t frag_page;

  frag_page.print(std::cout);

  frag_page.alloc();

  frag_page.print(std::cout);
}

void basic_1() {
  zlob::z_frag_page_t frag_page;

  line();
  frag_page.print(std::cout);

  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  frag_page.alloc_fragment(100);

  line();
  frag_page.print(std::cout);
}

void basic_2() {
  zlob::z_frag_page_t frag_page;

  line();
  frag_page.print(std::cout);

  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  ulint frag = frag_page.alloc_fragment(100);

  line();
  frag_page.print(std::cout);

  frag_page.dealloc_fragment(frag);
  line();
  frag_page.print(std::cout);
}

void basic_3() {
  zlob::z_frag_page_t frag_page;
  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  std::list<ulint> fragments;

  for (int i = 0; i < 5; ++i) {
    ulint frag = frag_page.alloc_fragment(100);
    fragments.push_back(frag);
  }

  line();
  frag_page.print(std::cout);

  while (!fragments.empty()) {
    frag_page.dealloc_fragment(fragments.front());
    fragments.pop_front();
  }

  line();
  frag_page.print(std::cout);
}

void basic_4() {
  zlob::z_frag_page_t frag_page;
  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  std::list<ulint> fragments;

  ulint frag = frag_page.alloc_fragment(100);

  while (frag != FRAG_ID_NULL) {
    fragments.push_back(frag);
    frag = frag_page.alloc_fragment(100);
  }

  line();
  frag_page.print(std::cout);

  while (!fragments.empty()) {
    frag_page.dealloc_fragment(fragments.front());
    fragments.pop_front();
  }

  line();
  frag_page.print(std::cout);
}

void basic_5() {
  zlob::z_frag_page_t frag_page;
  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  std::list<ulint> fragments;

  ulint frag = frag_page.alloc_fragment(100);

  while (frag != FRAG_ID_NULL) {
    fragments.push_back(frag);
    frag = frag_page.alloc_fragment(100);
  }

  frag = frag_page.alloc_fragment(32);

  if (frag != FRAG_ID_NULL) {
    fragments.push_back(frag);
  }

  line();
  frag_page.print(std::cout);

  while (!fragments.empty()) {
    frag_page.dealloc_fragment(fragments.front());
    fragments.pop_front();
  }

  line();
  frag_page.print(std::cout);
}

void basic_6() {
  zlob::z_frag_page_t frag_page;
  frag_page.alloc();

  line();
  frag_page.print(std::cout);

  std::vector<frag_id_t> fragments;

  ulint frag = frag_page.alloc_fragment(100);

  while (frag != FRAG_ID_NULL) {
    fragments.push_back(frag);
    frag = frag_page.alloc_fragment(100);
  }

  line();
  frag_page.print(std::cout);

  for (ulint i = 0; i < fragments.size(); i += 2) {
    frag_page.dealloc_fragment(fragments[i]);
  }

  line();
  frag_page.print(std::cout);
}

void test7() {
  zlob::z_frag_page_t frag_page;
  frag_page.alloc();

  frag_id_t f1 = frag_page.alloc_fragment(5692);

  ut_ad(f1 != FRAG_ID_NULL);

  std::cout << "ONE" << std::endl;
  frag_page.print(std::cout);

  frag_id_t f2 = frag_page.alloc_fragment(433);
  ut_ad(f2 != FRAG_ID_NULL);

  std::cout << "TWO" << std::endl;
  frag_page.print(std::cout);

  frag_id_t f3 = frag_page.alloc_fragment(419);
  ut_ad(f3 != FRAG_ID_NULL);

  frag_node_t node3 = frag_page.get_frag_node(f3);
  std::cout << node3 << std::endl;
}

int main() { test7(); }
