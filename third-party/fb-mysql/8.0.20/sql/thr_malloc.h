/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef THR_MALLOC_INCLUDED
#define THR_MALLOC_INCLUDED

#include <stddef.h>

struct CHARSET_INFO;
struct MEM_ROOT;
typedef unsigned int PSI_memory_key;
extern thread_local MEM_ROOT **THR_MALLOC;

void init_sql_alloc(PSI_memory_key key, MEM_ROOT *root, size_t block_size,
                    size_t pre_alloc_size);

void *sql_calloc(size_t);
char *sql_strdup(const char *str);
char *sql_strmake(const char *str, size_t len);
void *sql_memdup(const void *ptr, size_t size);
char *sql_strmake_with_convert(const char *str, size_t arg_length,
                               const CHARSET_INFO *from_cs,
                               size_t max_res_length, const CHARSET_INFO *to_cs,
                               size_t *result_length);

#endif /* THR_MALLOC_INCLUDED */
