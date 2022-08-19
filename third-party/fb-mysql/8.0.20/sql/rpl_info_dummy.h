/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_INFO_DUMMY_H
#define RPL_INFO_DUMMY_H

#include <stddef.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "sql/rpl_info_handler.h"  // Rpl_info_handler

class Server_ids;

/**
  Defines a dummy handler that should only be internally accessed.
  This class is useful for debugging and performance tests.

  The flag abort indicates if the execution should abort if some
  methods are called. See the code for further details.
*/
class Rpl_info_dummy : public Rpl_info_handler {
 public:
  Rpl_info_dummy(const int nparam);
  virtual ~Rpl_info_dummy() {}

 private:
  int do_init_info();
  int do_init_info(uint instance);
  enum_return_check do_check_info();
  enum_return_check do_check_info(uint instance);
  void do_end_info();
  int do_flush_info(const bool force);
  int do_remove_info();
  int do_clean_info();

  int do_prepare_info_for_read();
  int do_prepare_info_for_write();
  bool do_set_info(const char *format, va_list args);
  bool do_set_info(const int pos, const char *value);
  bool do_set_info(const int pos, const uchar *value, const size_t size);
  bool do_set_info(const int pos, const int value);
  bool do_set_info(const int pos, const ulong value);
  bool do_set_info(const int pos, const float value);
  bool do_set_info(const int pos, const Server_ids *value);
  bool do_set_info(const int pos, const std::nullptr_t value);
  bool do_set_info(const int pos, const std::nullptr_t value,
                   const size_t size);
  Rpl_info_handler::enum_field_get_status do_get_info(
      const int pos, char *value, const size_t size, const char *default_value);
  Rpl_info_handler::enum_field_get_status do_get_info(
      const int pos, uchar *value, const size_t size,
      const uchar *default_value);
  Rpl_info_handler::enum_field_get_status do_get_info(const int pos, int *value,
                                                      const int default_value);
  Rpl_info_handler::enum_field_get_status do_get_info(
      const int pos, ulong *value, const ulong default_value);
  Rpl_info_handler::enum_field_get_status do_get_info(
      const int pos, float *value, const float default_value);
  Rpl_info_handler::enum_field_get_status do_get_info(
      const int pos, Server_ids *value, const Server_ids *default_value);
  char *do_get_description_info();
  bool do_is_transactional();
  bool do_update_is_transactional();
  uint do_get_rpl_info_type();

  static const bool abort = false;

  Rpl_info_dummy &operator=(const Rpl_info_dummy &info);
  Rpl_info_dummy(const Rpl_info_dummy &info);
};
#endif /* RPL_INFO_DUMMY_H */
