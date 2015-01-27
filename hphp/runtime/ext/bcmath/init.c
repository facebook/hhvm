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
#include "bcmath.h"
#include "private.h"

#if SANDER_0
 bc_num _bc_Free_list = NULL;
#endif

/* new_num allocates a number and sets fields to known values. */

bc_num
_bc_new_num_ex (length, scale, persistent)
     int length, scale, persistent;
{
  bc_num temp;

  temp = (bc_num)malloc(sizeof(bc_struct)+length + scale);
#if 0
  if (_bc_Free_list != NULL) {
    temp = _bc_Free_list;
    _bc_Free_list = temp->n_next;
  } else {
    temp = (bc_num)malloc (sizeof(bc_struct));
    if (temp == NULL) bc_out_of_memory ();
  }
#endif
  temp->n_sign = PLUS;
  temp->n_len = length;
  temp->n_scale = scale;
  temp->n_refs = 1;
  temp->n_ptr = (char *)malloc(length + scale);
  if (temp->n_ptr == NULL) bc_out_of_memory();
  temp->n_value = temp->n_ptr;
  memset(temp->n_ptr, 0, length+scale);
  return temp;
}


/* "Frees" a bc_num NUM.  Actually decreases reference count and only
   frees the storage if reference count is zero. */

void
_bc_free_num_ex (num, persistent)
    bc_num *num;
    int persistent;
{
  if (*num == NULL) return;
  (*num)->n_refs--;
  if ((*num)->n_refs == 0) {
    if ((*num)->n_ptr) {
      free((*num)->n_ptr);
    }
    free(*num);
#if 0
    (*num)->n_next = _bc_Free_list;
    _bc_Free_list = *num;
#endif
  }
  *num = NULL;
}


/* Intitialize the number package! */

void
bc_init_numbers (TSRMLS_D)
{
  BCG(_zero_) = _bc_new_num_ex (1,0,1);
  BCG(_one_)  = _bc_new_num_ex (1,0,1);
  BCG(_one_)->n_value[0] = 1;
  BCG(_two_)  = _bc_new_num_ex (1,0,1);
  BCG(_two_)->n_value[0] = 2;
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

