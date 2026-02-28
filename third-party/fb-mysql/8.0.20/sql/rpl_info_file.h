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

#ifndef RPL_INFO_FILE_H
#define RPL_INFO_FILE_H

#include <stddef.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"                // IO_CACHE
#include "sql/rpl_info_handler.h"  // Rpl_info_handler

class Server_ids;

extern bool skip_flush_master_info;
extern bool skip_flush_relay_worker_info;

/**
  Defines a file hander.
*/
class Rpl_info_file : public Rpl_info_handler {
  friend class Rpl_info_factory;

 public:
  virtual ~Rpl_info_file();

 private:
  /**
    This uniquely identifies a file.

    When introducing multi-master replication one needs to ensure
    that files' names are unique. If tables are used, there is no
    issue at all. It is highly recommend to avoid using files as
    they do not provide an atomic behavior.
  */
  char info_fname[FN_REFLEN + 128];

  /**
    This is used to identified a name's pattern. For instance,
    worker-relay-log.info.*.
  */
  char pattern_fname[FN_REFLEN + 128];

  /*
    info_fd - file descriptor of the info file. set only during
    initialization or clean up - safe to read anytime
  */
  File info_fd;

  /* IO_CACHE of the info file - set only during init or end */
  IO_CACHE info_file;

  /*
    The flag indicates whether the file name include the instance number or not.
  */
  bool name_indexed;

  int do_init_info();
  int do_init_info(uint instance);
  enum_return_check do_check_info();
  enum_return_check do_check_info(uint instance);
  void do_end_info();
  int do_flush_info(const bool force);
  int do_remove_info();
  int do_clean_info();
  /**
    Returns the number of files that corresponds to param_info_fname.
    If param_info_fname is a regular expression, @c expression is
    set.

    @param[in]  nparam              Number of fields in the file.
    @param[in]  param_pattern_fname File's name.
    @param[in]  indexed             indicates whether the file is indexed and
                                    if so there is a range to count in.
    @param[in]  nullable_bitmap     bitmap that holds the fields that are
                                    allowed to be `NULL`.
    @param[out] counter             Number of files found.

    @retval false Success
    @retval true  Error
  */
  static bool do_count_info(const int nparam, const char *param_pattern_fname,
                            bool indexed, MY_BITMAP const *nullable_bitmap,
                            uint *counter);
  static int do_reset_info(int const nparam, const char *param_pattern_fname,
                           bool name_indexed, MY_BITMAP const *nullable_bitmap);

  int do_prepare_info_for_read();
  int do_prepare_info_for_write();
  bool do_set_info(const char *format, va_list args);
  bool do_set_info(const int pos, const char *value);
  bool do_set_info(const int pos, const uchar *value, const size_t size);
  bool do_set_info(const int pos, const int value);
  bool do_set_info(const int pos, const ulong value);
  bool do_set_info(const int pos, const float value);
  bool do_set_info(const int pos, const Server_ids *value);
  /**
    Setter needed to set nullable fields to `NULL`.

    @param pos the index of the field to set to `NULL`.
    @param value unused value, needed to desimbiguate polimorphism.

    @return true if there was an error and false otherwise.
   */
  bool do_set_info(const int pos, const std::nullptr_t value);
  /**
    Setter needed to set nullable fields to `NULL`.

    @param pos the index of the field to set to `NULL`.
    @param value unused value, needed to desimbiguate polimorphism.
    @param size unused value size, needed to desimbiguate polimorphism.

    @return true if there was an error and false otherwise.
   */
  bool do_set_info(const int pos, const std::nullptr_t value,
                   const size_t size);
  /**
    Checks if the value returned from the read function is an actual error or
    just the side-effect of a nullable field.

    @param pos the position of the field to check.
    @param n_read_bytes number of read  bytes, returned from the read function.

    @return `FIELD_VALUE_NOT_NULL` if the number of read bytes is bigger or
            equal than 0 or the column is not nullable.
            `FIELD_VALUE_IS_NULL` if the number of read bytes is 0 and the
            columns is nullable.
            `FAILURE` if the number of read bytes is lower than 0.
   */
  Rpl_info_handler::enum_field_get_status check_for_error(int pos,
                                                          long n_read_bytes);
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
  uint do_get_rpl_info_type();

  bool do_is_transactional();
  bool do_update_is_transactional();

  Rpl_info_file(int const nparam, const char *param_pattern_fname,
                const char *param_info_fname, bool name_indexed,
                MY_BITMAP const *nullable_bitmap);

  Rpl_info_file(const Rpl_info_file &info);
  Rpl_info_file &operator=(const Rpl_info_file &info);
};
#endif /* RPL_INFO_FILE_H */
