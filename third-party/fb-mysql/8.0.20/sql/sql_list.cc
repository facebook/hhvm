/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/sql_list.h"

#include "my_alloc.h"
#include "my_sys.h"

list_node end_of_list;

void free_list(I_List<i_string> *list) {
  i_string *tmp;
  while ((tmp = list->get())) delete tmp;
}

base_list::base_list(const base_list &rhs, MEM_ROOT *mem_root) {
  if (rhs.elements) {
    /*
      It's okay to allocate an array of nodes at once: we never
      call a destructor for list_node objects anyway.
    */
    first = (list_node *)mem_root->Alloc(sizeof(list_node) * rhs.elements);
    if (first) {
      elements = rhs.elements;
      list_node *dst = first;
      list_node *src = rhs.first;
      for (; dst < first + elements - 1; dst++, src = src->next) {
        dst->info = src->info;
        dst->next = dst + 1;
      }
      /* Copy the last node */
      dst->info = src->info;
      dst->next = &end_of_list;
      /* Setup 'last' member */
      last = &dst->next;
      return;
    }
  }
  elements = 0;
  first = &end_of_list;
  last = &first;
}
