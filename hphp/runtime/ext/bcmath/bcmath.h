/* bcmath.h: bcmath library header.     */
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

#pragma once

#include "hphp/runtime/ext/bcmath/config.h"
#include <stdint.h>

typedef enum {PLUS, MINUS} sign;

typedef struct bc_struct *bc_num;

typedef struct bc_struct
    {
      sign  n_sign;
      int   n_len;      /* The number of digits before the decimal point. */
      int   n_scale;    /* The number of digits after the decimal point. */
      int   n_refs;     /* The number of pointers to this number. */
      bc_num n_next;    /* Linked list for available list. */
      char *n_ptr;      /* The pointer to the actual storage.
                           If NULL, n_value points to the inside of
                           another number (bc_multiply...) and should
                           not be "freed." */
      char *n_value;    /* The number. Not zero char terminated.
                           May not point to the same place as n_ptr as
                           in the case of leading zeros generated. */
    } bc_struct;


/* The base used in storing the numbers in n_value above.
   Currently this MUST be 10. */

#define BASE 10

/*  Some useful macros and constants. */

#define CH_VAL(c)     (c - '0')
#define BCD_CHAR(d)   (d + '0')

#ifdef MIN
#undef MIN
#undef MAX
#endif
#define MAX(a, b)      ((a)>(b)?(a):(b))
#define MIN(a, b)      ((a)>(b)?(b):(a))
#define ODD(a)        ((a)&1)

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef LONG_MAX
#define LONG_MAX 0x7ffffff
#endif


/* Function Prototypes */

bc_num _bc_new_num_ex(int length, int scale, int persistent);

void _bc_free_num_ex(bc_num *num, int persistent);

bc_num bc_copy_num(bc_num num);

void bc_init_num(bc_num *num TSRMLS_DC);

void bc_str2num(bc_num *num, char *str, int scale TSRMLS_DC);

char *bc_num2str(bc_num num);

void bc_int2num(bc_num *num, int val);

long bc_num2long(bc_num num);

int bc_compare(bc_num n1, bc_num n2);

char bc_is_zero(bc_num num TSRMLS_DC);

char bc_is_near_zero(bc_num num, int scale);

char bc_is_neg(bc_num num);

void bc_add(bc_num n1, bc_num n2, bc_num *result, int scale_min);

void bc_sub(bc_num n1, bc_num n2, bc_num *result, int scale_min);

void bc_multiply(bc_num n1, bc_num n2, bc_num *prod, int scale TSRMLS_DC);

int bc_divide(bc_num n1, bc_num n2, bc_num *quot, int scale TSRMLS_DC);

int bc_modulo(bc_num num1, bc_num num2, bc_num *result,
                           int scale TSRMLS_DC);

int bc_divmod(bc_num num1, bc_num num2, bc_num *quot,
                           bc_num *rem, int scale TSRMLS_DC);

int bc_raisemod(bc_num base, bc_num expo, bc_num mod,
                             bc_num *result, int scale TSRMLS_DC);

void bc_raise(bc_num num1, bc_num num2, bc_num *result,
                           int scale TSRMLS_DC);

int bc_sqrt(bc_num *num, int scale TSRMLS_DC);

void bc_out_num(bc_num num, int o_base, void (* out_char)(int),
                             int leading_zero TSRMLS_DC);

/* Prototypes needed for external utility routines. */

void bc_rt_warn(char *mesg ,...);
void bc_rt_error(char *mesg ,...);

void* bc_malloc(size_t total);
void bc_free(void* ptr);

#define bc_new_num(length, scale)    _bc_new_num_ex((length), (scale), 0)
#define bc_free_num(num)             _bc_free_num_ex((num), 0)

struct BCMathGlobals {
  bc_num _zero_;
  bc_num _one_;
  bc_num _two_;
  int64_t bc_precision;
};
struct BCMathGlobals *get_bcmath_globals();
#define BCG(n) (get_bcmath_globals()->n)
