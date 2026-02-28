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

#ifndef COMPONENTS_SERVICES_PSI_COND_BITS_H
#define COMPONENTS_SERVICES_PSI_COND_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_cond Cond Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented cond key.
  To instrument a condition, a condition key must be obtained
  using @c register_cond.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_cond_key;

/**
  @def PSI_COND_VERSION_1
  Performance Schema Cond Interface number for version 1.
  This version is supported.
*/
#define PSI_COND_VERSION_1 1

/**
  @def PSI_CURRENT_COND_VERSION
  Performance Schema Cond Interface number for the most recent version.
  The most current version is @c PSI_COND_VERSION_1
*/
#define PSI_CURRENT_COND_VERSION 1

/**
  Interface for an instrumented condition.
  This is an opaque structure.
*/
struct PSI_cond;
typedef struct PSI_cond PSI_cond;

/**
  Interface for an instrumented condition operation.
  This is an opaque structure.
*/
struct PSI_cond_locker;
typedef struct PSI_cond_locker PSI_cond_locker;

/** Operation performed on an instrumented condition. */
enum PSI_cond_operation {
  /** Wait. */
  PSI_COND_WAIT = 0,
  /** Wait, with timeout. */
  PSI_COND_TIMEDWAIT = 1
};
typedef enum PSI_cond_operation PSI_cond_operation;

/**
  Condition information.
  @since PSI_COND_VERSION_1
  This structure is used to register an instrumented cond.
*/
struct PSI_cond_info_v1 {
  /**
    Pointer to the key assigned to the registered cond.
  */
  PSI_cond_key *m_key;
  /**
    The name of the cond to register.
  */
  const char *m_name;
  /**
    The flags of the cond to register.
    @sa PSI_FLAG_SINGLETON
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_cond_info_v1 PSI_cond_info_v1;

/**
  State data storage for @c start_cond_wait_v1_t.
  This structure provide temporary storage to a condition locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa start_cond_wait_v1_t
*/
struct PSI_cond_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current operation. */
  enum PSI_cond_operation m_operation;
  /** Current condition. */
  struct PSI_cond *m_cond;
  /** Current mutex. */
  struct PSI_mutex *m_mutex;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait;
};
typedef struct PSI_cond_locker_state_v1 PSI_cond_locker_state_v1;

/**
  Cond registration API.
  @param category a category name (typically a plugin name)
  @param info an array of cond info to register
  @param count the size of the info array
*/
typedef void (*register_cond_v1_t)(const char *category,
                                   struct PSI_cond_info_v1 *info, int count);

/**
  Cond instrumentation initialisation API.
  @param key the registered key
  @param identity the address of the cond itself
  @return an instrumented cond
*/
typedef struct PSI_cond *(*init_cond_v1_t)(PSI_cond_key key,
                                           const void *identity);

/**
  Cond instrumentation destruction API.
  @param cond the rcond to destroy
*/
typedef void (*destroy_cond_v1_t)(struct PSI_cond *cond);

/**
  Record a condition instrumentation signal event.
  @param cond the cond instrumentation
*/
typedef void (*signal_cond_v1_t)(struct PSI_cond *cond);

/**
  Record a condition instrumentation broadcast event.
  @param cond the cond instrumentation
*/
typedef void (*broadcast_cond_v1_t)(struct PSI_cond *cond);

/**
  Record a condition instrumentation wait start event.
  @param state data storage for the locker
  @param cond the instrumented cond to lock
  @param op the operation to perform
  @param src_file the source file name
  @param src_line the source line number
  @return a cond locker, or NULL
*/
typedef struct PSI_cond_locker *(*start_cond_wait_v1_t)(
    struct PSI_cond_locker_state_v1 *state, struct PSI_cond *cond,
    struct PSI_mutex *mutex, enum PSI_cond_operation op, const char *src_file,
    unsigned int src_line);

/**
  Record a condition instrumentation wait end event.
  @param locker a thread locker for the running thread
  @param rc the wait operation return code
*/
typedef void (*end_cond_wait_v1_t)(struct PSI_cond_locker *locker, int rc);

typedef struct PSI_cond_info_v1 PSI_cond_info;
typedef struct PSI_cond_locker_state_v1 PSI_cond_locker_state;

/** @} (end of group psi_abi_cond) */

#endif /* COMPONENTS_SERVICES_PSI_COND_BITS_H */
