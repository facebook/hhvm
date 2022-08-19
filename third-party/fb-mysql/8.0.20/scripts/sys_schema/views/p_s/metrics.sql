-- Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
--
--   This program is free software; you can redistribute it and/or modify
--   it under the terms of the GNU General Public License as published by
--   the Free Software Foundation; version 2 of the License.
--
--   This program is distributed in the hope that it will be useful,
--   but WITHOUT ANY WARRANTY; without even the implied warranty of
--   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--   GNU General Public License for more details.
--
--   You should have received a copy of the GNU General Public License
--   along with this program; if not, write to the Free Software
--   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA 

-- IMPORTANT
-- If you update this view, also update the "5.7+ and the Performance Schema disabled"
-- query in procedures/diagnostics.sql

-- View: metrics
-- 
-- Creates a union of the following information:
--
--    *  performance_schema.global_status
--    *  information_schema.INNODB_METRICS
--    *  Performance Schema global memory usage information
--    *  Current time
--
-- For view has the following columns:
-- 
--    * Variable_name: The name of the variable
--    * Variable_value: The value of the variable
--    * Type: The type of the variable. This will depend on the source, e.g. Global Status, InnoDB Metrics - ..., etc.
--    * Enabled: Whether the variable is enabled or not. Possible values are 'YES', 'NO', 'PARTIAL'.
--      PARTIAL is currently only supported for the memory usage variables and means some but not all of the memory/% instruments
--      are enabled.
--
-- mysql> SELECT * FROM metrics;
-- +-----------------------------------------------+-------------------------...+--------------------------------------+---------+
-- | Variable_name                                 | Variable_value          ...| Type                                 | Enabled |
-- +-----------------------------------------------+-------------------------...+--------------------------------------+---------+
-- | aborted_clients                               | 0                       ...| Global Status                        | YES     |
-- | aborted_connects                              | 0                       ...| Global Status                        | YES     |
-- | binlog_cache_disk_use                         | 0                       ...| Global Status                        | YES     |
-- | binlog_cache_use                              | 0                       ...| Global Status                        | YES     |
-- | binlog_stmt_cache_disk_use                    | 0                       ...| Global Status                        | YES     |
-- | binlog_stmt_cache_use                         | 0                       ...| Global Status                        | YES     |
-- | bytes_received                                | 217081                  ...| Global Status                        | YES     |
-- | bytes_sent                                    | 27257                   ...| Global Status                        | YES     |
-- ...
-- | innodb_rwlock_x_os_waits                      | 0                       ...| InnoDB Metrics - server              | YES     |
-- | innodb_rwlock_x_spin_rounds                   | 2723                    ...| InnoDB Metrics - server              | YES     |
-- | innodb_rwlock_x_spin_waits                    | 1                       ...| InnoDB Metrics - server              | YES     |
-- | trx_active_transactions                       | 0                       ...| InnoDB Metrics - transaction         | NO      |
-- ...
-- | trx_rseg_current_size                         | 0                       ...| InnoDB Metrics - transaction         | NO      |
-- | trx_rseg_history_len                          | 4                       ...| InnoDB Metrics - transaction         | YES     |
-- | trx_rw_commits                                | 0                       ...| InnoDB Metrics - transaction         | NO      |
-- | trx_undo_slots_cached                         | 0                       ...| InnoDB Metrics - transaction         | NO      |
-- | trx_undo_slots_used                           | 0                       ...| InnoDB Metrics - transaction         | NO      |
-- | memory_current_allocated                      | 138244216               ...| Performance Schema                   | PARTIAL |
-- | memory_total_allocated                        | 138244216               ...| Performance Schema                   | PARTIAL |
-- | NOW()                                         | 2015-05-31 13:27:50.382 ...| System Time                          | YES     |
-- | UNIX_TIMESTAMP()                              | 1433042870.382          ...| System Time                          | YES     |
-- +-----------------------------------------------+-------------------------...+--------------------------------------+---------+
-- 412 rows in set (0.02 sec)

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW metrics (
  Variable_name,
  Variable_value,
  Type,
  Enabled
) AS
(
SELECT LOWER(VARIABLE_NAME) AS Variable_name, VARIABLE_VALUE AS Variable_value, 'Global Status' AS Type, 'YES' AS Enabled
  FROM performance_schema.global_status
) UNION ALL (
SELECT NAME AS Variable_name, COUNT AS Variable_value,
       CONCAT('InnoDB Metrics - ', SUBSYSTEM) AS Type,
       IF(STATUS = 'enabled', 'YES', 'NO') AS Enabled
  FROM information_schema.INNODB_METRICS
  -- De duplication - some variables exists both in GLOBAL_STATUS and INNODB_METRICS
  -- Keep the one from GLOBAL_STATUS as it is always enabled and it's more likely to be used for existing tools.
 WHERE NAME NOT IN (
     'lock_row_lock_time', 'lock_row_lock_time_avg', 'lock_row_lock_time_max', 'lock_row_lock_waits',
     'buffer_pool_reads', 'buffer_pool_read_requests', 'buffer_pool_write_requests', 'buffer_pool_wait_free',
     'buffer_pool_read_ahead', 'buffer_pool_read_ahead_evicted', 'buffer_pool_pages_total', 'buffer_pool_pages_misc',
     'buffer_pool_pages_data', 'buffer_pool_bytes_data', 'buffer_pool_pages_dirty', 'buffer_pool_bytes_dirty',
     'buffer_pool_pages_free', 'buffer_pages_created', 'buffer_pages_written', 'buffer_pages_read',
     'buffer_data_reads', 'buffer_data_written', 'file_num_open_files',
     'os_log_bytes_written', 'os_log_fsyncs', 'os_log_pending_fsyncs', 'os_log_pending_writes',
     'log_waits', 'log_write_requests', 'log_writes', 'innodb_dblwr_writes', 'innodb_dblwr_pages_written', 'innodb_page_size')
) UNION ALL (
SELECT 'memory_current_allocated' AS Variable_name, SUM(CURRENT_NUMBER_OF_BYTES_USED) AS Variable_value, 'Performance Schema' AS Type,
        IF((SELECT COUNT(*) FROM performance_schema.setup_instruments WHERE NAME LIKE 'memory/%' AND ENABLED = 'YES') = 0, 'NO',
        IF((SELECT COUNT(*) FROM performance_schema.setup_instruments WHERE NAME LIKE 'memory/%' AND ENABLED = 'NO') = 0, 'YES',
            'PARTIAL')) AS Enabled
  FROM performance_schema.memory_summary_global_by_event_name
) UNION ALL (
SELECT 'memory_total_allocated' AS Variable_name, SUM(SUM_NUMBER_OF_BYTES_ALLOC) AS Variable_value, 'Performance Schema' AS Type,
        IF((SELECT COUNT(*) FROM performance_schema.setup_instruments WHERE NAME LIKE 'memory/%' AND ENABLED = 'YES') = 0, 'NO',
        IF((SELECT COUNT(*) FROM performance_schema.setup_instruments WHERE NAME LIKE 'memory/%' AND ENABLED = 'NO') = 0, 'YES',
            'PARTIAL')) AS Enabled
  FROM performance_schema.memory_summary_global_by_event_name
) UNION ALL (
SELECT 'NOW()' AS Variable_name, NOW(3) AS Variable_value, 'System Time' AS Type, 'YES' AS Enabled
) UNION ALL (
SELECT 'UNIX_TIMESTAMP()' AS Variable_name, ROUND(UNIX_TIMESTAMP(NOW(3)), 3) AS Variable_value, 'System Time' AS Type, 'YES' AS Enabled
)
 ORDER BY Type, Variable_name;
