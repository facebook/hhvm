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
-- View: x$session
--
-- Filter sys.processlist to only show user sessions and not background threads.
-- This is a non-blocking closer replacement to
-- [INFORMATION_SCHEMA. | SHOW FULL] PROCESSLIST
-- 
-- Performs less locking than the legacy sources, whilst giving extra information.
--
-- mysql> select * from sys.x$session\G
-- *************************** 1. row ***************************
--                 thd_id: 24
--                conn_id: 2
--                   user: root@localhost
--                     db: sys
--                command: Query
--                  state: Sending data
--                   time: 0
--      current_statement: select * from sys.x$session
--      statement_latency: 16285980000
--               progress: NULL
--           lock_latency: 15450000000
--          rows_examined: 0
--              rows_sent: 0
--          rows_affected: 0
--             tmp_tables: 4
--        tmp_disk_tables: 1
--              full_scan: YES
--         last_statement: NULL
-- last_statement_latency: NULL
--         current_memory: 3383772
--              last_wait: wait/synch/mutex/innodb/trx_mutex
--      last_wait_latency: 56550
--                 source: trx0trx.h:1520
--            trx_latency: 17893350207000
--              trx_state: ACTIVE
--         trx_autocommit: NO
--                    pid: 5559
--           program_name: mysql
--

CREATE OR REPLACE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$session
 AS
SELECT * FROM sys.x$processlist
WHERE conn_id IS NOT NULL AND command != 'Daemon';
