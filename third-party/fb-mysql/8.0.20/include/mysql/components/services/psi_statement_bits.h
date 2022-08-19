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

#ifndef COMPONENTS_SERVICES_PSI_STATEMENT_BITS_H
#define COMPONENTS_SERVICES_PSI_STATEMENT_BITS_H

#ifndef MYSQL_ABI_CHECK
#include <stddef.h> /* size_t */
#endif

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_statement Statement Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented statement key.
  To instrument a statement, a statement key must be obtained using @c
  register_statement.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_statement_key;

/**
  @def PSI_STATEMENT_VERSION_1
  Performance Schema Statement Interface number for version 1.
  This version is deprecated.
*/
#define PSI_STATEMENT_VERSION_1 1

/**
  @def PSI_STATEMENT_VERSION_2
  Performance Schema Statement Interface number for version 2.
  This version is supported.
*/
#define PSI_STATEMENT_VERSION_2 2

/**
  @def PSI_CURRENT_STATEMENT_VERSION
  Performance Schema Statement Interface number for the most recent version.
  The most current version is @c PSI_STATEMENT_VERSION_2
*/
#define PSI_CURRENT_STATEMENT_VERSION 2

/**
  Interface for an instrumented statement.
  This is an opaque structure.
*/
struct PSI_statement_locker;
typedef struct PSI_statement_locker PSI_statement_locker;

/**
  Interface for an instrumented prepared statement.
  This is an opaque structure.
*/
struct PSI_prepared_stmt;
typedef struct PSI_prepared_stmt PSI_prepared_stmt;

/**
  Interface for an instrumented statement digest operation.
  This is an opaque structure.
*/
struct PSI_digest_locker;
typedef struct PSI_digest_locker PSI_digest_locker;

/**
  Interface for an instrumented stored procedure share.
  This is an opaque structure.
*/
struct PSI_sp_share;
typedef struct PSI_sp_share PSI_sp_share;

/**
  Interface for an instrumented stored program.
  This is an opaque structure.
*/
struct PSI_sp_locker;
typedef struct PSI_sp_locker PSI_sp_locker;

/**
  Statement instrument information.
  @since PSI_STATEMENT_VERSION_1
  This structure is used to register an instrumented statement.
*/
struct PSI_statement_info_v1 {
  /** The registered statement key. */
  PSI_statement_key m_key;
  /** The name of the statement instrument to register. */
  const char *m_name;
  /**
    The flags of the statement instrument to register.
    @sa PSI_FLAG_MUTABLE
  */
  unsigned int m_flags;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_statement_info_v1 PSI_statement_info_v1;

/* Duplicate of NAME_LEN, to avoid dependency on mysql_com.h */
#define PSI_SCHEMA_NAME_LEN (64 * 3)

/**
  State data storage for @c get_thread_statement_locker_v1_t,
  @c get_thread_statement_locker_v1_t.
  This structure provide temporary storage to a statement locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa get_thread_statement_locker_v1_t
*/
struct PSI_statement_locker_state_v1 {
  /** Discarded flag. */
  bool m_discarded;
  /** In prepare flag. */
  bool m_in_prepare;
  /** Metric, no index used flag. */
  unsigned char m_no_index_used;
  /** Metric, no good index used flag. */
  unsigned char m_no_good_index_used;
  /** Internal state. */
  unsigned int m_flags;
  /** Instrumentation class. */
  void *m_class;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_statement;
  /** Locked time. */
  unsigned long long m_lock_time;
  /** CPU time. */
  unsigned long long m_cpu_time;
  /** Rows sent. */
  unsigned long long m_rows_sent;
  /** Rows examined. */
  unsigned long long m_rows_examined;
  /** Rows deleted. */
  unsigned long long m_rows_deleted;
  /** Rows inserted. */
  unsigned long long m_rows_inserted;
  /** Rows updated. */
  unsigned long long m_rows_updated;
  /** Number of bytes written into temp table space. */
  unsigned long long m_tmp_table_bytes_written;
  /** Number of bytes written into filesort space. */
  unsigned long long m_filesort_bytes_written;
  /** Number of index dive queries executed during compilation. */
  unsigned long m_index_dive_count;
  /** CPU time spent for index dive queries. */
  unsigned long long m_index_dive_cpu;
  /** CPU time spent for plan compilation. */
  unsigned long long m_compilation_cpu;
  /** Elapsed time. */
  unsigned long long m_elapsed_time;
  /** Skipped count */
  unsigned long long m_skipped;
  /** Metric, temporary tables created on disk. */
  unsigned long m_created_tmp_disk_tables;
  /** Metric, temporary tables created. */
  unsigned long m_created_tmp_tables;
  /** Metric, number of select full join. */
  unsigned long m_select_full_join;
  /** Metric, number of select full range join. */
  unsigned long m_select_full_range_join;
  /** Metric, number of select range. */
  unsigned long m_select_range;
  /** Metric, number of select range check. */
  unsigned long m_select_range_check;
  /** Metric, number of select scan. */
  unsigned long m_select_scan;
  /** Metric, number of sort merge passes. */
  unsigned long m_sort_merge_passes;
  /** Metric, number of sort merge. */
  unsigned long m_sort_range;
  /** Metric, number of sort rows. */
  unsigned long m_sort_rows;
  /** Metric, number of sort scans. */
  unsigned long m_sort_scan;
  /** Statement digest. */
  const struct sql_digest_storage *m_digest;
  /** Current schema name. */
  char m_schema_name[PSI_SCHEMA_NAME_LEN];
  /** Length in bytes of @c m_schema_name. */
  unsigned int m_schema_name_length;
  /** Statement character set number. */
  unsigned int m_cs_number;
  /** Statement query sample. */
  const char *m_query_sample;
  /** Length in bytes of @c m_query_sample. */
  unsigned int m_query_sample_length;
  /** True if @c m_query_sample was truncated. */
  bool m_query_sample_truncated;
  /** Peak filesort disk usage by statement. */
  unsigned long long m_filesort_disk_usage_peak;
  /** Peak temp table disk usage by statement. */
  unsigned long long m_tmp_table_disk_usage_peak;

  PSI_sp_share *m_parent_sp_share;
  PSI_prepared_stmt *m_parent_prepared_stmt;
};
typedef struct PSI_statement_locker_state_v1 PSI_statement_locker_state_v1;

struct PSI_sp_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Stored Procedure share. */
  PSI_sp_share *m_sp_share;
};
typedef struct PSI_sp_locker_state_v1 PSI_sp_locker_state_v1;

/**
  Statement registration API.
  @param category a category name
  @param info an array of statement info to register
  @param count the size of the info array
*/
typedef void (*register_statement_v1_t)(const char *category,
                                        struct PSI_statement_info_v1 *info,
                                        int count);

/**
  Get a statement instrumentation locker.
  @param state data storage for the locker
  @param key the statement instrumentation key
  @param charset client character set
  @return a statement locker, or NULL
*/
typedef struct PSI_statement_locker *(*get_thread_statement_locker_v1_t)(
    struct PSI_statement_locker_state_v1 *state, PSI_statement_key key,
    const void *charset, PSI_sp_share *sp_share);

/**
  Refine a statement locker to a more specific key.
  Note that only events declared mutable can be refined.
  @param locker the statement locker for the current event
  @param key the new key for the event
  @sa PSI_FLAG_MUTABLE
*/
typedef struct PSI_statement_locker *(*refine_statement_v1_t)(
    struct PSI_statement_locker *locker, PSI_statement_key key);

/**
  Start a new statement event.
  @param locker the statement locker for this event
  @param db the active database name for this statement
  @param db_length the active database name length for this statement
  @param src_file source file name
  @param src_line source line number
*/
typedef void (*start_statement_v1_t)(struct PSI_statement_locker *locker,
                                     const char *db, unsigned int db_length,
                                     const char *src_file,
                                     unsigned int src_line);

/**
  Set the statement text for a statement event.
  Note that the statement text pointer must remain valid until end statement
  is called.
  @param locker the current statement locker
  @param text the statement text
  @param text_len the statement text length
*/
typedef void (*set_statement_text_v1_t)(struct PSI_statement_locker *locker,
                                        const char *text,
                                        unsigned int text_len);

/**
  Set a statement query id.
  Introduced in MySQL 8.0.14
  @param locker the statement locker
  @param query_id the query id
*/
typedef void (*set_statement_query_id_t)(struct PSI_statement_locker *locker,
                                         unsigned long long query_id);

/**
  Set a statement event lock time.
  @param locker the statement locker
  @param lock_time the locked time, in microseconds
*/
typedef void (*set_statement_lock_time_t)(struct PSI_statement_locker *locker,
                                          unsigned long long lock_time);

/**
  Set a statement event cpu time.
  @param locker the statement locker
  @param cpu_time the cpu time, in microseconds
*/
typedef void (*set_statement_cpu_time_t)(struct PSI_statement_locker *locker,
                                         unsigned long long cpu_time);

/**
  Set a statement event rows sent metric.
  @param locker the statement locker
  @param count the number of rows sent
*/
typedef void (*set_statement_rows_sent_t)(struct PSI_statement_locker *locker,
                                          unsigned long long count);

/**
  Set a statement event rows examined metric.
  @param locker the statement locker
  @param count the number of rows examined
*/
typedef void (*set_statement_rows_examined_t)(
    struct PSI_statement_locker *locker, unsigned long long count);

/**
  Inc a statement event rows deleted metric.
  @param locker the statement locker
  @param count the number of rows deleted
*/
typedef void (*inc_statement_rows_deleted_t)(
    struct PSI_statement_locker *locker, unsigned long long count);

/**
  Inc a statement event rows inserted metric.
  @param locker the statement locker
  @param count the number of rows inserted
*/
typedef void (*inc_statement_rows_inserted_t)(
    struct PSI_statement_locker *locker, unsigned long long count);

/**
  Inc a statement event rows updated metric.
  @param locker the statement locker
  @param count the number of rows updated
*/
typedef void (*inc_statement_rows_updated_t)(
    struct PSI_statement_locker *locker, unsigned long long count);

/**
  Increment a statement event temp table bytes written
  @param locker the statement locker
  @param tmp_table_bytes_written temp table bytes written
*/
typedef void (*inc_statement_tmp_table_bytes_written_t)(
    struct PSI_statement_locker *locker,
    unsigned long long tmp_table_bytes_written);

/**
  Increment a statement event filesort bytes written
  @param locker the statement locker
  @param filesort_bytes_written filesort bytes written
*/
typedef void (*inc_statement_filesort_bytes_written_t)(
    struct PSI_statement_locker *locker,
    unsigned long long filesort_bytes_written);

/**
  Increment a statement event index dive count
  @param locker the statement locker
  @param index_dive_count index dive count
*/
typedef void (*inc_statement_index_dive_count_t)(
    struct PSI_statement_locker *locker, unsigned long index_dive_count);

/**
  Increment a statement event index dive cpu
  @param locker the statement locker
  @param index_dive_cpu index dive cpu
*/
typedef void (*inc_statement_index_dive_cpu_t)(
    struct PSI_statement_locker *locker, unsigned long long index_dive_cpu);

/**
  Increment a statement event compilation cpu
  @param locker the statement locker
  @param compilation_cpu compilation cpu
*/
typedef void (*inc_statement_compilation_cpu_t)(
    struct PSI_statement_locker *locker, unsigned long long compilation_cpu);

/**
  Increment a statement event "created tmp disk tables" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_created_tmp_disk_tables_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "created tmp tables" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_created_tmp_tables_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "select full join" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_select_full_join_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "select full range join" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_select_full_range_join_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "select range join" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_select_range_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "select range check" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_select_range_check_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "select scan" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_select_scan_t)(struct PSI_statement_locker *locker,
                                            unsigned long count);

/**
  Increment a statement event "sort merge passes" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_sort_merge_passes_t)(
    struct PSI_statement_locker *locker, unsigned long count);

/**
  Increment a statement event "sort range" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_sort_range_t)(struct PSI_statement_locker *locker,
                                           unsigned long count);

/**
  Increment a statement event "sort rows" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_sort_rows_t)(struct PSI_statement_locker *locker,
                                          unsigned long count);

/**
  Increment a statement event "sort scan" metric.
  @param locker the statement locker
  @param count the metric increment value
*/
typedef void (*inc_statement_sort_scan_t)(struct PSI_statement_locker *locker,
                                          unsigned long count);

/**
  Set a statement event "no index used" metric.
  @param locker the statement locker
*/
typedef void (*set_statement_no_index_used_t)(
    struct PSI_statement_locker *locker);

/**
  Set a statement event "no good index used" metric.
  @param locker the statement locker
*/
typedef void (*set_statement_no_good_index_used_t)(
    struct PSI_statement_locker *locker);

/**
  Update a statement event "filesort disk usage" metric.
  The metric maintains the max value seen during statement execution.
  @param locker the statement locker
  @param value new disk usage value
*/
typedef void (*update_statement_filesort_disk_usage_t)(
    struct PSI_statement_locker *locker, unsigned long long value);

/**
  Update a statement event "tmp table disk usage" metric.
  The metric maintains the max value seen during statement execution.
  @param locker the statement locker
  @param value new disk usage value
*/
typedef void (*update_statement_tmp_table_disk_usage_t)(
    struct PSI_statement_locker *locker, unsigned long long value);

/**
  End a statement event.
  @param locker the statement locker
  @param stmt_da the statement diagnostics area.
  @sa Diagnostics_area
*/
typedef void (*end_statement_v1_t)(struct PSI_statement_locker *locker,
                                   void *stmt_da);

/**
  Get a prepare statement.
  @param locker a statement locker for the running thread.
*/
typedef PSI_prepared_stmt *(*create_prepared_stmt_v1_t)(
    void *identity, unsigned int stmt_id, PSI_statement_locker *locker,
    const char *stmt_name, size_t stmt_name_length, const char *name,
    size_t length);

/**
  destroy a prepare statement.
  @param prepared_stmt prepared statement.
*/
typedef void (*destroy_prepared_stmt_v1_t)(PSI_prepared_stmt *prepared_stmt);

/**
  repreare a prepare statement.
  @param prepared_stmt prepared statement.
*/
typedef void (*reprepare_prepared_stmt_v1_t)(PSI_prepared_stmt *prepared_stmt);

/**
  Record a prepare statement instrumentation execute event.
  @param locker a statement locker for the running thread.
  @param prepared_stmt prepared statement.
*/
typedef void (*execute_prepared_stmt_v1_t)(PSI_statement_locker *locker,
                                           PSI_prepared_stmt *prepared_stmt);

/**
  Set the statement text for a prepared statment event.
  @param prepared_stmt prepared statement.
  @param text the prepared statement text
  @param text_len the prepared statement text length
*/
typedef void (*set_prepared_stmt_text_v1_t)(PSI_prepared_stmt *prepared_stmt,
                                            const char *text,
                                            unsigned int text_len);
/**
  Get a digest locker for the current statement.
  @param locker a statement locker for the running thread
*/
typedef struct PSI_digest_locker *(*digest_start_v1_t)(
    struct PSI_statement_locker *locker);

/**
  Add a computed digest to the current digest instrumentation.
  @param locker a digest locker for the current statement
  @param digest the computed digest
*/
typedef void (*digest_end_v1_t)(struct PSI_digest_locker *locker,
                                const struct sql_digest_storage *digest);

/**
  Acquire a sp share instrumentation.
  @param object_type type of stored program
  @param schema_name schema name of stored program
  @param schema_name_length length of schema_name
  @param object_name object name of stored program
  @param object_name_length length of object_name
  @return a stored program share instrumentation, or NULL
*/
typedef struct PSI_sp_share *(*get_sp_share_v1_t)(
    unsigned int object_type, const char *schema_name,
    unsigned int schema_name_length, const char *object_name,
    unsigned int object_name_length);

/**
  Release a stored program share.
  @param share the stored program share to release
*/
typedef void (*release_sp_share_v1_t)(struct PSI_sp_share *share);

typedef PSI_sp_locker *(*start_sp_v1_t)(struct PSI_sp_locker_state_v1 *state,
                                        struct PSI_sp_share *sp_share);

typedef void (*end_sp_v1_t)(struct PSI_sp_locker *locker);

typedef void (*drop_sp_v1_t)(unsigned int object_type, const char *schema_name,
                             unsigned int schema_name_length,
                             const char *object_name,
                             unsigned int object_name_length);

typedef struct PSI_statement_info_v1 PSI_statement_info;
typedef struct PSI_statement_locker_state_v1 PSI_statement_locker_state;
typedef struct PSI_sp_locker_state_v1 PSI_sp_locker_state;

/** @} (end of group psi_abi_statement) */

#endif /* COMPONENTS_SERVICES_PSI_STATEMENT_BITS_H */
