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

// Some wrappers related to the MEM_ROOT for the current thread.
// Mostly obsolete.

#include "sql/thr_malloc.h"

#include <string.h>
#include <sys/types.h>
#include <algorithm>

#include "m_ctype.h"
#include "my_alloc.h"
#include "my_macros.h"
#include "my_sys.h"
#include "sql/mysqld.h"
#include "sql/psi_memory_key.h"
#include "sql_string.h"

using std::max;
using std::min;

extern "C" void sql_alloc_error_handler(void);

void init_sql_alloc(PSI_memory_key key, MEM_ROOT *mem_root, size_t block_size,
                    size_t pre_alloc) {
  init_alloc_root(key, mem_root, block_size, pre_alloc);
  mem_root->set_error_handler(sql_alloc_error_handler);
}

void *sql_calloc(size_t size) {
  void *ptr;
  if ((ptr = (*THR_MALLOC)->Alloc(size))) memset(ptr, 0, size);
  return ptr;
}

char *sql_strdup(const char *str) {
  size_t len = strlen(str) + 1;
  char *pos;
  if ((pos = (char *)(*THR_MALLOC)->Alloc(len))) memcpy(pos, str, len);
  return pos;
}

char *sql_strmake(const char *str, size_t len) {
  char *pos;
  if ((pos = (char *)(*THR_MALLOC)->Alloc(len + 1))) {
    memcpy(pos, str, len);
    pos[len] = 0;
  }
  return pos;
}

void *sql_memdup(const void *ptr, size_t len) {
  void *pos;
  if ((pos = (*THR_MALLOC)->Alloc(len))) memcpy(pos, ptr, len);
  return pos;
}

char *sql_strmake_with_convert(const char *str, size_t arg_length,
                               const CHARSET_INFO *from_cs,
                               size_t max_res_length, const CHARSET_INFO *to_cs,
                               size_t *result_length) {
  char *pos;
  size_t new_length = to_cs->mbmaxlen * arg_length;
  max_res_length--;  // Reserve place for end null

  new_length = std::min(new_length, max_res_length);
  if (!(pos = (char *)(*THR_MALLOC)->Alloc(new_length + 1)))
    return pos;  // Error

  if ((from_cs == &my_charset_bin) || (to_cs == &my_charset_bin)) {
    // Safety if to_cs->mbmaxlen > 0
    new_length = min(arg_length, max_res_length);
    memcpy(pos, str, new_length);
  } else {
    uint dummy_errors;
    new_length = copy_and_convert(pos, new_length, to_cs, str, arg_length,
                                  from_cs, &dummy_errors);
  }
  pos[new_length] = 0;
  *result_length = new_length;
  return pos;
}
