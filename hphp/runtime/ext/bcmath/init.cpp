/* init.c: bcmath library file. */
/*
    Copyright (C) 1991, 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
    Copyright (C) 2000 Philip A. Nelson

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.  (COPYING.LIB)

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to:

      The Free Software Foundation, Inc.
      51 Franklin Street, Fifth Floor
      Boston, MA  02110-1301, USA

    You may contact the author by:
       e-mail:  philnelson@acm.org
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062

*************************************************************************/

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include "bcmath.h"
#include "private.h"

#include <folly/ScopeGuard.h>

/* new_num allocates a number and sets fields to known values. */

bc_num
_bc_new_num_ex (int length, int scale, int persistent)
{
  bc_num temp;

  assert(length >= 0);
  assert(scale >= 0);

  temp = (bc_num)bc_malloc(sizeof(bc_struct) + (size_t)length + (size_t)scale);
  SCOPE_FAIL { bc_free(temp); };
  temp->n_sign = PLUS;
  temp->n_len = length;
  temp->n_scale = scale;
  temp->n_refs = 1;
  temp->n_ptr = (char *)bc_malloc(length + scale);
  temp->n_value = temp->n_ptr;
  memset(temp->n_ptr, 0, length + scale);
  return temp;
}


/* "Frees" a bc_num NUM.  Actually decreases reference count and only
   frees the storage if reference count is zero. */

void
_bc_free_num_ex (bc_num* num, int persistent)
{
  if (*num == NULL) return;
  (*num)->n_refs--;
  if ((*num)->n_refs == 0) {
    if ((*num)->n_ptr) {
      bc_free((*num)->n_ptr);
    }
    bc_free(*num);
  }
  *num = NULL;
}

/* Make a copy of a number!  Just increments the reference count! */

bc_num
bc_copy_num (bc_num num)
{
  num->n_refs++;
  return num;
}


/* Initialize a number NUM by making it a copy of zero. */

void
bc_init_num (bc_num *num TSRMLS_DC)
{
  *num = bc_copy_num (BCG(_zero_));
}
