/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/locking_service.h"

#include <string.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/service_parser.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"    // current_thd
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/mdl.h"            // MDL_request_list
#include "sql/sql_class.h"      // THD
#include "sql/sql_error.h"

/**
  We want to convert ER_LOCK_DEADLOCK error to ER_LOCK_SERVICE_DEADLOCK error.
  The former means that implicit rollback of transaction has occurred
  which doesn't (and should not) happen when we get deadlock while waiting
  for locking service locks.

  We also want to convert ER_LOCK_WAIT_TIMEOUT as this error indicates that
  it helps to restart the transaction. This does not apply to locking service
  locks.
*/

class Locking_service_deadlock_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_LOCK_DEADLOCK) {
      my_error(ER_LOCKING_SERVICE_DEADLOCK, MYF(0));
      return true;
    } else if (sql_errno == ER_LOCK_WAIT_TIMEOUT) {
      my_error(ER_LOCKING_SERVICE_TIMEOUT, MYF(0));
      return true;
    }

    return false;
  }
};

static const size_t MAX_LOCKING_SERVICE_LOCK_NAME_LENGTH = 64;

/**
  Check if the given name has valid length.

  @param name     Name to check (namespace or lock)

  @retval true if invalid, false otherwise.
 */

static inline bool check_lock_name(const char *name) {
  if (!name || strlen(name) == 0 ||
      strlen(name) > MAX_LOCKING_SERVICE_LOCK_NAME_LENGTH) {
    my_error(ER_LOCKING_SERVICE_WRONG_NAME, MYF(0), name);
    return true;
  }
  return false;
}

class Release_all_locking_service_locks : public MDL_release_locks_visitor {
 public:
  Release_all_locking_service_locks() {}

  virtual bool release(MDL_ticket *ticket) {
    return ticket->get_key()->mdl_namespace() == MDL_key::LOCKING_SERVICE;
  }
};

class Release_locking_service_locks : public MDL_release_locks_visitor {
 private:
  const char *m_lock_namespace;

 public:
  explicit Release_locking_service_locks(const char *lock_namespace)
      : m_lock_namespace(lock_namespace) {}

  virtual bool release(MDL_ticket *ticket) {
    return (ticket->get_key()->mdl_namespace() == MDL_key::LOCKING_SERVICE &&
            strcmp(m_lock_namespace, ticket->get_key()->db_name()) == 0);
  }
};

int acquire_locking_service_locks_nsec(MYSQL_THD opaque_thd,
                                       const char *lock_namespace,
                                       const char **lock_names, size_t lock_num,
                                       enum_locking_service_lock_type lock_type,
                                       Timeout_type lock_timeout_nsec) {
  if (lock_num == 0) return 0;

  // Check that namespace length is acceptable
  if (check_lock_name(lock_namespace)) return 1;

  THD *thd;
  if (opaque_thd)
    thd = static_cast<THD *>(opaque_thd);
  else
    thd = current_thd;

  // Initialize MDL_requests.
  MDL_request_list mdl_requests;
  for (size_t i = 0; i < lock_num; i++) {
    // Check that lock name length is acceptable
    if (check_lock_name(lock_names[i])) return 1;

    MDL_request *new_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT(
        new_request, MDL_key::LOCKING_SERVICE, lock_namespace, lock_names[i],
        (lock_type == LOCKING_SERVICE_READ ? MDL_SHARED : MDL_EXCLUSIVE),
        MDL_EXPLICIT);
    mdl_requests.push_front(new_request);
  }

  // Acquire locks
  Locking_service_deadlock_error_handler handler;
  thd->push_internal_handler(&handler);
  bool res =
      thd->mdl_context.acquire_locks_nsec(&mdl_requests, lock_timeout_nsec);
  thd->pop_internal_handler();
  if (res) return 1;

  return 0;
}

int release_locking_service_locks(MYSQL_THD opaque_thd,
                                  const char *lock_namespace) {
  // Check that namespace length is acceptable
  if (check_lock_name(lock_namespace)) return 1;

  THD *thd;
  if (opaque_thd)
    thd = static_cast<THD *>(opaque_thd);
  else
    thd = current_thd;

  Release_locking_service_locks lock_visitor(lock_namespace);
  thd->mdl_context.release_locks(&lock_visitor);

  return 0;
}

void release_all_locking_service_locks(THD *thd) {
  Release_all_locking_service_locks lock_visitor;
  thd->mdl_context.release_locks(&lock_visitor);
}

/*
  Wrapper functions for the plugin service API. The UDF implementation
  cannot call these as we get name conficts with the macros defined
  in service_locking.h as UDFs are built with MYSQL_DYNAMIC_PLUGIN
  yet are not able to call service API functions.
*/
int mysql_acquire_locking_service_locks_nsec(
    MYSQL_THD opaque_thd, const char *lock_namespace, const char **lock_names,
    size_t lock_num, enum_locking_service_lock_type lock_type,
    Timeout_type lock_timeout_nsec) {
  return acquire_locking_service_locks_nsec(opaque_thd, lock_namespace,
                                            lock_names, lock_num, lock_type,
                                            lock_timeout_nsec);
}

int mysql_release_locking_service_locks(MYSQL_THD opaque_thd,
                                        const char *lock_namespace) {
  return release_locking_service_locks(opaque_thd, lock_namespace);
}
