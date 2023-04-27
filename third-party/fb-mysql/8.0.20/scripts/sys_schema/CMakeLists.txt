# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA


SET(SYS_SCHEMA_SOURCES
  before_setup.sql

  init_priv.sql

  views/version.sql

  tables/sys_config.sql
  tables/sys_config_alter.sql
  tables/sys_config_data.sql

  triggers/sys_config_insert_set_user.sql
  triggers/sys_config_update_set_user.sql

  functions/extract_schema_from_file_name.sql
  functions/extract_schema_from_file_name_linux.sql
  functions/extract_table_from_file_name.sql
  functions/extract_table_from_file_name_linux.sql
  functions/format_bytes.sql
  functions/format_path.sql
  functions/format_statement.sql
  functions/format_time.sql
  functions/list_add.sql
  functions/list_drop.sql
  functions/ps_is_account_enabled.sql
  functions/ps_is_consumer_enabled.sql
  functions/ps_is_instrument_default_enabled.sql
  functions/ps_is_instrument_default_timed.sql
  functions/ps_is_thread_instrumented.sql
  functions/ps_thread_id.sql
  functions/ps_thread_account.sql
  functions/ps_thread_stack.sql
  functions/ps_thread_trx_info.sql
  functions/quote_identifier.sql
  functions/sys_get_config.sql
  functions/version_major.sql
  functions/version_minor.sql
  functions/version_patch.sql
  
  views/i_s/innodb_buffer_stats_by_schema.sql
  views/i_s/x_innodb_buffer_stats_by_schema.sql
  views/i_s/innodb_buffer_stats_by_table.sql
  views/i_s/x_innodb_buffer_stats_by_table.sql
  views/i_s/schema_object_overview.sql
  views/i_s/schema_auto_increment_columns.sql
  views/i_s/x_schema_flattened_keys.sql
  views/i_s/schema_redundant_indexes.sql
  views/p_s/ps_check_lost_instrumentation.sql
  views/p_s/latest_file_io.sql
  views/p_s/x_latest_file_io.sql
  views/p_s/io_by_thread_by_latency.sql
  views/p_s/x_io_by_thread_by_latency.sql
  views/p_s/io_global_by_file_by_bytes.sql
  views/p_s/x_io_global_by_file_by_bytes.sql
  views/p_s/io_global_by_file_by_latency.sql
  views/p_s/x_io_global_by_file_by_latency.sql
  views/p_s/io_global_by_wait_by_bytes.sql
  views/p_s/x_io_global_by_wait_by_bytes.sql
  views/p_s/io_global_by_wait_by_latency.sql
  views/p_s/x_io_global_by_wait_by_latency.sql
  views/p_s/innodb_lock_waits.sql
  views/p_s/x_innodb_lock_waits.sql
  views/p_s/memory_by_user_by_current_bytes.sql
  views/p_s/x_memory_by_user_by_current_bytes.sql
  views/p_s/memory_by_host_by_current_bytes.sql
  views/p_s/x_memory_by_host_by_current_bytes.sql
  views/p_s/memory_by_thread_by_current_bytes.sql
  views/p_s/x_memory_by_thread_by_current_bytes.sql
  views/p_s/memory_global_by_current_bytes.sql
  views/p_s/x_memory_global_by_current_bytes.sql
  views/p_s/memory_global_total.sql
  views/p_s/x_memory_global_total.sql
  views/p_s/fb_ps_schema_table_statistics_io.sql
  views/p_s/schema_index_statistics.sql
  views/p_s/x_schema_index_statistics.sql
  views/p_s/x_ps_schema_table_statistics_io.sql
  views/p_s/schema_table_statistics.sql
  views/p_s/x_schema_table_statistics.sql
  views/p_s/schema_table_statistics_with_buffer.sql
  views/p_s/x_schema_table_statistics_with_buffer.sql
  views/p_s/schema_tables_with_full_table_scans.sql
  views/p_s/x_schema_tables_with_full_table_scans.sql
  views/p_s/schema_unused_indexes.sql
  views/p_s/schema_table_lock_waits.sql
  views/p_s/x_schema_table_lock_waits.sql
  views/p_s/statement_analysis.sql
  views/p_s/x_statement_analysis.sql
  views/p_s/statements_with_errors_or_warnings.sql
  views/p_s/x_statements_with_errors_or_warnings.sql
  views/p_s/statements_with_full_table_scans.sql
  views/p_s/x_statements_with_full_table_scans.sql
  views/p_s/x_ps_digest_avg_latency_distribution.sql
  views/p_s/x_ps_digest_95th_percentile_by_avg_us.sql
  views/p_s/statements_with_runtimes_in_95th_percentile.sql
  views/p_s/x_statements_with_runtimes_in_95th_percentile.sql
  views/p_s/statements_with_sorting.sql
  views/p_s/x_statements_with_sorting.sql
  views/p_s/statements_with_temp_tables.sql
  views/p_s/x_statements_with_temp_tables.sql
  views/p_s/user_summary_by_file_io_type.sql
  views/p_s/x_user_summary_by_file_io_type.sql
  views/p_s/user_summary_by_file_io.sql
  views/p_s/x_user_summary_by_file_io.sql
  views/p_s/user_summary_by_statement_type.sql
  views/p_s/x_user_summary_by_statement_type.sql
  views/p_s/user_summary_by_statement_latency.sql
  views/p_s/x_user_summary_by_statement_latency.sql
  views/p_s/user_summary_by_stages.sql
  views/p_s/x_user_summary_by_stages.sql
  views/p_s/user_summary.sql
  views/p_s/x_user_summary.sql
  views/p_s/host_summary_by_file_io_type.sql
  views/p_s/x_host_summary_by_file_io_type.sql
  views/p_s/host_summary_by_file_io.sql
  views/p_s/x_host_summary_by_file_io.sql
  views/p_s/host_summary_by_statement_type.sql
  views/p_s/x_host_summary_by_statement_type.sql
  views/p_s/host_summary_by_statement_latency.sql
  views/p_s/x_host_summary_by_statement_latency.sql
  views/p_s/host_summary_by_stages.sql
  views/p_s/x_host_summary_by_stages.sql
  views/p_s/host_summary.sql
  views/p_s/x_host_summary.sql
  views/p_s/wait_classes_global_by_avg_latency.sql
  views/p_s/x_wait_classes_global_by_avg_latency.sql
  views/p_s/wait_classes_global_by_latency.sql
  views/p_s/x_wait_classes_global_by_latency.sql
  views/p_s/waits_by_user_by_latency.sql
  views/p_s/x_waits_by_user_by_latency.sql
  views/p_s/waits_by_host_by_latency.sql
  views/p_s/x_waits_by_host_by_latency.sql
  views/p_s/waits_global_by_latency.sql
  views/p_s/x_waits_global_by_latency.sql
  views/p_s/metrics.sql
  views/p_s/processlist.sql
  views/p_s/x_processlist.sql
  views/p_s/sessions.sql
  views/p_s/x_sessions.sql
  views/p_s/session_ssl_status.sql

  procedures/create_synonym_db.sql
  procedures/execute_prepared_stmt.sql
  procedures/diagnostics.sql
  procedures/ps_statement_avg_latency_histogram.sql
  procedures/ps_trace_statement_digest.sql
  procedures/ps_trace_thread.sql
  procedures/ps_setup_disable_background_threads.sql
  procedures/ps_setup_disable_consumer.sql
  procedures/ps_setup_disable_instrument.sql
  procedures/ps_setup_disable_thread.sql
  procedures/ps_setup_enable_background_threads.sql
  procedures/ps_setup_enable_consumer.sql
  procedures/ps_setup_enable_instrument.sql
  procedures/ps_setup_enable_thread.sql
  procedures/ps_setup_reload_saved.sql
  procedures/ps_setup_reset_to_default.sql
  procedures/ps_setup_save.sql
  procedures/ps_setup_show_disabled.sql
  procedures/ps_setup_show_disabled_consumers.sql
  procedures/ps_setup_show_disabled_instruments.sql
  procedures/ps_setup_show_enabled.sql
  procedures/ps_setup_show_enabled_consumers.sql
  procedures/ps_setup_show_enabled_instruments.sql
  procedures/ps_truncate_all_tables.sql
  procedures/statement_performance_analyzer.sql
  procedures/table_exists.sql

  after_setup.sql
  )


ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ALL_SYS_SCHEMA.sql
  COMMAND ${CMAKE_COMMAND}
  -D "OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
  -D "SYS_SCHEMA_SOURCES=${SYS_SCHEMA_SOURCES}"
  -P ${CMAKE_CURRENT_SOURCE_DIR}/run_gensysschema.cmake
  DEPENDS ${SYS_SCHEMA_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating Sys Schema SQL code"
  VERBATIM
)

ADD_CUSTOM_TARGET(all_sys_schema DEPENDS ALL_SYS_SCHEMA.sql)


ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands.h
  COMMAND comp_sql
  mysql_sys_schema
  ${CMAKE_CURRENT_BINARY_DIR}/ALL_SYS_SCHEMA.sql
  sql_commands.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql all_sys_schema
  ${CMAKE_CURRENT_BINARY_DIR}/ALL_SYS_SCHEMA.sql
  VERBATIM
)

ADD_CUSTOM_TARGET(sql_commands
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sql_commands.h)

ADD_CUSTOM_TARGET(
  GenSysSchemaC
  ALL
  DEPENDS comp_sql sql_commands
  COMMENT "Generating Sys Schema C code"
)

IF(NOT WITHOUT_SERVER)
  ADD_CUSTOM_TARGET(GenSysSchema
    ALL
    DEPENDS sql_commands
  )
ENDIF()

