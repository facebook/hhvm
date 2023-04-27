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

#ifndef DD_KILL_IMMUNIZER_INCLUDED
#define DD_KILL_IMMUNIZER_INCLUDED

#include "mutex_lock.h"     // MUTEX_LOCK
#include "sql/sql_class.h"  // THD

namespace dd {

/**
  RAII class for immunizing the THD from kill operations.

  Interruptions to operations on new Data Dictionary tables due to KILL QUERY,
  KILL CONNECTION or statement execution timeout would leave DD in inconsistent
  state. So the operations on the New Data Dictionary tables are made immune to
  these operations using DD_kill_immunizer.

  Note:
    DD operations are made immune to KILL operations till WL#7743 and WL#7016
    are implemented. So as part of these WL's DD_kill_immunizer should be
    removed.
*/

class DD_kill_immunizer {
 public:
  DD_kill_immunizer(THD *thd)
      : m_thd(thd), m_killed_state(THD::NOT_KILLED), m_is_active(true) {
    MUTEX_LOCK(thd_data_lock, &thd->LOCK_thd_data);

    // If DD_kill_immunizer is initialized as part of nested Transaction_ro's
    // then store reference to parent kill_immunizer else NULL value is saved in
    // m_saved_kill_immunizer.
    m_saved_kill_immunizer = thd->kill_immunizer;

    // Save either thd->killed value or parent kill_immunizer's m_killed_state
    // value.
    m_saved_killed_state = thd->kill_immunizer
                               ? thd->kill_immunizer->m_killed_state
                               : thd->killed.load();

    // Set current DD_kill_immunizer object to THD.
    thd->kill_immunizer = this;

    // Set killed state of THD as NOT_KILLED.
    thd->killed = THD::NOT_KILLED;
  }

  ~DD_kill_immunizer() {
    MUTEX_LOCK(thd_data_lock, &m_thd->LOCK_thd_data);

    /*
      Current instance is of top level kill immunizer, set kill immune mode to
      inactive(or exiting).
    */
    if (m_saved_kill_immunizer == nullptr)
      m_is_active = false;
    else
      // Set kill_immunizer of THD to parent. We must do it before calling awake
      // to transfer the kill state to parent kill_immunizer.
      m_thd->kill_immunizer = m_saved_kill_immunizer;

    // If there were any concurrent kill operations in kill immune mode, call
    // THD::awake() with the m_killed_state. This will either propagate the
    // m_killed_state to the parent kill_immunizer and return or assign state to
    // the THD::killed and complete the THD::awake().  Otherwise, if it is a top
    // level kill immunizer just reset THD::killed state else we need not have
    // to do anything.
    if (m_killed_state)
      m_thd->awake(m_killed_state);
    else if (m_saved_kill_immunizer == nullptr)
      m_thd->killed = m_saved_killed_state;

    // Reset kill_immunizer of THD.
    m_thd->kill_immunizer = m_saved_kill_immunizer;
  }

  /*
    Check if thread is in the kill immune mode.
  */
  bool is_active() { return m_is_active; }

  // Save kill state set while kill immune mode is active.
  void save_killed_state(THD::killed_state state) {
    mysql_mutex_assert_owner(&m_thd->LOCK_thd_data);

    if (m_killed_state == THD::NOT_KILLED) m_killed_state = state;
  }

 private:
  THD *m_thd;

  // When kill_immunizer is set(i.e. operation on DD tables is in progress) to
  // THD then there might be some concurrent KILL operations. The KILL state
  // from those operations is stored in the m_killed_state. While exiting from
  // the kill immune mode THD::awake() is called with this value.
  THD::killed_state m_killed_state;

  // In case of nested Transaction_ro, m_saved_kill_immunizer is used to refer
  // the parent Transaction_ro's kill_immunizer. This is used to propogate the
  // m_killed_state to the parent kill_immunizer.
  DD_kill_immunizer *m_saved_kill_immunizer;

  // THD::killed value is saved before entering the kill immune mode.
  // If kill_immunizer is of some Transaction_ro in the nested Transaction_ro
  // then parent kill_immunizer's m_killed_state is saved in the
  // m_saved_killed_state for reference.
  THD::killed_state m_saved_killed_state;

  /*
    kill immunizer state (active or exiting). The state is set to active when
    kill immunizer is invoked. Kill_immunizer state is set to inactive(or
    exiting) while exiting from the kill immune mode (current instance is of top
    level kill immunizer at this point).
  */
  bool m_is_active;
};

}  // namespace dd
#endif  // DD_KILL_IMMUNIZER_INCLUDED
