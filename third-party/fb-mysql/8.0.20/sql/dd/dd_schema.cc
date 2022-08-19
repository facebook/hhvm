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

#include "sql/dd/dd_schema.h"

#include <atomic>
#include <memory>  // unique_ptr

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_time.h"  // TIME_to_ulonglong_datetime
#include "mysql_com.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd.h"                       // dd::get_dictionary
#include "sql/dd/impl/utils.h"               // dd::my_time_t_to_ull_datetime()
#include "sql/dd/types/schema.h"             // dd::Schema
#include "sql/item_create.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"     // lower_case_table_names
#include "sql/sql_class.h"  // THD
#include "sql/system_variables.h"
#include "sql/tztime.h"  // Time_zone

namespace dd {

bool schema_exists(THD *thd, const char *schema_name, bool *exists) {
  DBUG_TRACE;

  // We must make sure the schema is released and unlocked in the right order.
  Schema_MDL_locker mdl_handler(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *sch = nullptr;
  bool error = mdl_handler.ensure_locked(schema_name) ||
               thd->dd_client()->acquire(schema_name, &sch);
  DBUG_ASSERT(exists);
  *exists = (sch != nullptr);
  // Error has been reported by the dictionary subsystem.
  return error;
}

bool create_schema(THD *thd, const char *schema_name,
                   const CHARSET_INFO *charset_info, bool default_encryption,
                   const char *db_metadata) {
  // Create dd::Schema object.
  std::unique_ptr<dd::Schema> schema(dd::create_object<dd::Schema>());

  // Set schema name and collation id.
  schema->set_name(schema_name);
  DBUG_ASSERT(charset_info);
  schema->set_default_collation_id(charset_info->number);
  schema->set_default_encryption(default_encryption);
  schema->set_db_metadata(db_metadata != nullptr ? db_metadata : "");

  // Get statement start time.
  ulonglong ull_curtime = my_time_t_to_ull_datetime(thd->query_start_in_secs());

  schema->set_created(ull_curtime);
  schema->set_last_altered(ull_curtime);

  // Store the schema. Error will be reported by the dictionary subsystem.
  return thd->dd_client()->store(schema.get());
}

bool mdl_lock_schema(THD *thd, const char *schema_name,
                     enum_mdl_duration duration, MDL_ticket **ticket) {
  /*
    Make sure we have at least an IX lock on the schema name.
    Acquire a lock unless we already have it.
  */
  char name_buf[NAME_LEN + 1];
  const char *converted_name = schema_name;
  if (lower_case_table_names == 2) {
    // Lower case table names == 2 is tested on OSX.
    /* purecov: begin tested */
    my_stpcpy(name_buf, converted_name);
    my_casedn_str(&my_charset_utf8_tolower_ci, name_buf);
    converted_name = name_buf;
    /* purecov: end */
  }

  // If we do not already have one, acquire a new lock.
  if (thd->mdl_context.owns_equal_or_stronger_lock(
          MDL_key::SCHEMA, converted_name, "", MDL_INTENTION_EXCLUSIVE)) {
    return false;
  }

  // Create a request for an IX_lock on the converted schema name.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::SCHEMA, converted_name, "",
                   MDL_INTENTION_EXCLUSIVE, duration);

  /*
    Acquire the lock request created above, and check if
    acquisition fails (e.g. timeout or deadlock).
  */
  if (thd->mdl_context.acquire_lock_nsec(
          &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }
  if (ticket != nullptr) {
    *ticket = mdl_request.ticket;
  }
  return false;
}

bool Schema_MDL_locker::ensure_locked(const char *schema_name) {
  return mdl_lock_schema(m_thd, schema_name, MDL_EXPLICIT, &m_ticket);
}

Schema_MDL_locker::~Schema_MDL_locker() {
  if (m_ticket) m_thd->mdl_context.release_lock(m_ticket);
}

}  // namespace dd
