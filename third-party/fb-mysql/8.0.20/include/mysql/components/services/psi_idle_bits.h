/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_IDLE_BITS_H
#define COMPONENTS_SERVICES_PSI_IDLE_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_idle Idle Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Interface for an instrumented idle operation.
  This is an opaque structure.
*/
struct PSI_idle_locker;
typedef struct PSI_idle_locker PSI_idle_locker;

/**
  State data storage for @c start_idle_wait_v1_t.
  This structure provide temporary storage to an idle locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa start_idle_wait_v1_t.
*/
struct PSI_idle_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait;
};
typedef struct PSI_idle_locker_state_v1 PSI_idle_locker_state_v1;

/**
  Record an idle instrumentation wait start event.
  @param state data storage for the locker
  @param src_file the source file name
  @param src_line the source line number
  @return an idle locker, or NULL
*/
typedef struct PSI_idle_locker *(*start_idle_wait_v1_t)(
    struct PSI_idle_locker_state_v1 *state, const char *src_file,
    unsigned int src_line);

/**
  Record an idle instrumentation wait end event.
  @param locker a thread locker for the running thread
*/
typedef void (*end_idle_wait_v1_t)(struct PSI_idle_locker *locker);

typedef struct PSI_idle_locker_state_v1 PSI_idle_locker_state;

/** @} (end of group psi_abi_idle) */

#endif /* COMPONENTS_SERVICES_PSI_IDLE_BITS_H */
