/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

Without limiting anything contained in the foregoing, this file,
which is part of C Driver for MySQL (Connector/C), is also subject to the
Universal FOSS Exception, version 1.0, a copy of which can be found at
http://oss.oracle.com/licenses/universal-foss-exception.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef UDF_REGISTRATION_TYPES_H
#define UDF_REGISTRATION_TYPES_H

#ifndef MYSQL_ABI_CHECK
#include <stdbool.h>
#endif

/**
Type of the user defined function return slot and arguments
*/
enum Item_result {
  INVALID_RESULT = -1, /** not valid for UDFs */
  STRING_RESULT = 0,   /** char * */
  REAL_RESULT,         /** double */
  INT_RESULT,          /** long long */
  ROW_RESULT,          /** not valid for UDFs */
  DECIMAL_RESULT       /** char *, to be converted to/from a decimal */
};

typedef struct UDF_ARGS {
  unsigned int arg_count;           /**< Number of arguments */
  enum Item_result *arg_type;       /**< Pointer to item_results */
  char **args;                      /**< Pointer to argument */
  unsigned long *lengths;           /**< Length of string arguments */
  char *maybe_null;                 /**< Set to 1 for all maybe_null args */
  char **attributes;                /**< Pointer to attribute name */
  unsigned long *attribute_lengths; /**< Length of attribute arguments */
  void *extension;
} UDF_ARGS;

/**
Information about the result of a user defined function

@todo add a notion for determinism of the UDF.

@sa Item_udf_func::update_used_tables()
*/
typedef struct UDF_INIT {
  bool maybe_null;          /** 1 if function can return NULL */
  unsigned int decimals;    /** for real functions */
  unsigned long max_length; /** For string functions */
  char *ptr;                /** free pointer for function data */
  bool const_item;          /** 1 if function always returns the same value */
  void *extension;
} UDF_INIT;

enum Item_udftype { UDFTYPE_FUNCTION = 1, UDFTYPE_AGGREGATE };

typedef void (*Udf_func_clear)(UDF_INIT *, unsigned char *, unsigned char *);
typedef void (*Udf_func_add)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                             unsigned char *);
typedef void (*Udf_func_deinit)(UDF_INIT *);
typedef bool (*Udf_func_init)(UDF_INIT *, UDF_ARGS *, char *);
typedef void (*Udf_func_any)(void);
typedef double (*Udf_func_double)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                                  unsigned char *);
typedef long long (*Udf_func_longlong)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                                       unsigned char *);
typedef char *(*Udf_func_string)(UDF_INIT *, UDF_ARGS *, char *,
                                 unsigned long *, unsigned char *,
                                 unsigned char *);

#endif /* UDF_REGISTRATION_TYPES_H */
