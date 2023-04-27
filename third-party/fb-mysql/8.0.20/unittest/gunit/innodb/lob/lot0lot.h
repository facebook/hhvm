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

#include <ostream>
#include "lot0types.h"

struct lob_tester_t {
  void print_info();

  /** Insert 10MB of data and fetch it.  This LOB will have the FIRST page,
  more than one DATA page and also more than one INDEX page. This test case
  assumes a page size of 16K. */
  void fetch_full();

  /** An LOB that will have one FIRST page and one DATA page. Do an
  insert_middle() on the second page, which doesn't spill on other pages. This
  is done by same transaction. This test case assumes a page size of 16K. */
  void test_4();

  /** An LOB that will have only one FIRST page. Do remove_middle() on the
  first page, which doesn't spill on other pages. This test case assumes a
  page size of 16K. */
  void remove_middle_1();

  /** A generic remove_middle() tester. */
  void remove_middle_gen(ulint lob_size, ulint offset, ulint rm_len);

  /** A generic replace() tester. */
  void replace_generic(ulint lob_size, ulint offset, ulint replace_len);

  /** A stress tester for insert_middle(). Fixed insert length, with
  a variable offset.  */
  void insert_middle_stress();

  /** A generic insert_middle() tester.
  @param[in]  lob_size  the size of the LOB.
  @param[in]  offset    The offset into LOB where new data inserted.
  @param[in]  len       The length of new data inserted.*/
  void insert_middle_generic(ulint lob_size, ulint offset, ulint len);

  /** A stress test for replace. */
  void replace_stress();

  /** A stress tester for remove_middle(). Fixed removal length, with
  a variable offset.  */
  void remove_middle_stress_1();

  /** Insert an LOB and then do rollback of the transaction. */
  void insert_rollback();

  ulint get_page_size() const { return (UNIV_PAGE_SIZE); }

  static std::ostream &print_lob(std::ostream &out, byte *lob, ulint size) {
    for (ulint i = 0; i < size; ++i) {
      out << lob[i];
    }
    out << std::endl;
    return (out);
  }
};
