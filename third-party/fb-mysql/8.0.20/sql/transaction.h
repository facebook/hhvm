/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"  // IWYU pragma: keep

class THD;
struct handlerton;

bool trans_check_state(THD *thd);
void trans_reset_one_shot_chistics(THD *thd);
void trans_track_end_trx(THD *thd);

bool trans_begin(THD *thd, uint flags = 0, bool *need_ok = nullptr,
                 handlerton *hton = nullptr);
bool trans_commit(THD *thd, bool ignore_global_read_lock = false);
bool trans_commit_implicit(THD *thd, bool ignore_global_read_lock = false);
bool trans_rollback(THD *thd);
bool trans_rollback_implicit(THD *thd);

bool trans_commit_stmt(THD *thd, bool ignore_global_read_lock = false);
bool trans_rollback_stmt(THD *thd);
bool trans_commit_attachable(THD *thd);

bool trans_savepoint(THD *thd, LEX_STRING name);
bool trans_rollback_to_savepoint(THD *thd, LEX_STRING name);
bool trans_release_savepoint(THD *thd, LEX_STRING name);

#endif /* TRANSACTION_H */
