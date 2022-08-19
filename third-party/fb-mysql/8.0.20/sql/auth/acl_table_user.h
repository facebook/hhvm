/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ACL_TABLE_USER_INCLUDED
#define ACL_TABLE_USER_INCLUDED

#include "my_config.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/types.h>
#include <memory>
#include <string>
#include <utility>

#include "my_alloc.h"
#include "sql/auth/acl_table_base.h"
#include "sql/auth/partial_revokes.h"
#include "sql/auth/user_table.h"

class ACL_USER;
class RowIterator;
class THD;
class User_table_schema;
struct LEX_USER;
struct TABLE;

namespace acl_table {
enum class User_attribute_type {
  ADDITIONAL_PASSWORD = 0,
  RESTRICTIONS,
  PASSWORD_LOCKING
};

struct Password_lock {
  /**
     read from the user config. The number of days to keep the accont locked
  */
  int password_lock_time_days;
  /**
    read from the user config. The number of failed login attemps before the
    account is locked
  */
  uint failed_login_attempts;

  Password_lock();

  Password_lock &operator=(const Password_lock &other);

  Password_lock &operator=(Password_lock &&other);

  Password_lock(const Password_lock &other);

  Password_lock(Password_lock &&other);
};

// Forward and alias declarations
using acl_table_user_writer_status =
    std::pair<Table_op_error_code, struct timeval>;

/**
  mysql.user table writer. It updates or drop a one single row from the table.
*/

class Acl_table_user_writer_status {
 public:
  Acl_table_user_writer_status(MEM_ROOT *mem_root);
  Acl_table_user_writer_status(bool skip, ulong rights, Table_op_error_code err,
                               struct timeval pwd_timestamp, std::string cred,
                               MEM_ROOT *mem_root, Password_lock &password_lock)
      : skip_cache_update(skip),
        updated_rights(rights),
        error(err),
        password_change_timestamp(pwd_timestamp),
        second_cred(cred),
        restrictions(mem_root),
        password_lock(password_lock) {}

  bool skip_cache_update;
  ulong updated_rights;
  Table_op_error_code error;
  struct timeval password_change_timestamp;
  std::string second_cred;
  Restrictions restrictions;
  Password_lock password_lock;
};

class Acl_table_user_writer : public Acl_table {
 public:
  Acl_table_user_writer(THD *thd, TABLE *table, LEX_USER *combo, ulong rights,
                        bool revoke_grant, bool can_create_user,
                        Pod_user_what_to_update what_to_update,
                        Restrictions *restrictions = nullptr);
  virtual ~Acl_table_user_writer();
  virtual Acl_table_op_status finish_operation(Table_op_error_code &error);
  Acl_table_user_writer_status driver();

  bool setup_table(int &error, bool &builtin_password);

  /* Set of functions to set user table data */
  bool update_authentication_info(Acl_table_user_writer_status &return_value);
  bool update_privileges(Acl_table_user_writer_status &return_value);
  bool update_ssl_properties();
  bool update_user_resources();
  bool update_password_expiry();
  bool update_account_locking();
  bool update_password_history();
  bool update_password_reuse();
  bool update_password_require_current();
  bool update_user_attributes(std::string &current_password,
                              Acl_table_user_writer_status &return_value);

  ulong get_user_privileges();
  std::string get_current_credentials();

 private:
  LEX_USER *m_combo;
  ulong m_rights;
  bool m_revoke_grant;
  bool m_can_create_user;
  Pod_user_what_to_update m_what_to_update;
  User_table_schema *m_table_schema;
  Restrictions *m_restrictions;
};

/**
  mysql.user table reader. It reads all raws from table and create in-memory
  cache.
*/

class Acl_table_user_reader : public Acl_table {
 public:
  Acl_table_user_reader(THD *thd, TABLE *table);
  ~Acl_table_user_reader();
  bool driver();
  bool setup_table(bool &is_old_db_layout);
  bool read_row(bool &is_old_db_layout, bool &super_users_with_empty_plugin);
  virtual Acl_table_op_status finish_operation(Table_op_error_code &error);

  /* Set of function to read user table data */
  void reset_acl_user(ACL_USER &user);
  void read_account_name(ACL_USER &user);
  bool read_authentication_string(ACL_USER &user);
  void read_privileges(ACL_USER &user);
  void read_ssl_fields(ACL_USER &user);
  void read_user_resources(ACL_USER &user);
  bool read_plugin_info(ACL_USER &user, bool &super_users_with_empty_plugin,
                        bool &is_old_db_layout);
  bool read_password_expiry(ACL_USER &user, bool &password_expired);
  void read_password_locked(ACL_USER &user);
  void read_password_last_changed(ACL_USER &user);
  void read_password_lifetime(ACL_USER &user);
  void read_password_history_fields(ACL_USER &user);
  void read_password_reuse_time_fields(ACL_USER &user);
  void read_password_require_current(ACL_USER &user);
  bool read_user_attributes(ACL_USER &user);
  void add_row_to_acl_users(ACL_USER &user);

 private:
  User_table_schema *m_table_schema;
  unique_ptr_destroy_only<RowIterator> m_iterator;
  MEM_ROOT m_mem_root;
  Restrictions *m_restrictions;
};

}  // namespace acl_table
#endif /* ACL_TABLE_USER_INCLUDED */
