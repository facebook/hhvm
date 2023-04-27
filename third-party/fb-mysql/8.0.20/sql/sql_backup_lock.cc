/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql_backup_lock.h"

#include <utility>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"               // ER_SPECIFIC_ACCESS_DENIED_ERROR
#include "sql/auth/sql_security_ctx.h"  // Security_context
#include "sql/mdl.h"
#include "sql/system_variables.h"
#include "sql_class.h"  // THD

/**
  Check if a current user has the privilege BACKUP_ADMIN required to run
  the statement LOCK INSTANCE FOR BACKUP.

  @param thd    Current thread

  @retval false  A user has the privilege BACKUP_ADMIN
  @retval true   A user doesn't have the privilege BACKUP_ADMIN
*/

static bool check_backup_admin_privilege(THD *thd) {
  Security_context *sctx = thd->security_context();

  if (!sctx->has_global_grant(STRING_WITH_LEN("BACKUP_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "BACKUP_ADMIN");
    return true;
  }

  return false;
}

bool Sql_cmd_lock_instance::execute(THD *thd) {
  if (check_backup_admin_privilege(thd) ||
      acquire_exclusive_backup_lock_nsec(
          thd,
          DBUG_EVALUATE_IF("stop_slave_dont_release_backup_lock", 5,
                           thd->variables.lock_wait_timeout_nsec),
          false))
    return true;

  my_ok(thd);
  return false;
}

bool Sql_cmd_unlock_instance::execute(THD *thd) {
  if (check_backup_admin_privilege(thd)) return true;

  release_backup_lock(thd);

  my_ok(thd);
  return false;
}

/**
  Acquire either exclusive or shared Backup Lock.

  @param[in] thd                    Current thread context
  @param[in] mdl_type               Type of metadata lock to acquire for backup
  @param[in] mdl_duration           Duration of metadata lock
  @param[in] lock_wait_timeout      How many seconds to wait before timeout.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

static bool acquire_mdl_for_backup_nsec(THD *thd, enum_mdl_type mdl_type,
                                        enum_mdl_duration mdl_duration,
                                        ulonglong lock_wait_timeout_nsec) {
  MDL_request mdl_request;

  DBUG_ASSERT(mdl_type == MDL_SHARED || mdl_type == MDL_INTENTION_EXCLUSIVE);

  MDL_REQUEST_INIT(&mdl_request, MDL_key::BACKUP_LOCK, "", "", mdl_type,
                   mdl_duration);

  return thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                            lock_wait_timeout_nsec);
}

/**
  MDL_release_locks_visitor subclass to release MDL for BACKUP_LOCK.
*/

class Release_all_backup_locks : public MDL_release_locks_visitor {
 public:
  virtual bool release(MDL_ticket *ticket) {
    return ticket->get_key()->mdl_namespace() == MDL_key::BACKUP_LOCK;
  }
};

void release_backup_lock(THD *thd) {
  Release_all_backup_locks lock_visitor;
  thd->mdl_context.release_locks(&lock_visitor);
}

/*
  The following rationale is for justification of choice for specific lock
  types to support BACKUP LOCK.

  IX and S locks are mutually incompatible. On the other hand both these
  lock types are compatible with themselves. IX lock has lower priority
  than S locks. So we can use S lock for rare Backup operation which should
  not be starved by more frequent DDL operations using IX locks.

  From all listed above follows that S lock should be considered as Exclusive
  Backup Lock and IX lock should be considered as Shared Backup Lock.
*/

bool acquire_exclusive_backup_lock_nsec(THD *thd,
                                        ulonglong lock_wait_timeout_nsec,
                                        bool for_trx) {
  enum_mdl_duration duration = (for_trx ? MDL_TRANSACTION : MDL_EXPLICIT);
  return acquire_mdl_for_backup_nsec(thd, MDL_SHARED, duration,
                                     lock_wait_timeout_nsec);
}

bool acquire_shared_backup_lock_nsec(THD *thd, ulong lock_wait_timeout_nsec,
                                     bool for_trx) {
  enum_mdl_duration duration = (for_trx ? MDL_TRANSACTION : MDL_EXPLICIT);
  return acquire_mdl_for_backup_nsec(thd, MDL_INTENTION_EXCLUSIVE, duration,
                                     lock_wait_timeout_nsec);
}

Is_instance_backup_locked_result is_instance_backup_locked(THD *thd) {
  Is_instance_backup_locked_result res;
  MDL_key key(MDL_key::BACKUP_LOCK, "", "");
  MDL_lock_is_owned_visitor backup_lock_owner;

  if (thd->mdl_context.find_lock_owner(&key, &backup_lock_owner))
    res = Is_instance_backup_locked_result::OOM;
  else {
    if (backup_lock_owner.exists())
      res = Is_instance_backup_locked_result::LOCKED;
    else
      res = Is_instance_backup_locked_result::NOT_LOCKED;
  }

  return res;
}
