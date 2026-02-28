/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_import.h"

#include <sys/types.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/mysql_lex_string.h"
#include "prealloced_array.h"  // Prealloced_array
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client::Auto_releaser
#include "sql/dd/impl/sdi_utils.h"  // dd::sdi_utils::handle_errors
#include "sql/dd/sdi_api.h"         // dd::sdi::Import_target
#include "sql/dd/sdi_file.h"        // dd::sdi_file::expand_pattern
#include "sql/dd/string_type.h"     // dd::String_type
#include "sql/mdl.h"                // MDL_request
#include "sql/mysqld.h"             // is_secure_file_path
#include "sql/psi_memory_key.h"     // key_memory_DD_import
#include "sql/sql_backup_lock.h"    // acquire_shared_backup_lock
#include "sql/sql_class.h"          // THD
#include "sql/sql_error.h"
#include "sql/stateless_allocator.h"
#include "sql/system_variables.h"
#include "sql/transaction.h"  // trans_rollback_stmt

namespace {

typedef Prealloced_array<dd::sdi::Import_target, 5> Targets_type;

}  // namespace

Sql_cmd_import_table::Sql_cmd_import_table(
    const Sdi_patterns_type &sdi_patterns)
    : m_sdi_patterns(sdi_patterns) {}

bool Sql_cmd_import_table::execute(THD *thd) {
  DBUG_ASSERT(!m_sdi_patterns.empty());

  auto rbgrd = dd::sdi_utils::make_guard(thd, [&](THD *) {
    trans_rollback_stmt(thd);
    trans_rollback(thd);
  });

  // Need to keep this alive until after commit/rollback has been done
  dd::cache::Dictionary_client::Auto_releaser ar{thd->dd_client()};

  if (check_access(thd, FILE_ACL, nullptr, nullptr, nullptr, false, false)) {
    return true;
  }

  // Convert supplied sdi patterns into path,in_datadir pairs
  dd::sdi_file::Paths_type paths{key_memory_DD_import};
  paths.reserve(m_sdi_patterns.size());
  for (auto &pattern : m_sdi_patterns) {
    if (thd->charset() == files_charset_info) {
      if (dd::sdi_file::expand_pattern(thd, pattern, &paths)) {
        return true;
      }
      continue;
    }

    LEX_STRING converted;
    if (thd->convert_string(&converted, files_charset_info, pattern.str,
                            pattern.length, thd->charset())) {
      return true;
    }

    if (dd::sdi_file::expand_pattern(thd, converted, &paths)) {
      return true;
    }
  }

  Targets_type targets{key_memory_DD_import};

  auto tgtgrd = dd::sdi_utils::make_guard(thd, [&](THD *) {
    for (auto &tgt : targets) {
      tgt.rollback();
    }
  });

  for (auto &p : paths) {
    // Move the path string from paths to avoid copy - paths is now
    // empty shell
    targets.emplace_back(std::move(p.first), p.second);
  }
  // Have a valid list of sdi files to import

  dd::String_type shared_buffer;
  MDL_request_list mdl_requests;
  for (auto &t : targets) {
    if (t.load(thd, &shared_buffer)) {
      return true;
    }

    if (check_privileges(thd, t)) {
      return true;
    }

    mdl_requests.push_front(mdl_request(t, thd->mem_root));
  }
  // Table objects and their schema names have been loaded, privileges
  // checked and EXCLUSIVE MDL requests for the tables been added to
  // mdl_requests.

  std::vector<dd::String_type> schema_names;
  schema_names.reserve(targets.size());
  for (auto &t : targets) {
    schema_names.push_back(*t.can_schema_name());
  }
  std::sort(schema_names.begin(), schema_names.end());
  auto uniq_end = std::unique(schema_names.begin(), schema_names.end());
  schema_names.erase(uniq_end, schema_names.end());

  for (auto &sn : schema_names) {
    MDL_request *r = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT(r, MDL_key::SCHEMA, sn.c_str(), "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
    mdl_requests.push_front(r);
  }

  MDL_request *mdl_request_for_backup_lock = new (thd->mem_root) MDL_request;
  MDL_REQUEST_INIT(mdl_request_for_backup_lock, MDL_key::BACKUP_LOCK, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  mdl_requests.push_front(mdl_request_for_backup_lock);

  // If we cannot acquire protection against GRL, err out.
  if (thd->global_read_lock.can_acquire_protection()) return true;

  MDL_request *mdl_request_for_grl = new (thd->mem_root) MDL_request;
  MDL_REQUEST_INIT(mdl_request_for_grl, MDL_key::GLOBAL, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  mdl_requests.push_front(mdl_request_for_grl);

  if (thd->mdl_context.acquire_locks_nsec(
          &mdl_requests, thd->variables.lock_wait_timeout_nsec)) {
    return true;
  }

  // Now when we have protection against concurrent change of read_only
  // option we can safely re-check its value.
  if (check_readonly(thd, true)) return true;

  // Now we have MDL on all schemas and tables involved

  for (auto &t : targets) {
    if (t.store_in_dd(thd)) {
      return true;
    }
  }

  rbgrd.release();
  tgtgrd.release();

  // Downgrade failing delete_file errors to warning, and
  // allow the transaction to commit.
  dd::sdi_utils::handle_errors(
      thd,
      [](uint, const char *, Sql_condition::enum_severity_level *level,
         const char *) {
        (*level) = Sql_condition::SL_WARNING;
        return false;
      },
      [&]() {
        for (auto &tgt : targets) {
          (void)tgt.commit();
        }
        return false;
      });

  my_ok(thd);
  return trans_commit_stmt(thd) || trans_commit(thd);
}

enum_sql_command Sql_cmd_import_table::sql_command_code() const {
  return SQLCOM_IMPORT;
}
