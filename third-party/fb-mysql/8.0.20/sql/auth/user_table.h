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

#ifndef USER_TABLE_INCLUDED
#define USER_TABLE_INCLUDED

#include "my_inttypes.h"

// Forward declarations
class Restrictions;
class THD;
struct LEX_USER;
struct TABLE;

namespace acl_table {

const ulong USER_ATTRIBUTE_NONE = 0L;
const ulong USER_ATTRIBUTE_RETAIN_PASSWORD = (1L << 0);
const ulong USER_ATTRIBUTE_DISCARD_PASSWORD = (1L << 1);
const ulong USER_ATTRIBUTE_RESTRICTIONS = (1L << 3);
const ulong USER_ATTRIBUTE_FAILED_LOGIN_ATTEMPTS = (1L << 4);
const ulong USER_ATTRIBUTE_PASSWORD_LOCK_TIME = (1L << 5);

class Pod_user_what_to_update {
 public:
  Pod_user_what_to_update() : m_what(0L), m_user_attributes(0L) {}
  ulong m_what;
  ulong m_user_attributes;
};
}  // namespace acl_table

int replace_user_table(THD *thd, TABLE *table, LEX_USER *combo, ulong rights,
                       bool revoke_grant, bool can_create_user,
                       acl_table::Pod_user_what_to_update &what_to_update,
                       Restrictions *restrictions = nullptr);

bool read_user_table(THD *thd, TABLE *table);
#endif /* USER_TABLE_INCLUDED */
