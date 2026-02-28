/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef MERGE_SORT_INCLUDED
#define MERGE_SORT_INCLUDED

/**
  @file

  @brief
  Merge sort and insert sort implementations. These sorting functions
  are primarily intended for sorting of JOIN_TABs before the greedy
  search algorithm is applied. Since the JOIN_TAB comparison functions
  (Join_tab_compare*) are not transitive, the resulting order depends
  on the sorting implementation to a certain degree.

  Since the std::stable_sort and std::sort implementations differ
  between platforms, the result of sorting JOIN_TABs may also differ.
  In turn, the query execution plan would differ between platforms and
  that is a problem with mtr tests (EXPLAIN output would vary).

  If you intend to sort something transitive (which means almost
  everything except JOIN_TABs) you should most likely use one of the
  std sorting functions instead of this.
*/

#include <queue>

#include "my_dbug.h"

/**
 Sorts the elements in the range [first,last) into ascending order
 using insertion sort.

 @param first   First element in an array of pointers to be sorted
 @param last    Element after the last element in an array of pointers
                to be sorted
 @param comp    Comparison function object that, taking two pointers
                of the same type as those contained in the range,
                returns true if the first argument goes before the
                second argument in the specific strict weak ordering
                it defines, and false otherwise.

 In our case comp should be a function object with an operator:

 bool operator()(Element_type*, Element_type*)
*/

template <typename Element_type, typename Comp_func>
void insert_sort(Element_type **first, Element_type **last, Comp_func comp) {
  for (Element_type **high_water_mark = first + 1; high_water_mark < last;
       high_water_mark++) {
    for (Element_type **cur = high_water_mark; cur > first; cur--) {
      if (comp(*(cur - 1), *cur)) break;

      Element_type *tmp = *(cur - 1);
      *(cur - 1) = *cur;
      *cur = tmp;
    }
  }
}

/**
 Sorts the elements in the range [first,last) into ascending order
 using merge sort.

 @param first   First element in an array of pointers to be sorted
 @param last    Element after the last element in an array of pointers
                to be sorted
 @param comp    Comparison function object that, taking two pointers
                of the same type as those contained in the range,
                returns true if the first argument goes before the
                second argument in the specific strict weak ordering
                it defines, and false otherwise.

 In our case comp should be a function object with an operator:

 bool operator()(Element_type*, Element_type*)
*/

template <typename Element_type, typename Comp_func>
void merge_sort(Element_type **first, Element_type **last, Comp_func comp) {
  const uint elements = static_cast<uint>(last - first);

  /*
    Tests showed that the value 5 was a good number for JOIN_TAB
    ordering, which is the primary use case for this function
  */
  if (elements < 5) {
    insert_sort(first, last, comp);
    return;
  }
  Element_type **middle = first + (elements) / 2;

  merge_sort(first, middle, comp);
  merge_sort(middle, last, comp);

  std::queue<Element_type *> merged;

  Element_type **cur1 = first;
  Element_type **cur2 = middle;

  for (uint i = 0; i < elements; i++) {
    DBUG_ASSERT(cur1 < middle || cur2 < last);

    if (cur1 == middle)
      merged.push(*cur2++);
    else if (cur2 == last)
      merged.push(*cur1++);
    else if (comp(*cur1, *cur2))
      merged.push(*cur1++);
    else
      merged.push(*cur2++);
  }

  Element_type **result = first;
  while (!merged.empty()) {
    *result++ = merged.front();
    merged.pop();
  }
}

#endif /* MERGE_SORT_INCLUDED */
