-- Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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
-- View: innodb_lock_waits
--
-- Give a snapshot of which InnoDB locks transactions are waiting for.
-- The lock waits are ordered by the age of the lock descending.
--
-- Versions: 8.0+
--
-- mysql> select * from sys.innodb_lock_waits\G
-- *************************** 1. row ***************************
--                 wait_started: 2017-01-24 14:25:39
--                     wait_age: 00:00:02
--                wait_age_secs: 2
--                 locked_table: `test`.`t1`
--          locked_table_schema: test
--            locked_table_name: t1
--       locked_table_partition: NULL
--    locked_table_subpartition: NULL
--                 locked_index: GEN_CLUST_INDEX
--                  locked_type: RECORD
--               waiting_trx_id: 2063
--          waiting_trx_started: 2017-01-24 14:25:39
--              waiting_trx_age: 00:00:02
--      waiting_trx_rows_locked: 1
--    waiting_trx_rows_modified: 0
--                  waiting_pid: 6
--                waiting_query: update test.t1 set j = j + sleep(100) where i = 1
--              waiting_lock_id: 2063:61:5:2
--            waiting_lock_mode: X
--              blocking_trx_id: 2060
--                 blocking_pid: 5
--               blocking_query: update test.t1 set j = j + sleep(100) where i = 1
--             blocking_lock_id: 2060:61:5:2
--           blocking_lock_mode: X
--         blocking_trx_started: 2017-01-24 14:24:19
--             blocking_trx_age: 00:01:22
--     blocking_trx_rows_locked: 1
--   blocking_trx_rows_modified: 0
--      sql_kill_blocking_query: KILL QUERY 5
-- sql_kill_blocking_connection: KILL 5
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER
VIEW innodb_lock_waits (
  wait_started,
  wait_age,
  wait_age_secs,
  locked_table,
  locked_table_schema,
  locked_table_name,
  locked_table_partition,
  locked_table_subpartition,
  locked_index,
  locked_type,
  waiting_trx_id,
  waiting_trx_started,
  waiting_trx_age,
  waiting_trx_rows_locked,
  waiting_trx_rows_modified,
  waiting_pid,
  waiting_query,
  waiting_lock_id,
  waiting_lock_mode,
  blocking_trx_id,
  blocking_pid,
  blocking_query,
  blocking_lock_id,
  blocking_lock_mode,
  blocking_trx_started,
  blocking_trx_age,
  blocking_trx_rows_locked,
  blocking_trx_rows_modified,
  sql_kill_blocking_query,
  sql_kill_blocking_connection
) AS
SELECT r.trx_wait_started AS wait_started,
       TIMEDIFF(NOW(), r.trx_wait_started) AS wait_age,
       TIMESTAMPDIFF(SECOND, r.trx_wait_started, NOW()) AS wait_age_secs,
       CONCAT(sys.quote_identifier(rl.object_schema), '.', sys.quote_identifier(rl.object_name)) AS locked_table,
       rl.object_schema AS locked_table_schema,
       rl.object_name AS locked_table_name,
       rl.partition_name AS locked_table_partition,
       rl.subpartition_name AS locked_table_subpartition,
       rl.index_name AS locked_index,
       rl.lock_type AS locked_type,
       r.trx_id AS waiting_trx_id,
       r.trx_started as waiting_trx_started,
       TIMEDIFF(NOW(), r.trx_started) AS waiting_trx_age,
       r.trx_rows_locked AS waiting_trx_rows_locked,
       r.trx_rows_modified AS waiting_trx_rows_modified,
       r.trx_mysql_thread_id AS waiting_pid,
       sys.format_statement(r.trx_query) AS waiting_query,
       rl.engine_lock_id AS waiting_lock_id,
       rl.lock_mode AS waiting_lock_mode,
       b.trx_id AS blocking_trx_id,
       b.trx_mysql_thread_id AS blocking_pid,
       sys.format_statement(b.trx_query) AS blocking_query,
       bl.engine_lock_id AS blocking_lock_id,
       bl.lock_mode AS blocking_lock_mode,
       b.trx_started AS blocking_trx_started,
       TIMEDIFF(NOW(), b.trx_started) AS blocking_trx_age,
       b.trx_rows_locked AS blocking_trx_rows_locked,
       b.trx_rows_modified AS blocking_trx_rows_modified,
       CONCAT('KILL QUERY ', b.trx_mysql_thread_id) AS sql_kill_blocking_query,
       CONCAT('KILL ', b.trx_mysql_thread_id) AS sql_kill_blocking_connection
    FROM performance_schema.data_lock_waits w
 INNER JOIN information_schema.innodb_trx b  ON b.trx_id = CAST(w.blocking_engine_transaction_id AS CHAR)
 INNER JOIN information_schema.innodb_trx r  ON r.trx_id = CAST(w.requesting_engine_transaction_id AS CHAR)
 INNER JOIN performance_schema.data_locks bl ON bl.engine_lock_id = w.blocking_engine_lock_id
 INNER JOIN performance_schema.data_locks rl ON rl.engine_lock_id = w.requesting_engine_lock_id
 ORDER BY r.trx_wait_started;
