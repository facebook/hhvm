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

#ifndef COMPONENTS_SERVICES_PSI_TABLE_BITS_H
#define COMPONENTS_SERVICES_PSI_TABLE_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_table Table Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

struct TABLE_SHARE;

/**
  Interface for an instrumented table operation.
  This is an opaque structure.
*/
struct PSI_table_locker;
typedef struct PSI_table_locker PSI_table_locker;

/** IO operation performed on an instrumented table. */
enum PSI_table_io_operation {
  /** Row fetch. */
  PSI_TABLE_FETCH_ROW = 0,
  /** Row write. */
  PSI_TABLE_WRITE_ROW = 1,
  /** Row update. */
  PSI_TABLE_UPDATE_ROW = 2,
  /** Row delete. */
  PSI_TABLE_DELETE_ROW = 3
};
typedef enum PSI_table_io_operation PSI_table_io_operation;

/**
  State data storage for @c start_table_io_wait_v1_t,
  @c start_table_lock_wait_v1_t.
  This structure provide temporary storage to a table locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa start_table_io_wait_v1_t
  @sa start_table_lock_wait_v1_t
*/
struct PSI_table_locker_state {
  /** Internal state. */
  unsigned int m_flags;
  /** Current io operation. */
  enum PSI_table_io_operation m_io_operation;
  /** Current table handle. */
  struct PSI_table *m_table;
  /** Current table share. */
  struct PSI_table_share *m_table_share;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait;
  /**
    Implementation specific.
    For table io, the table io index.
    For table lock, the lock type.
  */
  unsigned int m_index;
};
typedef struct PSI_table_locker_state PSI_table_locker_state;

/**
  Interface for an instrumented table share.
  This is an opaque structure.
*/
struct PSI_table_share;
typedef struct PSI_table_share PSI_table_share;

/**
  Interface for an instrumented table handle.
  This is an opaque structure.
*/
struct PSI_table;
typedef struct PSI_table PSI_table;

/** Lock operation performed on an instrumented table. */
enum PSI_table_lock_operation {
  /** Table lock, in the server layer. */
  PSI_TABLE_LOCK = 0,
  /** Table lock, in the storage engine layer. */
  PSI_TABLE_EXTERNAL_LOCK = 1
};
typedef enum PSI_table_lock_operation PSI_table_lock_operation;

/**
  Acquire a table share instrumentation.
  @param temporary True for temporary tables
  @param share The SQL layer table share
  @return a table share instrumentation, or NULL
*/
typedef struct PSI_table_share *(*get_table_share_v1_t)(
    bool temporary, struct TABLE_SHARE *share);

/**
  Release a table share.
  @param share the table share to release
*/
typedef void (*release_table_share_v1_t)(struct PSI_table_share *share);

/**
  Drop a table share.
  @param temporary True for temporary tables
  @param schema_name the table schema name
  @param schema_name_length the table schema name length
  @param table_name the table name
  @param table_name_length the table name length
*/
typedef void (*drop_table_share_v1_t)(bool temporary, const char *schema_name,
                                      int schema_name_length,
                                      const char *table_name,
                                      int table_name_length);

/**
  Open an instrumentation table handle.
  @param share the table to open
  @param identity table handle identity
  @return a table handle, or NULL
*/
typedef struct PSI_table *(*open_table_v1_t)(struct PSI_table_share *share,
                                             const void *identity);

/**
  Unbind a table handle from the current thread.
  This operation happens when an opened table is added to the open table cache.
  @param table the table to unbind
*/
typedef void (*unbind_table_v1_t)(struct PSI_table *table);

/**
  Rebind a table handle to the current thread.
  This operation happens when a table from the open table cache
  is reused for a thread.
  @param table the table to unbind
*/
typedef PSI_table *(*rebind_table_v1_t)(PSI_table_share *share,
                                        const void *identity, PSI_table *table);

/**
  Close an instrumentation table handle.
  Note that the table handle is invalid after this call.
  @param table the table handle to close
*/
typedef void (*close_table_v1_t)(struct TABLE_SHARE *server_share,
                                 struct PSI_table *table);

/**
  Record a table instrumentation io wait start event.
  @param state data storage for the locker
  @param table the instrumented table
  @param op the operation to perform
  @param index the operation index
  @param src_file the source file name
  @param src_line the source line number
*/
typedef struct PSI_table_locker *(*start_table_io_wait_v1_t)(
    struct PSI_table_locker_state *state, struct PSI_table *table,
    enum PSI_table_io_operation op, unsigned int index, const char *src_file,
    unsigned int src_line);

/**
  Record a table instrumentation io wait end event.
  @param locker a table locker for the running thread
  @param numrows the number of rows involved in io
*/
typedef void (*end_table_io_wait_v1_t)(struct PSI_table_locker *locker,
                                       unsigned long long numrows);

/**
  Record a table instrumentation lock wait start event.
  @param state data storage for the locker
  @param table the instrumented table
  @param op the operation to perform
  @param flags the operation flags
  @param src_file the source file name
  @param src_line the source line number
*/
typedef struct PSI_table_locker *(*start_table_lock_wait_v1_t)(
    struct PSI_table_locker_state *state, struct PSI_table *table,
    enum PSI_table_lock_operation op, unsigned long flags, const char *src_file,
    unsigned int src_line);

/**
  Record a table instrumentation lock wait end event.
  @param locker a table locker for the running thread
*/
typedef void (*end_table_lock_wait_v1_t)(struct PSI_table_locker *locker);

/**
  Record a table unlock event.
  @param table instrumentation for the table being unlocked
*/
typedef void (*unlock_table_v1_t)(struct PSI_table *table);

/** @} (end of group psi_abi_table) */

#endif /* COMPONENTS_SERVICES_PSI_TABLE_BITS_H */
