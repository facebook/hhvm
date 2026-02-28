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

#ifndef COMPONENTS_SERVICES_PSI_RWLOCK_BITS_H
#define COMPONENTS_SERVICES_PSI_RWLOCK_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_rwlock Rwlock Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented rwlock key.
  To instrument a rwlock, a rwlock key must be obtained
  using @c register_rwlock.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_rwlock_key;

/**
  @def PSI_RWLOCK_VERSION_1
  Performance Schema Rwlock Interface number for version 1.
  This version is obsolete.
*/
#define PSI_RWLOCK_VERSION_1 1

/**
  @def PSI_RWLOCK_VERSION_2
  Performance Schema Rwlock Interface number for version 2.
  This version is supported.
*/
#define PSI_RWLOCK_VERSION_2 2

/**
  @def PSI_CURRENT_RWLOCK_VERSION
  Performance Schema Rwlock Interface number for the most recent version.
  The most current version is @c PSI_RWLOCK_VERSION_2
*/
#define PSI_CURRENT_RWLOCK_VERSION 2

/**
  Interface for an instrumented rwlock.
  This is an opaque structure.
*/
struct PSI_rwlock;
typedef struct PSI_rwlock PSI_rwlock;

/**
  Interface for an instrumented rwlock operation.
  This is an opaque structure.
*/
struct PSI_rwlock_locker;
typedef struct PSI_rwlock_locker PSI_rwlock_locker;

#if 0
/*
  Keeping the original version 1 definition in the source,
  in case we ever need to provide support for version 1.
*/

/**
  Operation performed on an instrumented rwlock.
  For basic READ / WRITE lock,
  operations are "READ" or "WRITE".
  For SX-locks, operations are "SHARED", "SHARED-EXCLUSIVE" or "EXCLUSIVE".
*/
enum PSI_rwlock_operation {
  /** Read lock. */
  PSI_RWLOCK_READLOCK = 0,
  /** Write lock. */
  PSI_RWLOCK_WRITELOCK = 1,
  /** Read lock attempt. */
  PSI_RWLOCK_TRYREADLOCK = 2,
  /** Write lock attempt. */
  PSI_RWLOCK_TRYWRITELOCK = 3,

  /** Shared lock. */
  PSI_RWLOCK_SHAREDLOCK = 4,
  /** Shared Exclusive lock. */
  PSI_RWLOCK_SHAREDEXCLUSIVELOCK = 5,
  /** Exclusive lock. */
  PSI_RWLOCK_EXCLUSIVELOCK = 6,
  /** Shared lock attempt. */
  PSI_RWLOCK_TRYSHAREDLOCK = 7,
  /** Shared Exclusive lock attempt. */
  PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK = 8,
  /** Exclusive lock attempt. */
  PSI_RWLOCK_TRYEXCLUSIVELOCK = 9
};
#endif

/**
  Operation performed on an instrumented rwlock.
  For basic READ / WRITE lock,
  operations are "READ" or "WRITE".
  For SX-locks, operations are "SHARED", "SHARED-EXCLUSIVE" or "EXCLUSIVE".
*/
enum PSI_rwlock_operation {
  /** Read lock. */
  PSI_RWLOCK_READLOCK = 0,
  /** Write lock. */
  PSI_RWLOCK_WRITELOCK = 1,
  /** Read lock attempt. */
  PSI_RWLOCK_TRYREADLOCK = 2,
  /** Write lock attempt. */
  PSI_RWLOCK_TRYWRITELOCK = 3,
  /** Unlock (Read or Write). */
  PSI_RWLOCK_UNLOCK = 4,

  /** Shared lock. */
  PSI_RWLOCK_SHAREDLOCK = 5,
  /** Shared Exclusive lock. */
  PSI_RWLOCK_SHAREDEXCLUSIVELOCK = 6,
  /** Exclusive lock. */
  PSI_RWLOCK_EXCLUSIVELOCK = 7,
  /** Shared lock attempt. */
  PSI_RWLOCK_TRYSHAREDLOCK = 8,
  /** Shared Exclusive lock attempt. */
  PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK = 9,
  /** Exclusive lock attempt. */
  PSI_RWLOCK_TRYEXCLUSIVELOCK = 10,
  /** Unlock a shared lock. */
  PSI_RWLOCK_SHAREDUNLOCK = 11,
  /** Unlock a shared exclusive lock. */
  PSI_RWLOCK_SHAREDEXCLUSIVEUNLOCK = 12,
  /** Unlock an exclusive lock. */
  PSI_RWLOCK_EXCLUSIVEUNLOCK = 13
};
typedef enum PSI_rwlock_operation PSI_rwlock_operation;

/**
  Rwlock information.
  @since PSI_RWLOCK_VERSION_1
  This structure is used to register an instrumented rwlock.
*/
struct PSI_rwlock_info_v1 {
  /**
    Pointer to the key assigned to the registered rwlock.
  */
  PSI_rwlock_key *m_key;
  /**
    The name of the rwlock to register.
  */
  const char *m_name;
  /**
    The flags of the rwlock to register.
    @sa PSI_FLAG_SINGLETON
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_rwlock_info_v1 PSI_rwlock_info_v1;

/**
  State data storage for @c start_rwlock_rdwait_v1_t, @c
  start_rwlock_wrwait_v1_t.
  This structure provide temporary storage to a rwlock locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa start_rwlock_rdwait_v1_t
  @sa start_rwlock_wrwait_v1_t
*/
struct PSI_rwlock_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current operation. */
  enum PSI_rwlock_operation m_operation;
  /** Current rwlock. */
  struct PSI_rwlock *m_rwlock;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start{0};
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait{nullptr};
};
typedef struct PSI_rwlock_locker_state_v1 PSI_rwlock_locker_state_v1;

/**
  Rwlock registration API.
  @param category a category name (typically a plugin name)
  @param info an array of rwlock info to register
  @param count the size of the info array
*/
typedef void (*register_rwlock_v1_t)(const char *category,
                                     struct PSI_rwlock_info_v1 *info,
                                     int count);

/**
  Rwlock instrumentation initialization API.
  @param key the registered rwlock key
  @param identity the address of the rwlock itself
  @return an instrumented rwlock
*/
typedef struct PSI_rwlock *(*init_rwlock_v1_t)(PSI_rwlock_key key,
                                               const void *identity);

/**
  Rwlock instrumentation destruction API.
  @param rwlock the rwlock to destroy
*/
typedef void (*destroy_rwlock_v1_t)(struct PSI_rwlock *rwlock);

/**
  Record a rwlock instrumentation read wait start event.
  @param state data storage for the locker
  @param rwlock the instrumented rwlock to lock
  @param op the operation to perform
  @param src_file the source file name
  @param src_line the source line number
  @return a rwlock locker, or NULL
*/
typedef struct PSI_rwlock_locker *(*start_rwlock_rdwait_v1_t)(
    struct PSI_rwlock_locker_state_v1 *state, struct PSI_rwlock *rwlock,
    enum PSI_rwlock_operation op, const char *src_file, unsigned int src_line);

/**
  Record a rwlock instrumentation read wait end event.
  @param locker a thread locker for the running thread
  @param rc the wait operation return code
*/
typedef void (*end_rwlock_rdwait_v1_t)(struct PSI_rwlock_locker *locker,
                                       int rc);

/**
  Record a rwlock instrumentation write wait start event.
  @param state data storage for the locker
  @param rwlock the instrumented rwlock to lock
  @param op the operation to perform
  @param src_file the source file name
  @param src_line the source line number
  @return a rwlock locker, or NULL
*/
typedef struct PSI_rwlock_locker *(*start_rwlock_wrwait_v1_t)(
    struct PSI_rwlock_locker_state_v1 *state, struct PSI_rwlock *rwlock,
    enum PSI_rwlock_operation op, const char *src_file, unsigned int src_line);

/**
  Record a rwlock instrumentation write wait end event.
  @param locker a thread locker for the running thread
  @param rc the wait operation return code
*/
typedef void (*end_rwlock_wrwait_v1_t)(struct PSI_rwlock_locker *locker,
                                       int rc);

/**
  Record a rwlock instrumentation unlock event.
  @param rwlock the rwlock instrumentation
*/
typedef void (*unlock_rwlock_v1_t)(struct PSI_rwlock *rwlock);

/**
  Record a rwlock instrumentation unlock event.
  @param rwlock the rwlock instrumentation
  @param op the unlock operation performed
*/
typedef void (*unlock_rwlock_v2_t)(struct PSI_rwlock *rwlock,
                                   enum PSI_rwlock_operation op);

typedef struct PSI_rwlock_info_v1 PSI_rwlock_info;
typedef struct PSI_rwlock_locker_state_v1 PSI_rwlock_locker_state;

/** @} (end of group psi_abi_rwlock) */

#endif /* COMPONENTS_SERVICES_PSI_RWLOCK_BITS_H */
