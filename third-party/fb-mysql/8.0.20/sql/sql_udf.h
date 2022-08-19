#ifndef SQL_UDF_INCLUDED
#define SQL_UDF_INCLUDED

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

/* This file defines structures needed by udf functions */

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "mysql/udf_registration_types.h"

class Item;
class Item_result_field;
class String;
class THD;
class my_decimal;
struct CHARSET_INFO;

struct udf_func {
  LEX_STRING name;
  Item_result returns;
  Item_udftype type;
  char *dl;
  void *dlhandle;
  Udf_func_any func;
  Udf_func_init func_init;
  Udf_func_deinit func_deinit;
  Udf_func_clear func_clear;
  Udf_func_add func_add;
  ulong usage_count;
};

/*
  A structure of extension attributes for a UDF argument.
  The extension pointer of UDF_ARGS may point to the object of this
  structure. There are udf_extension component services to set and get
  the extension attributes of argument.
*/
struct Udf_args_extension {
  Udf_args_extension() : charset_info(nullptr) {}
  const CHARSET_INFO **charset_info;
};

/*
  A structure of extension attributes for return value of UDF.
  The extension pointer of UDF_INIT may point to the object of this
  structure. There are udf_extension component services to set and get
  the extension attributes of return value.
*/
struct Udf_return_value_extension {
  Udf_return_value_extension(const CHARSET_INFO *charset_info = nullptr,
                             Item_result result_type = INVALID_RESULT)
      : charset_info(charset_info), result_type(result_type) {}
  const CHARSET_INFO *charset_info;
  Item_result result_type;
};

class udf_handler {
 protected:
  udf_func *u_d;
  String *buffers;
  UDF_ARGS f_args;
  UDF_INIT initid;
  char *num_buffer;
  uchar error, is_null;
  bool initialized;
  Item **args;
  Udf_args_extension m_args_extension; /**< A struct that holds the extension
                                          arguments for each UDF argument */
  Udf_return_value_extension
      m_return_value_extension; /**< A struct that holds the extension arguments
                                   for return value */
 public:
  table_map used_tables_cache;
  bool not_original;

  udf_handler(udf_func *udf_arg);
  ~udf_handler();
  udf_handler(const udf_handler &) = default;
  udf_handler(udf_handler &&) = default;
  udf_handler &operator=(const udf_handler &) = default;
  udf_handler &operator=(udf_handler &&) = default;

  const char *name() const { return u_d ? u_d->name.str : "?"; }
  Item_result result_type() const {
    return (Item_result)(u_d ? (u_d->returns) : STRING_RESULT);
  }
  bool fix_fields(THD *thd, Item_result_field *item, uint arg_count,
                  Item **args);
  void cleanup();
  double val_real(bool *null_value);
  longlong val_int(bool *null_value);
  String *val_str(String *str, String *save_str);
  my_decimal *val_decimal(bool *null_value, my_decimal *dec_buf);
  void clear();
  void add(bool *null_value);

 private:
  bool get_arguments();
  String *result_string(const char *res, size_t res_length, String *str,
                        String *save_str);
  void get_string(uint index);
  bool get_and_convert_string(uint index);
};

void udf_init_globals();
void udf_read_functions_table();
void udf_unload_udfs();
void udf_deinit_globals();
udf_func *find_udf(const char *name, size_t len = 0, bool mark_used = false);
void free_udf(udf_func *udf);
bool mysql_create_function(THD *thd, udf_func *udf);
bool mysql_drop_function(THD *thd, const LEX_STRING *name);
ulong udf_hash_size(void);
void udf_hash_rlock(void);
void udf_hash_unlock(void);
typedef void udf_hash_for_each_func_t(udf_func *, void *);
void udf_hash_for_each(udf_hash_for_each_func_t *func, void *arg);
#endif /* SQL_UDF_INCLUDED */
