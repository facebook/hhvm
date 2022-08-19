/* Copyright (c) 2019 Percona LLC and/or its affiliates. All rights reserved.

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

#include <include/my_sqlcommand.h>
#include <sql/auth/sql_security_ctx.h>
#include <sql/protocol_classic.h>
#include <sql/sql_class.h>

/**
  We need it for merge_test_small.cc
*/
bool slave_high_priority_ddl = 0;
ulonglong slave_high_priority_lock_wait_timeout_nsec = 1.0;
std::atomic<ulonglong> slave_high_priority_ddl_killed_connections(0);
std::atomic<ulonglong> slave_high_priority_ddl_executed(0);
bool support_high_priority(enum enum_sql_command) noexcept { return false; }
bool Security_context::check_access(ulong, const std::string &, bool) {
  return false;
}

Vio *Protocol_classic::get_vio() { return nullptr; }
const Vio *Protocol_classic::get_vio() const { return nullptr; }

void THD::enter_stage(const PSI_stage_info *, PSI_stage_info *, const char *,
                      const char *, const unsigned int) SUPPRESS_TSAN {}
