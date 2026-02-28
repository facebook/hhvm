-- Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
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
-- View: schema_table_lock_waits
--
-- Shows sessions that are blocked waiting on table metadata locks, and 
-- who is blocking them.
--
-- mysql> select * from sys.x$schema_table_lock_waits\G
-- *************************** 1. row ***************************
--                object_schema: test
--                  object_name: t
--            waiting_thread_id: 43
--                  waiting_pid: 21
--              waiting_account: msandbox@localhost
--            waiting_lock_type: SHARED_UPGRADABLE
--        waiting_lock_duration: TRANSACTION
--                waiting_query: alter table test.t add foo int
--           waiting_query_secs: 990
--  waiting_query_rows_affected: 0
--  waiting_query_rows_examined: 0
--           blocking_thread_id: 42
--                 blocking_pid: 20
--             blocking_account: msandbox@localhost
--           blocking_lock_type: SHARED_NO_READ_WRITE
--       blocking_lock_duration: TRANSACTION
--      sql_kill_blocking_query: KILL QUERY 20
-- sql_kill_blocking_connection: KILL 20
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$schema_table_lock_waits (
  object_schema,
  object_name,
  waiting_thread_id,
  waiting_pid,
  waiting_account,
  waiting_lock_type,
  waiting_lock_duration,
  waiting_query,
  waiting_query_secs,
  waiting_query_rows_affected,
  waiting_query_rows_examined,
  blocking_thread_id,
  blocking_pid,
  blocking_account,
  blocking_lock_type,
  blocking_lock_duration,
  sql_kill_blocking_query,
  sql_kill_blocking_connection
) AS
SELECT g.object_schema AS object_schema,
       g.object_name AS object_name,
       pt.thread_id AS waiting_thread_id,
       pt.processlist_id AS waiting_pid,
       sys.ps_thread_account(p.owner_thread_id) AS waiting_account,
       p.lock_type AS waiting_lock_type,
       p.lock_duration AS waiting_lock_duration,
       pt.processlist_info AS waiting_query,
       pt.processlist_time AS waiting_query_secs,
       ps.rows_affected AS waiting_query_rows_affected,
       ps.rows_examined AS waiting_query_rows_examined,
       gt.thread_id AS blocking_thread_id,
       gt.processlist_id AS blocking_pid,
       sys.ps_thread_account(g.owner_thread_id) AS blocking_account,
       g.lock_type AS blocking_lock_type,
       g.lock_duration AS blocking_lock_duration,
       CONCAT('KILL QUERY ', gt.processlist_id) AS sql_kill_blocking_query,
       CONCAT('KILL ', gt.processlist_id) AS sql_kill_blocking_connection
  FROM performance_schema.metadata_locks g
 INNER JOIN performance_schema.metadata_locks p 
    ON g.object_type = p.object_type
   AND g.object_schema = p.object_schema
   AND g.object_name = p.object_name
   AND g.lock_status = 'GRANTED'
   AND p.lock_status = 'PENDING'
 INNER JOIN performance_schema.threads gt ON g.owner_thread_id = gt.thread_id
 INNER JOIN performance_schema.threads pt ON p.owner_thread_id = pt.thread_id
  LEFT JOIN performance_schema.events_statements_current gs ON g.owner_thread_id = gs.thread_id
  LEFT JOIN performance_schema.events_statements_current ps ON p.owner_thread_id = ps.thread_id
 WHERE g.object_type = 'TABLE';
