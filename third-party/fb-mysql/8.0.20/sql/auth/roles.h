/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ROLES_H_INCLUDED
#define ROLES_H_INCLUDED

// Forward declarations
class THD;
struct LEX_USER;
template <class T>
class List;
class Security_context;
enum class role_enum;

namespace Roles {
class Role_activation {
 public:
  explicit Role_activation(THD *thd, Security_context *sctx, role_enum type,
                           const List<LEX_USER> *role_list = nullptr,
                           bool raise_error = true);
  Role_activation(const Role_activation &) = delete;
  Role_activation(Role_activation &&) = delete;
  Role_activation &operator=(const Role_activation &) = delete;
  Role_activation &operator=(Role_activation &&) = delete;
  ~Role_activation() { m_valid = false; }

  bool activate();

 private:
  bool activate_role_none();
  bool activate_role_default();
  bool activate_role_all();
  bool activate_role_name();

 private:
  THD *m_thd;
  Security_context *m_sctx;
  role_enum m_type;
  const List<LEX_USER> *m_role_list;
  bool m_raise_error;
  bool m_valid;
};
}  // namespace Roles

#endif /* ROLES_H_INCLUDED */
