-- Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; version 2 of the License.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--
-- View: x$processlist
--
-- A detailed non-blocking processlist view to replace 
-- [INFORMATION_SCHEMA. | SHOW FULL] PROCESSLIST
-- 
-- Performs less locking than the legacy sources, whilst giving extra information.
--
-- mysql> select * from sys.x$processlist where conn_id is not null and command != 'daemon' and conn_id != connection_id()\G
-- ...
-- *************************** 2. row ***************************
--                 thd_id: 720
--                conn_id: 698
--                   user: msandbox@localhost
--                     db: test
--                command: Query
--                  state: alter table (read PK and internal sort)
--                   time: 2
--      current_statement: alter table t1 add column l int
--      statement_latency: 2349834276374
--               progress: 60.00
--           lock_latency: 339707000000
--          rows_examined: 0
--              rows_sent: 0
--          rows_affected: 0
--             tmp_tables: 0
--        tmp_disk_tables: 0
--              full_scan: NO
--         last_statement: NULL
-- last_statement_latency: NULL
--         current_memory: 10186821
--              last_wait: wait/io/file/innodb/innodb_data_file
--      last_wait_latency: Still Waiting
--                 source: fil0fil.cc:5351
--            trx_latency: NULL
--              trx_state: NULL
--         trx_autocommit: NULL
--                    pid: 5559
--           program_name: mysql
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER
VIEW x$processlist (
  thd_id,
  conn_id,
  user,
  db,
  command,
  state,
  time,
  current_statement,
  statement_latency,
  progress,
  lock_latency,
  rows_examined,
  rows_sent,
  rows_affected,
  tmp_tables,
  tmp_disk_tables,
  full_scan,
  last_statement,
  last_statement_latency,
  current_memory,
  last_wait,
  last_wait_latency,
  source,
  trx_latency,
  trx_state,
  trx_autocommit,
  pid,
  program_name
) AS
SELECT pps.thread_id AS thd_id,
       pps.processlist_id AS conn_id,
       IF(pps.name IN ('thread/sql/one_connection', 'thread/thread_pool/tp_one_connection'),
          CONCAT(pps.processlist_user, '@', pps.processlist_host),
          REPLACE(pps.name, 'thread/', '')) user,
       pps.processlist_db AS db,
       pps.processlist_command AS command,
       pps.processlist_state AS state,
       pps.processlist_time AS time,
       pps.processlist_info AS current_statement,
       IF(esc.end_event_id IS NULL,
          esc.timer_wait,
          NULL) AS statement_latency,
       IF(esc.end_event_id IS NULL,
          ROUND(100 * (estc.work_completed / estc.work_estimated), 2),
          NULL) AS progress,
       esc.lock_time AS lock_latency,
       esc.rows_examined AS rows_examined,
       esc.rows_sent AS rows_sent,
       esc.rows_affected AS rows_affected,
       esc.created_tmp_tables AS tmp_tables,
       esc.created_tmp_disk_tables AS tmp_disk_tables,
       IF(esc.no_good_index_used > 0 OR esc.no_index_used > 0, 'YES', 'NO') AS full_scan,
       IF(esc.end_event_id IS NOT NULL,
          esc.sql_text,
          NULL) AS last_statement,
       IF(esc.end_event_id IS NOT NULL,
          esc.timer_wait,
          NULL) AS last_statement_latency,
       mem.current_allocated AS current_memory,
       ewc.event_name AS last_wait,
       IF(ewc.end_event_id IS NULL AND ewc.event_name IS NOT NULL,
          'Still Waiting', 
          ewc.timer_wait) last_wait_latency,
       ewc.source,
       etc.timer_wait AS trx_latency,
       etc.state AS trx_state,
       etc.autocommit AS trx_autocommit,
       conattr_pid.attr_value as pid,
       conattr_progname.attr_value as program_name
  FROM performance_schema.threads AS pps
  LEFT JOIN performance_schema.events_waits_current AS ewc USING (thread_id)
  LEFT JOIN performance_schema.events_stages_current AS estc USING (thread_id)
  LEFT JOIN performance_schema.events_statements_current AS esc USING (thread_id)
  LEFT JOIN performance_schema.events_transactions_current AS etc USING (thread_id)
  LEFT JOIN sys.x$memory_by_thread_by_current_bytes AS mem USING (thread_id)
  LEFT JOIN performance_schema.session_connect_attrs AS conattr_pid
    ON conattr_pid.processlist_id=pps.processlist_id and conattr_pid.attr_name='_pid'
  LEFT JOIN performance_schema.session_connect_attrs AS conattr_progname
    ON conattr_progname.processlist_id=pps.processlist_id and conattr_progname.attr_name='program_name'
 ORDER BY pps.processlist_time DESC, last_wait_latency DESC;
