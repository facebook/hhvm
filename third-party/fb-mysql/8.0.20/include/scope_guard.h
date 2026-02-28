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

#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

template <typename TLambda>
class Scope_guard {
 public:
  Scope_guard(const TLambda &rollback_lambda)
      : m_committed(false), m_rollback_lambda(rollback_lambda) {}
  Scope_guard(const Scope_guard<TLambda> &) = delete;
  Scope_guard(Scope_guard<TLambda> &&moved)
      : m_committed(moved.m_committed),
        m_rollback_lambda(moved.m_rollback_lambda) {
    /* Set moved guard to "invalid" state, the one in which the rollback lambda
      will not be executed. */
    moved.m_committed = true;
  }
  ~Scope_guard() {
    if (!m_committed) {
      m_rollback_lambda();
    }
  }

  inline void commit() { m_committed = true; }

  inline void rollback() {
    if (!m_committed) {
      m_rollback_lambda();
      m_committed = true;
    }
  }

 private:
  bool m_committed;
  const TLambda m_rollback_lambda;
};

template <typename TLambda>
Scope_guard<TLambda> create_scope_guard(const TLambda rollback_lambda) {
  return Scope_guard<TLambda>(rollback_lambda);
}

#endif /* SCOPE_GUARD_H */
