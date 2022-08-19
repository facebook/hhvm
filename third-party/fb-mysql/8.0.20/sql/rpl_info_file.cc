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

#include "sql/rpl_info_file.h"

#include "my_config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_inttypes.h"
#include "my_loglevel.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "m_string.h"
#include "my_dbug.h"
#include "my_dir.h"           // MY_STAT
#include "my_thread_local.h"  // my_errno
#include "mysql/components/services/log_builtins.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"     // ER_*
#include "sql/dynamic_ids.h"  // Server_ids
#include "sql/log.h"
#include "sql/mysqld.h"  // mysql_data_home
#include "sql/psi_memory_key.h"
#include "sql_string.h"

long init_ulongvar_from_file(ulong *var, IO_CACHE *f, ulong default_val);
long init_strvar_from_file(char *var, size_t max_size, IO_CACHE *f,
                           const char *default_val);
long init_intvar_from_file(int *var, IO_CACHE *f, int default_val);
long init_floatvar_from_file(float *var, IO_CACHE *f, float default_val);
long init_dynarray_intvar_from_file(char *buffer, size_t size,
                                    char **buffer_act, IO_CACHE *f);

Rpl_info_file::Rpl_info_file(const int nparam, const char *param_pattern_fname,
                             const char *param_info_fname, bool indexed_arg,
                             MY_BITMAP const *nullable_bitmap)
    : Rpl_info_handler(nparam, nullable_bitmap),
      info_fd(-1),
      name_indexed(indexed_arg) {
  DBUG_TRACE;

  fn_format(pattern_fname, param_pattern_fname, mysql_data_home, "", 4 + 32);
  fn_format(info_fname, param_info_fname, mysql_data_home, "", 4 + 32);
}

Rpl_info_file::~Rpl_info_file() {
  DBUG_TRACE;

  do_end_info();
}

int Rpl_info_file::do_init_info(uint instance) {
  DBUG_TRACE;

  char fname_local[FN_REFLEN];
  char *pos = my_stpcpy(fname_local, pattern_fname);
  if (name_indexed) sprintf(pos, "%u", instance);

  fn_format(info_fname, fname_local, mysql_data_home, "", 4 + 32);
  return do_init_info();
}

int Rpl_info_file::do_init_info() {
  int error = 0;
  DBUG_TRACE;

  /* does info file exist ? */
  enum_return_check ret_check = do_check_info();
  if (ret_check == REPOSITORY_DOES_NOT_EXIST) {
    /*
      If someone removed the file from underneath our feet, just close
      the old descriptor and re-create the old file
    */
    if (info_fd >= 0) {
      if (my_b_inited(&info_file)) end_io_cache(&info_file);
      my_close(info_fd, MYF(MY_WME));
    }
    if ((info_fd = my_open(info_fname, O_CREAT | O_RDWR, MYF(MY_WME))) < 0) {
      LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_CREATE_NEW_INFO_FILE, info_fname,
             my_errno());
      error = 1;
    } else if (init_io_cache(&info_file, info_fd, IO_SIZE * 2, READ_CACHE, 0L,
                             false, MYF(MY_WME))) {
      LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_CREATE_CACHE_FOR_INFO_FILE,
             info_fname);
      error = 1;
    }
    if (error) {
      if (info_fd >= 0) my_close(info_fd, MYF(0));
      info_fd = -1;
    }
  }
  /* file exists */
  else if (ret_check == REPOSITORY_EXISTS) {
    if (info_fd >= 0)
      reinit_io_cache(&info_file, READ_CACHE, 0L, false, false);
    else {
      if ((info_fd = my_open(info_fname, O_RDWR, MYF(MY_WME))) < 0) {
        LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_OPEN_INFO_FILE, info_fname,
               my_errno());
        error = 1;
      } else if (init_io_cache(&info_file, info_fd, IO_SIZE * 2, READ_CACHE, 0L,
                               false, MYF(MY_WME))) {
        LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_CREATE_CACHE_FOR_INFO_FILE,
               info_fname);
        error = 1;
      }
      if (error) {
        if (info_fd >= 0) my_close(info_fd, MYF(0));
        info_fd = -1;
      }
    }
  } else
    error = 1;
  return error;
}

int Rpl_info_file::do_prepare_info_for_read() {
  cursor = 0;
  prv_error = false;
  prv_get_error = Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
  return (reinit_io_cache(&info_file, READ_CACHE, 0L, false, false));
}

int Rpl_info_file::do_prepare_info_for_write() {
  cursor = 0;
  prv_error = false;
  prv_get_error = Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
  return (reinit_io_cache(&info_file, WRITE_CACHE, 0L, false, true));
}

inline enum_return_check do_check_repository_file(const char *fname) {
  if (my_access(fname, F_OK)) return REPOSITORY_DOES_NOT_EXIST;

  if (my_access(fname, F_OK | R_OK | W_OK)) return ERROR_CHECKING_REPOSITORY;

  return REPOSITORY_EXISTS;
}

/*
  The method verifies existence of an instance of the repository.

  @param instance  an index in the repository
  @retval REPOSITORY_EXISTS when the check is successful
  @retval REPOSITORY_DOES_NOT_EXIST otherwise

  @note This method also verifies overall integrity
  of the repositories to make sure they are indexed without any gaps.
*/
enum_return_check Rpl_info_file::do_check_info(uint instance) {
  uint i;
  enum_return_check last_check = REPOSITORY_EXISTS;
  char fname_local[FN_REFLEN];
  char *pos = nullptr;

  for (i = 1; i <= instance && last_check == REPOSITORY_EXISTS; i++) {
    pos = my_stpcpy(fname_local, pattern_fname);
    if (name_indexed) sprintf(pos, "%u", i);
    fn_format(fname_local, fname_local, mysql_data_home, "", 4 + 32);
    last_check = do_check_repository_file(fname_local);
  }
  return last_check;
}

enum_return_check Rpl_info_file::do_check_info() {
  return do_check_repository_file(info_fname);
}

/*
  The function counts number of files in a range starting
  from one. The range degenerates into one item when @c indexed is false.
  Scanning ends once the next indexed file is not found.

  @param      nparam    Number of fields
  @param      param_pattern
                        a string pattern to generate
                        the actual file name
  @param      indexed   indicates whether the file is indexed and if so
                        there is a range to count in.
  @param[in]  nullable_bitmap
                        bitmap that holds the fields that are allowed to be
                        `NULL`.
  @param[out] counter   the number of discovered instances before the first
                        unsuccess in locating the next file.

  @retval false     All OK
  @retval true      An error
*/
bool Rpl_info_file::do_count_info(const int nparam, const char *param_pattern,
                                  bool indexed,
                                  MY_BITMAP const *nullable_bitmap,
                                  uint *counter) {
  uint i = 0;
  Rpl_info_file *info = nullptr;

  char fname_local[FN_REFLEN];
  char *pos = nullptr;
  enum_return_check last_check = REPOSITORY_EXISTS;

  DBUG_TRACE;

  if (!(info = new Rpl_info_file(nparam, param_pattern, "", indexed,
                                 nullable_bitmap)))
    return true;

  for (i = 1; last_check == REPOSITORY_EXISTS; i++) {
    pos = my_stpcpy(fname_local, param_pattern);
    if (indexed) {
      sprintf(pos, "%u", i);
    }
    fn_format(fname_local, fname_local, mysql_data_home, "", 4 + 32);
    if ((last_check = do_check_repository_file(fname_local)) ==
        REPOSITORY_EXISTS)
      (*counter)++;
    // just one loop pass for MI and RLI file
    if (!indexed) break;
  }
  delete info;

  return false;
}

int Rpl_info_file::do_flush_info(const bool force) {
  int error = 0;

  DBUG_TRACE;

  bool sync = sync_period && ++(sync_counter) >= sync_period;

  if (flush_io_cache(&info_file)) error = 1;
  if (!error && (force || sync)) {
    if (my_sync(info_fd, MYF(MY_WME))) error = 1;
    sync_counter = 0;
  }

  return error;
}

void Rpl_info_file::do_end_info() {
  DBUG_TRACE;

  if (info_fd >= 0) {
    if (my_b_inited(&info_file)) end_io_cache(&info_file);
    my_close(info_fd, MYF(MY_WME));
    info_fd = -1;
  }
}

int Rpl_info_file::do_remove_info() {
  MY_STAT stat_area;
  int error = 0;

  DBUG_TRACE;

  if (my_stat(info_fname, &stat_area, MYF(0)) &&
      my_delete(info_fname, MYF(MY_WME)))
    error = 1;

  return error;
}

int Rpl_info_file::do_clean_info() {
  /*
    There is nothing to do here. Maybe we can truncate the
    file in the future. Howerver, for now, there is no need.
  */
  return 0;
}

int Rpl_info_file::do_reset_info(const int nparam, const char *param_pattern,
                                 bool indexed,
                                 MY_BITMAP const *nullable_bitmap) {
  int error = false;
  uint i = 0;
  Rpl_info_file *info = nullptr;
  char fname_local[FN_REFLEN];
  char *pos = nullptr;
  enum_return_check last_check = REPOSITORY_EXISTS;

  DBUG_TRACE;

  if (!(info = new Rpl_info_file(nparam, param_pattern, "", indexed,
                                 nullable_bitmap)))
    return true;

  for (i = 1; last_check == REPOSITORY_EXISTS; i++) {
    pos = my_stpcpy(fname_local, param_pattern);
    if (indexed) {
      sprintf(pos, "%u", i);
    }
    fn_format(fname_local, fname_local, mysql_data_home, "", 4 + 32);
    if ((last_check = do_check_repository_file(fname_local)) ==
        REPOSITORY_EXISTS)
      if (my_delete(fname_local, MYF(MY_WME))) error = true;
    // just one loop pass for MI and RLI file
    if (!indexed) break;
  }
  delete info;

  return error;
}

bool Rpl_info_file::do_set_info(const char *format, va_list args) {
  return (my_b_vprintf(&info_file, format, args) <= 0);
}

bool Rpl_info_file::do_set_info(const int pos, const char *value) {
  if (value == nullptr) {
    return do_set_info(pos, nullptr);
  }
  return (my_b_printf(&info_file, "%s\n", value) > (size_t)0 ? false : true);
}

bool Rpl_info_file::do_set_info(const int pos, const uchar *value,
                                const size_t size) {
  if (value == nullptr) {
    return do_set_info(pos, nullptr, size);
  }
  return (my_b_write(&info_file, value, size));
}

bool Rpl_info_file::do_set_info(const int, const ulong value) {
  return (my_b_printf(&info_file, "%lu\n", value) > (size_t)0 ? false : true);
}

bool Rpl_info_file::do_set_info(const int, const int value) {
  return (my_b_printf(&info_file, "%d\n", value) > (size_t)0 ? false : true);
}

bool Rpl_info_file::do_set_info(const int, const float value) {
  /*
    64 bytes provide enough space considering that the precision is 3
    bytes (See the appropriate set funciton):

    FLT_MAX  The value of this macro is the maximum number representable
             in type float. It is supposed to be at least 1E+37.
    FLT_MIN  Similar to the FLT_MAX, we have 1E-37.

    If a file is manually and not properly changed, this function may
    crash the server.
  */
  char buffer[64];

  sprintf(buffer, "%.3f", value);

  return (my_b_printf(&info_file, "%s\n", buffer) > (size_t)0 ? false : true);
}

bool Rpl_info_file::do_set_info(const int, const Server_ids *value) {
  bool error = true;
  String buffer;

  /*
    This produces a line listing the total number and all the server_ids.
  */
  if (const_cast<Server_ids *>(value)->pack_dynamic_ids(&buffer)) goto err;

  error =
      (my_b_printf(&info_file, "%s\n", buffer.c_ptr_safe()) > (size_t)0 ? false
                                                                        : true);
err:
  return error;
}

bool Rpl_info_file::do_set_info(const int pos, const std::nullptr_t) {
  if (!this->is_field_nullable(pos)) return true;
  return (my_b_printf(&info_file, "\n") > (size_t)0 ? false : true);
}

bool Rpl_info_file::do_set_info(const int pos, const std::nullptr_t,
                                const size_t) {
  if (!this->is_field_nullable(pos)) return true;
  return (my_b_printf(&info_file, "\n") > (size_t)0 ? false : true);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::check_for_error(
    int pos, long n_read_bytes) {
  if (n_read_bytes == 0 && this->is_field_nullable(pos))
    return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_IS_NULL;
  else if (n_read_bytes < 0)
    return Rpl_info_handler::enum_field_get_status::FAILURE;
  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, char *value, const size_t size, const char *default_value) {
  long n_bytes_read =
      init_strvar_from_file(value, size, &info_file, default_value);
  return this->check_for_error(pos, n_bytes_read);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, uchar *value, const size_t size, const uchar *) {
  long n_bytes_read = my_b_read(&info_file, value, size);
  return this->check_for_error(pos, n_bytes_read);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, ulong *value, const ulong default_value) {
  long n_bytes_read = init_ulongvar_from_file(value, &info_file, default_value);
  return this->check_for_error(pos, n_bytes_read);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, int *value, const int default_value) {
  long n_bytes_read =
      init_intvar_from_file(value, &info_file, (int)default_value);
  return this->check_for_error(pos, n_bytes_read);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, float *value, const float default_value) {
  long n_bytes_read = init_floatvar_from_file(value, &info_file, default_value);
  return this->check_for_error(pos, n_bytes_read);
}

Rpl_info_handler::enum_field_get_status Rpl_info_file::do_get_info(
    const int pos, Server_ids *value, const Server_ids *) {
  /*
    Static buffer to use most of the times. However, if it is not big
    enough to accommodate the server ids, a new buffer is allocated.
  */
  const int array_size = 16 * (sizeof(long) * 3 + 1);
  char buffer[array_size];
  char *buffer_act = buffer;

  long n_bytes_read = init_dynarray_intvar_from_file(buffer, sizeof(buffer),
                                                     &buffer_act, &info_file);
  if (n_bytes_read > 0) value->unpack_dynamic_ids(buffer_act);

  if (buffer != buffer_act) {
    /*
      Release the buffer allocated while reading the server ids
      from the file.
    */
    my_free(buffer_act);
  }

  return this->check_for_error(pos, n_bytes_read);
}

char *Rpl_info_file::do_get_description_info() { return info_fname; }

bool Rpl_info_file::do_is_transactional() { return false; }

bool Rpl_info_file::do_update_is_transactional() {
  DBUG_EXECUTE_IF("simulate_update_is_transactional_error", { return true; });
  return false;
}

uint Rpl_info_file::do_get_rpl_info_type() { return INFO_REPOSITORY_FILE; }

/**
  Tries to read a string of maximum size `max_size` from the `IO_CACHE` `f`
  current line - being a line a string terminated by '\n'.

  @param  var         Put the read values in this static buffer
  @param  max_size    Size of the static buffer
  @param  f           IO_CACHE of the replication info file.
  @param default_val  Default value to be assigned in case unable to read bytes.

  @retval >= 0  Number or read bytes
  @retval < 0   An error
*/
long init_strvar_from_file(char *var, size_t max_size, IO_CACHE *f,
                           const char *default_val) {
  long length;
  DBUG_TRACE;

  if ((length = static_cast<long>(my_b_gets(f, var, max_size)))) {
    char *last_p = var + length - 1;
    if (*last_p == '\n') {
      *last_p = 0;  // if we stopped on newline, kill it
      return length - 1;
    } else {
      /*
        If we truncated a line or stopped on last char, remove all chars
        up to and including newline.
      */
      int c;
      while (((c = my_b_get(f)) != '\n' && c != my_b_EOF))
        ;
    }
    return length;
  } else if (default_val) {
    strmake(var, default_val, max_size - 1);
    return strlen(default_val);
  }
  return -1;
}

/**
  Tries to read an integer value from the `IO_CACHE` `f` current line - being a
  line a string terminated by '\n'.

  @param  var         Put the read value in this parameter.
  @param  f           IO_CACHE of the replication info file.
  @param default_val  Default value to be assigned in case unable to read bytes.

  @retval >= 0  Number or read bytes
  @retval < 0   An error
*/
long init_intvar_from_file(int *var, IO_CACHE *f, int default_val) {
  /*
    32 bytes provide enough space:

    INT_MIN    –2,147,483,648
    INT_MAX    +2,147,483,647
  */
  char buf[32];
  DBUG_TRACE;

  long length;
  if ((length = static_cast<long>(my_b_gets(f, buf, sizeof(buf))))) {
    *var = atoi(buf);
    return length;
  } else if (default_val) {
    *var = default_val;
    return sizeof(default_val);
  }
  return -1;
}

/**
  Tries to read an unsigned integer value from the `IO_CACHE` `f` current line -
  being a line a string terminated by '\n'.

  @param  var         Put the read value in this parameter.
  @param  f           IO_CACHE of the replication info file.
  @param default_val  Default value to be assigned in case unable to read bytes.

  @retval >= 0  Number or read bytes
  @retval < 0   An error
*/
long init_ulongvar_from_file(ulong *var, IO_CACHE *f, ulong default_val) {
  /*
    32 bytes provide enough space:

    ULONG_MAX   32 bit compiler   +4,294,967,295
                64 bit compiler   +18,446,744,073,709,551,615
  */
  char buf[32];
  DBUG_TRACE;

  long length;
  if ((length = static_cast<long>(my_b_gets(f, buf, sizeof(buf))))) {
    *var = strtoul(buf, nullptr, 10);
    return length;
  } else if (default_val) {
    *var = default_val;
    return sizeof(default_val);
  }
  return -1;
}

/**
  Tries to read a float value from the `IO_CACHE` `f` current line - being a
  line a string terminated by '\n'.

  @param  var         Put the read value in this parameter.
  @param  f           IO_CACHE of the replication info file.
  @param default_val  Default value to be assigned in case unable to read bytes.

  @retval >= 0  Number or read bytes
  @retval < 0   An error
*/
long init_floatvar_from_file(float *var, IO_CACHE *f, float default_val) {
  /*
    64 bytes provide enough space considering that the precision is 3
    bytes (See the appropriate set funciton):

    FLT_MAX  The value of this macro is the maximum number representable
             in type float. It is supposed to be at least 1E+37.
    FLT_MIN  Similar to the FLT_MAX, we have 1E-37.

    If a file is manually and not properly changed, this function may
    crash the server.
  */
  char buf[64];
  DBUG_TRACE;

  long length;
  if ((length = static_cast<long>(my_b_gets(f, buf, sizeof(buf))))) {
    if (sscanf(buf, "%f", var) != 1)
      return -1;
    else
      return length;
  } else if (default_val != 0.0) {
    *var = default_val;
    return sizeof(default_val);
  }
  return -1;
}

/**
   TODO - Improve this function to use String and avoid this weird computation
   to calculate the size of the buffers.

   Particularly, this function is responsible for restoring IGNORE_SERVER_IDS
   list of servers whose events the slave is going to ignore (to not log them
   in the relay log).

   Items being read are supposed to be decimal output of values of a  type
   shorter or equal of @c long and separated by the single space.

   @param  buffer      Put the read values in this static buffer
   @param  size        Size of the static buffer
   @param  buffer_act  Points to the final buffer as dynamic buffer may
                       be used if the static buffer is not big enough.
   @param  f           IO_CACHE of the replication info file.

   @retval >= 0  Number or read bytes
   @retval < 0   An error
*/
long init_dynarray_intvar_from_file(char *buffer, size_t size,
                                    char **buffer_act, IO_CACHE *f) {
  char *buf = buffer;  // actual buffer can be dynamic if static is short
  char *buf_act = buffer;
  char *last;
  uint num_items;  // number of items of `arr'
  size_t read_size;

  DBUG_TRACE;

  if ((read_size = my_b_gets(f, buf_act, size)) == 0) {
    return static_cast<long>(read_size);  // no line in master.info
  }
  if (read_size + 1 == size && buf[size - 2] != '\n') {
    /*
      short read happend; allocate sufficient memory and make the 2nd read
    */
    char buf_work[(sizeof(long) * 3 + 1) * 16];
    memcpy(buf_work, buf, sizeof(buf_work));
    num_items = atoi(my_strtok_r(buf_work, " ", &last));
    size_t snd_size;
    /*
      max size upper bound approximate estimation bases on the formula:
      (the items number + items themselves) *
          (decimal size + space) - 1 + `\n' + '\0'
    */
    size_t max_size = (1 + num_items) * (sizeof(long) * 3 + 1) + 1;
    if (!(buf_act = (char *)my_malloc(key_memory_Rpl_info_file_buffer, max_size,
                                      MYF(MY_WME))))
      return -1;
    *buffer_act = buf_act;
    memcpy(buf_act, buf, read_size);
    snd_size = my_b_gets(f, buf_act + read_size, max_size - read_size);
    if (snd_size == 0 ||
        ((snd_size + 1 == max_size - read_size) && buf[max_size - 2] != '\n')) {
      /*
        failure to make the 2nd read or short read again
      */
      return -1;
    }
  }
  return static_cast<long>(read_size);
}
