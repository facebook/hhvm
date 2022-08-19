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

#ifndef RPL_INFO_HANDLER_H
#define RPL_INFO_HANDLER_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>
#include <type_traits>

#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"

class Rpl_info_values;
class Server_ids;

enum enum_info_repository {
  INFO_REPOSITORY_FILE = 0,
  INFO_REPOSITORY_TABLE,
  INFO_REPOSITORY_DUMMY,
  /*
    Add new types of repository before this
    entry.
  */
  INVALID_INFO_REPOSITORY
};

/*
  Defines status on the repository.
*/
enum enum_return_check {
  REPOSITORY_DOES_NOT_EXIST = 1,
  REPOSITORY_EXISTS = 2,
  ERROR_CHECKING_REPOSITORY = 4,
  REPOSITORY_CLEARED = 10
};

class Rpl_info_handler {
  friend class Rpl_info_factory;

 public:
  /*
   * enum class to indicate the status of getting field using
   * do_get_info() of handler classes.
   */
  enum class enum_field_get_status : int {
    /* status is success but field has NULL value. */
    FIELD_VALUE_NOT_NULL = 0,
    FIELD_VALUE_IS_NULL = 1,
    FAILURE = 2
  };

  Rpl_info_handler(const Rpl_info_handler &handler) = delete;
  Rpl_info_handler &operator=(const Rpl_info_handler &handler) = delete;

  /**
    After creating an object and assembling components, this method is
    used to initialize internal structures. Everything that does not
    depend on other components (e.g. mutexes) should be placed in the
    object's constructor though.

    @retval false success,
    @retval true  otherwise error.
  */
  int init_info() { return do_init_info(); }

  /**
    Checks the repository's status.

    @retval REPOSITORY_EXISTS         reposistory is ready to
                                      be used.
    @retval REPOSITORY_DOES_NOT_EXIST repository needs to be
                                      configured.
    @retval ERROR_CHECKING_REPOSITORY error while checking the
                                      reposistory.
  */
  enum_return_check check_info() { return do_check_info(); }

  /**
    Flushes and syncs in-memory information into a stable storage (i.e.
    repository). Usually, syncing after flushing depends on other options
    such as @c relay-log-info-sync, @c master-info-sync. These options
    dictate after how many events or transactions the information
    should be synced. We can ignore them and always sync by setting the
    parameter @c force, which is by default @c false, to @c true.

    So if the number of events is below a threshold, the parameter
    @c force is false and we are using a file system as a storage
    system, it may happen that the changes will only end up in the
    operating system's cache and a crash may lead to inconsistencies.

    @retval false No error
    @retval true  Failure
  */
  int flush_info(const bool force) { return do_flush_info(force); }

  /**
    Deletes any information in it and in some cases the repository.
    The decision to remove the repository is delegated to the
    developer.

    @retval false No error
    @retval true  Failure
  */
  int remove_info() { return do_remove_info(); }

  /**
    Deletes any information in the repository. In contrast to the
    @c remove_info() method, the repository is not removed.

    @retval false No error
    @retval true  Failure
  */
  int clean_info() { return do_clean_info(); }

  /**
    Closes access to the repository.

    @retval false No error
    @retval true  Failure
  */
  void end_info() { do_end_info(); }

  /**
    Enables the storage system to receive reads, i.e.
    getters.

    @retval false No error
    @retval true  Failure
  */
  int prepare_info_for_read() { return (do_prepare_info_for_read()); }

  /**
    Enables the storage system to receive writes, i.e.
    setters.

    @retval false No error
    @retval true  Failure
  */
  int prepare_info_for_write() { return (do_prepare_info_for_write()); }

  /**
    Gets the type of the repository that is used.

    @return Type of repository.
  */
  uint get_rpl_info_type() { return (do_get_rpl_info_type()); }
  /**
     Returns a string corresponding to the type.
  */
  const char *get_rpl_info_type_str();

  /**
    Sets the value of a field to @c value.
    Any call must be done in the right order which
    is defined by the caller that wants to persist
    the information.

    @param[in] value Value to be set.

    @retval false No error
    @retval true Failure
  */
  template <class TypeHandler>
  bool set_info(TypeHandler const value) {
    if (cursor >= ninfo || prv_error) return true;

    if (!(prv_error = do_set_info(cursor, value))) cursor++;

    return (prv_error);
  }

  template <class TypeHandler>
  bool set_info(TypeHandler const value, const size_t size) {
    if (cursor >= ninfo || prv_error) return true;

    if (!(prv_error = do_set_info(cursor, value, size))) cursor++;

    return (prv_error);
  }

  /**
    set the value of a field pointed at @c pk_cursor to
    @ value.

    @param[in]   pk_cursor   cursor for the filed value.
    @param[in]   value       fieled[pk_cursor] would be set
                             this value.

    @retval      false       ok
    @retval      true       error.
  */

  template <class TypeHandler>
  bool set_info(int pk_cursor, TypeHandler const value) {
    if (pk_cursor >= ninfo) return true;

    return (do_set_info(pk_cursor, value));
  }

  /**
    Sets all the values in args in specified format.

    @param n      number of args after format
    @param format print format of args

    @return false No Error
    @return true  Failure

    Note that this function should be only used with FILE repository.
  */
  bool set_info(int n, const char *format, ...) {
    DBUG_ASSERT(get_rpl_info_type() == INFO_REPOSITORY_FILE);
    va_list args;
    va_start(args, format);
    if (cursor >= ninfo || prv_error) {
      va_end(args);
      return true;
    }
    if (!(prv_error = do_set_info(format, args))) cursor += n;
    va_end(args);
    return (prv_error);
  }

  /**
    Returns the value of a field.
    Any call must be done in the right order which
    is defined by the caller that wants to return
    the information.

    @param[in] value Value to be set.
    @param[in] default_value Returns a default value
                             if the field is empty.

    @retval false No error
    @retval true Failure
  */
  template <class TypeHandlerPointer, class TypeHandler>
  enum_field_get_status get_info(TypeHandlerPointer value,
                                 TypeHandler const default_value) {
    if (cursor >= ninfo || prv_get_error == enum_field_get_status::FAILURE)
      return enum_field_get_status::FAILURE;

    if ((prv_get_error = do_get_info(cursor, value, default_value)) !=
        enum_field_get_status::FAILURE)
      cursor++;

    return (prv_get_error);
  }

  /**
    Returns the value of a string field.
    Any call must be done in the right order which
    is defined by the caller that wants to return
    the information.

    @param[in] value Value to be returned.
    @param[in] size  Max size of the string to be
                     returned.
    @param[in] default_value Returns a default value
                             if the field is empty.

    TypeHandler is either char* or uchar*, while
    default_value is const char* or const uchar*.
    Some type trait magic is required to make
    char* / uchar* into const char* / uchar*.

    @retval false No error
    @retval true Failure
  */
  template <class TypeHandler>
  enum_field_get_status get_info(
      TypeHandler value, const size_t size,
      std::add_const_t<std::remove_pointer_t<TypeHandler>> *default_value) {
    if (cursor >= ninfo || prv_get_error == enum_field_get_status::FAILURE)
      return enum_field_get_status::FAILURE;

    if ((prv_get_error = do_get_info(cursor, value, size, default_value)) !=
        enum_field_get_status::FAILURE)
      cursor++;

    return (prv_get_error);
  }

  /**
    Returns the value of a Server_id field.
    Any call must be done in the right order which
    is defined by the caller that wants to return
    the information.

    @param[out] value Value to be return.
    @param[in] default_value Returns a default value
                             if the field is empty.

    @retval false No error
    @retval true Failure
  */
  enum_field_get_status get_info(Server_ids *value,
                                 const Server_ids *default_value) {
    if (cursor >= ninfo || prv_get_error == enum_field_get_status::FAILURE)
      return enum_field_get_status::FAILURE;

    if ((prv_get_error = do_get_info(cursor, value, default_value)) !=
        enum_field_get_status::FAILURE)
      cursor++;

    return (prv_get_error);
  }

  /**
    Returns the number of fields handled by this handler.

    @return Number of fields handled by the handler.
  */
  int get_number_info() { return ninfo; }

  /**
    Configures the number of events after which the info (e.g.
    master info, relay log info) must be synced when flush() is
    called.

    @param[in] period Number of events.
  */
  void set_sync_period(uint period);

  /**
    Increments sync_counter.
  */
  void inc_sync_counter() { sync_counter++; }

  /**
    Returns sync_counter.

    @return sync_counter
  */
  uint get_sync_counter() { return sync_counter; }

  /**
    Returns a string describing the repository. For instance, if the
    repository is a file, the returned string is path where data is
    stored.

    @return a pointer to a string.
  */
  char *get_description_info() { return (do_get_description_info()); }

  /**
    Any transactional repository may have its updates rolled back in case
    of a failure. If this is possible, the repository is classified as
    transactional.

    @retval true If transactional.
    @retval false Otherwise.
  */
  bool is_transactional() { return do_is_transactional(); }

  /**
    Updates the value returned by the member function is_transactional()
    because it may be expensive to compute it whenever is_transactional()
    is called.

    In the current implementation, the type of the repository can only be
    changed when replication, i.e. slave, is stopped. For that reason,
    this member function, i.e. update_is__transactional(), must be called
    when slave is starting.

    @retval false No error
    @retval true Failure
  */
  bool update_is_transactional() { return do_update_is_transactional(); }

  /*
    Pre-store information before writing it to the repository and if
    necessary after reading it from the repository. The decision is
    delegated to the sub-classes.
  */
  Rpl_info_values *field_values;

  virtual ~Rpl_info_handler();

 protected:
  /* Number of fields to be stored in the repository. */
  int ninfo;

  /* From/To where we should start reading/writing. */
  int cursor;

  /* Registers if there was failure while accessing a field/information. */
  bool prv_error;
  enum_field_get_status prv_get_error;
  /*
   Keeps track of the number of events before fsyncing. The option
   --sync-master-info and --sync-relay-log-info determine how many
   events should be processed before fsyncing.
  */
  uint sync_counter;

  /*
   The number of events after which we should fsync.
  */
  uint sync_period;

  /**
    Bitset holding which of the fields are allowed to be `NULL`.
   */
  MY_BITMAP nullable_fields;

  Rpl_info_handler(const int nparam, MY_BITMAP const *nullable_bitmap);

  /**
    Checks whether or not the field at position `pos` is allowed to be `NULL`.

    @return true if the field is allowed to be `NULL` and false otherwise.
   */
  bool is_field_nullable(int pos);

 private:
  virtual int do_init_info() = 0;
  virtual int do_init_info(uint instance) = 0;
  virtual enum_return_check do_check_info() = 0;
  virtual enum_return_check do_check_info(uint instance) = 0;
  virtual int do_flush_info(const bool force) = 0;
  virtual int do_remove_info() = 0;
  virtual int do_clean_info() = 0;
  virtual void do_end_info() = 0;
  virtual int do_prepare_info_for_read() = 0;
  virtual int do_prepare_info_for_write() = 0;

  virtual bool do_set_info(const char *format, va_list args) = 0;
  virtual bool do_set_info(const int pos, const char *value) = 0;
  virtual bool do_set_info(const int pos, const uchar *value,
                           const size_t size) = 0;
  virtual bool do_set_info(const int pos, const ulong value) = 0;
  virtual bool do_set_info(const int pos, const int value) = 0;
  virtual bool do_set_info(const int pos, const float value) = 0;
  virtual bool do_set_info(const int pos, const Server_ids *value) = 0;
  virtual bool do_set_info(const int pos, const std::nullptr_t value) = 0;
  virtual bool do_set_info(const int pos, const std::nullptr_t value,
                           const size_t size) = 0;
  virtual enum_field_get_status do_get_info(const int pos, char *value,
                                            const size_t size,
                                            const char *default_value) = 0;
  virtual enum_field_get_status do_get_info(const int pos, uchar *value,
                                            const size_t size,
                                            const uchar *default_value) = 0;
  virtual enum_field_get_status do_get_info(const int pos, ulong *value,
                                            const ulong default_value) = 0;
  virtual enum_field_get_status do_get_info(const int pos, int *value,
                                            const int default_value) = 0;
  virtual enum_field_get_status do_get_info(const int pos, float *value,
                                            const float default_value) = 0;
  virtual enum_field_get_status do_get_info(
      const int pos, Server_ids *value, const Server_ids *default_value) = 0;
  virtual char *do_get_description_info() = 0;
  virtual bool do_is_transactional() = 0;
  virtual bool do_update_is_transactional() = 0;
  virtual uint do_get_rpl_info_type() = 0;
};

bool operator!(Rpl_info_handler::enum_field_get_status status);

#ifndef DBUG_OFF
extern ulong w_rr;
extern uint mts_debug_concurrent_access;
#endif
#endif /* RPL_INFO_HANDLER_H */
