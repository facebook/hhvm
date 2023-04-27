/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQLD_INCLUDED
#define MYSQLD_INCLUDED

#include "my_config.h"

#include <signal.h>
#include <stdint.h>  // int32_t
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <atomic>
#include <functional>

#include <mysql/components/minimal_chassis.h>
#include <mysql/components/services/dynamic_loader_scheme_file.h>
#include "lex_string.h"
#include "m_ctype.h"
#include "my_command.h"
#include "my_compiler.h"
#include "my_compress.h"
#include "my_getopt.h"
#include "my_hostname.h"  // HOSTNAME_LENGTH
#include "my_inttypes.h"
#include "my_io.h"
#include "my_psi_config.h"
#include "my_rdtsc.h" /* my_timer* */
#include "my_sharedlib.h"
#include "my_sqlcommand.h"  // SQLCOM_END
#include "my_sys.h"         // MY_TMPDIR
#include "my_thread.h"      // my_thread_attr_t
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_file_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/components/services/psi_socket_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/status_var.h"
#include "mysql_com.h"  // SERVER_VERSION_LENGTH
#ifdef _WIN32
#include "sql/nt_servc.h"
#endif  // _WIN32
#include <vector>
#include "sql/rpl_lag_manager.h"
#include "sql/set_var.h"  // enum_var_type
#include "sql/sql_bitmap.h"
#include "sql/sql_const.h"  // UUID_LENGTH
#include "sql/sql_info.h"
#include "sql/system_variables.h"
#include "sql/thr_malloc.h"

#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
#include "storage/perfschema/pfs_server.h"
#endif /* WITH_PERFSCHEMA_STORAGE_ENGINE */

class Rpl_global_filter;
class THD;
class Time_zone;
struct MEM_ROOT;
struct handlerton;

#if MAX_INDEXES <= 64
typedef Bitmap<64> Key_map; /* Used for finding keys */
#elif MAX_INDEXES > 255
#error "MAX_INDEXES values greater than 255 is not supported."
#else
typedef Bitmap<((MAX_INDEXES + 7) / 8 * 8)> Key_map; /* Used for finding keys */
#endif

/* Bits from testflag */
#define TEST_PRINT_CACHED_TABLES 1
#define TEST_NO_KEY_GROUP 2
#define TEST_MIT_THREAD 4
/*
  TEST_BLOCKING is made obsolete and is not used any
  where in the code base and is retained here so that
  the other bit flag values are not changed.
*/
#define OBSOLETE_TEST_BLOCKING 8
#define TEST_KEEP_TMP_TABLES 16
#define TEST_READCHECK 64 /**< Force use of readcheck */
#define TEST_NO_EXTRA 128
#define TEST_NO_STACKTRACE 512
#define TEST_SIGINT 1024 /**< Allow sigint on threads */
#define TEST_SYNCHRONIZATION          \
  2048 /**< get server to do sleep in \
          some places */
#define TEST_DO_QUICK_LEAK_CHECK       \
  4096 /**< Do Valgrind leak check for \
          each command. */

#define SPECIAL_NO_NEW_FUNC 2     /* Skip new functions */
#define SPECIAL_SKIP_SHOW_DB 4    /* Don't allow 'show db' */
#define SPECIAL_NO_RESOLVE 64     /* Don't use gethostname */
#define SPECIAL_NO_HOST_CACHE 512 /* Don't cache hosts */
#define SPECIAL_SHORT_LOG_FORMAT 1024

/* Function prototypes */

/**
  Signal the server thread for restart.

  @return false if the thread has been successfully signalled for restart
          else true.
*/

bool signal_restart_server();
void kill_mysql(void);
void refresh_status();
bool is_secure_file_path(const char *path);
ulong sql_rnd_with_mutex();

struct System_status_var *get_thd_status_var(THD *thd, bool *aggregated);
void aggregate_status_var(std::function<void(THD *)> cb, unsigned long tid);
bool update_Sys_slow_log_path(const char *const log_file_name,
                              const bool need_lock);

// These are needed for unit testing.
void set_remaining_args(int argc, char **argv);
int init_common_variables();
void my_init_signals();
bool gtid_server_init();
void gtid_server_cleanup();
void clean_up_mysqld_mutexes();
void delete_pid_file(myf flags);

extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *files_charset_info;
extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *national_charset_info;
extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *table_alias_charset;
extern CHARSET_INFO *character_set_filesystem;

enum enum_server_operational_state {
  SERVER_BOOTING,      /* Server is not operational. It is starting */
  SERVER_OPERATING,    /* Server is fully initialized and operating */
  SERVER_SHUTTING_DOWN /* erver is shutting down */
};
enum_server_operational_state get_server_state();

extern bool rpl_slave_flow_control;
extern bool opt_improved_dup_key_error;
extern bool opt_large_files, server_id_supplied;
extern bool opt_bin_log, opt_trim_binlog;
extern bool opt_binlog_trx_meta_data;
extern bool opt_log_slave_updates;
extern bool opt_log_unsafe_statements;
extern bool opt_log_global_var_changes;
extern bool opt_general_log, opt_slow_log, opt_general_log_raw;
extern char *opt_gap_lock_logname;
extern ulonglong log_output_options;
extern bool opt_log_queries_not_using_indexes;
extern ulong opt_log_throttle_queries_not_using_indexes;
extern ulong opt_log_throttle_ddl;
extern bool opt_log_slow_extra;
extern bool opt_disable_networking, opt_skip_show_db;
extern bool opt_skip_name_resolve;
extern bool opt_help;
extern bool opt_verbose;
extern bool opt_character_set_client_handshake;
extern MYSQL_PLUGIN_IMPORT std::atomic<int32>
    connection_events_loop_aborted_flag;
extern bool opt_no_dd_upgrade;
extern long opt_upgrade_mode;
extern bool opt_initialize;
extern bool opt_safe_user_create;
extern bool opt_local_infile, opt_myisam_use_mmap;
extern bool opt_slave_compressed_protocol;
extern ulong opt_slave_compression_lib;
extern ulonglong opt_slave_dump_thread_wait_sleep_usec;
extern bool rpl_wait_for_semi_sync_ack;
extern ulong slave_exec_mode_options;
extern ulong slave_use_idempotent_for_recovery_options;
extern Rpl_global_filter rpl_global_filter;
extern int32_t opt_regexp_time_limit;
extern int32_t opt_regexp_stack_limit;
#ifdef _WIN32
extern bool opt_no_monitor;
#endif  // _WIN32
extern bool opt_debugging;
extern bool opt_validate_config;

enum enum_slave_type_conversions {
  SLAVE_TYPE_CONVERSIONS_ALL_LOSSY,
  SLAVE_TYPE_CONVERSIONS_ALL_NON_LOSSY,
  SLAVE_TYPE_CONVERSIONS_ALL_UNSIGNED,
  SLAVE_TYPE_CONVERSIONS_ALL_SIGNED,
  SLAVE_TYPE_CONVERSIONS_ALL_NON_TRUNCATION
};
extern ulong slave_run_triggers_for_rbr;
extern ulonglong slave_type_conversions_options;
extern char *opt_rbr_column_type_mismatch_whitelist;

extern bool read_only, opt_readonly;
extern bool super_read_only, opt_super_readonly;
extern bool skip_master_info_check_for_read_only_error_msg_extra;
extern bool send_error_before_closing_timed_out_connection;
extern char *opt_read_only_error_msg_extra;
extern bool lower_case_file_system;

enum enum_slave_rows_search_algorithms {
  SLAVE_ROWS_TABLE_SCAN = (1U << 0),
  SLAVE_ROWS_INDEX_SCAN = (1U << 1),
  SLAVE_ROWS_HASH_SCAN = (1U << 2)
};
extern ulonglong slave_rows_search_algorithms_options;
extern bool opt_require_secure_transport;

#ifndef DBUG_OFF
extern uint slave_rows_last_search_algorithm_used;
#endif
#ifdef _WIN32
extern bool opt_enable_named_pipe;
extern char *named_pipe_full_access_group;
extern bool opt_enable_shared_memory;
extern mysql_rwlock_t LOCK_named_pipe_full_access_group;
#endif
extern bool opt_allow_suspicious_udfs;
extern const char *opt_secure_file_priv;
extern bool opt_log_slow_admin_statements, opt_log_slow_slave_statements;
extern bool sp_automatic_privileges, opt_noacl;
extern bool opt_old_style_user_limits, trust_function_creators;
extern bool check_proxy_users, mysql_native_password_proxy_users,
    sha256_password_proxy_users;
#ifdef _WIN32
extern const char *shared_memory_base_name;
#endif
extern const char *mysqld_unix_port;
extern char *default_tz_name;
extern Time_zone *default_tz;
extern const char *default_storage_engine;
extern const char *default_tmp_storage_engine;
extern ulonglong temptable_max_ram;
extern bool temptable_track_shared_block_ram;
extern bool temptable_use_mmap;
extern bool using_udf_functions;
extern bool locked_in_memory;
extern bool opt_using_transactions;
extern ulong current_pid;
extern ulong expire_logs_days;
extern ulong max_slowlog_size;
extern ulonglong slowlog_space_limit;
extern ulong binlog_expire_logs_seconds;
extern uint sync_binlog_period, sync_relaylog_period, sync_relayloginfo_period,
    sync_masterinfo_period, opt_mts_checkpoint_period, opt_mts_checkpoint_group;
extern ulonglong update_binlog_pos_threshold;
extern ulong opt_tc_log_size, tc_log_max_pages_used, tc_log_page_size;
extern ulong tc_log_page_waits;
extern bool relay_log_purge;
extern bool relay_log_recovery;
extern std::atomic<bool> offline_mode;
extern uint test_flags, select_errors, ha_open_options;
extern uint protocol_version, mysqld_port;
extern bool enable_binlog_hlc;
extern bool maintain_database_hlc;
extern ulong wait_for_hlc_timeout_ms;
extern ulong wait_for_hlc_sleep_threshold_ms;
extern double wait_for_hlc_sleep_scaling_factor;
extern ulonglong hlc_upper_bound_delta;
extern char *default_collation_for_utf8mb4_init;
extern bool enable_acl_fast_lookup;
extern bool enable_acl_cache_deadlock_detection;
extern bool enable_acl_db_cache;
extern bool enable_super_log_bin_read_only;

enum enum_delay_key_write {
  DELAY_KEY_WRITE_NONE,
  DELAY_KEY_WRITE_ON,
  DELAY_KEY_WRITE_ALL
};
extern ulong delay_key_write_options;

extern ulong opt_log_timestamps;
extern const char *timestamp_type_names[];
extern char *opt_general_logname, *opt_slow_logname, *opt_bin_logname,
    *opt_relay_logname;
extern char *mysql_home_ptr, *pidfile_name_ptr;
extern char *default_auth_plugin;
extern uint default_password_lifetime;
extern bool password_require_current;
/*
  @warning : The real value is in @ref partial_revokes. The @ref
  opt_partial_revokes is just a tool to trick the Sys_var class into
  operating on an atomic variable.

  Thus : do not use or access @ref opt_partial_revokes in your code.
         If you need the value of the flag please use the @ref partial_revokes
         global.
  @todo :
    @ref opt_partial_revokes to be removed when the Sys_var classes can operate
         safely on an atomic.
 */
extern bool opt_partial_revokes;
extern char *my_bind_addr_str;
extern char *my_admin_bind_addr_str;
extern uint mysqld_admin_port;
extern bool listen_admin_interface_in_separate_thread;
extern char *binlog_file_basedir_ptr, *binlog_index_basedir_ptr;
extern char *per_user_session_var_default_val_ptr;
extern char *per_user_session_var_user_name_delimiter_ptr;
extern char glob_hostname[HOSTNAME_LENGTH + 1];
extern char system_time_zone[30], *opt_init_file;
extern const char *opt_tc_log_file;
extern char *opt_gap_lock_exception_list;
extern char server_uuid[UUID_LENGTH + 1];
extern const char *server_uuid_ptr;
extern const double log_10[309];
extern ulong binlog_cache_use, binlog_cache_disk_use;
extern ulong binlog_stmt_cache_use, binlog_stmt_cache_disk_use;
extern ulonglong binlog_bytes_written;
extern ulong aborted_threads;
extern ulong delayed_insert_timeout;
extern ulong delayed_insert_limit, delayed_queue_size;
extern std::atomic<int32> atomic_slave_open_temp_tables;
extern ulong slow_launch_time;
extern ulong table_cache_size;
extern ulong schema_def_size;
extern ulong stored_program_def_size;
extern ulong table_def_size;
extern ulong tablespace_def_size;
extern MYSQL_PLUGIN_IMPORT ulong max_connections;
extern ulong opt_max_running_queries, opt_max_waiting_queries;
extern ulong opt_max_db_connections;
extern ulong max_digest_length;
extern ulong max_connect_errors, connect_timeout;
extern ulong max_nonsuper_connections;
extern std::atomic<ulong> nonsuper_connections;
extern bool opt_slave_allow_batching;
extern ulong slave_trans_retries;
extern uint slave_net_timeout;
extern ulong opt_information_schema_engine;
extern ulong opt_mts_slave_parallel_workers;
extern bool opt_mts_dynamic_rebalance;
extern double opt_mts_imbalance_threshold;
extern ulong opt_mts_dependency_replication;
extern ulonglong opt_mts_dependency_size;
extern double opt_mts_dependency_refill_threshold;
extern ulonglong opt_mts_dependency_max_keys;
extern ulonglong opt_mts_pending_jobs_size_max;
extern ulong opt_mts_dependency_order_commits;
extern ulonglong opt_mts_dependency_cond_wait_timeout;
extern ulong rpl_stop_slave_timeout;
extern bool rpl_skip_tx_api;
extern bool log_bin_use_v1_row_events;
extern ulong what_to_log, flush_time;
extern ulong max_prepared_stmt_count, prepared_stmt_count;
extern ulong open_files_limit;
extern bool clone_startup;
extern bool clone_recovery_error;
extern ulong binlog_cache_size, binlog_stmt_cache_size;
extern ulonglong max_binlog_cache_size, max_binlog_stmt_cache_size;
extern int32 opt_binlog_max_flush_queue_time;
extern long opt_binlog_group_commit_sync_delay;
extern ulong opt_binlog_group_commit_sync_no_delay_count;
extern ulong max_binlog_size, max_relay_log_size;
extern ulong slave_max_allowed_packet;
extern ulonglong opt_binlog_rows_event_max_rows;
extern ulong binlog_row_event_max_size;
extern ulong opt_binlog_rows_event_max_size;
extern ulong binlog_checksum_options;
extern ulong binlog_row_metadata;
extern const char *binlog_checksum_type_names[];
extern bool opt_master_verify_checksum;
extern bool opt_slave_sql_verify_checksum;
extern ulong opt_slave_check_before_image_consistency;
extern uint32 gtid_executed_compression_period;
extern bool binlog_gtid_simple_recovery;
extern ulong binlog_error_action;
extern ulong locked_account_connection_count;
extern bool opt_group_replication_plugin_hooks;
extern bool opt_core_file;
extern bool skip_core_dump_on_error;
extern std::atomic<ulonglong> json_contains_key_count;
extern std::atomic<ulonglong> json_array_length_count;
extern std::atomic<ulonglong> json_extract_legacy_count;
extern std::atomic<ulonglong> json_extract_value_count;
extern std::atomic<ulonglong> json_contains_legacy_count;
enum enum_binlog_error_action {
  /// Ignore the error and let server continue without binlogging
  IGNORE_ERROR = 0,
  /// Abort the server
  ABORT_SERVER = 1,
  /// Rollback the trx which failed to flush to binlog and continue.
  /// Transaction which fail to write to binlog during ordered-commit will be
  /// rolled back. The server is not aborted and continues to be up and running
  /// Other cases of flush-error (outside of ordered-commit) will
  /// continue to abort server.
  ROLLBACK_TRX = 2
};
extern const char *binlog_error_action_list[];

extern bool opt_log_ddl;
extern bool slave_high_priority_ddl;
extern ulonglong slave_high_priority_lock_wait_timeout_nsec;
extern double slave_high_priority_lock_wait_timeout_double;
extern std::atomic<ulonglong> slave_high_priority_ddl_killed_connections;
extern std::atomic<ulonglong> slave_high_priority_ddl_executed;
extern std::atomic<ulonglong> slave_commit_order_deadlocks;
extern bool disable_instant_ddl;
extern bool enable_deprecation_warning;

extern ulonglong rbr_unsafe_queries;
extern ulong relay_io_connected;
extern ulong relay_io_events, relay_sql_events;
extern ulonglong relay_io_bytes, relay_sql_bytes;
extern ulonglong relay_sql_wait_time;

/* Minimum HLC value for this instance. It is ensured that the next 'event' will
   get a HLC timestamp greater than this value */
extern ulonglong minimum_hlc_ns;

/* Maximum allowed forward drift in the HLC as compared to wall clock */
extern ulonglong maximum_hlc_drift_ns;
extern bool enable_raft_plugin;
extern bool disable_raft_log_repointing;
extern ulong opt_raft_signal_async_dump_threads;
extern bool disallow_raft;
extern bool override_enable_raft_check;
extern bool abort_on_raft_purge_error;
extern ulonglong apply_log_retention_num;
extern ulonglong apply_log_retention_duration;
extern bool recover_raft_log;
/* Apply log related variables for raft */
extern char *opt_apply_logname;
extern char *opt_applylog_index_name;

/* What should the server do when trxs fail inside ordered commit */
extern ulong opt_commit_consensus_error_action;

/* Enable query checksum validation for queries with a checksum sent */
extern bool enable_query_checksum;

/* Enable resultset checksums */
extern bool enable_resultset_checksum;

extern char *optimizer_force_index_rewrite;
extern mysql_mutex_t LOCK_optimizer_force_index_rewrite_map;
extern std::unordered_map<std::string, std::string>
    optimizer_force_index_rewrite_map;

extern uint net_compression_level;
extern long zstd_net_compression_level;
extern long lz4f_net_compression_level;
extern bool enable_blind_replace;
extern bool set_read_only_on_shutdown;
extern bool show_binlogs_encryption;

/* SHOW STATS var: Name of current timer */
extern const char *timer_in_use;
/* Current timer stats */
extern struct my_timer_unit_info my_timer;
/* Get current time */
extern ulonglong (*my_timer_now)(void);
/* Get time passed since "then" */
inline ulonglong my_timer_since(ulonglong then) {
  return (my_timer_now() - then) - my_timer.overhead;
}
inline ulonglong my_timer_difftime(ulonglong start, ulonglong stop) {
  return (stop - start) - my_timer.overhead;
}
/* Get time passed since "then", and update then to now */
inline ulonglong my_timer_since_and_update(ulonglong *then) {
  ulonglong now = my_timer_now();
  ulonglong ret = (now - (*then)) - my_timer.overhead;
  *then = now;
  return ret;
}
/* Convert native timer units in a ulonglong into seconds in a double */
inline double my_timer_to_seconds(ulonglong when) {
  double ret = (double)(when);
  ret /= (double)(my_timer.frequency);
  return ret;
}
/* Convert native timer units in a ulonglong into milliseconds in a double */
inline double my_timer_to_milliseconds(ulonglong when) {
  double ret = (double)(when);
  ret *= 1000.0;
  ret /= (double)(my_timer.frequency);
  return ret;
}
/* Convert native timer units in a ulonglong into microseconds in a double */
inline double my_timer_to_microseconds(ulonglong when) {
  double ret = (double)(when);
  ret *= 1000000.0;
  ret /= (double)(my_timer.frequency);
  return ret;
}
/* Convert microseconds in a double to native timer units in a ulonglong */
inline ulonglong microseconds_to_my_timer(double when) {
  double ret = when;
  ret *= (double)(my_timer.frequency);
  ret /= 1000000.0;
  return (ulonglong)ret;
}

extern ulong write_control_level;

/* Global variable to denote the maximum CPU time (specified in milliseconds)
 * limit for DML queries.
 */
extern uint write_cpu_limit_milliseconds;

/* Global variable to denote the frequency (specified in number of rows) of
 * checking whether DML queries exceeded the CPU time limit enforced by
 * 'write_time_check_batch'
 */
extern uint write_time_check_batch;

extern bool is_slave;
extern std::atomic<int> slave_stats_daemon_thread_counter;
extern uint write_stats_count;
extern ulong write_stats_frequency;
extern char *latest_write_throttling_rule;
extern char *latest_write_throttle_permissible_dimensions_in_order;
extern std::vector<enum_wtr_dimension>
    write_throttle_permissible_dimensions_in_order;
extern ulong write_start_throttle_lag_milliseconds;
extern ulong write_stop_throttle_lag_milliseconds;
extern double write_throttle_min_ratio;
extern uint write_throttle_monitor_cycles;
extern uint write_throttle_lag_pct_min_secondaries;
extern ulong write_auto_throttle_frequency;
extern uint write_throttle_rate_step;
/* Controls collecting MySQL findings (aka SQL conditions) */
extern ulong sql_findings_control;
/* The maximum size of the memory to store SQL findings */
extern ulonglong max_sql_findings_size;
// client attributes names system variable
extern std::vector<std::string> client_attribute_names;
extern char *latest_client_attribute_names;

/* Controls whether MySQL send an error when running duplicate statements */
extern uint sql_maximum_duplicate_executions;
/* Controls the mode of enforcement of duplicate executions of the same stmt */
extern ulong sql_duplicate_executions_control;

/* Global variable to control collecting column statistics */
extern ulong column_stats_control;

/* Global variable to control collecting index statistics */
extern ulong index_stats_control;

extern bool read_only_slave;
extern bool flush_only_old_table_cache_entries;
extern ulong stored_program_cache_size;
extern ulong back_log;
extern "C" MYSQL_PLUGIN_IMPORT ulong server_id;
extern time_t server_start_time;
extern char *opt_mysql_tmpdir;
extern size_t mysql_unpacked_real_data_home_len;
extern MYSQL_PLUGIN_IMPORT MY_TMPDIR mysql_tmpdir_list;
extern const char *show_comp_option_name[];
extern const char *first_keyword, *binary_keyword;
extern MYSQL_PLUGIN_IMPORT const char *my_localhost;
extern const char *in_left_expr_name;
extern SHOW_VAR status_vars[];
extern struct System_variables max_system_variables;
extern struct System_status_var global_status_var;
extern struct rand_struct sql_rand;
extern handlerton *myisam_hton;
extern handlerton *heap_hton;
extern handlerton *temptable_hton;
extern handlerton *innodb_hton;
extern uint opt_server_id_bits;
extern ulong opt_server_id_mask;
extern const char *load_default_groups[];
extern struct my_option my_long_early_options[];
extern bool mysqld_server_started;
extern "C" MYSQL_PLUGIN_IMPORT int orig_argc;
extern "C" MYSQL_PLUGIN_IMPORT char **orig_argv;
extern my_thread_attr_t connection_attrib;
extern bool old_mode;
extern bool avoid_temporal_upgrade;
extern LEX_STRING opt_init_connect, opt_init_slave;
extern ulong connection_errors_internal;
extern ulong connection_errors_peer_addr;
extern std::atomic<ulong> connection_errors_net_ER_NET_ERROR_ON_WRITE;
extern std::atomic<ulong> connection_errors_net_ER_NET_PACKETS_OUT_OF_ORDER;
extern std::atomic<ulong> connection_errors_net_ER_NET_PACKET_TOO_LARGE;
extern std::atomic<ulong> connection_errors_net_ER_NET_READ_ERROR;
extern std::atomic<ulong> connection_errors_net_ER_NET_READ_INTERRUPTED;
extern std::atomic<ulong> connection_errors_net_ER_NET_UNCOMPRESS_ERROR;
extern std::atomic<ulong> connection_errors_net_ER_NET_WRITE_INTERRUPTED;
extern std::atomic<ulong> acl_db_cache_slow_lookup;
extern ulong acl_db_cache_size;
extern std::atomic<ulong> acl_fast_lookup_miss;
extern bool acl_fast_lookup_enabled;
extern char *opt_log_error_suppression_list;
extern char *opt_log_error_services;
extern char *opt_protocol_compression_algorithms;
extern char *thread_priority_str;

/* Global tmp disk usage max and check. */
extern ulonglong max_tmp_disk_usage;
const ulonglong TMP_DISK_USAGE_DISABLED = -1;
bool is_tmp_disk_usage_over_max();

/* Filesort current usage and peak since startup. */
extern std::atomic<ulonglong> filesort_disk_usage;
extern std::atomic<ulonglong> filesort_disk_usage_peak;

/* Peak for filesort usage atomically reset by show status. */
extern std::atomic<ulonglong> filesort_disk_usage_period_peak;

/* Tmp table current usage and peak since startup. */
extern std::atomic<ulonglong> tmp_table_disk_usage;
extern std::atomic<ulonglong> tmp_table_disk_usage_peak;

/* Peak for tmp table usage atomically reset by show status. */
extern std::atomic<ulonglong> tmp_table_disk_usage_period_peak;

/* Common peak stats operations. */
void update_peak(std::atomic<ulonglong> *peak, ulonglong new_value);
ulonglong reset_peak(std::atomic<ulonglong> *peak,
                     const std::atomic<ulonglong> &value);

/** The size of the host_cache. */
extern uint host_cache_size;
extern ulong log_error_verbosity;
extern ulong slave_tx_isolation;
extern bool enable_xa_transaction;

/* Enable logging queries to a unix local datagram socket */
extern bool log_datagram;
extern ulong log_datagram_usecs;
extern int log_datagram_sock;
bool setup_datagram_socket(sys_var *self, THD *thd, enum_var_type type);

extern bool opt_parthandler_allow_drop_partition;

extern bool persisted_globals_load;
extern bool opt_keyring_operations;
extern bool opt_table_encryption_privilege_check;
extern char *opt_keyring_migration_user;
extern char *opt_keyring_migration_host;
extern char *opt_keyring_migration_password;
extern char *opt_keyring_migration_socket;
extern char *opt_keyring_migration_source;
extern char *opt_keyring_migration_destination;
extern ulong opt_keyring_migration_port;
/**
  Variable to check if connection related options are set
  as part of keyring migration.
*/
extern bool migrate_connect_options;

extern LEX_CSTRING sql_statement_names[(uint)SQLCOM_END + 1];

extern thread_local MEM_ROOT **THR_MALLOC;

extern PSI_file_key key_file_binlog_cache;
extern PSI_file_key key_file_binlog_index_cache;

#ifdef HAVE_PSI_INTERFACE

extern PSI_mutex_key key_LOCK_tc;
extern PSI_mutex_key key_hash_filo_lock;
extern PSI_mutex_key key_LOCK_error_log;
extern PSI_mutex_key key_LOCK_thd_data;
extern PSI_mutex_key key_LOCK_thd_sysvar;
extern PSI_mutex_key key_LOCK_thd_protocol;
extern PSI_mutex_key key_LOCK_thd_db_read_only_hash;
extern PSI_mutex_key key_LOCK_thd_db_context;
extern PSI_mutex_key key_LOCK_thd_db_default_collation_hash;
extern PSI_mutex_key key_LOCK_thd_audit_data;
extern PSI_mutex_key key_LOG_LOCK_log;
extern PSI_mutex_key key_master_info_data_lock;
extern PSI_mutex_key key_master_info_run_lock;
extern PSI_mutex_key key_master_info_sleep_lock;
extern PSI_mutex_key key_master_info_thd_lock;
extern PSI_mutex_key key_master_info_rotate_lock;
extern PSI_mutex_key key_mutex_slave_reporting_capability_err_lock;
extern PSI_mutex_key key_relay_log_info_data_lock;
extern PSI_mutex_key key_relay_log_info_sleep_lock;
extern PSI_mutex_key key_relay_log_info_thd_lock;
extern PSI_mutex_key key_relay_log_info_log_space_lock;
extern PSI_mutex_key key_relay_log_info_run_lock;
extern PSI_mutex_key key_mutex_slave_parallel_pend_jobs;
extern PSI_mutex_key key_mutex_slave_parallel_worker;
extern PSI_mutex_key key_mutex_slave_parallel_worker_count;
extern PSI_mutex_key key_structure_guard_mutex;
extern PSI_mutex_key key_TABLE_SHARE_LOCK_ha_data;
extern PSI_mutex_key key_LOCK_query_plan;
extern PSI_mutex_key key_LOCK_thd_query;
extern PSI_mutex_key key_LOCK_cost_const;
extern PSI_mutex_key key_LOCK_current_cond;
extern PSI_mutex_key key_RELAYLOG_LOCK_commit;
extern PSI_mutex_key key_RELAYLOG_LOCK_commit_queue;
extern PSI_mutex_key key_RELAYLOG_LOCK_done;
extern PSI_mutex_key key_RELAYLOG_LOCK_flush_queue;
extern PSI_mutex_key key_RELAYLOG_LOCK_index;
extern PSI_mutex_key key_RELAYLOG_LOCK_log;
extern PSI_mutex_key key_RELAYLOG_LOCK_log_end_pos;
extern PSI_mutex_key key_RELAYLOG_LOCK_sync;
extern PSI_mutex_key key_RELAYLOG_LOCK_sync_queue;
extern PSI_mutex_key key_RELAYLOG_LOCK_non_xid_trxs;
extern PSI_mutex_key key_RELAYLOG_LOCK_xids;
extern PSI_mutex_key key_RELAYLOG_LOCK_lost_gtids_for_tailing;
extern PSI_mutex_key key_gtid_ensure_index_mutex;
extern PSI_mutex_key key_hlc_wait_mutex;
extern PSI_mutex_key key_mts_temp_table_LOCK;
extern PSI_mutex_key key_mts_gaq_LOCK;
extern PSI_mutex_key key_thd_timer_mutex;
extern PSI_mutex_key key_LOCK_global_write_statistics;
extern PSI_mutex_key key_LOCK_global_write_throttling_rules;
extern PSI_mutex_key key_LOCK_global_write_throttling_log;
extern PSI_mutex_key key_LOCK_replication_lag_auto_throttling;
extern PSI_mutex_key key_LOCK_global_sql_findings;
extern PSI_mutex_key key_LOCK_global_active_sql;

extern PSI_mutex_key key_commit_order_manager_mutex;
extern PSI_mutex_key key_mutex_slave_worker_hash;

extern PSI_mutex_key key_LOCK_ac_node;
extern PSI_mutex_key key_LOCK_ac_info;

extern PSI_rwlock_key key_rwlock_LOCK_column_statistics;
extern PSI_rwlock_key key_rwlock_LOCK_logger;
extern PSI_rwlock_key key_rwlock_channel_map_lock;
extern PSI_rwlock_key key_rwlock_channel_lock;
extern PSI_rwlock_key key_rwlock_receiver_sid_lock;
extern PSI_rwlock_key key_rwlock_rpl_filter_lock;
extern PSI_rwlock_key key_rwlock_channel_to_filter_lock;
extern PSI_rwlock_key key_rwlock_LOCK_gap_lock_exceptions;
extern PSI_rwlock_key key_rwlock_LOCK_index_statistics;
extern PSI_rwlock_key key_rwlock_resource_group_mgr_map_lock;
extern PSI_rwlock_key key_rwlock_LOCK_ac;

extern PSI_cond_key key_PAGE_cond;
extern PSI_cond_key key_COND_active;
extern PSI_cond_key key_COND_pool;
extern PSI_cond_key key_COND_cache_status_changed;
extern PSI_cond_key key_item_func_sleep_cond;
extern PSI_cond_key key_master_info_data_cond;
extern PSI_cond_key key_master_info_start_cond;
extern PSI_cond_key key_master_info_stop_cond;
extern PSI_cond_key key_master_info_sleep_cond;
extern PSI_cond_key key_master_info_rotate_cond;
extern PSI_cond_key key_relay_log_info_data_cond;
extern PSI_cond_key key_relay_log_info_log_space_cond;
extern PSI_cond_key key_relay_log_info_start_cond;
extern PSI_cond_key key_relay_log_info_stop_cond;
extern PSI_cond_key key_relay_log_info_sleep_cond;
extern PSI_cond_key key_cond_slave_parallel_pend_jobs;
extern PSI_cond_key key_cond_slave_parallel_worker;
extern PSI_cond_key key_cond_mts_gaq;
extern PSI_cond_key key_RELAYLOG_COND_done;
extern PSI_cond_key key_RELAYLOG_update_cond;
extern PSI_cond_key key_RELAYLOG_prep_xids_cond;
extern PSI_cond_key key_BINLOG_non_xid_trxs_cond;
extern PSI_cond_key key_RELAYLOG_non_xid_trxs_cond;
extern PSI_cond_key key_gtid_ensure_index_cond;
extern PSI_cond_key key_hlc_wait_cond;
extern PSI_cond_key key_COND_thr_lock;
extern PSI_cond_key key_cond_slave_worker_hash;
extern PSI_cond_key key_commit_order_manager_cond;
extern PSI_cond_key key_COND_ac_node;
extern PSI_thread_key key_thread_bootstrap;
extern PSI_thread_key key_thread_handle_manager;
extern PSI_thread_key key_thread_handle_slave_stats_daemon;
extern PSI_thread_key key_thread_one_connection;
extern PSI_thread_key key_thread_compress_gtid_table;
extern PSI_thread_key key_thread_parser_service;
extern PSI_thread_key key_thread_handle_con_admin_sockets;

extern PSI_file_key key_file_binlog;
extern PSI_file_key key_file_binlog_index;
extern PSI_file_key key_file_dbopt;
extern PSI_file_key key_file_ERRMSG;
extern PSI_file_key key_select_to_file;
extern PSI_file_key key_file_fileparser;
extern PSI_file_key key_file_frm;
extern PSI_file_key key_file_load;
extern PSI_file_key key_file_loadfile;
extern PSI_file_key key_file_log_event_data;
extern PSI_file_key key_file_log_event_info;
extern PSI_file_key key_file_misc;
extern PSI_file_key key_file_tclog;
extern PSI_file_key key_file_trg;
extern PSI_file_key key_file_trn;
extern PSI_file_key key_file_init;
extern PSI_file_key key_file_general_log;
extern PSI_file_key key_file_slow_log;
extern PSI_file_key key_file_gap_lock_log;
extern PSI_file_key key_file_relaylog;
extern PSI_file_key key_file_relaylog_cache;
extern PSI_file_key key_file_relaylog_index;
extern PSI_file_key key_file_relaylog_index_cache;
extern PSI_file_key key_file_sdi;
extern PSI_file_key key_file_hash_join;

extern PSI_socket_key key_socket_tcpip;
extern PSI_socket_key key_socket_unix;
extern PSI_socket_key key_socket_client_connection;

#endif /* HAVE_PSI_INTERFACE */

/*
  MAINTAINER: Please keep this list in order, to limit merge collisions.
  Hint: grep PSI_stage_info | sort -u
*/
extern PSI_stage_info stage_after_create;
extern PSI_stage_info stage_alter_inplace_prepare;
extern PSI_stage_info stage_alter_inplace;
extern PSI_stage_info stage_alter_inplace_commit;
extern PSI_stage_info stage_changing_master;
extern PSI_stage_info stage_checking_master_version;
extern PSI_stage_info stage_checking_permissions;
extern PSI_stage_info stage_cleaning_up;
extern PSI_stage_info stage_closing_tables;
extern PSI_stage_info stage_compressing_gtid_table;
extern PSI_stage_info stage_connecting_to_master;
extern PSI_stage_info stage_converting_heap_to_ondisk;
extern PSI_stage_info stage_copy_to_tmp_table;
extern PSI_stage_info stage_creating_table;
extern PSI_stage_info stage_creating_tmp_table;
extern PSI_stage_info stage_deleting_from_main_table;
extern PSI_stage_info stage_deleting_from_reference_tables;
extern PSI_stage_info stage_discard_or_import_tablespace;
extern PSI_stage_info stage_end;
extern PSI_stage_info stage_executing;
extern PSI_stage_info stage_execution_of_init_command;
extern PSI_stage_info stage_explaining;
extern PSI_stage_info
    stage_finished_reading_one_binlog_switching_to_next_binlog;
extern PSI_stage_info stage_flushing_relay_log_and_master_info_repository;
extern PSI_stage_info stage_flushing_relay_log_info_file;
extern PSI_stage_info stage_freeing_items;
extern PSI_stage_info stage_fulltext_initialization;
extern PSI_stage_info stage_init;
extern PSI_stage_info stage_killing_slave;
extern PSI_stage_info stage_logging_slow_query;
extern PSI_stage_info stage_making_temp_file_append_before_load_data;
extern PSI_stage_info stage_manage_keys;
extern PSI_stage_info stage_master_has_sent_all_binlog_to_slave;
extern PSI_stage_info stage_opening_tables;
extern PSI_stage_info stage_optimizing;
extern PSI_stage_info stage_preparing;
extern PSI_stage_info stage_purging_old_relay_logs;
extern PSI_stage_info stage_query_end;
extern PSI_stage_info stage_queueing_master_event_to_the_relay_log;
extern PSI_stage_info stage_reading_event_from_the_relay_log;
extern PSI_stage_info stage_registering_slave_on_master;
extern PSI_stage_info stage_removing_tmp_table;
extern PSI_stage_info stage_rename;
extern PSI_stage_info stage_rename_result_table;
extern PSI_stage_info stage_requesting_binlog_dump;
extern PSI_stage_info stage_searching_rows_for_update;
extern PSI_stage_info stage_sending_binlog_event_to_slave;
extern PSI_stage_info stage_setup;
extern PSI_stage_info stage_slave_has_read_all_relay_log;
extern PSI_stage_info stage_slave_waiting_event_from_coordinator;
extern PSI_stage_info stage_slave_waiting_for_workers_to_process_queue;
extern PSI_stage_info stage_slave_waiting_worker_queue;
extern PSI_stage_info stage_slave_waiting_worker_to_free_events;
extern PSI_stage_info stage_slave_waiting_worker_to_release_partition;
extern PSI_stage_info stage_slave_waiting_workers_to_exit;
extern PSI_stage_info stage_rpl_apply_row_evt_write;
extern PSI_stage_info stage_rpl_apply_row_evt_update;
extern PSI_stage_info stage_rpl_apply_row_evt_delete;
extern PSI_stage_info stage_sql_thd_waiting_until_delay;
extern PSI_stage_info stage_statistics;
extern PSI_stage_info stage_system_lock;
extern PSI_stage_info stage_update;
extern PSI_stage_info stage_updating;
extern PSI_stage_info stage_updating_main_table;
extern PSI_stage_info stage_updating_reference_tables;
extern PSI_stage_info stage_user_sleep;
extern PSI_stage_info stage_verifying_table;
extern PSI_stage_info stage_waiting_for_gtid_to_be_committed;
extern PSI_stage_info stage_waiting_for_handler_commit;
extern PSI_stage_info stage_waiting_for_master_to_send_event;
extern PSI_stage_info stage_waiting_for_master_update;
extern PSI_stage_info stage_waiting_for_relay_log_space;
extern PSI_stage_info stage_waiting_for_slave_mutex_on_exit;
extern PSI_stage_info stage_waiting_for_slave_thread_to_start;
extern PSI_stage_info stage_waiting_for_table_flush;
extern PSI_stage_info stage_waiting_for_the_next_event_in_relay_log;
extern PSI_stage_info stage_waiting_for_the_slave_thread_to_advance_position;
extern PSI_stage_info stage_waiting_to_finalize_termination;
extern PSI_stage_info stage_worker_waiting_for_its_turn_to_commit;
extern PSI_stage_info stage_worker_waiting_for_commit_parent;
extern PSI_stage_info stage_suspending;
extern PSI_stage_info stage_starting;
extern PSI_stage_info stage_waiting_for_no_channel_reference;
extern PSI_stage_info stage_hook_begin_trans;
extern PSI_stage_info stage_binlog_transaction_compress;
extern PSI_stage_info stage_binlog_transaction_decompress;
extern PSI_stage_info stage_slave_waiting_for_dependencies;
extern PSI_stage_info stage_slave_waiting_for_dependency_workers;
extern PSI_stage_info stage_waiting_for_hlc;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
/**
  Statement instrumentation keys (sql).
  The last entry, at [SQLCOM_END], is for parsing errors.
*/
extern PSI_statement_info sql_statement_info[(uint)SQLCOM_END + 1];

/**
  Statement instrumentation keys (com).
  The last entry, at [COM_END], is for packet errors.
*/
extern PSI_statement_info com_statement_info[(uint)COM_TOP_END];

/**
  Statement instrumentation key for replication.
*/
extern PSI_statement_info stmt_info_rpl;
#endif /* HAVE_PSI_STATEMENT_INTERFACE */

extern struct st_VioSSLFd *ssl_acceptor_fd;

extern bool opt_large_pages;
extern uint opt_large_page_size;
extern char lc_messages_dir[FN_REFLEN];
extern bool legacy_global_read_lock_mode;
extern char *lc_messages_dir_ptr;
extern const char *log_error_dest;
extern MYSQL_PLUGIN_IMPORT char reg_ext[FN_EXTLEN];
extern MYSQL_PLUGIN_IMPORT uint reg_ext_length;
extern MYSQL_PLUGIN_IMPORT uint lower_case_table_names;

extern long tc_heuristic_recover;

extern ulong specialflag;
extern size_t mysql_data_home_len;
extern const char *mysql_real_data_home_ptr;
extern MYSQL_PLUGIN_IMPORT char *mysql_data_home;
extern "C" MYSQL_PLUGIN_IMPORT char server_version[SERVER_VERSION_LENGTH];
extern MYSQL_PLUGIN_IMPORT char mysql_real_data_home[];
extern char mysql_unpacked_real_data_home[];
extern MYSQL_PLUGIN_IMPORT struct System_variables global_system_variables;
extern char default_logfile_name[FN_REFLEN];
extern bool log_bin_supplied;
extern char default_binlogfile_name[FN_REFLEN];
extern MYSQL_PLUGIN_IMPORT char pidfile_name[];

#define mysql_tmpdir (my_tmpdir(&mysql_tmpdir_list))

/* Time handling client commands for replication */
extern ulonglong command_slave_seconds;

extern bool skip_flush_master_info;
extern bool skip_flush_relay_worker_info;

/*
  Server mutex locks and condition variables.
 */
extern mysql_mutex_t LOCK_status;
extern mysql_mutex_t LOCK_uuid_generator;
extern mysql_mutex_t LOCK_crypt;
extern mysql_mutex_t LOCK_manager;
extern mysql_mutex_t LOCK_slave_stats_daemon;
extern mysql_mutex_t LOCK_global_write_statistics;
extern mysql_mutex_t LOCK_global_write_throttling_rules;
extern mysql_mutex_t LOCK_global_write_throttling_log;
extern mysql_mutex_t LOCK_replication_lag_auto_throttling;
extern mysql_mutex_t LOCK_global_sql_findings;
extern mysql_mutex_t LOCK_global_active_sql;
extern mysql_mutex_t LOCK_global_system_variables;
extern mysql_mutex_t LOCK_user_conn;
extern mysql_mutex_t LOCK_log_throttle_qni;
extern mysql_mutex_t LOCK_log_throttle_ddl;
extern mysql_mutex_t LOCK_prepared_stmt_count;
extern mysql_mutex_t LOCK_error_messages;
extern mysql_mutex_t LOCK_sql_slave_skip_counter;
extern mysql_mutex_t LOCK_slave_net_timeout;
extern mysql_mutex_t LOCK_slave_trans_dep_tracker;
extern mysql_mutex_t LOCK_mandatory_roles;
extern mysql_mutex_t LOCK_password_history;
extern mysql_mutex_t LOCK_password_reuse_interval;
extern mysql_mutex_t LOCK_default_password_lifetime;
extern mysql_mutex_t LOCK_server_started;
extern mysql_mutex_t LOCK_reset_gtid_table;
extern mysql_mutex_t LOCK_compress_gtid_table;
extern mysql_mutex_t LOCK_keyring_operations;
extern mysql_mutex_t LOCK_collect_instance_log;
extern mysql_mutex_t LOCK_tls_ctx_options;
extern mysql_mutex_t LOCK_rotate_binlog_master_key;

extern mysql_cond_t COND_server_started;
extern mysql_cond_t COND_compress_gtid_table;
extern mysql_cond_t COND_manager;
extern mysql_cond_t COND_slave_stats_daemon;

extern mysql_rwlock_t LOCK_column_statistics;
extern mysql_rwlock_t LOCK_index_statistics;
extern mysql_rwlock_t LOCK_sys_init_connect;
extern mysql_rwlock_t LOCK_sys_init_slave;
extern mysql_rwlock_t LOCK_system_variables_hash;

extern uint performance_schema_max_sql_text_length;

extern ulong opt_ssl_fips_mode;

extern char *opt_disabled_storage_engines;

extern sigset_t mysqld_signal_mask;
/* query_id */
typedef int64 query_id_t;
extern std::atomic<query_id_t> atomic_global_query_id;

int *get_remaining_argc();
char ***get_remaining_argv();

/* increment query_id and return it.  */
inline MY_ATTRIBUTE((warn_unused_result)) query_id_t next_query_id() {
  return ++atomic_global_query_id;
}

#define ER(X) please_use_ER_THD_or_ER_DEFAULT_instead(X)

/* Accessor function for _connection_events_loop_aborted flag */
inline MY_ATTRIBUTE(
    (warn_unused_result)) bool connection_events_loop_aborted() {
  return connection_events_loop_aborted_flag.load();
}

/* only here because of unireg_init(). */
static inline void set_connection_events_loop_aborted(bool value) {
  connection_events_loop_aborted_flag.store(value);
}

/**

  Check if --help option or --validate-config is specified.

  @retval false   Neither 'help' or 'validate-config' option is enabled.
  @retval true    Either 'help' or 'validate-config' or both options
                  are enabled.
*/
inline bool is_help_or_validate_option() {
  return (opt_help || opt_validate_config);
}

/**
  Get mysqld offline mode.

  @return a bool indicating the offline mode status of the server.
*/
inline bool mysqld_offline_mode() { return offline_mode.load(); }

/**
  Set offline mode with a given value

  @param value true or false indicating the offline mode status of server.
*/
inline void set_mysqld_offline_mode(bool value) { offline_mode.store(value); }

/* write_stats_capture_enabled
     Returns TRUE if capturing of write statistics is enabled
 */
inline bool write_stats_capture_enabled() {
  return write_stats_count > 0 && write_stats_frequency > 0;
}

/* sql_id_is_needed
     Returns TRUE if SQL_ID is needed:
     - SQL Findings
     - write statistics
     - write throttling
     - column statistics
 */
inline bool sql_id_is_needed() {
  bool needed = (sql_findings_control == SQL_INFO_CONTROL_ON ||
                         column_stats_control == SQL_INFO_CONTROL_ON ||
                         write_stats_capture_enabled() ||
                         write_control_level != CONTROL_LEVEL_OFF
                     ? true
                     : false);
  return needed;
}

/**
  Get status partial_revokes on server

  @return a bool indicating partial_revokes status of the server.
    @retval true  Parital revokes is ON
    @retval flase Partial revokes is OFF
*/
bool mysqld_partial_revokes();

/**
  Set partial_revokes with a given value

  @param value true or false indicating the status of partial revokes
               turned ON/OFF on server.
*/
void set_mysqld_partial_revokes(bool value);

#ifdef _WIN32

bool is_windows_service();
NTService *get_win_service_ptr();
bool update_named_pipe_full_access_group(const char *new_group_name);

#endif

extern LEX_STRING opt_mandatory_roles;
extern bool opt_mandatory_roles_cache;
extern bool opt_always_activate_granted_roles;

extern mysql_component_t mysql_component_mysql_server;
extern mysql_component_t mysql_component_performance_schema;
/* This variable is a registry handler, defined in mysql_server component and
   used as a output parameter for minimal chassis. */
extern SERVICE_TYPE_NO_CONST(registry) * srv_registry;
/* These global variables which are defined and used in
   mysql_server component */
extern SERVICE_TYPE(dynamic_loader_scheme_file) * scheme_file_srv;
extern SERVICE_TYPE(dynamic_loader) * dynamic_loader_srv;

/* Histogram struct to track various latencies */
#define NUMBER_OF_HISTOGRAM_BINS 10
struct latency_histogram {
  size_t num_bins;
  ulonglong step_size;
  double step_ratio;
  std::atomic<ulonglong> count_per_bin[NUMBER_OF_HISTOGRAM_BINS];
};

typedef latency_histogram counter_histogram;

/* Convert native timer units in a ulonglong into microseconds in a ulonglong */
inline ulonglong my_timer_to_microseconds_ulonglong(ulonglong when) {
  ulonglong ret = (ulonglong)(when);
  ret *= 1000000;
  ret = (ulonglong)((ret + my_timer.frequency - 1) / my_timer.frequency);
  return ret;
}

/** Compression statistics for a fil_space */
/**
  Create a new Histogram.

  @param current_histogram    The histogram being initialized.
  @param step_size_with_unit  Configurable system variable containing
                              step size and unit of the Histogram.
*/
void latency_histogram_init(latency_histogram *current_histogram,
                            const char *step_size_with_unit);

void counter_histogram_init(latency_histogram *current_histogram,
                            ulonglong step_value);

/**
  Increment the count of a bin in Histogram.

  @param current_histogram  The current histogram.
  @param value              Value of which corresponding bin has to be found.
  @param count              Amount by which the count of a bin has to be
                            increased.

*/
void latency_histogram_increment(latency_histogram *current_histogram,
                                 ulonglong value, ulonglong count);

void counter_histogram_increment(latency_histogram *current_histogram,
                                 ulonglong value);

/**
  Get the count corresponding to a bin of the Histogram.

  @param current_histogram  The current histogram.
  @param bin_num            The bin whose count has to be returned.

  @return                   Returns the count of that bin.
*/
ulonglong latency_histogram_get_count(latency_histogram *current_histogram,
                                      size_t bin_num);

/**
  Validate if the string passed to the configurable histogram step size
  conforms to proper syntax.

  @param step_size_with_unit  The configurable step size string to be checked.

  @return                     1 if invalid, 0 if valid.
*/
int histogram_validate_step_size_string(const char *step_size_with_unit);

#define HISTOGRAM_BUCKET_NAME_MAX_SIZE \
  64 /**< This is the maximum size     \
        of the string:                 \
        "LowerBucketValue-"            \
        "UpperBucketValue<units>"      \
        where bucket is the latency    \
        histogram bucket and units     \
        can be us,ms or s */

/** To return the displayable histogram name from
  my_timer_to_display_string() */
struct histogram_display_string {
  char name[HISTOGRAM_BUCKET_NAME_MAX_SIZE];
};

/**
  This function is called to convert the histogram bucket ranges in system time
  units to a string and calculates units on the fly, which can be displayed in
  the output of SHOW GLOBAL STATUS.
  The string has the following form:

  <HistogramName>_<BucketLowerValue>-<BucketUpperValue><Unit>

  @param bucket_lower_display  Lower Range value of the Histogram Bucket
  @param bucket_upper_display  Upper Range value of the Histogram Bucket
  @param is_last_bucket        Flag to denote last bucket in the histogram

  @return                      The display string for the Histogram Bucket
*/
histogram_display_string histogram_bucket_to_display_string(
    ulonglong bucket_lower_display, ulonglong bucket_upper_display,
    bool is_last_bucket = false);

/**
  This function is called by the plugin callback function
  to add entries into the latency_histogram_xxxx array, by forming
  the appropriate display string and fetching the histogram bin
  counts.

  @param current_histogram       Histogram whose values are currently added
                                 in the SHOW_VAR array
  @param latency_histogram_data  SHOW_VAR array for the corresponding Histogram
  @param histogram_values        Values to be exported to Innodb status.
                                 This array contains the bin counts of the
                                 respective Histograms.
*/
void prepare_latency_histogram_vars(latency_histogram *current_histogram,
                                    SHOW_VAR *latency_histogram_data,
                                    ulonglong *histogram_values);

void prepare_counter_histogram_vars(latency_histogram *current_histogram,
                                    SHOW_VAR *latency_histogram_data,
                                    ulonglong *histogram_values);

/**
   Frees old histogram bucket display strings before assigning new ones.
*/
void free_latency_histogram_sysvars(SHOW_VAR *latency_histogram_data);

/**
   Get parallel replication config
*/
ulong get_mts_parallel_option();

/**
   Returns whether slave commit order is preserved
*/
bool get_slave_preserve_commit_order();

/**
 * Set the priority of an OS thread.
 *
 * @param thread_priority_str  A string of the format os_thread_id:nice_val.
 * @return                     true on success, false otherwise.
 */
bool set_thread_priority(char *thread_priority_str);

/**
 * Set priority of the current thread.
 *
 * @return true on success, false otherwise.
 */
bool set_current_thread_priority();

/**
 * Set the priority of an OS thread.
 *
 * @param tid  The OS thread id.
 * @param pri  The priority to set the thread to.
 * @return     true on success, false otherwise.
 */
bool set_system_thread_priority(pid_t tid, int pri);

/**
 * Check if we should be using another index for FORCE INDEX.
 *
 * @param lookup The current index used in FORCE IDNEX.
 * @param out    The new index to use, only populated when true is returned
 * @return       true if a different index should be used
 */
bool lookup_optimizer_force_index_rewrite(const std::string &lookup,
                                          std::string *out);

#endif /* MYSQLD_INCLUDED */
