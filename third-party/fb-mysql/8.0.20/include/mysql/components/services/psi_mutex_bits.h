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

#ifndef COMPONENTS_SERVICES_PSI_MUTEX_BITS_H
#define COMPONENTS_SERVICES_PSI_MUTEX_BITS_H

/**
  @file
  Instrumentation helpers for mutexes.
  This header file provides the necessary declarations
  to use the mutex API with the performance schema instrumentation.
  In some compilers (SunStudio), 'static inline' functions, when declared
  but not used, are not optimized away (because they are unused) by default,
  so that including a static inline function from a header file does
  create unwanted dependencies, causing unresolved symbols at link time.
  Other compilers, like gcc, optimize these dependencies by default.
*/

/**
  @defgroup psi_abi_mutex Mutex Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented mutex key.
  To instrument a mutex, a mutex key must be obtained using @c register_mutex.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_mutex_key;

/**
  @def PSI_MUTEX_VERSION_1
  Performance Schema Mutex Interface number for version 1.
  This version is supported.
*/
#define PSI_MUTEX_VERSION_1 1

/**
  @def PSI_CURRENT_MUTEX_VERSION
  Performance Schema Mutex Interface number for the most recent version.
  The most current version is @c PSI_MUTEX_VERSION_1
*/
#define PSI_CURRENT_MUTEX_VERSION 1

/**
  Mutex information.
  @since PSI_MUTEX_VERSION_1
  This structure is used to register an instrumented mutex.
*/
struct PSI_mutex_info_v1 {
  /**
    Pointer to the key assigned to the registered mutex.
  */
  PSI_mutex_key *m_key;
  /**
    The name of the mutex to register.
  */
  const char *m_name;
  /**
    The flags of the mutex to register.
    @sa PSI_FLAG_SINGLETON
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};

/**
  Interface for an instrumented mutex.
  This is an opaque structure.
*/
struct PSI_mutex;
typedef struct PSI_mutex PSI_mutex;

/**
  Interface for an instrumented mutex operation.
  This is an opaque structure.
*/
struct PSI_mutex_locker;
typedef struct PSI_mutex_locker PSI_mutex_locker;

/** Operation performed on an instrumented mutex. */
enum PSI_mutex_operation {
  /** Lock. */
  PSI_MUTEX_LOCK = 0,
  /** Lock attempt. */
  PSI_MUTEX_TRYLOCK = 1
};
typedef enum PSI_mutex_operation PSI_mutex_operation;

/**
  State data storage for @c start_mutex_wait_v1_t.
  This structure provide temporary storage to a mutex locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa start_mutex_wait_v1_t
*/
struct PSI_mutex_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current operation. */
  enum PSI_mutex_operation m_operation;
  /** Current mutex. */
  struct PSI_mutex *m_mutex;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start{0};
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait{nullptr};
};
typedef struct PSI_mutex_locker_state_v1 PSI_mutex_locker_state_v1;

/**
  Mutex registration API.
  @param category a category name (typically a plugin name)
  @param info an array of mutex info to register
  @param count the size of the info array
*/
typedef void (*register_mutex_v1_t)(const char *category,
                                    struct PSI_mutex_info_v1 *info, int count);

/**
  Mutex instrumentation initialization API.
  @param key the registered mutex key
  @param identity the address of the mutex itself
  @return an instrumented mutex
*/
typedef struct PSI_mutex *(*init_mutex_v1_t)(PSI_mutex_key key,
                                             const void *identity);

/**
  Mutex instrumentation destruction API.
  @param mutex the mutex to destroy
*/
typedef void (*destroy_mutex_v1_t)(struct PSI_mutex *mutex);

/**
  Record a mutex instrumentation unlock event.
  @param mutex the mutex instrumentation
*/
typedef void (*unlock_mutex_v1_t)(struct PSI_mutex *mutex);

/**
  Record a mutex instrumentation wait start event.
  @param state data storage for the locker
  @param mutex the instrumented mutex to lock
  @param op the operation to perform
  @param src_file the source file name
  @param src_line the source line number
  @return a mutex locker, or NULL
*/
typedef struct PSI_mutex_locker *(*start_mutex_wait_v1_t)(
    struct PSI_mutex_locker_state_v1 *state, struct PSI_mutex *mutex,
    enum PSI_mutex_operation op, const char *src_file, unsigned int src_line);

/**
  Record a mutex instrumentation wait end event.
  @param locker a thread locker for the running thread
  @param rc the wait operation return code
*/
typedef void (*end_mutex_wait_v1_t)(struct PSI_mutex_locker *locker, int rc);

typedef struct PSI_mutex_info_v1 PSI_mutex_info_v1;
typedef PSI_mutex_info_v1 PSI_mutex_info;
typedef struct PSI_mutex_locker_state_v1 PSI_mutex_locker_state;

/** @} (end of group psi_abi_mutex) */

#endif /* COMPONENTS_SERVICES_PSI_MUTEX_BITS_H */
